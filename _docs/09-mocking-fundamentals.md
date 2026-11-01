---
title: "GoogleMock fundamentals"
order: 9
part: "Mocking"
description: "Replace a real collaborator with a mock so you can test a unit in isolation: declare a mock with MOCK_METHOD, set expectations with EXPECT_CALL, match arguments, assert how many times a call happens, and stub background interactions with ON_CALL."
duration: "35 min"
---

## Learning objectives

By the end of this chapter you can:

- Explain what a mock is, and why an abstract interface is the seam that lets one stand in for a real collaborator.
- Declare a mock class from an interface with `MOCK_METHOD`, matching each method's signature and qualifiers.
- Set an expectation with `EXPECT_CALL`, including argument matchers, a cardinality, and a return action.
- Use `Times(0)` to assert a call never happens, which catches a unit that does too much.
- Choose between `ON_CALL` for background stubs and `EXPECT_CALL` for the interaction a test is actually about.

## Diagram

{% include excalidraw.html file="09-mock-seam" alt="Dispatcher, the unit under test, holds a CarrierGateway reference and calls it. CarrierGateway is an abstract interface, the seam. A RealCarrier implements it for production and talks to the network; a MockCarrierGateway implements it for the test with a MOCK_METHOD per virtual. The test sets the mock's return values and observes how it is called." caption="Figure 9.1: An abstract collaborator is a seam: the real implementation serves production, a mock serves the test." %}

## Why this matters

Every test so far checked a function that computed its own answer from its arguments. Real code is rarely that self-contained. A unit calls other units: a service calls a gateway, a handler calls a repository, a planner calls a pricing engine. If a test of the planner has to stand up a real pricing engine, the test is slow, it depends on that engine being correct and available, and a failure no longer points at the planner. A mock breaks that coupling. It's a stand-in for a collaborator that you program to return what you want and that records how it was called, so a test can drive the unit through every path, including the ones a real collaborator would make hard to reach, like a carrier being unavailable or a quote coming back too high.

The thing that makes a mock possible is a seam: a point where the real collaborator can be swapped for a fake one without the unit under test noticing. In C++ the most direct seam is an abstract interface. The unit depends on the interface, production supplies a real implementation, and the test supplies a mock. GoogleMock, which ships alongside GoogleTest and is already linked into every example in this book, generates the mock for you from the interface and gives you a small language for saying what you expect of it. This chapter tests a `Dispatcher` that depends on a `CarrierGateway`, and every technique follows from that one design decision to depend on an interface rather than a concrete class.

The code is in `examples/09-mocking-fundamentals/`. The run script there builds and runs it; its `README.md` covers how to drive each lane.

## How the code works

### The seam: an interface and the unit that depends on it

`CarrierGateway` is an abstract interface. It declares what a carrier can do and nothing about how:

```cpp
class CarrierGateway {
 public:
  virtual ~CarrierGateway() = default;

  [[nodiscard]] virtual bool serves(std::string_view region) const = 0;
  [[nodiscard]] virtual int quote_cents(int weight_grams,
                                        std::string_view region) const = 0;
  virtual std::string book(int weight_grams, std::string_view region) = 0;
};
```

Every method is pure virtual, so `CarrierGateway` can't be instantiated; only a derived class that implements all three can. That's exactly the property a mock needs. The virtual destructor is there because the unit holds the collaborator through a base-class reference, and deleting through a base pointer without a virtual destructor is undefined; every interface meant to be implemented gets one.

`Dispatcher` is the unit under test. It holds a `CarrierGateway&` and orchestrates a booking:

```cpp
std::optional<std::string> Dispatcher::dispatch(int weight_grams,
                                                std::string_view region,
                                                int budget_cents) {
  if (!carrier_.serves(region)) {
    return std::nullopt;  // unserved: never quote, never book
  }
  const int cost = carrier_.quote_cents(weight_grams, region);
  if (cost > budget_cents) {
    return std::nullopt;  // over budget: never book
  }
  return carrier_.book(weight_grams, region);
}
```

Each step is a call to the collaborator, and the order and the conditions are the logic worth testing. An unserved region must stop before quoting. A quote over budget must stop before booking. The happy path must call all three and return the booking reference. Those are statements about which calls happen and with which arguments, and a mock is the tool that lets a test assert exactly that.

`Dispatcher` takes the gateway by reference in its constructor. That is the injection: the dependency comes from outside rather than being constructed inside, so production passes a real gateway and a test passes a mock. That is the injection in miniature; the next chapter goes deeper into designing for this; here the single reference member is all the seam the tests need.

### Declaring the mock

A mock is a class that implements the interface, with each method generated by `MOCK_METHOD`:

```cpp
class MockCarrierGateway : public shipping::CarrierGateway {
 public:
  MOCK_METHOD(bool, serves, (std::string_view region), (const, override));
  MOCK_METHOD(int, quote_cents, (int weight_grams, std::string_view region),
              (const, override));
  MOCK_METHOD(std::string, book, (int weight_grams, std::string_view region),
              (override));
};
```

`MOCK_METHOD` takes the return type, the method name, the parameter list in parentheses, and a final parenthesized list of qualifiers. The qualifiers must match the interface: `serves` and `quote_cents` are `const` in the interface, so their mocks are `(const, override)`, while `book` mutates and is `(override)`. The `override` is not decoration; it makes the compiler check that the signature matches a virtual in the base, so a typo in a parameter type becomes a build error rather than a silently unmocked method. GoogleMock expands each macro into a real override plus the machinery that records calls and dispatches to whatever behavior a test sets.

One rule about the parentheses saves confusion later: a parameter list or return type that itself contains a comma, like `std::map<int, int>`, must be wrapped in its own parentheses, because the preprocessor would otherwise read the comma as a macro-argument separator. None of the signatures here need it, but a mock of a method returning a templated type with two parameters will.

### Setting an expectation with EXPECT_CALL

`EXPECT_CALL` states what you expect to happen to the mock during the test. The happy-path test sets one for each step:

```cpp
TEST(Dispatcher, BooksWhenServedAndWithinBudget) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(StrEq("EU"))).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(2500, StrEq("EU"))).WillOnce(Return(1500));
  EXPECT_CALL(carrier, book(2500, StrEq("EU"))).WillOnce(Return("BK-100"));

  shipping::Dispatcher dispatcher(carrier);
  const std::optional<std::string> reference =
      dispatcher.dispatch(2500, "EU", 2000);

  ASSERT_TRUE(reference.has_value());
  EXPECT_EQ(*reference, "BK-100");
}
```

Each `EXPECT_CALL` reads as a sentence: on this mock, this method, called with these arguments, should happen, and when it does, do this. `WillOnce(Return(1500))` sets the return value for that call and, implicitly, says the call should happen exactly once. The expectations are checked when the mock is destroyed at the end of the test, so an expected call that never arrives fails the test even though no assertion in the body ran. The body itself then checks the output: `dispatch` returned the reference that `book` was programmed to produce, which confirms the value flowed back through.

{% include excalidraw.html file="09-expect-call-anatomy" alt="An EXPECT_CALL broken into parts: EXPECT_CALL(carrier, names which mock; quote_cents(Ge(0), _)) names the method and its argument matchers; .Times(1) is the cardinality; .WillOnce(Return(800)) is the action. A note says omitting Times and using WillOnce implies exactly one call." caption="Figure 9.2: The parts of an EXPECT_CALL: the mock, the method and its argument matchers, the cardinality, and the action." %}

Figure 9.2 names the four parts. You won't write all of them every time: the cardinality defaults sensibly, and a method with no return value needs no action. But every `EXPECT_CALL` is some subset of those parts, and reading an unfamiliar one is a matter of finding them.

### Matchers describe arguments

The arguments inside `serves(StrEq("EU"))` are not values, they're matchers. `StrEq("EU")` matches a string equal to `"EU"`; a bare value like `2500` is shorthand for a matcher that compares equal to it. When a test doesn't care about an argument, `_` matches anything, and matchers like `Ge(0)` match by relation rather than exact value:

```cpp
TEST(Dispatcher, PassesTheWeightThroughToTheQuote) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(_)).WillOnce(Return(true));
  EXPECT_CALL(carrier, quote_cents(Ge(0), _)).WillOnce(Return(800));
  EXPECT_CALL(carrier, book(Ge(0), _)).WillOnce(Return("BK-200"));

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_EQ(dispatcher.dispatch(300, "US", 1000), "BK-200");
}
```

Matching on `Ge(0)` says the weight reaching the quote must be non-negative without pinning it to `300`, and `_` on the region says this test doesn't care which region. A matcher is as loose or as tight as the test needs: pin the arguments a test is actually about, and relax the rest with `_` so an unrelated change doesn't break it. GoogleMock ships a large matcher library, from `Eq`, `Ne`, `Lt`, `Gt` through string matchers like `HasSubstr` and `StartsWith` to container matchers like `ElementsAre` and `Contains`, and `_` for anything. The ones here are enough for the dispatcher; the mocking cookbook covers the rest.

### Cardinality: asserting a call never happens

The most useful cardinality in this example is zero. When the region is unserved, `dispatch` must not quote and must not book, and `Times(0)` turns either unwanted call into a failure:

```cpp
TEST(Dispatcher, DoesNotQuoteOrBookWhenRegionIsUnserved) {
  MockCarrierGateway carrier;
  EXPECT_CALL(carrier, serves(StrEq("XX"))).WillOnce(Return(false));
  EXPECT_CALL(carrier, quote_cents(_, _)).Times(0);
  EXPECT_CALL(carrier, book(_, _)).Times(0);

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_FALSE(dispatcher.dispatch(2500, "XX", 10000).has_value());
}
```

This catches a class of bug that an output check alone misses. A `dispatch` that quoted an unserved region and then returned `nullopt` anyway would still return the right value, so a test that only checked the return would pass while the unit did real work it should have skipped. The `Times(0)` expectations assert the absence of the calls, which is the actual contract. The over-budget test uses the same idea: it lets `serves` and `quote_cents` happen, then pins `book` to `Times(0)`, so booking something the budget cannot afford is a failure even though the return value would look correct.

Other cardinalities follow the same shape: `Times(n)` for exactly n, `AtLeast(n)`, `AtMost(n)`, and `AnyNumber()` for a call whose count does not matter. `WillOnce` without a `Times` means exactly one, and two `WillOnce` actions in a row mean exactly two calls, each returning its own value.

### ON_CALL for background, EXPECT_CALL for the subject

Not every call a test triggers is a call the test is about. The dispatcher has to get past `serves` and `quote_cents` to reach `book`, but a test focused on the booking doesn't want three expectations competing for attention. `ON_CALL` sets default behavior without asserting the call happens, so the background steps can be stubbed and the one `EXPECT_CALL` carries the point:

```cpp
TEST(Dispatcher, OnCallStubsBackgroundInteractions) {
  MockCarrierGateway carrier;
  ON_CALL(carrier, serves(_)).WillByDefault(Return(true));
  ON_CALL(carrier, quote_cents(_, _)).WillByDefault(Return(500));
  EXPECT_CALL(carrier, book(2500, StrEq("EU"))).WillOnce(Return("BK-300"));

  shipping::Dispatcher dispatcher(carrier);
  EXPECT_EQ(dispatcher.dispatch(2500, "EU", 2000), "BK-300");
}
```

The difference is intent. `EXPECT_CALL` is an assertion: this call must happen, and the test fails if it doesn't. `ON_CALL` is only a default: if this call happens, here is what it returns, and if it never happens, nothing is wrong. Using `ON_CALL` for the setup keeps the test focused on what it checks, the booking, not the plumbing that leads to it. That convenience has a cost in the test output, which the next section shows before you rely on it.

> **AI sidebar.** Asked to mock a collaborator, an assistant tends to over-specify: an `EXPECT_CALL` for every method the unit touches, each with exact-value arguments, so the test breaks the moment the unit is refactored even slightly. Push back toward intent. Ask which calls the test is actually about and make those `EXPECT_CALL`s, and let the rest be `ON_CALL` defaults or relax their arguments with `_`. Watch for the opposite failure too: a generated test that mocks the collaborator but only checks the return value, never asserting the calls that are the whole reason to use a mock, and in particular never using `Times(0)` to pin the calls that must not happen. The sharpest question to ask of a generated mock test is what it would catch that a plain return-value check would not; if the answer's nothing, the mock is decoration.

## Build, run, observe

Run the default lane:

```bash
[host]$ cd examples/09-mocking-fundamentals
[host]$ ./demo.sh test
```

```console
100% tests passed, 0 tests failed out of 5
```

Five tests: the happy path, the two short-circuits, the matcher test, and the `ON_CALL` test. That last one prints a warning worth understanding. Running it alone shows it:

```console
[host]$ ./build/unit_tests --gtest_filter='*OnCall*'
GMOCK WARNING:
Uninteresting mock function call - taking default action specified at:
tests/dispatcher_test.cpp:87:
    Function call: serves("EU")
          Returns: true
...
[       OK ] Dispatcher.OnCallStubsBackgroundInteractions (0 ms)
```

The call to `serves` happened, it had an `ON_CALL` default so it returned something sensible, but there was no `EXPECT_CALL` for it, so GoogleMock considers it uninteresting and warns. The test still passes; a naggy mock warns about uninteresting calls but doesn't fail on them. The warning is GoogleMock pointing out that you stubbed a call you're not asserting, which is exactly what `ON_CALL` is for here. The next chapter shows `NiceMock`, which silences these warnings when the background noise is intentional.

The suite passes under `./demo.sh asan` and `./demo.sh tsan` as well. The mock allocates its internal state, so ASan exercises that the mock and the unit clean up after themselves.

## See it fail

A mock's job is to catch a unit that does the wrong thing, so this section breaks the dispatcher and watches the mock fail. Removing the budget check from `dispatch`, so it books regardless of the quote, leaves the over-budget test with a `book` call it expected never to see:

```console
[ RUN      ] Dispatcher.DoesNotBookWhenQuoteExceedsBudget
tests/dispatcher_test.cpp:63: Failure
Mock function called more times than expected - returning default value.
    Function call: book(2500, "EU")
          Returns: ""
         Expected: to be never called
           Actual: called once - over-saturated and active
```

The message names the call, shows the arguments it was made with, and states the contradiction: expected never, called once. That's the `Times(0)` expectation doing its job. A plain return-value check would also have failed here, because the broken `dispatch` returns a booking reference instead of `nullopt`, but the mock failure is more precise: it points at the exact illegal interaction rather than only at the wrong final answer.

## Cross-check

Listing the cases confirms the five tests in the one suite:

```console
[host]$ ./build/unit_tests --gtest_list_tests
Dispatcher.
  BooksWhenServedAndWithinBudget
  DoesNotQuoteOrBookWhenRegionIsUnserved
  DoesNotBookWhenQuoteExceedsBudget
  PassesTheWeightThroughToTheQuote
  OnCallStubsBackgroundInteractions
```

The names read as the contract of `dispatch`: it books when served and within budget, does not quote or book when unserved, does not book over budget, passes the weight through, and reaches the booking once the background calls are stubbed.

## What you learned

- A mock stands in for a collaborator at an abstract-interface seam, so a unit can be tested without its real dependencies.
- `MOCK_METHOD` generates a mock method from an interface method; the final qualifier list must match the signature, and `override` makes the compiler check it.
- `EXPECT_CALL` sets an expectation from four parts: the mock, the method and its argument matchers, a cardinality, and an action; `WillOnce` implies exactly one call.
- `Times(0)` asserts a call never happens, catching a unit that does work it should skip, which a return-value check alone would miss.
- `ON_CALL` stubs background interactions without asserting them; reserve `EXPECT_CALL` for the interaction the test is about.

The next chapter builds on this: designing code so it can be mocked at all, and the advanced mocking tools, `NiceMock` and `StrictMock`, call sequences, and richer actions.

## For deeper coverage

- The [GoogleMock for dummies guide](https://google.github.io/googletest/gmock_for_dummies.html): the first mock, `MOCK_METHOD`, and `EXPECT_CALL`.
- The [GoogleMock cookbook](https://google.github.io/googletest/gmock_cook_book.html): the matcher and action libraries, cardinalities, and `ON_CALL` versus `EXPECT_CALL`.
- The [GoogleMock matchers reference](https://google.github.io/googletest/reference/matchers.html): the full matcher catalogue.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0), `examples/09-mocking-fundamentals` builds and reports 5/5 on `./demo.sh test`, `asan`, and `tsan`; the uninteresting-call warning, the `Times(0)` failure, and the `--gtest_list_tests` listing are quoted from real runs with host paths removed. Re-run all lanes before a release.*
