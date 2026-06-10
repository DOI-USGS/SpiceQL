#include <cmath>
#include <ctime>
#include <stdexcept>
#include <string>
#include <vector>

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
    int leap = LEAP_SECONDS[0].tai_utc;
    for (int i = 0; i < NUM_LEAP_SECONDS; i++) {
      if (y > LEAP_SECONDS[i].y ||
          (y == LEAP_SECONDS[i].y && (m > LEAP_SECONDS[i].m ||
          (m == LEAP_SECONDS[i].m && d >= LEAP_SECONDS[i].d)))) {
        leap = LEAP_SECONDS[i].tai_utc;
      }
    }
    return leap;
  }
}

double calendarTimeToEphemTime(std::string calendarTime) {

  struct tm t = {};
  char *end = strptime(calendarTime.c_str(), "%Y-%m-%dT%H:%M:%S", &t);
  std::time_t utc_unix = timegm(&t);

  // Parse fractional seconds if present
  double fractional_seconds = 0.0;
  if (end != nullptr && *end == '.') {
    // Parse fractional part after decimal point
    fractional_seconds = std::stod(end);
  }

  // Look up the correct leap-second count for this UTC date
  int y = t.tm_year + 1900, m = t.tm_mon + 1, d = t.tm_mday;
  int leap = getLeapSeconds(y, m, d);

  // Convert UTC Unix time to seconds since J2000 epoch (UTC)
  double seconds_since_j2000_utc = (double)utc_unix - J2000_UTC_UNIX + fractional_seconds;

  // Calculate TDB-TAI using periodic terms
  // Use the approximate ephemeris time for the periodic correction
  double mean_m = M[0] + M[1] * seconds_since_j2000_utc;
  double E = mean_m + EB * sin(mean_m);
  double tdb_tai = DELTA_T_A + K * sin(E);

  // ET (TDB) = seconds since J2000 (UTC) + leap seconds + TDB-TAI offset
  return seconds_since_j2000_utc + leap + tdb_tai;
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
  double etTime = seconds_since_j2000_utc + J2000_UTC_UNIX;
  std::time_t utc_unix = (std::time_t)etTime;
  double frac = etTime - (int)etTime;
  if (frac < 0) { utc_unix--; frac += 1.0; }
  int precisionMult = pow(10, prec);
  frac *= precisionMult;

  // Buffer size: YYYY-MM-DDTHH:MM:SS + . + digits + null
  int required_size = 19 + 1 + prec + 1;
  if (utclen < required_size) {
    throw std::runtime_error("A utclen of {" + std::to_string(utclen) + "} to small for expected string size of "
                        "{" + std::to_string(required_size) + "}");
  }

  char buffer[utclen];
  strftime(buffer, 20, "%Y-%m-%dT%H:%M:%S", std::gmtime(&utc_unix));

  // Add fractional seconds (without 'Z' to match NAIF format)
  std::string precisionStr = ".%0" + std::to_string(prec) + "d";
  snprintf(buffer + 19, prec + 2, precisionStr.c_str(), (int)frac);
  return buffer;
}
