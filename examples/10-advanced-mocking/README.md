# 10-advanced-mocking: strictness, ordering, actions, and fakes

The example for Chapter 10. `shipping::BookingService` depends on two injected
collaborators, a `CarrierGateway` and an `AuditLog`, and on success it books the
carrier and then records the booking, in that order. The tests show the advanced
GoogleMock tools:

- `InSequence` asserts the booking happens before the audit record.
- `NiceMock` silences the uninteresting-call warning for a background collaborator;
  `StrictMock` turns any unexpected call into a failure.
- `SaveArg` captures a call argument so the audited message can be asserted.
- `FakeAuditLog` is a working in-memory implementation, inspected after the fact,
  which shows faking as the alternative to mocking.

## Layout

```
10-advanced-mocking/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target (links GTest::gmock)
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # carrier.hpp, audit.hpp (interfaces), booking_service.hpp/.cpp
└── tests/                # booking_service_test.cpp: InSequence, Nice/StrictMock, SaveArg, a fake
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## What it verifies

`BookingService` books then records in that order (`InSequence`); a `NiceMock`
audit stays quiet while only the carrier is asserted; a `StrictMock` carrier pins
exactly the expected calls; `SaveArg` captures the audited message as `booked BK-9`;
and a `FakeAuditLog` accumulates one event per successful booking. The chapter also
removes the unserved early return to show the `StrictMock` catching the unexpected
`quote_cents` call.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 5/5 passing from a clean build.
</div>
