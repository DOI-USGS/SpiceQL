# Changelog

All changes that impact users of this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!---
This document is intended for users of the applications and API. Changes to things
like tests should not be noted in this document.

When updating this file for a PR, add an entry for your change under Unreleased
and one of the following headings:
 - Added - for new features.
 - Changed - for changes in existing functionality.
 - Deprecated - for soon-to-be removed features.
 - Removed - for now removed features.
 - Fixed - for any bug fixes.
 - Security - in case of vulnerabilities.

If the heading does not yet exist under Unreleased, then add it as a 3rd heading,
with three #.


When preparing for a public release candidate add a new 2nd heading, with two #, under
Unreleased with the version number and the release date, in year-month-day
format. Then, add a link for the new version at the bottom of this document and
update the Unreleased link so that it compares against the latest release tag.


When preparing for a bug fix release create a new 2nd heading above the Fixed
heading to indicate that only the bug fixes and security fixes are in the bug fix
release.
-->

## [Unreleased]

### Fixed
- Fixed default SpiceQL REST URL [#63](https://github.com/DOI-USGS/SpiceQL/pull/63)
- Added missing db files (dawn, mariner10, near, and rosetta) to CMakeLists.txt [#69](https://github.com/DOI-USGS/SpiceQL/pull/69)

## 1.0.1

### Changed
- Changes to support integration with ISIS [#48](https://github.com/DOI-USGS/SpiceQL/pull/48)

### Fixed
- Fixed getLatestKernels bug [#49](https://github.com/DOI-USGS/SpiceQL/pull/49)

## 1.0.0

### Added

- Adds support for LO [#11](https://github.com/DOI-USGS/SpiceQL/issues/11)
- Adds support for Smart1 [#16](https://github.com/DOI-USGS/SpiceQL/issues/16)
- Adds support for Hayabusa2 ONC [#12](https://github.com/DOI-USGS/SpiceQL/issues/12)
- Adds support for Voyager [#13](https://github.com/DOI-USGS/SpiceQL/issues/13)
- Adds support for LROC MiniRF [#10](https://github.com/DOI-USGS/SpiceQL/issues/10)
- Adds support for MSL [#15](https://github.com/DOI-USGS/SpiceQL/issues/15)
- Adds support for MER [#14](https://github.com/DOI-USGS/SpiceQL/issues/14)
- Add github workflows [#20](https://github.com/DOI-USGS/SpiceQL/pull/20)
- Added PR template with licensing [#5](https://github.com/DOI-USGS/SpiceQL/pull/5)

### Changed

- Mkdocs update [#26](https://github.com/DOI-USGS/SpiceQL/pull/26)
- FastAPI app [#27](https://github.com/DOI-USGS/SpiceQL/pull/27)
- Clean up any mention of Sphinx [#32](https://github.com/DOI-USGS/SpiceQL/pull/32)
- Supporting ALE's ISD to Kernel functionality [#33](https://github.com/DOI-USGS/SpiceQL/pull/33)
- New inventory class [#31](https://github.com/DOI-USGS/SpiceQL/pull/31)
- Ale changes [#36](https://github.com/DOI-USGS/SpiceQL/pull/36)
- Accept string for ets param [#34](https://github.com/DOI-USGS/SpiceQL/pull/34)
- Dockerize FastAPI [#35](https://github.com/DOI-USGS/SpiceQL/pull/35)
- Validates quality param [#40](https://github.com/DOI-USGS/SpiceQL/pull/40)
- Planetary body search [#42](https://github.com/DOI-USGS/SpiceQL/pull/42)

### Fixed
- Bug fixes + tests [#38](https://github.com/DOI-USGS/SpiceQL/pull/38)
- Bugfixes [#39](https://github.com/DOI-USGS/SpiceQL/pull/39)
- Fixing bugs in the time search [#41](https://github.com/DOI-USGS/SpiceQL/pull/41)
- Fixed bug where cks were using interval scope [#43](https://github.com/DOI-USGS/SpiceQL/pull/43)
- SCLK Bug fixes [#44](https://github.com/DOI-USGS/SpiceQL/pull/44)

