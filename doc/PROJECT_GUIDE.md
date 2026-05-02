# C++ Project Boilerplate & Solution Arsenal

> **Philosophy**: A single, self-contained C++ project template with **zero third-party dependencies**. Every tool, utility, and subsystem you need should live inside this repo — maintained by you, tailored by you, owned by you.

---

## 1. Why Zero Dependencies?

External dependencies create hidden costs:

- **Build fragility**: Upstream breaking changes, abandoned projects, version conflicts.
- **Toolchain lock-in**: Vendored libs often assume specific compilers, CMake versions, or OS features.
- **Hidden complexity**: You inherit code you didn’t write and may not fully understand.
- **Portability pain**: Cross-compiling or moving to restricted environments becomes harder.

**This boilerplate flips the model**: instead of pulling in `spdlog`, `sqlite3`, `args.hxx`, etc., you build **your own** minimal replacements and grow them over time. You trade initial convenience for long-term control.

---

## 2. What This Repo Is

1. **Boilerplate**: A ready-to-build CMake project you can clone, rename, and start coding in under 60 seconds.
2. **Arsenal Platform**: A curated collection of your personal C++ solutions — logging, CLI parsing, data structures, threading utilities, etc. — all header-only or single-file where possible.
3. **Reference Implementation**: Working code that demonstrates how to structure a modern C++17 project without external libraries.

---

## 3. Directory Layout

```
.
├── CMakeLists.txt          # Root build config
├── build_n_run.sh          # One-command build & run
├── config.h.in             # CMake-injected build metadata
├── main.cpp / main.h       # Application entry point
│
├── core/                   # [YOUR ARSENAL] Reusable core utilities
│   ├── log/                # Your own logger (target: replace spdlog)
│   ├── cli/                # Your own argument parser (target: replace args.hxx)
│   ├── db/                 # Your own DB wrapper (target: replace sqlite3 direct use)
│   ├── fs/                 # Filesystem helpers
│   ├── str/                # String manipulation utilities
│   ├── time/               # Time formatting & timers
│   └── meta/               # Type traits, RTTI helpers, etc.
│
├── app/                    # Application-specific code (not reusable)
│   └── ...
│
├── doc/                    # Documentation
│   └── PROJECT_GUIDE.md    # This file
│
└── tools/                  # Standalone scripts & dev tools
    └── ...
```

### Design Rules

| Rule | Rationale |
|------|-----------|
| `core/` contains only reusable, dependency-free code | It should be copy-pasteable into another project without modification. |
| `core/` modules should be **header-only** where feasible | Zero link-time complexity; just `#include` what you need. |
| `app/` contains business logic | This is throwaway or project-specific. It may use `core/` freely. |
| No `subModules/`, `third_party/`, or `vendor/` directories | If it didn’t come from your keyboard, it doesn’t belong here. |

---

## 4. The Arsenal Pattern

When you need a capability (e.g., logging, argument parsing, JSON), follow this workflow:

### Step 1: Try Standard Library First

C++17 and later are surprisingly capable:

| Need | Standard Solution |
|------|-------------------|
| Logging | `std::ostream`, `std::ofstream`, `std::format` (C++20) or `fmt::format`-style wrapper using `std::ostringstream` |
| CLI parsing | Write a 50-line loop over `argc`/`argv`; it’s often sufficient. |
| File I/O | `<filesystem>` |
| Containers | `std::vector`, `std::unordered_map`, `std::deque` |
| Strings | `std::string_view`, `std::string`, `std::to_string` |
| Time | `<chrono>` + `std::put_time` |
| Threads | `<thread>`, `<mutex>`, `<condition_variable>` |
| Networking | Consider `asio` only if absolutely necessary; otherwise raw BSD sockets wrapped in RAII. |

### Step 2: Write a Minimal Custom Module

If the standard library is insufficient, add a new module to `core/`:

```
core/
└── log/
    ├── log.hpp       # Public interface
    └── log.cpp       # Implementation (if not header-only)
```

**Constraints for `core/` modules:**
- Must compile with `-Wall -Wextra -Werror -pedantic`.
- Must not depend on other `core/` modules unless absolutely necessary (keep DAG clean).
- Must expose a minimal, obvious API.
- Must include a short usage comment at the top of the header.

### Step 3: Evolve It In-Place

Your first version of `core/log/log.hpp` might just be a `std::ofstream` wrapper with a mutex. That’s fine. Over time, you add:
- Log levels (`debug`, `info`, `warn`, `error`)
- Rotating file logic
- Console color output
- Async logging queue

Because you own the code, you extend it exactly where *your* projects need it — no bloat from features you’ll never use.

---

## 5. Current State vs. Target State

This repo currently contains legacy third-party submodules (`spdlog`, `sqlite3`, `args`, `pipeline`). These are **transitional**.

| Submodule | Target Replacement | Notes |
|-----------|-------------------|-------|
| `spdlog` | `core/log/` | Start with a simple RAII file+console logger. Add async later only if needed. |
| `sqlite3` | `core/db/` or remove | If you need SQL, consider whether a flat file or in-memory struct is enough. SQLite is high-quality, but it *is* a dependency. |
| `args` | `core/cli/` | A 100-line `argc`/`argv` parser covers 90% of CLI needs. |
| `pipeline` | `core/thread/` | Thread pools, work queues, and pipelines are excellent learning exercises. |
| `build_info` | `core/build/` or inline in CMake | Git hash, build time, etc. can be injected purely via CMake/`config.h.in`. |

### Migration Strategy

1. **Do not delete submodules immediately** if active projects depend on them.
2. **Create the `core/` replacement first.**
3. **Migrate one submodule at a time.** Update `main.cpp` and CMake to use the new `core/` module.
4. **Delete the submodule only after the replacement is stable.**

---

## 6. Build System Design

### CMake Principles

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_project CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Explicit source lists — never use file(GLOB)
add_executable(${PROJECT_NAME}
    main.cpp
    app/something.cpp
    core/log/log.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O2 -DNDEBUG>
)
```

### Why Explicit Source Lists?

`file(GLOB ...)` breaks when you add new files because CMake does not re-run automatically. Explicit lists are self-documenting and reproducible.

### No `find_package()` Calls

If you follow the zero-dependency rule, your root `CMakeLists.txt` should contain **no `find_package()` calls** except `Threads` (which is part of the system).

---

## 7. Bootstrapping a New Project

```bash
# 1. Clone the boilerplate
git clone <this-repo> my_new_project
cd my_new_project

# 2. Rename project in CMakeLists.txt
sed -i 's/cmake_cpp_project_template/my_new_project/g' CMakeLists.txt

# 3. Remove sample app code
rm main.cpp main.h spdlog_init.cpp spdlog_init.h

# 4. Create your entry point
touch main.cpp

# 5. Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

## 8. Code Style & Conventions

To keep the arsenal consistent:

- **Naming**: `snake_case` for variables and functions; `PascalCase` for types; `SCREAMING_SNAKE_CASE` for macros.
- **Headers**: Use `#pragma once` (simple, portable enough) or standard include guards. Never use leading underscores (`_MAIN_H_`) — reserved by the standard.
- **Namespaces**: Every `core/` module lives in its own namespace: `namespace core::log`, `namespace core::cli`, etc.
- **Error Handling**: Prefer `std::optional` and `std::expected` (C++23) or boolean success + out-parameters. Avoid exceptions for control flow.
- **Memory**: No raw `new`/`delete` in application code. Use `std::unique_ptr`, `std::vector`, and RAII wrappers.

---

## 9. Recommended First Modules for the Arsenal

If you’re starting from scratch, prioritize these. They provide the highest utility with the lowest implementation cost:

1. **`core/log`**: A thread-safe, multi-sink logger (console + rotating file). ~200 lines.
2. **`core/cli`**: A minimal argument parser supporting `--key value` and `--flag`. ~80 lines.
3. **`core/time`**: `get_timestamp()`, `Timer` RAII class for benchmarking. ~50 lines.
4. **`core/fs`**: Read file to string, write string to file, path helpers. ~60 lines.
5. **`core/str`**: `split()`, `trim()`, `join()`, `starts_with()`, `ends_with()`. ~100 lines.

Each of these is small enough to write in an afternoon and robust enough to use for years.

---

## 10. Anti-Patterns to Avoid

| Anti-Pattern | Why It’s Harmful |
|--------------|------------------|
| Copy-pasting Stack Overflow snippets into `core/` without understanding them | You now own that code’s bugs. |
| Letting `core/` modules depend on each other in a cycle | Makes testing and reuse impossible. |
| Adding "just in case" features to `core/` modules | YAGNI. Add features when a real project demands them. |
| Using macros for things functions can do | Macros don’t respect namespaces or scopes. |
| Keeping dead code "because it might be useful later" | That’s what Git history is for. Delete it. |

---

## 11. Summary Checklist

Before you consider this boilerplate "ready" for a new project:

- [ ] `CMakeLists.txt` has no `find_package()` calls (except `Threads`).
- [ ] Source file lists are explicit — no `file(GLOB)`.
- [ ] `core/` modules compile standalone with `-Wall -Wextra -Werror`.
- [ ] `main.cpp` is the only place with `int main(...)`.
- [ ] No raw `new`/`delete` in `app/` or `core/`.
- [ ] Every `core/` header has a 3-line usage example in a comment.
- [ ] The project builds and runs with `./build_n_run.sh`.

---

*Own your tools. Build your arsenal. Ship with confidence.*
