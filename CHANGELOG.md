# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Fixed

- **main.cpp**: Fixed broken `ENABLE_PROJECT_ARCHIEVE` block that referenced an undeclared `vm` variable. Now correctly uses the `args.hxx` API via `processArgs::getInstance()`.
- **spdlog_init.cpp**: Removed erroneous `enableFileSink = true` that unconditionally overwrote the directory writeability check, causing file sinks to be enabled even for invalid paths.
- **spdlog_sqlite3_sink/implementInterfaces.cpp**: 
  - Fixed log message content incorrectly being set to `msg.logger_name` instead of `msg.payload`.
  - Added null-guards for `msg.source.filename` and `msg.source.funcname` to prevent `std::string` construction-from-null runtime crashes when source location is unavailable.
- **spdlog_sqlite3_sink/logFrame.h**: Fixed `__str__()` printing `pid` twice and omitting `tid`.
- **argProcessing.h**: Changed `std::exit(0)` to `std::exit(EXIT_FAILURE)` on argument parse/validation errors.
- **build_n_run.sh**: Fixed `.build` directory logic so it correctly removes an existing build directory before recreating it.
- **CMakeLists.txt**: 
  - Fixed `-o2` (ignored by GCC) to `-O2` in Release compile options.
  - Removed undefined `${Boost_LIBRARIES}` from `target_link_libraries`.
- **subModules/build_info/build_info.h**: Removed duplicate `extern const std::string buildTime;` declaration.

### Removed

- **sqlte3wapper.h**: Deleted unused, broken header containing syntax errors and undefined identifiers.
