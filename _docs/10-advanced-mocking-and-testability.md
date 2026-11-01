---
title: "Advanced mocking and testability"
order: 10
part: "Mocking"
description: "Go beyond the first mock: design code with seams you can inject, control a mock's strictness with NiceMock and StrictMock, assert call ordering with InSequence, capture arguments with SaveArg, and know when a fake beats a mock."
duration: "35 min"
---

## Learning objectives

By the end of this chapter you can:

- Design a unit with injectable seams so its collaborators can be replaced in a test.
- Choose a mock's strictness: `NiceMock` to silence background calls, `StrictMock` to fail on any unexpected one.
- Assert the order of calls across collaborators with `InSequence`.
- Capture a call's argument with `SaveArg` and act on a call with the action library.
- Decide when a fake, a working stand-in you inspect afterward, serves a test better than a mock.

## Diagram

{% include excalidraw.html file="10-mock-strictness" alt="Three mock strictness levels and how each treats a call no EXPECT_CALL covers. A NiceMock allows it silently. A default naggy mock allows it but logs a GMOCK WARNING. A StrictMock fails the test immediately. A matching EXPECT_CALL always takes priority on any of the three." caption="Figure 10.1: NiceMock, the default mock, and StrictMock differ only in how they treat a call no expectation covers." %}

## Why this matters

The previous chapter mocked one collaborator through an interface that was already there. Real code is not always so cooperative. A unit that constructs its own dependencies, reaches for a global, or calls a free function directly has no seam, and without a seam there is nothing for a mock to replace. Designing for testability is the work of introducing those seams before the test needs them, so the unit can be driven in isolation. That design choice comes first, and the mocking tools come second.

Once a seam exists, a mock can be as forgiving or as demanding as the test needs. Some calls are the point of the test and every detail matters: which method, which arguments, in what order. Other calls are background noise the unit happens to make on the way, and asserting them would only make the test brittle. GoogleMock gives you controls for both ends of that range, and a second kind of test double, the fake, for the cases where setting up expectations is more trouble than writing a small working implementation. This chapter extends the booking example from the previous one with a second collaborator, an audit log, which gives ordering to assert and background calls to manage.

The code is in `examples/10-advanced-mocking/`. The run script there builds and runs it; its `README.md` covers how to drive each lane.

## How the code works

### Designing the seams

`BookingService` depends on two collaborators, and both arrive through the constructor:

```cpp
class BookingService {
 public:
  BookingService(CarrierGateway& carrier, AuditLog& audit)
      : carrier_(carrier), audit_(audit) {}

  std::optional<std::string> book(int weight_grams, std::string_view region,
                                  int budget_cents);

 private:
  CarrierGateway& carrier_;
  AuditLog& audit_;
};
```

Taking each dependency as a reference parameter is constructor injection, and it is the seam. The service never names a concrete carrier or a concrete log; it knows only the two interfaces, so production wires in real implementations and a test wires in doubles. This is the most important decision in the chapter, because it's what makes everything after it possible. A `BookingService` that constructed its own `HttpCarrierGateway` inside the constructor would be untestable without a network, and the fix would be to introduce exactly this seam.

Constructor injection is the default, but it isn't the only seam. A dependency used by just one method can be a parameter of that method. A dependency selected at compile time can be a template parameter, which swaps the collaborator without a virtual call. A legacy function that calls a global can often be made testable by extracting the global access behind a small interface and injecting that. The common thread is that the unit depends on an abstraction it doesn't create, and the test supplies the concrete thing. Injecting by reference also settles ownership: the service borrows its collaborators and never deletes them, so their lifetime stays with whoever created them, the production wiring in one case and the test body in the other. A mock declared as a local in the test outlives the `BookingService` that borrows it, which is what lets GoogleMock verify the mock's expectations when the test block ends.

The booking logic calls both collaborators, and the order is part of the contract:

```cpp
std::string reference = carrier_.book(weight_grams, region);
audit_.record("booked " + reference);  // after the booking, never before
```

Recording a booking before the carrier has confirmed it would log something that might not be true, so the audit must follow the booking. That ordering is a property worth asserting, and a mock can.

### NiceMock and StrictMock: controlling strictness

The previous chapter saw a default mock warn about an uninteresting call, one that happened but had no `EXPECT_CALL`. That warning is the middle of a three-point spectrum, shown in Figure 10.1. A `NiceMock<T>` suppresses it, a `StrictMock<T>` promotes it to a failure, and the plain mock sits between them, allowing the call but logging a warning.

`NiceMock` is for a collaborator a test does not care about. When the test is about the carrier interaction, the audit calls are background, and wrapping the audit mock in `NiceMock` keeps the output clean:

```cpp
TEST(BookingService, NiceMockSilencesTheBackgroundAudit) {
  MockCarrierGateway carrier;
  NiceMock<MockAuditLog> audit;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(false));

  shipping::BookingService service(carrier, audit);
  EXPECT_FALSE(service.book(2500, "XX", 9999).has_value());
}
```

The service records `"rejected: unserved"` on the audit, but with no `EXPECT_CALL` for it the default mock would warn. `NiceMock` says that silence is intended here, which is more precise than leaving a warning the reader would have to learn to ignore.

`StrictMock` is the opposite, and it replaces a whole row of `Times(0)` expectations. Expecting only `serves` on a strict carrier asserts that nothing else is called, because any other call fails the test on the spot:

```cpp
TEST(BookingService, StrictMockPinsExactlyTheExpectedCalls) {
  StrictMock<MockCarrierGateway> carrier;
  NiceMock<MockAuditLog> audit;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(false));

  shipping::BookingService service(carrier, audit);
  EXPECT_FALSE(service.book(2500, "XX", 9999).has_value());
}
```

Where the previous chapter wrote `Times(0)` on `quote_cents` and `book` to say they must not happen, `StrictMock` says it once for every method at once: the only calls allowed are the ones with an `EXPECT_CALL`. That is tighter, and tighter isn't always better. A strict mock fails when the unit makes a reasonable call the test simply did not mention, so it couples the test to the exact set of interactions and can break on a harmless change. Reach for `StrictMock` when the precise set of calls is the thing under test, and prefer a plain or nice mock otherwise.

Neither wrapper changes how expectations themselves work. A `NiceMock` and a `StrictMock` both honor `EXPECT_CALL` and `ON_CALL` exactly as a plain mock does, with the same matchers, cardinalities, and actions; they differ only in what happens to a call that no expectation matches. That single axis is the whole of Figure 10.1, and it is why a matching `EXPECT_CALL` behaves the same on all three.

### InSequence: asserting order

By default, expectations may be satisfied in any order. When order is part of the contract, an `InSequence` object makes every `EXPECT_CALL` in its scope ordered:

```cpp
TEST(BookingService, BooksThenRecordsInOrder) {
  MockCarrierGateway carrier;
  MockAuditLog audit;
  {
    const InSequence seq;
    EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
    EXPECT_CALL(carrier, quote_cents(_, _)).WillOnce(Return(1000));
    EXPECT_CALL(carrier, book(_, _)).WillOnce(Return("BK-1"));
    EXPECT_CALL(audit, record("booked BK-1"));
  }

  shipping::BookingService service(carrier, audit);
  EXPECT_EQ(service.book(2500, "EU", 2000), "BK-1");
}
```

Declaring `InSequence seq` means the four expectations must be met in the order written: serve, quote, book, then record. If the service recorded the booking before calling `book`, the record call would arrive before its turn and the test would fail. The scope matters: `InSequence` applies to every `EXPECT_CALL` created while the object is alive, which is why it sits in its own braces, ending before the service runs. For finer control, named `Sequence` objects can order some calls while leaving others free, and `After` can say one expectation follows another specific one; `InSequence` is the common case of ordering everything.

### Actions: SaveArg and the action library

`WillOnce` and `WillByDefault` take an action, and `Return` is only the most common one. An action can capture an argument, call a function, throw, or combine several. `SaveArg<N>` copies the Nth argument into a variable, which lets a test assert on a value the unit computed and passed to a collaborator:

```cpp
TEST(BookingService, CapturesTheAuditedMessageWithSaveArg) {
  MockCarrierGateway carrier;
  MockAuditLog audit;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(_, _)).WillOnce(Return(1000));
  EXPECT_CALL(carrier, book(_, _)).WillOnce(Return("BK-9"));

  std::string recorded;
  EXPECT_CALL(audit, record(_)).WillOnce(SaveArg<0>(&recorded));

  shipping::BookingService service(carrier, audit);
  service.book(2500, "EU", 2000);
  EXPECT_EQ(recorded, "booked BK-9");
}
```

`SaveArg<0>(&recorded)` copies the first argument of `record` into `recorded`, which the test then checks. This is why `AuditLog::record` takes an owned `const std::string&` rather than a `std::string_view`: `SaveArg` copies the argument it is given, and copying a view would capture a pointer into a string that is destroyed when the call returns. Matching on the message with `record("booked BK-9")` would also work here; `SaveArg` is for when the value is awkward to express as a matcher, or when the test wants to apply several assertions to it.

The action library is large. `Return` and `ReturnRef` supply a value or a reference; `Throw` makes the mock throw; `Invoke` and the lambda form call your own code with the arguments; `DoAll` runs several actions in order, commonly a `SaveArg` followed by a `Return`; and `WillRepeatedly` sets the behavior for every call after the explicit `WillOnce` actions are used up. The cookbook lists them all. The ones here cover the example, and most tests need only `Return` and the occasional `SaveArg` or `Invoke`.

### Faking instead of mocking

A mock is not the only kind of test double. A fake is a working implementation, kept simple enough for a test, that you drive and then inspect. Here a `FakeAuditLog` stores its events in a vector:

```cpp
class FakeAuditLog : public shipping::AuditLog {
 public:
  void record(const std::string& event) override { events.push_back(event); }
  std::vector<std::string> events;
};
```

A test using it asserts on the accumulated state after the run, rather than on expectations set before it:

```cpp
TEST(BookingService, FakeAuditLogAccumulatesEverySuccess) {
  NiceMock<MockCarrierGateway> carrier;
  ON_CALL(carrier, serves(_)).WillByDefault(Return(true));
  ON_CALL(carrier, quote_cents(_, _)).WillByDefault(Return(500));
  ON_CALL(carrier, book(_, _)).WillByDefault(Return("BK-5"));
  FakeAuditLog audit;

  shipping::BookingService service(carrier, audit);
  service.book(2500, "EU", 2000);
  service.book(300, "US", 1000);

  ASSERT_EQ(audit.events.size(), 2U);
  EXPECT_EQ(audit.events[0], "booked BK-5");
  EXPECT_EQ(audit.events[1], "booked BK-5");
}
```

{% include excalidraw.html file="10-mock-vs-fake" alt="Two columns contrasting a mock and a fake. A mock is set up with EXPECT_CALL expectations rather than behavior, asserts the interaction (which calls, which arguments, which order), and is verified when the mock is destroyed. A fake is a working implementation with real behavior kept simple, asserts the result by inspecting its state after the run, and is checked by an ordinary EXPECT_EQ afterward." caption="Figure 10.2: A mock verifies the interaction as it happens; a fake is a working stand-in whose state you inspect afterward." %}

Figure 10.2 draws the contrast. A mock is programmed with expectations up front and verifies the interaction as it happens; a fake is a real implementation whose state the test reads afterward with ordinary assertions. A fake is the better choice when the collaborator is called many times and you care about the cumulative result rather than each call, when several tests share the same stand-in behavior, or when the interaction is too involved to express cleanly as a sequence of expectations. A mock is better when the interaction itself, the exact calls and their order, is what you want to pin. The two coexist: this test fakes the audit log and mocks the carrier in the same body, each where it fits.

> **AI sidebar.** Asked for advanced mocking, an assistant reaches for `StrictMock` everywhere and wraps every test in one `InSequence`, which produces tests that fail on any refactor that reorders a harmless call or adds a logging line. Treat strictness and ordering as choices to justify, not defaults: ask whether the order matters here or whether the test just happened to write the calls in some order, and whether a strict mock is pinning a real contract or only the current implementation. The other common miss is reaching for a mock when a fake would read better, setting up elaborate `EXPECT_CALL` chains for a collaborator the test only needs to accumulate state. When you see a long block of expectations for a dependency whose calls are not individually interesting, ask whether a small fake would say the same thing more clearly.

## Build, run, observe

Run the default lane:

```bash
[host]$ cd examples/10-advanced-mocking
[host]$ ./demo.sh test
```

```console
100% tests passed, 0 tests failed out of 5
```

Five tests: the ordered sequence, the nice-mock and strict-mock pair, the `SaveArg` capture, and the fake. None of them prints a stray warning, which is the point of the `NiceMock` wrappers: the background audit calls are stubbed and silenced, so the only output is the pass line. The suite also passes under `./demo.sh asan` and `./demo.sh tsan`; the fake allocates a growing vector, which ASan watches for a leak across the two bookings.

## See it fail

The point of a `StrictMock` is to catch a call the test never sanctioned, so here is one doing it. Removing the early return from the unserved branch, so `book` falls through and quotes and books anyway, makes the strict carrier reject the stray calls:

```console
[ RUN      ] BookingService.StrictMockPinsExactlyTheExpectedCalls
unknown file: Failure
Uninteresting mock function call - returning default value.
    Function call: quote_cents(2500, "XX")
          Returns: 0
...
[  FAILED  ] BookingService.StrictMockPinsExactlyTheExpectedCalls
```

The strict carrier expected only `serves`, so the moment the broken service calls `quote_cents`, the call is an uninteresting one on a strict mock, and that's a failure rather than a warning. The message names the unexpected call and its arguments. This is the same bug a `Times(0)` on `quote_cents` would catch, but the strict mock catches it without the test having to enumerate every forbidden call.

## Cross-check

Listing the cases confirms the five tests in the one suite:

```console
[host]$ ./build/unit_tests --gtest_list_tests
BookingService.
  BooksThenRecordsInOrder
  NiceMockSilencesTheBackgroundAudit
  StrictMockPinsExactlyTheExpectedCalls
  CapturesTheAuditedMessageWithSaveArg
  FakeAuditLogAccumulatesEverySuccess
```

Each name states the technique and the behavior it pins: order, strictness in both directions, argument capture, and the fake.

## What you learned

- A seam is a design choice: constructor injection by reference is the default, and a method parameter or a template parameter are the alternatives when they fit.
- `NiceMock<T>` silences uninteresting calls on a background collaborator; `StrictMock<T>` fails on any unexpected call and replaces a row of `Times(0)` expectations.
- `InSequence` orders every `EXPECT_CALL` in its scope, so cross-collaborator ordering like book-then-record can be asserted.
- `SaveArg<N>` captures an argument for later assertion; the action library (`Return`, `Throw`, `Invoke`, `DoAll`, `WillRepeatedly`) covers the rest.
- A fake is a working stand-in inspected after the run; prefer it to a mock when the accumulated result matters more than each individual call.

This chapter closes Part 3. The next part turns to concurrency: testing threaded code, and using the sanitizers to find the data races and memory bugs that only threads produce.

## For deeper coverage

- The [GoogleMock cookbook](https://google.github.io/googletest/gmock_cook_book.html): `NiceMock` and `StrictMock`, sequences, and the full action library.
- The [GoogleMock actions reference](https://google.github.io/googletest/reference/actions.html): `Return`, `SaveArg`, `Invoke`, `DoAll`, and the rest.
- *Working Effectively with Legacy Code* by Michael Feathers ([Pearson](https://www.pearson.com/en-us/subject-catalog/p/working-effectively-with-legacy-code/P200000009127)): seams and dependency-breaking techniques for code that has none.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0), `examples/10-advanced-mocking` builds and reports 5/5 on `./demo.sh test`, `asan`, and `tsan`; the `StrictMock` failure and the `--gtest_list_tests` listing are quoted from real runs with host paths removed. Re-run all lanes before a release.*
