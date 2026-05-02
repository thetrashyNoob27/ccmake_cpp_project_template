# Test Case Build & Run Guide

Location: `doc/test_case_build_n_run.md`

---

## Quick Start

Test cases live in `test/` and use **Google Test** (vendored at `test/lib/googletest`).

### 1. Build Tests

Tests are **not built by default**. Enable them with `-DBUILD_TESTS=ON`:

```bash
cd .build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)
```

> **Tip:** Do **not** delete the `.build` folder every time. `make` only recompiles changed files.

### 2. Run Tests

#### Option A — Run the test binary directly

```bash
cd .build
./test/test_runner
```

Output example:
```text
[==========] Running 15 tests from 4 test suites.
...
[  PASSED  ] 15 tests.
```

#### Option B — Use CTest

```bash
cd .build
ctest --output-on-failure
```

Output example:
```text
Test project /home/peter/programming/ccmake_cpp_project_template/.build
      Start  1: SampleTest.BasicAssertions
 1/18 Test  #1: SampleTest.BasicAssertions ..............................   Passed    0.00 sec
...
100% tests passed, 0 tests failed out of 18
```

> **Note:** Some pipeline tests may be timing-sensitive. If one fails intermittently under `ctest`, try running `./test/test_runner` directly to confirm.

---

## Adding a New Test

1. **Create** a new `.cpp` file under `test/core/`
2. **Register** it in `test/CMakeLists.txt`:

   ```cmake
   add_executable(test_runner
       core/test_sample.cpp
       core/test_pipeline.cpp
       core/test_build_verification.cpp
       core/your_new_test.cpp      # <-- add here
   )
   ```

3. **Rebuild**:

   ```bash
   cd .build
   make -j$(nproc)
   ```

4. **Run**:

   ```bash
   ./test/test_runner
   ```

---

## File Locations

| Path | Purpose |
|------|---------|
| `test/CMakeLists.txt` | Test build configuration |
| `test/core/*.cpp` | Test source files |
| `test/lib/googletest` | Vendored Google Test framework |
| `.build/test/test_runner` | Compiled test executable |

---

## Clean Build (if needed)

Only do this if you suspect stale build artifacts:

```bash
rm -rf .build
mkdir .build && cd .build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)
```
