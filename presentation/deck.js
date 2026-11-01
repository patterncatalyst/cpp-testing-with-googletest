// deck.js - "Testing C++ with GoogleTest" - a 60-minute conference cut.
// Red Hat-branded 16:9 deck mined from the finished 16 chapters.
// Build:  export NODE_PATH=$(npm root -g) && node deck.js   (from presentation/)

"use strict";

const H = require("./deck-helpers.js");
const {
  COLOR, FONT, W, ASSETS, PNG,
  newDeck, addFooter, addContentTitle, addBullets,
  addDiagramSlide, addCodeSlide, addCaption, addSectionDivider, addNotes,
} = H;

const OUT = "cpp-testing-with-googletest.pptx";

const pres = newDeck();
pres.title = "Testing C++ with GoogleTest";
let pageNum = 0;

function S() {
  const s = pres.addSlide(); pageNum += 1; addFooter(s, pageNum); return s;
}
function divider(code, title, subtitle, notes) {
  const s = pres.addSlide(); pageNum += 1;
  addSectionDivider(s, code, title, subtitle);
  addNotes(s, notes);
}
function bulletSlide(eyebrow, title, bullets, notes, opts = {}) {
  const s = S();
  addContentTitle(s, eyebrow, title);
  addBullets(s, bullets, { fontSize: opts.fontSize ?? 17 });
  addNotes(s, notes);
  return s;
}
function figSlide(eyebrow, title, png, caption, notes) {
  const s = S();
  addDiagramSlide(s, eyebrow, title, png, caption);
  addNotes(s, notes);
  return s;
}
function codeSlide(eyebrow, title, codeLines, caption, notes, opts = {}) {
  const s = S();
  addCodeSlide(s, eyebrow, title, "C++", codeLines, caption, opts);
  addNotes(s, notes);
  return s;
}

// ===========================================================================
// Cover
// ===========================================================================
{
  const s = pres.addSlide();
  pageNum += 1;
  s.background = { color: COLOR.white };
  try { s.addImage({ path: `${ASSETS}/cover-panel.png`, x: 0, y: 0, w: W, h: 7.5 }); } catch (e) {}
  s.addText("TESTING C++ WITH GOOGLETEST", {
    x: 6.00, y: 1.98, w: 6.90, h: 0.34,
    fontFace: FONT.title, fontSize: 14, bold: true, color: COLOR.red, charSpacing: 6,
    align: "left", valign: "middle",
  });
  s.addText([
    { text: "Testing C++", options: { breakLine: true } },
    { text: "with GoogleTest" },
  ], {
    x: 5.95, y: 2.42, w: 6.95, h: 2.00,
    fontFace: FONT.title, fontSize: 48, bold: true, color: COLOR.ink,
    align: "left", valign: "top",
  });
  s.addText("From your first test to sanitizers, coverage, and CI", {
    x: 6.00, y: 4.65, w: 6.70, h: 0.90,
    fontFace: FONT.body, fontSize: 18, italic: true, color: COLOR.caption,
    align: "left", valign: "top",
  });
  try { s.addImage({ path: `${ASSETS}/logo-candidate-2.png`, x: 11.10, y: 6.80, w: 1.55, h: 0.37 }); } catch (e) {}
  addNotes(s, "Testing C++ with GoogleTest, a sixty-minute cut of the book. The thread running through every slide is one claim: a test is only worth the behavior it actually pins. Compiling and going green is a checkpoint, not the finish line, because a suite that asserts nothing meaningful is still broken. We start from an empty file and a first TEST, add assertions and fixtures, parameterized and typed cases, the three ways C++ signals failure, the modern language features, mocks, concurrency with the sanitizers, and the tooling that debugs, measures, and runs it all. Every example lives in a shipping and logistics domain, built on Fedora with Clang as the primary compiler, GoogleTest through Conan, and no containers. The aim by the end is not a list of macros but a way of deciding what a test should prove and how to make the whole pipeline say something true about the code.");
}

// ===========================================================================
// Opening: the thesis and the setup
// ===========================================================================
bulletSlide(
  "THE THROUGH LINE",
  "A test is only worth the behavior it pins",
  [
    "Build success is a checkpoint, never the finish line: a suite that compiles and goes green while asserting nothing meaningful is still broken.",
    "Every example shows the test failing when the code is wrong, then the fix, so the test proves the behavior and not just the exit code.",
    "One domain throughout: parcels, carriers, rates, manifests, so each technique lands on code that looks like real work.",
  ],
  "This is the standard the whole deck holds itself to. It is easy to write a test that runs the code, reports success, and checks nothing a bug would violate, and such a test is worse than none because it buys false confidence. So every example in the book, and every technique here, is shown twice: once passing, and once with the code broken so you can see the test go red and read what it says. That see-it-fail step is the proof that the assertion is doing the work. Keeping it all in one shipping domain means the examples accumulate: the same parcels and carriers that teach a first assertion come back to teach mocking and concurrency, so the cognitive cost is paying attention to the testing technique, not relearning the problem each time."
);

bulletSlide(
  "THE TOOLCHAIN",
  "Local tools, two compilers, three lanes",
  [
    "CMake presets drive CTest; GoogleTest and GoogleMock come pinned through Conan; Clang is primary with GCC as a cross-check. No containers, no VM.",
    "Every example builds on three lanes: the default build, AddressSanitizer with UndefinedBehaviorSanitizer together, and ThreadSanitizer on its own.",
    "warnings-as-errors and clang-tidy run on every compile, so a lint finding or a warning fails the build before a test ever runs.",
  ],
  "The setup is plain and reproducible. CMake presets mean one command configures and builds each lane; CTest runs the suite; Conan pins GoogleTest exactly so a version bump is a decision, not a surprise. Clang is the house compiler and GCC runs as a periodic cross-check, because a second compiler catches conformance gaps the first lets through. The three lanes matter because they catch different things: the default lane checks behavior, ASan and UBSan catch memory errors and undefined behavior, and TSan catches data races. ASan and TSan cannot share a binary, so there are always two sanitizer passes. Warnings are errors and clang-tidy is on from the first build, which pushes a whole class of problems to compile time where they are cheapest to fix."
);

// ===========================================================================
// Section 1: Foundations
// ===========================================================================
divider("01", "Foundations", "From an empty file to fixtures",
  "Part one is the groundwork: why test C++ at all, how the build is wired, the anatomy of a single TEST, the difference between a fatal and a non-fatal check, and fixtures for shared setup. Nothing here is exotic, but the habits set here, choosing cases that cover the branches and watching a test fail before trusting it, carry through everything after.");

figSlide(
  "YOUR FIRST TEST",
  "Anatomy of a TEST, and choosing cases",
  "03-billable-weight-branches",
  "Figure: the branches of billable weight, each a case the suite must cover.",
  "A GoogleTest case is a TEST macro with a suite name and a case name, a body that calls the code, and assertions that pin the result. The skill is not the macro; it is choosing cases that cover the decision points. Billable weight is the greater of a parcel's actual weight and its volumetric weight, which is a branch: one case where actual wins, one where volumetric wins, and the boundary where they are equal. Each branch in the logic is a case in the suite, and the case names read as the contract. Before trusting any of them, break the comparison and watch the right case go red, which is the proof the assertion is doing work."
);

codeSlide(
  "ASSERTIONS",
  "ASSERT stops, EXPECT continues",
  [
    "// EXPECT_* records a failure and keeps going",
    "EXPECT_EQ(parcel.billable_grams(), 2500);",
    "",
    "// ASSERT_* records a failure and returns from the test",
    "// use it when the rest of the body would crash otherwise",
    "ASSERT_TRUE(result.has_value());",
    "EXPECT_EQ(*result, 2500);   // safe: ASSERT guarded the deref",
  ],
  "EXPECT to check, ASSERT to guard a precondition the rest of the test depends on.",
  "GoogleTest has two families of checks and the difference is control flow. An EXPECT failure is recorded and the test keeps running, so one run can report several independent problems. An ASSERT failure is recorded and the test returns immediately, which you want when continuing would crash or when the rest of the body is meaningless after this point. The canonical pattern is on the slide: ASSERT that an optional holds a value, then EXPECT on the value, because dereferencing an empty optional is undefined and the ASSERT stops the test before that happens. Reaching for ASSERT everywhere throws away the multiple-failures benefit; reaching for EXPECT everywhere risks a crash after the first failure. The rule is simple: EXPECT to check a result, ASSERT to guard a precondition."
);

figSlide(
  "FIXTURES",
  "Shared setup, per-test isolation",
  "05-fixture-lifecycle",
  "Figure: a fresh fixture per test, SetUp before each, TearDown or RAII after.",
  "When several tests need the same starting state, a fixture holds it. You derive a class from testing::Test, put the shared objects in it, and build them in SetUp; then TEST_F bodies run against a fresh instance. The lifecycle is the point of the figure: GoogleTest constructs a new fixture for every test and runs SetUp before each one, so no test can see state another test left behind. That isolation is what makes a fixture safe; the moment tests share a mutable instance, a failure in one can depend on the order they ran. Prefer RAII members that clean themselves up over an explicit TearDown, and reach for a fixture only when the shared setup actually repeats, not by reflex."
);

// ===========================================================================
// Section 2: Beyond the basics
// ===========================================================================
divider("02", "Beyond the basics", "Parameterized, typed, failure paths, modern C++",
  "Part two covers the cases a plain TEST handles awkwardly: the same body over many inputs or many types, the three different ways C++ signals a failure, and the C++20 and C++23 features that change where computation happens. Each needs a testing technique that follows from how the feature actually works.");

figSlide(
  "PARAMETERIZED & TYPED",
  "One body, many inputs and many types",
  "06-parameterized-and-typed",
  "Figure: TEST_P runs a body over a value table; TYPED_TEST runs it over a type list.",
  "When the same assertion should hold across a table of inputs, copying the body once per row is noise that hides the one row that matters. TEST_P writes the body once and runs it over a list of parameters, with a name generator so each instance gets a readable name in the output. When the same logic should hold across several types, a clamp that works for int, long, and double, TYPED_TEST runs one body over a type list. Both turn a wall of near-duplicate tests into one body plus a data set, and both report each case separately so a failure names the exact input or type. The caution is to keep the parameter a real axis of variation, not an excuse to hide distinct behaviors behind one body."
);

figSlide(
  "FAILURE PATHS",
  "Three kinds of failure, three tests",
  "07-failure-styles",
  "Figure: a recoverable error returns a value, a broken contract throws, a bug aborts.",
  "C++ signals failure three ways, and they are not interchangeable. A recoverable error a caller should handle, like parsing bad input, is a return value, and C++23's std::expected carries either the value or an error so the caller cannot ignore it; test both the discriminator and the payload. A broken contract at an API boundary is an exception; test not just that it throws but that the message names the reason. A bug that should be impossible is an assert that aborts; test it with EXPECT_DEATH, which forks a child process and checks that it dies with the expected message. Choosing the mechanism is the real decision, because the test follows from it: get the mechanism wrong and the test fights you; get it right and it nearly writes itself."
);

codeSlide(
  "MODERN C++",
  "Concepts, ranges, coroutines under test",
  [
    "// a concept: test it at compile time, both directions",
    "static_assert(Weighable<Parcel>);",
    "static_assert(!Weighable<int>);",
    "",
    "// a lazy range pipeline: materialize, then assert",
    "auto heavy = parcels | views::filter(over_threshold)",
    "                     | views::transform(&Parcel::weight_grams);",
    "EXPECT_EQ(std::ranges::fold_left(heavy, 0, plus{}), 4200);",
    "",
    "// an infinite generator: take a prefix so the test ends",
    "for (int v : running_total(500) | views::take(3)) ...",
  ],
  "Each modern feature is tested at the moment it actually runs.",
  "The C++20 and C++23 features change where computation happens, and the test follows that. A concept is checked by the compiler, so the cheapest test is a static_assert, and you write both directions: the type that should satisfy it does, and one that should not does not. A ranges pipeline is lazy and holds no result, so a test forces it to produce one by folding or collecting it, and pins the filter's boundaries. A generator yields on demand and may never end, so a test of an infinite one takes a fixed prefix, which is the only reason the loop terminates. The common thread for an AI pairing partner too: test your logic, the predicate and the boundary, not the standard library that is already tested far more thoroughly than your suite will."
);

// ===========================================================================
// Section 3: Mocking
// ===========================================================================
divider("03", "Mocking", "Test a unit in isolation",
  "Part three is GoogleMock: replacing a real collaborator with a double so a unit can be tested without its dependencies. The enabling move is a design decision, depending on an abstract interface, and the rest follows: declaring the mock, setting expectations, matching arguments, and choosing how strict to be.");

figSlide(
  "THE SEAM",
  "A mock stands in at an interface",
  "09-mock-seam",
  "Figure: the unit depends on an interface; production supplies the real one, the test a mock.",
  "A mock is only possible where there is a seam, a point where the real collaborator can be swapped without the unit noticing. In C++ the direct seam is an abstract interface: the unit holds a reference to it, production passes a real implementation that talks to the network, and the test passes a mock. GoogleMock generates the mock from the interface with MOCK_METHOD, one per virtual, matching the signature's qualifiers. This is the most important decision in the chapter, because everything after it depends on the unit taking its collaborator from outside rather than constructing it. A class that builds its own concrete dependency is untestable without that dependency, and the fix is exactly this seam."
);

figSlide(
  "EXPECT_CALL",
  "The anatomy of an expectation",
  "09-expect-call-anatomy",
  "Figure: which mock, which method and argument matchers, how many times, what to do.",
  "EXPECT_CALL reads as a sentence with four parts: on this mock, this method called with arguments matching these matchers, this many times, do this. Matchers describe arguments without pinning them: an exact value, Ge for a relation, or underscore for anything, so a test pins what it is about and relaxes the rest. The most useful cardinality is zero: Times(0) asserts a call never happens, which catches a unit that does work it should have skipped, a bug a return-value check alone would miss. ON_CALL sets a default without asserting the call happens, so the background steps can be stubbed and one EXPECT_CALL carries the point of the test. The discipline is to make expectations about intent, not to pin every incidental call, which is what makes a mock test survive a harmless refactor."
);

figSlide(
  "STRICTNESS & DOUBLES",
  "NiceMock, StrictMock, and fakes",
  "10-mock-strictness",
  "Figure: an uninteresting call is silent, a warning, or a failure, by mock strictness.",
  "The three mocks differ only in how they treat a call no expectation covers. A NiceMock stays silent, which suits a background collaborator a test does not care about. A StrictMock turns any unexpected call into a failure, which pins the exact set of interactions and replaces a row of Times(0) expectations, at the cost of breaking on a harmless new call. The plain mock sits between, warning but not failing. Beyond strictness, InSequence asserts ordering across calls, SaveArg captures an argument for a later assertion, and a fake, a small working implementation you inspect afterward, is often clearer than a mock when you care about the accumulated result rather than each call. Strictness and ordering are choices to justify, not defaults to apply everywhere."
);

// ===========================================================================
// Section 4: Concurrency
// ===========================================================================
divider("04", "Testing concurrency", "Threads, sanitizers, and flakiness",
  "Part four is where the examples start real threads, and the hardest part of testing shows up: nondeterminism. A concurrent test has to assert something true of every interleaving, the sanitizers catch the races a single run hides, and the flaky tests that remain get hunted with determinism and repetition.");

figSlide(
  "THREADED CODE",
  "Assert the aggregate, force the overlap",
  "11-concurrent-test-anatomy",
  "Figure: a latch releases N threads together; after they join, assert the aggregate.",
  "A concurrent test cannot assert the schedule, which changes every run, but it can assert an aggregate that holds across all schedules: eight threads each recording a thousand parcels must leave a count of exactly eight thousand. The test uses a std::latch to release all the threads at once, maximizing the overlap so a missing lock has the best chance to show, and std::jthread to join them cleanly. For work that returns a value, std::async hands back a future, and get is the synchronization point that must come before reading shared state. The catch, and the bridge to the next slides, is that a green run proves the invariant held on one schedule, not on all of them, which is exactly the gap the sanitizers and repetition close."
);

figSlide(
  "SANITIZERS",
  "ASan, UBSan, TSan: read the report",
  "12-three-sanitizers",
  "Figure: each sanitizer, what it finds, and the bug it catches here.",
  "Sanitizers are compiler instrumentation that reports undefined behavior as it happens, catching bugs a result-only assertion misses. AddressSanitizer finds memory errors, a use-after-free or a buffer overflow; UndefinedBehaviorSanitizer finds undefined operations like a signed overflow, with the operands and type; ThreadSanitizer finds data races, two unsynchronized accesses whether or not the race corrupted the result. A report reads in parts: the error kind and the faulting access, then the stacks that explain it, the free and the allocation for ASan, the two conflicting accesses for TSan. ASan and UBSan share one binary and TSan needs its own, which is why the matrix always runs two sanitizer passes. The reason they live on a lane rather than in a single assertion is that they check the undefined things you never thought to assert."
);

figSlide(
  "FLAKINESS",
  "Make it deterministic, then stress it",
  "13-flakiness-sources",
  "Figure: each source of flakiness and the technique that removes it.",
  "A flaky test passes and fails on the same code, and it is worse than a consistent failure because it trains everyone to rerun until green and then ignore the suite. The cure is almost always the same shape: find the uncontrolled input and inject it. A unit that reads the clock becomes testable when it takes the time as a parameter, so the test picks the moment instead of sleeping. A function that seeds its own random engine becomes reproducible when the engine is passed in and the test seeds it with a constant. Once the inputs are controlled, gtest_repeat with gtest_shuffle stresses the suite across many runs and orders to flush out what remains, printing a seed so a failing order can be replayed. Fix a flake at its cause; quarantine with a DISABLED prefix only as a stopgap, and avoid retries, which hide the signal."
);

// ===========================================================================
// Section 5: Tooling and workflow
// ===========================================================================
divider("05", "Tooling and workflow", "Debug, measure, and run it all",
  "Part five is the tooling around the tests: debugging a failing one under GDB, measuring what the suite actually exercises with llvm-cov, and running the whole matrix reproducibly in CI so every change is checked the same way.");

figSlide(
  "DEBUGGING",
  "From a red test to a located bug",
  "14-debug-loop",
  "Figure: isolate, break, inspect, find the cause, re-run until green.",
  "A failing test tells you something is wrong; a debugger tells you why. A GoogleTest binary runs under GDB like any program, and two conveniences make it pleasant: gtest_filter runs only the failing case, and the debug build lets GDB show source and set breakpoints by name. The move that makes it practical on a function called many times is a conditional breakpoint, break the function if the argument equals the failing input, so GDB stops exactly on the case that matters. From there print shows state, next and step move through the code, and backtrace shows the call chain from the test body down to the wrong line. A debugger and a sanitizer complement each other: the sanitizer says a class of bug happened, the debugger lets you watch any bug unfold."
);

figSlide(
  "COVERAGE",
  "Measure what the suite runs",
  "15-coverage-pipeline",
  "Figure: instrument, run, merge, report with llvm-cov.",
  "Coverage shows the code the tests never touch, which a green suite says nothing about. Clang's source-based coverage is a four-step pipeline: compile instrumented, run to emit a profile, merge it, and report with llvm-cov, which gives line, region, and branch numbers down to which side of each if was taken. Branch coverage is the one to watch, because it is the hardest to fool, and llvm-cov show names the exact uncovered branch so a gap becomes a location. The caution is the lesson: coverage measures what ran, not what was checked. A test that executes a line without asserting its result covers it just as fully as one that pins the answer, so treat coverage as a floor to clear, never a target to chase, and read an uncovered branch as a prompt to write a real assertion."
);

figSlide(
  "CI",
  "One pipeline, local and in CI",
  "16-ci-matrix",
  "Figure: a push fans out into the lanes and the coverage gate; all green to merge.",
  "The last step makes skipping a check impossible. One script runs the whole matrix, the default lane, both sanitizer lanes, and a coverage gate that fails below a floor, and it stops on the first red stage. A GitHub Actions workflow runs the same script, so a developer reproduces a CI failure locally with one command and green in CI means the same as green on my machine. The lanes run as a matrix, the sanitizers as separate jobs because they cannot share a binary, and the toolchain is pinned so a red build means a code change, not a drifting dependency. Making the checks required to merge is what turns CI from a dashboard into a gate. The whole deck comes together here: the suite, the sanitizers, and the pipeline all made to say something true about the code, on every push."
);

// ===========================================================================
// Closing + appendix
// ===========================================================================
bulletSlide(
  "WHAT TO CARRY AWAY",
  "The through line, at every scale",
  [
    "A test is only worth the behavior it pins; prove the assertion by watching it fail before you trust it.",
    "Match the tool to the question: a value, an exception, or an abort; a mock or a fake; a sanitizer lane or a debugger session.",
    "Run the whole matrix the same way locally and in CI, over a pinned toolchain, so a pass means the same thing everywhere.",
  ],
  "If one idea survives the hour, let it be the first bullet: a test is only worth the behavior it would catch going wrong, and the way to know it catches that is to watch it fail, by breaking the code, before trusting the green. Everything else is choosing the right tool for the question in front of you. Is this failure a recoverable value, a broken contract, or an impossible state. Does this unit need a mock that asserts the interaction or a fake you inspect after. Is this a memory question for a sanitizer or a logic question for a debugger. And then the pipeline ties it together, so the standards are a failing exit code rather than a thing someone remembers to run. Start with one real test and grow the suite the same way."
);

{
  const s = S();
  addContentTitle(s, "APPENDIX · FOR DEEPER COVERAGE", "Reading list");
  addBullets(s, [
    "Glenford Myers, Tom Badgett, and Corey Sandler, The Art of Software Testing (Wiley).",
    "Gerard Meszaros, xUnit Test Patterns (Addison-Wesley).",
    "Michael Feathers, Working Effectively with Legacy Code (Addison-Wesley).",
    "Anthony Williams, C++ Concurrency in Action (Manning).",
    "Craig Scott, Professional CMake: A Practical Guide (Crascit).",
    "Winters, Manshreck, and Wright, Software Engineering at Google (O'Reilly).",
  ], { fontSize: 15 });
  addNotes(s, "The reference works behind the book, each linked to its publisher in the bibliography. Myers is the classic on test-case design and boundary analysis. Meszaros is the catalogue of test doubles, fixtures, and test smells behind the mocking chapters. Feathers is the source for seams and dependency-breaking, which is what makes untestable code testable. Williams is the standard treatment of the C++ thread library and testing concurrent code. Scott is the practical CMake reference behind the build. And Software Engineering at Google is the view of testing and continuous integration at scale that the tooling chapters build toward. Each chapter's deeper-coverage section points at the relevant one.");
}

// ===========================================================================
pres.writeFile({ fileName: OUT })
  .then(p => console.log("WROTE", p, "slides:", pageNum))
  .catch(e => { console.error(e); process.exit(1); });
