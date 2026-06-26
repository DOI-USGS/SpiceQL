#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
#include <vector>

// Windows compatibility
#ifdef _WIN32
  #define timegm _mkgmtime
#endif

// Leap-second table: UTC dates when TAI-UTC incremented.
// Source: IERS Bulletin C / NAIF naif0012.tls.
namespace {
  struct LeapSecond { int y, m, d; double tai_utc; };

  static const LeapSecond LEAP_SECONDS[] = {
    {1972,  1, 1, 10}, {1972,  7, 1, 11}, {1973,  1, 1, 12},
    {1974,  1, 1, 13}, {1975,  1, 1, 14}, {1976,  1, 1, 15},
    {1977,  1, 1, 16}, {1978,  1, 1, 17}, {1979,  1, 1, 18},
    {1980,  1, 1, 19}, {1981,  7, 1, 20}, {1982,  7, 1, 21},
    {1983,  7, 1, 22}, {1985,  7, 1, 23}, {1988,  1, 1, 24},
    {1990,  1, 1, 25}, {1991,  1, 1, 26}, {1992,  7, 1, 27},
    {1993,  7, 1, 28}, {1994,  7, 1, 29}, {1996,  1, 1, 30},
    {1997,  7, 1, 31}, {1999,  1, 1, 32}, {2006,  1, 1, 33},
    {2009,  1, 1, 34}, {2012,  7, 1, 35}, {2015,  7, 1, 36},
    {2017,  1, 1, 37},
  };

  static const int NUM_LEAP_SECONDS = sizeof(LEAP_SECONDS) / sizeof(LEAP_SECONDS[0]);

  // Constants for time conversion
  static const double DELTA_T_A = 32.184;  // TT - TAI offset (seconds)
  static const double K = 1.657e-3;
  static const double EB = 1.671e-2;
  static const double M[] = {6.239996e0, 1.99096871e-7};
  static const std::time_t J2000_UTC_UNIX = 946728000;  // 2000-01-01 12:00:00 UTC
  static const double J2000_TAI_UTC = 32.0;  // TAI-UTC at J2000 epoch

  // Helper function to get leap second count for a given date
  int getLeapSeconds(int y, int m, int d) {
    // For dates before the first tabulated leap second (1972-01-01), CSPICE
    // extrapolates TAI-UTC backward as a unit step, so the value just below the
    // first entry is one less than it (9, not 10). Seeding with the first
    // entry's value would make every pre-1972 epoch's ET a full second too large
    // (e.g. the Apollo missions).
    int leap = LEAP_SECONDS[0].tai_utc - 1;
    for (int i = 0; i < NUM_LEAP_SECONDS; i++) {
      if (y > LEAP_SECONDS[i].y ||
          (y == LEAP_SECONDS[i].y && (m > LEAP_SECONDS[i].m ||
          (m == LEAP_SECONDS[i].m && d >= LEAP_SECONDS[i].d)))) {
        leap = LEAP_SECONDS[i].tai_utc;
      }
    }
    return leap;
  }

  // Map a (possibly full) English month name to 1-12, matching on the first
  // three letters as CSPICE's str2et does. Returns 0 when the token is not a
  // recognized month name.
  int monthFromName(const std::string &token) {
    static const char *kMonths[] = {
      "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
      "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    if (token.size() < 3) {
      return 0;
    }
    std::string prefix = token.substr(0, 3);
    for (int i = 0; i < 12; i++) {
      if (prefix == kMonths[i]) {
        return i + 1;
      }
    }
    return 0;
  }

  // Parse a UTC calendar string into a normalized struct tm plus fractional
  // seconds. Supports the calendar formats CSPICE's str2et accepts, including:
  //   ISO 8601 with 'T' or space separators and an optional trailing 'Z'
  //     ("1986-01-18T12:19:52.18", "1986-01-18 12:19:52Z")
  //   Numeric dates with '-' or '/' separators in Y-M-D or M/D/Y order
  //     ("1986/01/18", "01/18/1986")
  //   Month-name dates in any field order
  //     ("1986 JAN 18 12:19:52", "JAN 18 1986", "18 JAN 1986")
  //   Day-of-year (ordinal) dates
  //     ("1986-018 // 12:19:52", "1986-018T12:19:52")
  //   Partial date/time, with missing fields defaulting to zero
  //     ("1986-01-18", "1986", "1986-01-18T12")
  //
  // The day-of-year case relies on timegm normalizing an out-of-range tm_mday,
  // so callers must read the calendar date back out of struct tm afterwards.
  void parseUtcString(const std::string &utc, struct tm &t, double &fractional_seconds) {
    t = {};
    fractional_seconds = 0.0;

    // Normalize separators into spaces so the string can be tokenized. Date
    // field separators ('-', '/', ','), the date/time 'T' separator, and a
    // trailing 'Z' are all collapsed; ':' and '.' are preserved for the time
    // token. None of the month names contain 'T' or 'Z', so the only ambiguous
    // letters are handled positionally (digit-T-digit / trailing-Z). The ISO
    // 'T' is recorded as a marker (\x01) so the field following it is treated
    // as the time component even when it carries no ':' (e.g. "...T12").
    std::string norm;
    norm.reserve(utc.size());
    for (size_t i = 0; i < utc.size(); i++) {
      char c = utc[i];
      char upper = (char)std::toupper((unsigned char)c);
      bool prevDigit = (i > 0) && std::isdigit((unsigned char)utc[i - 1]);

      if (c == '-' || c == '/' || c == ',') {
        norm += ' ';
      }
      else if ((upper == 'T') && prevDigit) {
        // ISO date/time separator. A 'T' after a digit is unambiguous (no month
        // name or time token contains 'T'), so this also handles a trailing 'T'
        // with no time field ("2003-01-02T"), where the time defaults to
        // midnight -- matching CSPICE's str2et.
        norm += '\x01';  // ISO date/time separator marker
      }
      else if ((upper == 'Z') && prevDigit && (i + 1 == utc.size())) {
        // trailing Zulu/UTC designator, drop it
      }
      else {
        norm += upper;
      }
    }

    // Split into whitespace-delimited tokens, separating the time token from
    // the date tokens. The time token is whichever token contains ':' or
    // follows the ISO 'T' marker.
    std::string timeToken;
    std::string monthName;
    std::vector<std::pair<long, int>> nums;  // (value, digit count)

    std::string tok;
    std::vector<std::string> tokens;
    bool tokIsTime = false;          // current token sits after the ISO 'T'
    std::vector<bool> tokenIsTime;
    for (size_t i = 0; i <= norm.size(); i++) {
      if (i == norm.size() || std::isspace((unsigned char)norm[i]) || norm[i] == '\x01') {
        if (!tok.empty()) {
          tokens.push_back(tok);
          tokenIsTime.push_back(tokIsTime);
          tok.clear();
        }
        // A 'T' marker flags the *next* token as the time component.
        tokIsTime = (i < norm.size() && norm[i] == '\x01');
      }
      else {
        tok += norm[i];
      }
    }

    for (size_t ti = 0; ti < tokens.size(); ti++) {
      const std::string &token = tokens[ti];
      if (token.find(':') != std::string::npos || tokenIsTime[ti]) {
        timeToken = token;
        continue;
      }
      int month = monthFromName(token);
      if (month != 0) {
        monthName = token;
        t.tm_mon = month - 1;
        continue;
      }
      // Pure integer date field.
      bool allDigits = !token.empty() &&
          std::all_of(token.begin(), token.end(),
                      [](char ch) { return std::isdigit((unsigned char)ch) != 0; });
      if (allDigits) {
        nums.push_back({std::stol(token), (int)token.size()});
      }
    }

    bool haveMonthName = !monthName.empty();
    int year = 0, day = 0, doy = 0;
    bool haveDoy = false;

    auto isYear = [](const std::pair<long, int> &n) { return n.second == 4; };

    if (haveMonthName) {
      // Month is already set; remaining numbers are the year and day in any
      // order (the 4-digit one is the year).
      for (const auto &n : nums) {
        if (isYear(n)) year = (int)n.first;
        else day = (int)n.first;
      }
    }
    else if (nums.size() >= 3) {
      // Year/month/day. Determine order from which end carries the 4-digit year.
      if (isYear(nums[0])) {            // Y M D
        year = (int)nums[0].first;
        t.tm_mon = (int)nums[1].first - 1;
        day = (int)nums[2].first;
      }
      else {                            // M D Y (US style)
        t.tm_mon = (int)nums[0].first - 1;
        day = (int)nums[1].first;
        year = (int)nums[2].first;
      }
    }
    else if (nums.size() == 2) {
      // year + day-of-year. CSPICE's str2et reads a two-number date as an
      // ordinal (YYYY-DOY) date regardless of the day field's width -- it never
      // treats "YYYY-MM" as a year/month -- so "2003-2", "2003-32", and
      // "2003-122" are days 2, 32, and 122 of 2003 (Jan 2, Feb 1, May 2).
      std::pair<long, int> yearTok = isYear(nums[0]) ? nums[0] : nums[1];
      std::pair<long, int> other = isYear(nums[0]) ? nums[1] : nums[0];
      year = (int)yearTok.first;
      doy = (int)other.first;
      haveDoy = true;
    }
    else if (nums.size() == 1) {
      year = (int)nums[0].first;        // date-only, year
    }
    else {
      throw std::runtime_error("Unable to parse UTC string: \"" + utc + "\"");
    }

    // Parse the time token (HH, HH:MM, or HH:MM:SS[.fff]). Missing fields are
    // left at zero.
    if (!timeToken.empty()) {
      int hour = 0, minute = 0;
      double sec = 0.0;
      std::vector<std::string> parts;
      std::string part;
      for (size_t i = 0; i <= timeToken.size(); i++) {
        if (i == timeToken.size() || timeToken[i] == ':') {
          parts.push_back(part);
          part.clear();
        }
        else {
          part += timeToken[i];
        }
      }
      if (parts.size() >= 1 && !parts[0].empty()) hour = std::stoi(parts[0]);
      if (parts.size() >= 2 && !parts[1].empty()) minute = std::stoi(parts[1]);
      if (parts.size() >= 3 && !parts[2].empty()) sec = std::stod(parts[2]);
      t.tm_hour = hour;
      t.tm_min = minute;
      t.tm_sec = (int)sec;
      fractional_seconds = sec - (double)t.tm_sec;
    }

    t.tm_year = year - 1900;
    if (haveDoy) {
      // Day-of-year: month already 0, let timegm normalize the ordinal day.
      t.tm_mon = 0;
      t.tm_mday = doy;
    }
    else {
      t.tm_mday = (day > 0) ? day : 1;
    }
  }
}

double calendarTimeToEphemTime(std::string calendarTime) {

  struct tm t = {};
  double fractional_seconds = 0.0;
  parseUtcString(calendarTime, t, fractional_seconds);

  std::time_t utc_unix = timegm(&t);

  // Look up the correct leap-second count for this UTC date. Read the date back
  // out of t so day-of-year inputs (normalized by timegm) are handled.
  int y = t.tm_year + 1900, m = t.tm_mon + 1, d = t.tm_mday;
  int leap = getLeapSeconds(y, m, d);

  // Convert UTC Unix time to seconds since J2000 epoch (UTC)
  double seconds_since_j2000_utc = (double)utc_unix - J2000_UTC_UNIX + fractional_seconds;

  // Calculate the TDB-TAI periodic correction. The mean-anomaly argument is ET
  // (TDB) seconds past J2000, but we only have UTC here, so solve the implicit
  // relation ET = (UTC seconds) + leap + TDB_TAI(ET) by fixed-point iteration.
  // Seeding with TAI (UTC + leap) and iterating converges to machine precision
  // in a couple of steps because |d(TDB_TAI)/dET| = K*M[1] ~ 3e-10.
  // (The inverse, ephemTimeToCalendarTime, already uses ET directly; iterating
  // here keeps the round-trip consistent and matches CSPICE's str2et.)
  double et = seconds_since_j2000_utc + leap;  // initial guess: TAI past J2000
  double tdb_tai = DELTA_T_A;
  for (int i = 0; i < 3; i++) {
    double mean_m = M[0] + M[1] * et;
    double E = mean_m + EB * sin(mean_m);
    tdb_tai = DELTA_T_A + K * sin(E);
    et = seconds_since_j2000_utc + leap + tdb_tai;
  }

  // ET (TDB) = seconds since J2000 (UTC) + leap seconds + TDB-TAI offset
  return et;
}


std::string ephemTimeToCalendarTime(double ephemTime, std::string format, int prec, int utclen) {
  // Calculate TDB-TAI using periodic terms
  double mean_m = M[0] + M[1] * ephemTime;
  double E = mean_m + EB * sin(mean_m);
  double tdb_tai = DELTA_T_A + K * sin(E);

  // Approximate UTC time for leap second lookup
  // ET ≈ seconds_since_j2000_utc + leap + tdb_tai
  // So: seconds_since_j2000_utc ≈ ET - leap - tdb_tai
  // Use J2000 leap count (32) for first approximation
  double approx_seconds_since_j2000 = ephemTime - J2000_TAI_UTC - tdb_tai;
  std::time_t approx_utc = (std::time_t)(approx_seconds_since_j2000 + J2000_UTC_UNIX);

  // Look up the correct leap-second count for this UTC date
  struct tm *g = std::gmtime(&approx_utc);
  int y = g->tm_year + 1900, m = g->tm_mon + 1, d = g->tm_mday;
  int leap = getLeapSeconds(y, m, d);

  // Now compute the actual UTC time
  // ET = seconds_since_j2000_utc + leap + tdb_tai
  // seconds_since_j2000_utc = ET - leap - tdb_tai
  double seconds_since_j2000_utc = ephemTime - leap - tdb_tai;

  // Julian Date format ("J") reports the UTC Julian date directly and is not
  // affected by the calendar rounding/carry logic below. J2000 (2000-01-01
  // 12:00:00 UTC) is JD 2451545.0, and seconds_since_j2000_utc is measured from
  // that same epoch.
  std::string fmt = format;
  std::transform(fmt.begin(), fmt.end(), fmt.begin(),
                 [](unsigned char c) { return (char)std::toupper(c); });

  if (fmt == "J") {
    double jd = 2451545.0 + seconds_since_j2000_utc / 86400.0;
    // Round half away from zero to match CSPICE (printf's %f rounds half to
    // even, which disagrees on exact .5 boundaries such as JD x.5).
    double scale = std::pow(10.0, prec);
    jd = std::floor(jd * scale + 0.5) / scale;
    std::vector<char> jbuf(32 + prec);
    snprintf(jbuf.data(), jbuf.size(), "JD %.*f", prec, jd);
    std::string result = jbuf.data();
    // CSPICE always emits the decimal point, even at precision 0 ("JD 2451545.").
    if (prec == 0) {
      result += '.';
    }
    if ((int)result.size() + 1 > utclen) {
      throw std::runtime_error("A utclen of {" + std::to_string(utclen) + "} to small for expected string size of "
                          "{" + std::to_string(result.size() + 1) + "}");
    }
    return result;
  }

  double etTime = seconds_since_j2000_utc + J2000_UTC_UNIX;
  std::time_t utc_unix = (std::time_t)std::floor(etTime);
  // Fractional part relative to the floored second. Subtracting the time_t
  // (rather than an (int) cast) avoids overflow for dates beyond 2038 and
  // keeps frac in [0, 1).
  double frac = etTime - (double)utc_unix;

  // Strip sub-resolution noise before rounding. A double holds ~15-16
  // significant digits, so at this epoch's magnitude the fractional second is
  // only meaningful to ulp(etTime) = |etTime| * 2^-52 seconds. Asking for more
  // decimals than that surfaces floating-point dust (e.g. .1234 stored as
  // .12339997). Snap the fraction to the resolvable number of decimals first so
  // those trailing digits round cleanly instead of leaking noise.
  if (etTime != 0.0) {
    double ulp = std::fabs(etTime) * 2.220446049250313e-16;  // 2^-52
    int resolvable = (int)std::floor(-std::log10(ulp));
    if (resolvable < 0) resolvable = 0;
    if (resolvable < prec) {
      double snapMult = std::pow(10.0, resolvable);
      frac = std::lround(frac * snapMult) / snapMult;
    }
  }

  long precisionMult = (long)std::lround(std::pow(10.0, prec));
  // Round the fraction to the requested precision instead of truncating, so a
  // value like x.600 does not degrade to x.599 through floating-point error.
  long fracDigits = std::lround(frac * (double)precisionMult);
  // A round-up can carry into the next whole second (e.g. 0.9999996 -> 1.000000
  // at prec=6); roll it into utc_unix so we never emit an out-of-range field.
  if (fracDigits >= precisionMult) {
    fracDigits -= precisionMult;
    utc_unix++;
  }

  // Break the rounded UTC instant into calendar fields.
  struct tm utc_tm = *std::gmtime(&utc_unix);
  int year = utc_tm.tm_year + 1900;
  int month = utc_tm.tm_mon + 1;
  int day = utc_tm.tm_mday;
  int doy = utc_tm.tm_yday + 1;
  int hour = utc_tm.tm_hour;
  int minute = utc_tm.tm_min;
  int second = utc_tm.tm_sec;

  static const char *kMonthAbbr[] = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };

  // Fractional-seconds suffix. prec == 0 omits the decimal point entirely, as
  // CSPICE's et2utc does. For prec > 0 the fraction is zero-padded to exactly
  // prec digits with no trailing-zero trimming -- matching et2utc_c, which
  // emits a fixed-width fraction for every calendar format (C, D, ISOC, ISOD),
  // e.g. ".582000" at prec 6 and ".000000" on a whole second. (The J format
  // uses %f above and is unaffected by this.)
  std::string fracStr;
  if (prec > 0) {
    std::vector<char> fbuf(prec + 2);
    std::string fracFmt = ".%0" + std::to_string(prec) + "ld";
    snprintf(fbuf.data(), fbuf.size(), fracFmt.c_str(), fracDigits);
    fracStr = fbuf.data();
  }

  // Assemble the date/time portion per the requested format code. Defaults to
  // ISOC for unrecognized codes to preserve prior behavior.
  std::vector<char> datebuf(64);
  if (fmt == "C") {
    // "1986 APR 12 16:31:09"
    snprintf(datebuf.data(), datebuf.size(), "%04d %s %02d %02d:%02d:%02d",
             year, kMonthAbbr[month - 1], day, hour, minute, second);
  }
  else if (fmt == "D") {
    // "1986-102 // 16:31:12"
    snprintf(datebuf.data(), datebuf.size(), "%04d-%03d // %02d:%02d:%02d",
             year, doy, hour, minute, second);
  }
  else if (fmt == "ISOD") {
    // "1986-102T16:31:12"
    snprintf(datebuf.data(), datebuf.size(), "%04d-%03dT%02d:%02d:%02d",
             year, doy, hour, minute, second);
  }
  else {
    // ISOC: "1986-04-12T16:31:12"
    snprintf(datebuf.data(), datebuf.size(), "%04d-%02d-%02dT%02d:%02d:%02d",
             year, month, day, hour, minute, second);
  }

  std::string result = std::string(datebuf.data()) + fracStr;
  if ((int)result.size() + 1 > utclen) {
    throw std::runtime_error("A utclen of {" + std::to_string(utclen) + "} to small for expected string size of "
                        "{" + std::to_string(result.size() + 1) + "}");
  }
  return result;
}
