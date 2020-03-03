#pragma once
#include <iosfwd>

enum class log_level { error, warn, info, debug, trace, invalid };

std::ostream& operator<<(std::ostream& out, const log_level& lvl);

std::istream& operator>>(std::istream& in, log_level& lvl);
