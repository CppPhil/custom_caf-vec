#include <ostream>

#include "enhanced_log_entry.hpp"
#include "entity.hpp"
#include "log_entry.hpp"

std::ostream& operator<<(std::ostream& out, const enhanced_log_entry& x) {
  return out << x.json_vstamp << ' ' << x.data.timestamp << ' '
             << x.data.component << ' ' << x.data.level << ' '
             << x.id.pretty_name << ' ' << x.data.class_name << ' '
             << x.data.function_name << ' ' << x.data.file_name << ':'
             << x.data.line_number << ' ' << x.data.message;
}
