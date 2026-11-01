// A second collaborator: an audit log the booking service notifies after each
// decision. Keeping it behind an interface lets a test supply a mock or a fake.
#ifndef SHIPPING_AUDIT_HPP
#define SHIPPING_AUDIT_HPP

#include <string>

namespace shipping {

class AuditLog {
 public:
  AuditLog() = default;
  AuditLog(const AuditLog&) = default;
  AuditLog& operator=(const AuditLog&) = default;
  AuditLog(AuditLog&&) = default;
  AuditLog& operator=(AuditLog&&) = default;
  virtual ~AuditLog() = default;

  // Record one audit event. Takes an owned string so a captured argument stays
  // valid after the call returns.
  virtual void record(const std::string& event) = 0;
};

}  // namespace shipping

#endif  // SHIPPING_AUDIT_HPP
