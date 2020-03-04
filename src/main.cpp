#include <cstdio>

#include <iostream>

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "entity_set.hpp"
#include "first_pass.hpp"
#include "second_pass.hpp"
#include "verbosity_level.hpp"

namespace vec {
namespace {
// two pass parser for CAF log files that enhances logs with vector
// clock timestamps
void entry_point(caf::actor_system& sys, const config& cfg) {
  if (cfg.output_file.empty()) {
    std::cerr << "*** no output file specified" << std::endl;
    return;
  }

  verbosity_level vl;

  switch (cfg.verbosity) {
    case 0:
      vl = verbosity_level::silent;
      break;
    case 1:
      vl = verbosity_level::informative;
      break;
    default:
      vl = verbosity_level::noisy;
  }

  // open output file
  std::ofstream out{cfg.output_file};

  if (!out) {
    std::cerr << "unable to open output file: " << cfg.output_file << std::endl;
    return;
  }

  static constexpr size_t irsize = sizeof(std::string)
                                   + sizeof(std::unique_ptr<std::ifstream>)
                                   + sizeof(first_pass_result);

  struct intermediate_res {
    std::string file_path;
    std::unique_ptr<std::ifstream> ifstream;
    first_pass_result res;
    char pad[irsize >= CAF_CACHE_LINE_SIZE ? 1 : CAF_CACHE_LINE_SIZE - irsize];

    intermediate_res() = default;

    intermediate_res(std::string file_path, std::unique_ptr<std::ifstream> fs,
                     first_pass_result&& fr)
      : file_path(std::move(file_path)),
        ifstream(std::move(fs)),
        res(std::move(fr)) {
      // nop
    }
  };

  // do a first pass on all files to extract node IDs and entities
  std::vector<intermediate_res> intermediate_results;
  intermediate_results.resize(cfg.remainder.size());

  for (size_t i = 0; i < cfg.remainder.size(); ++i) {
    auto& file = cfg.remainder[i];
    auto ptr = &intermediate_results[i];
    ptr->file_path = file;
    ptr->ifstream = std::make_unique<std::ifstream>(file);

    if (!*ptr->ifstream) {
      std::cerr << "could not open file: " << file << std::endl;
      continue;
    }

    sys.spawn([ptr, vl](caf::blocking_actor* self) {
      SPDLOG_INFO("Launched blocking_actor for first_pass.");

      auto& ifs = *ptr->ifstream;
      auto res = first_pass(self, ifs, vl);

      if (res) {
        // rewind stream and push intermediate results
        ifs.clear();
        ifs.seekg(0);
        ptr->res = std::move(*res);
      }
    });
  }

  sys.await_all_actors_done();

  // post-process collected entity IDs before second pass
  entity_set entities;
  std::vector<std::string> entity_names;
  auto sort_pred = [](const intermediate_res& x, const intermediate_res& y) {
    return x.res.this_node < y.res.this_node;
  };
  std::map<std::string, size_t> pretty_actor_names;
  size_t thread_count = 0;

  // make sure we insert in sorted order into the entities set
  std::sort(intermediate_results.begin(), intermediate_results.end(),
            sort_pred);

  for (auto& ir : intermediate_results) {
    auto node_as_string = caf::to_string(ir.res.this_node);

    for (auto& kvp : ir.res.entities) {
      std::string pretty_name;
      // make each (pretty) actor and thread name unique
      auto& pn = kvp.second.pretty_name;

      if (kvp.first.aid != 0)
        pretty_name = pn + std::to_string(++pretty_actor_names[pn]);
      //"actor" + std::to_string(kvp.first.aid);
      else
        pretty_name = "thread" + std::to_string(++thread_count);

      auto vid = entities.size(); // position in the vector timestamp
      entity_names.emplace_back(pretty_name);
      entities.emplace(entity{kvp.first.aid, kvp.first.tid, ir.res.this_node,
                              vid, kvp.second.hidden, std::move(pretty_name)});
    }
  }

  // check whether entities set is in the right order
  auto vid_cmp = [](const entity& x, const entity& y) { return x.vid < y.vid; };

  if (!std::is_sorted(entities.begin(), entities.end(), vid_cmp)) {
    std::cerr << "*** ERROR: entity set not sorted by vector timestamp ID:\n"
              << caf::deep_to_string(entities) << std::endl;
    return;
  }

  // do a second pass for all log files
  // first line is the regex to parse the remainder of the file
  out << R"((?<clock>\S+) (?<timestamp>\d+) (?<component>\S+) )"
      << R"((?<level>\S+) (?<host>\S+) (?<class>\S+) (?<function>\S+) )"
      << R"((?<file>\S+):(?<line>\d+) (?<event>.+))" << std::endl;
  // second line is the separator for multiple runs
  out << std::endl;
  std::mutex out_mtx;
  auto grp = sys.groups().anonymous();

  for (auto& fpr : intermediate_results) {
    sys.spawn_in_group(grp, [&](caf::blocking_actor* self) {
      try {
        second_pass(self, grp, entities, fpr.res.this_node, entity_names,
                    *fpr.ifstream, out, out_mtx, !cfg.include_hidden_actors,
                    vl);
      } catch (const std::runtime_error& ex) {
        fprintf(stderr, "Caught std::runtime_error: \"%s\"\n", ex.what());
      }
    });
  }

  sys.await_all_actors_done();
}

void initialize_spdlog() {
  constexpr size_t kibibyte = 1024;
  constexpr size_t queue_size = 10 * kibibyte;
  constexpr size_t thread_count = 1;

  spdlog::init_thread_pool(queue_size, thread_count);

  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  constexpr auto do_truncate = true;
  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
    "logs/caf-vec.log", do_truncate);
  std::vector<spdlog::sink_ptr> sinks = {stdout_sink, file_sink};
  auto logger = std::make_shared<spdlog::async_logger>(
    "caf-vec logger", sinks.begin(), sinks.end(), spdlog::thread_pool(),
    spdlog::async_overflow_policy::block);
  spdlog::set_default_logger(logger);
  spdlog::set_pattern("%^[%d.%m.%Y %T.%e]%$ [%s:%# %!] [tid %t]: %v");
}
} // namespace
} // namespace vec

namespace {
void caf_main(caf::actor_system& sys, const vec::config& cfg) {
  vec::initialize_spdlog();
  return vec::entry_point(sys, cfg);
}
} // namespace

CAF_MAIN()
