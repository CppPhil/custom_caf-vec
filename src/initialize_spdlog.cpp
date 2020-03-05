#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace vec {
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
  spdlog::set_pattern("%^[%d.%m.%Y %T.%e]%$ [%s:%# %!] [thread %t]: \"%v\"");
}
} // namespace vec
