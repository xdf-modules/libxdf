## [1.0.3] · 2026-07-14
### 🔧 Fixed
- `libxdfConfig.cmake` no longer unconditionally requires system `pugixml` for library consumers ([#64](https://github.com/xdf-modules/libxdf/pull/64) by [Clemens Brunner](https://github.com/cbrnr))

## [1.0.2] · 2026-07-07
### ✨ Added
- Add `libxdfConfigVersion.cmake` to allow CMake's `find_package()` to check for compatible versions ([#62](https://github.com/xdf-modules/libxdf/pull/62) by [Clemens Brunner](https://github.com/cbrnr))

## [1.0.1] · 2026-07-06
### 🔧 Fixed
- Ignore empty samples in regular-rate marker streams ([#61](https://github.com/xdf-modules/libxdf/pull/61) by [Clemens Brunner](https://github.com/cbrnr))

## [1.0.0] · 2026-07-05
### 🔧 Fixed
- Read all channels of multi-channel string streams instead of only the first, which previously misaligned the file cursor and corrupted subsequent reads ([#58](https://github.com/xdf-modules/libxdf/pull/58) by [Alberto Barradas](https://github.com/abcsds))

## [0.99.10] · 2026-03-20
### 🔧 Fixed
- Do not segfault when loading an XDF file with no data (only clock sync values) ([#42](https://github.com/xdf-modules/libxdf/pull/42) by [WesselVS](https://github.com/WesselVS))

### 🌀 Changed
- Bump the required C++ standard to C++17 in `xdf.pro` ([#44](https://github.com/xdf-modules/libxdf/pull/44) by [Yida Lin](https://github.com/Yida-Lin))
- Bump the minimum macOS deployment target ([#48](https://github.com/xdf-modules/libxdf/pull/48) by [Clemens Brunner](https://github.com/cbrnr))
- Reformat `CMakeLists.txt` and bump the minimum required CMake version ([#50](https://github.com/xdf-modules/libxdf/pull/50) by [Clemens Brunner](https://github.com/cbrnr))
- Bump bundled pugixml to 1.15 ([#51](https://github.com/xdf-modules/libxdf/pull/51) by [Clemens Brunner](https://github.com/cbrnr))
- Improve and simplify the README ([#52](https://github.com/xdf-modules/libxdf/pull/52) by [Clemens Brunner](https://github.com/cbrnr))

### 🗑️ Removed
- Remove unused cruft from the repository ([#53](https://github.com/xdf-modules/libxdf/pull/53) by [Clemens Brunner](https://github.com/cbrnr))
- Remove a dysfunctional documentation link ([#55](https://github.com/xdf-modules/libxdf/pull/55) by [Clemens Brunner](https://github.com/cbrnr))

## [0.99.9] · 2024-08-25
### 🔧 Fixed
- Include `<cstdint>` to fix builds with strict standard library implementations ([#37](https://github.com/xdf-modules/libxdf/pull/37) by [Gijom](https://github.com/Gijom))
- Fix missing pugixml dependency in the generated CMake package config file ([#38](https://github.com/xdf-modules/libxdf/pull/38) by [myd7349](https://github.com/myd7349))
- Fix a typo that leaked into the compiled binary ([#40](https://github.com/xdf-modules/libxdf/pull/40) by [Étienne Mollier](https://github.com/emollier))
- Fix a potentially uninitialized variable ([#41](https://github.com/xdf-modules/libxdf/pull/41) by [Antoine Bonnier](https://github.com/antoine21839))

### 🌀 Changed
- Update the CI workflow ([#39](https://github.com/xdf-modules/libxdf/pull/39) by [myd7349](https://github.com/myd7349))

## [0.99.8] · 2021-09-29
*(v0.99.7, published the day before, had a version-number mismatch and was immediately superseded by this release.)*

### 🔧 Fixed
- Install exported CMake targets to `libdir/cmake/libxdf` ([#32](https://github.com/xdf-modules/libxdf/pull/32) by [Juhani Numminen](https://github.com/jnumm))

### 🌀 Changed
- Update the minimum required macOS version to 10.13 ([#33](https://github.com/xdf-modules/libxdf/pull/33) by [Clemens Brunner](https://github.com/cbrnr))
- Rename the `master` branch to `main` in the CI workflow ([#36](https://github.com/xdf-modules/libxdf/pull/36) by [Yida Lin](https://github.com/Yida-Lin))

## [0.99.6] · 2020-11-12
### ✨ Added
- Add a GitHub Actions workflow to automatically build, package, and publish releases ([#31](https://github.com/xdf-modules/libxdf/pull/31) by [Chadwick Boulay](https://github.com/cboulay))

## [0.991] · 2020-11-11
### ✨ Added
- Add a reference to the README ([#30](https://github.com/xdf-modules/libxdf/pull/30) by [Yida Lin](https://github.com/Yida-Lin))

### 🔧 Fixed
- Fix a bug where minimum/maximum time stamps could be read as NaN ([#23](https://github.com/xdf-modules/libxdf/pull/23) by [Yida Lin](https://github.com/Yida-Lin))
- Fix the return type of `readLength()` ([#22](https://github.com/xdf-modules/libxdf/pull/22) by [Yida Lin](https://github.com/Yida-Lin))

### 🌀 Changed
- Wrap raw `ifstream::read()` calls in a helper function to avoid error-prone size arguments ([#18](https://github.com/xdf-modules/libxdf/pull/18) by [Tristan Stenner](https://github.com/tstenner))
- Various CMake build script improvements ([#20](https://github.com/xdf-modules/libxdf/pull/20) and [#21](https://github.com/xdf-modules/libxdf/pull/21) by [Tristan Stenner](https://github.com/tstenner))
- Change the license from GPL-3.0 to BSD-2, and update authors and copyright year ([#29](https://github.com/xdf-modules/libxdf/pull/29) by [Yida Lin](https://github.com/Yida-Lin))

## [0.99] · 2019-03-21
### 🔧 Fixed
- Correctly compute the major sample rate when a recording only contains streams with a variable sampling rate ([#16](https://github.com/xdf-modules/libxdf/pull/16) by [Rene Maget](https://github.com/rstdm))
- Synchronize samples that occur before the first clock synchronization value, avoiding huge (and sometimes crash-inducing) memory allocations caused by an incorrectly inflated recording duration ([#16](https://github.com/xdf-modules/libxdf/pull/16) by [Rene Maget](https://github.com/rstdm))

## [0.98] · 2018-07-12
### ✨ Added
- Add support for synchronizing time stamps across streams (by [Yida Lin](https://github.com/Yida-Lin))

## [0.96] · 2017-08-02
### ✨ Added
- Add support for building both a static and a shared library ([#15](https://github.com/xdf-modules/libxdf/pull/15) by [Clemens Brunner](https://github.com/cbrnr))

## [0.95] · 2017-08-02
### ✨ Added
- Add a vector to store the effective sample rate of each stream (by [Yida Lin](https://github.com/Yida-Lin))
- Add a CMake build in addition to qmake ([#10](https://github.com/xdf-modules/libxdf/pull/10) by [Clemens Brunner](https://github.com/cbrnr))
- Add a minimum macOS deployment target ([#14](https://github.com/xdf-modules/libxdf/pull/14) by [Clemens Brunner](https://github.com/cbrnr))

### 🔧 Fixed
- Remove compiler warnings ([#9](https://github.com/xdf-modules/libxdf/pull/9) by [Clemens Brunner](https://github.com/cbrnr))

### 🌀 Changed
- Update the README ([#11](https://github.com/xdf-modules/libxdf/pull/11) by [Clemens Brunner](https://github.com/cbrnr))

## [0.94] · 2017-04-13
### 🌀 Changed
- Switch to 1-based channel numbering in the user interface (by [Yida Lin](https://github.com/Yida-Lin))

## [0.93] · 2017-03-28
### 🔧 Fixed
- Fix several bugs (by [Yida Lin](https://github.com/Yida-Lin))
- Remove remaining `malloc.h` includes ([#5](https://github.com/xdf-modules/libxdf/pull/5) by [Clemens Brunner](https://github.com/cbrnr))
- Fix compiler warnings ([#6](https://github.com/xdf-modules/libxdf/pull/6) by [Clemens Brunner](https://github.com/cbrnr))

### 🌀 Changed
- Use `double` instead of `float` for time stamps (by [Yida Lin](https://github.com/Yida-Lin))
- Explicitly enable C99 ([#7](https://github.com/xdf-modules/libxdf/pull/7) by [Clemens Brunner](https://github.com/cbrnr))

## [0.92] · 2017-02-10
### ✨ Added
- Initial public release of libXDF, distributed as both prebuilt binaries and source code, for use with [SigViewer](https://github.com/cbrnr/sigviewer) (by [Yida Lin](https://github.com/Yida-Lin))

### 🔧 Fixed
- Switch the build system to CMake and fix a `malloc.h` build issue ([#3](https://github.com/xdf-modules/libxdf/pull/3) by [Clemens Brunner](https://github.com/cbrnr))
