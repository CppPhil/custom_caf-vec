#pragma once
#include <iosfwd>
#include <string>

#include <caf/all.hpp>

/// The ID of a mailbox in a logfile. Parsed from `<actor>@<node>` entries.
struct mailbox_id {
  /// Actor ID of the receiver.
  caf::actor_id aid;
  /// Node ID of the receiver.
  caf::node_id nid;
};

std::string to_string(const mailbox_id& x);

std::istream& operator>>(std::istream& in, mailbox_id& x);

std::ostream& operator<<(std::ostream& out, const mailbox_id& x);
