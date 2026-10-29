

# File daf\_reader.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**daf\_reader.h**](daf__reader_8h.md)

[Go to the documentation of this file](daf__reader_8h.md)


```C++
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <fstream>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace SpiceQL {
namespace daf {

  using Window = std::vector<std::pair<double, double>>;


  struct Summary {
    std::vector<double> dc;
    std::vector<int> ic;
  };


  inline Window merge(Window w) {
    std::sort(w.begin(), w.end());
    Window out;
    for (auto &iv : w) {
      if (!out.empty() && iv.first <= out.back().second) {
        out.back().second = std::max(out.back().second, iv.second);
      }
      else {
        out.push_back(iv);
      }
    }
    return out;
  }


  class File {
    public:
      explicit File(const std::string &path) : m_path(path) {
        m_size = m_handle.open(path);
        if (m_size < RECORD_BYTES) {
          throw std::runtime_error("[" + path + "] is too small to be a DAF");
        }

        char rec[RECORD_BYTES];
        readBytes(0, rec, RECORD_BYTES);

        m_idword = trim(std::string(rec, 8));
        if (m_idword.rfind("DAF/", 0) != 0 && m_idword != "NAIF/DAF") {
          throw std::runtime_error("[" + path + "] is not a binary DAF (ID word [" + m_idword + "])");
        }

        // LOCFMT names the file's binary format. Files older than it was added
        // leave it blank and are native, which every current platform reads as
        // little endian. If ND doesn't make sense either way, the file is
        // corrupt rather than foreign.
        std::string fmt(rec + 88, 8);
        if (fmt == "BIG-IEEE") {
          m_swap = !hostIsBigEndian();
        }
        else if (fmt == "LTL-IEEE") {
          m_swap = hostIsBigEndian();
        }
        else {
          m_swap = false;
          int32_t nd = decodeInt(rec + 8);
          if (nd < 0 || nd > 124) m_swap = true;
        }

        m_nd = decodeInt(rec + 8);
        m_ni = decodeInt(rec + 12);
        int32_t fward = decodeInt(rec + 76);

        // A summary has to fit in a summary record next to NEXT, PREV and NSUM
        if (m_nd < 0 || m_ni < 2 || summarySize() > 125) {
          throw std::runtime_error("[" + path + "] has an invalid ND/NI (" +
                                   std::to_string(m_nd) + "/" + std::to_string(m_ni) + ")");
        }

        readSummaries(fward);
      }

      std::string kind() const {
        return m_idword.rfind("DAF/", 0) == 0 ? m_idword.substr(4) : "";
      }

      int nd() const { return m_nd; }
      int ni() const { return m_ni; }
      const std::string &path() const { return m_path; }

      const std::vector<Summary> &summaries() const { return m_summaries; }

      std::vector<double> read(int64_t begin, int64_t end) const {
        if (begin < 1 || end < begin || (uint64_t)end * 8 > m_size) {
          throw std::runtime_error("DAF [" + m_path + "] read of words " + std::to_string(begin) +
                                   " to " + std::to_string(end) + " is outside the file");
        }
        size_t n = (size_t)(end - begin + 1);
        std::vector<double> out(n);
        readBytes((uint64_t)(begin - 1) * 8, reinterpret_cast<char *>(out.data()), n * 8);
        if (m_swap) {
          for (double &d : out) swapBytes(reinterpret_cast<char *>(&d), 8);
        }
        return out;
      }

      double read(int64_t addr) const {
        if (addr < 1 || (uint64_t)addr * 8 > m_size) {
          throw std::runtime_error("DAF [" + m_path + "] read of word " + std::to_string(addr) +
                                   " is outside the file");
        }
        char buf[8];
        readBytes((uint64_t)(addr - 1) * 8, buf, 8);
        return decodeDouble(buf);
      }

    private:
      static constexpr uint64_t RECORD_BYTES = 1024;

      class Handle {
        public:
          Handle() = default;
          Handle(const Handle &) = delete;
          Handle &operator=(const Handle &) = delete;

#ifdef _WIN32
          // Opens path and returns its size in bytes
          uint64_t open(const std::string &path) {
            m_path = path;
            m_stream.open(path, std::ios::binary);
            if (!m_stream) {
              throw std::runtime_error("Could not open DAF [" + path + "]");
            }
            m_stream.seekg(0, std::ios::end);
            return (uint64_t)m_stream.tellg();
          }

          void read(uint64_t offset, char *dst, size_t n) {
            m_stream.clear();
            m_stream.seekg((std::streamoff)offset);
            m_stream.read(dst, (std::streamsize)n);
            if ((size_t)m_stream.gcount() != n) {
              throw std::runtime_error("Short read from DAF [" + m_path + "]");
            }
          }

        private:
          std::string m_path;
          std::ifstream m_stream;
#else
          ~Handle() {
            if (m_fd >= 0) ::close(m_fd);
          }

          // Opens path and returns its size in bytes
          uint64_t open(const std::string &path) {
            m_path = path;
            m_fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
            struct stat st;
            if (m_fd < 0 || ::fstat(m_fd, &st) != 0 || !S_ISREG(st.st_mode)) {
              throw std::runtime_error("Could not open DAF [" + path + "]");
            }
            return (uint64_t)st.st_size;
          }

          void read(uint64_t offset, char *dst, size_t n) const {
            while (n > 0) {
              ssize_t got = ::pread(m_fd, dst, n, (off_t)offset);
              if (got < 0 && errno == EINTR) continue;
              if (got <= 0) {
                throw std::runtime_error("Short read from DAF [" + m_path + "]");
              }
              dst += got;
              offset += (uint64_t)got;
              n -= (size_t)got;
            }
          }

        private:
          std::string m_path;
          int m_fd = -1;
#endif
      };

      std::string m_path;
      mutable Handle m_handle;
      uint64_t m_size = 0;
      std::string m_idword;
      bool m_swap = false;
      int m_nd = 0;
      int m_ni = 0;
      std::vector<Summary> m_summaries;

      int summarySize() const { return m_nd + (m_ni + 1) / 2; }

      static bool hostIsBigEndian() {
        const uint16_t one = 1;
        unsigned char first;
        std::memcpy(&first, &one, 1);
        return first == 0;
      }

      static void swapBytes(char *p, size_t n) {
        std::reverse(p, p + n);
      }

      static std::string trim(std::string s) {
        size_t end = s.find_last_not_of(" \0", std::string::npos, 2);
        return end == std::string::npos ? "" : s.substr(0, end + 1);
      }

      int32_t decodeInt(const char *p) const {
        char buf[4];
        std::memcpy(buf, p, 4);
        if (m_swap) swapBytes(buf, 4);
        int32_t v;
        std::memcpy(&v, buf, 4);
        return v;
      }

      double decodeDouble(const char *p) const {
        char buf[8];
        std::memcpy(buf, p, 8);
        if (m_swap) swapBytes(buf, 8);
        double v;
        std::memcpy(&v, buf, 8);
        return v;
      }

      // Coverage reads are mostly a few words at a time (segment meta data,
      // single epochs), so small reads go through a cache of whole records,
      // the way CSPICE's DAF layer buffers records. Bigger reads go straight
      // to the file.
      //
      // Most of those reads jump around the file (summary records, searches
      // of epochs), where reading more than the record needed only
      // makes each read slower. Some walk forward through a segment instead:
      // a type 6 segment with 200,000 mini-segments takes two epochs from
      // each, so a record at a time is tens of thousands of reads per file.
      // So a miss just past the last one reads further ahead, doubling up to
      // MAX_READ_AHEAD records, and any other miss reads one record.
      static constexpr size_t CACHE_RECORDS = 128;
      static constexpr size_t MAX_READ_AHEAD = 64;
      static constexpr size_t DIRECT_READ_BYTES = 4 * RECORD_BYTES;
      mutable std::vector<char> m_cache = std::vector<char>(CACHE_RECORDS * RECORD_BYTES);
      mutable std::vector<int64_t> m_cachedRecord = std::vector<int64_t>(CACHE_RECORDS, -1);
      mutable uint64_t m_nextRecord = 0;
      mutable size_t m_readAhead = 0;

      // Record r (0-based) from the cache, reading it on a miss
      const char *record(uint64_t r) const {
        size_t slot = (size_t)(r % CACHE_RECORDS);
        char *buf = m_cache.data() + slot * RECORD_BYTES;
        if (m_cachedRecord[slot] != (int64_t)r) {
          // Forward means at or a little past where the last read ended
          bool forward = m_readAhead > 0 && r >= m_nextRecord && r < m_nextRecord + m_readAhead;
          m_readAhead = forward ? std::min(m_readAhead * 2, MAX_READ_AHEAD) : 1;

          // Stop at the end of the cache rather than wrap, and at the end of
          // the file, whose last record can be short
          uint64_t records = (m_size + RECORD_BYTES - 1) / RECORD_BYTES;
          size_t n = (size_t)std::min<uint64_t>({m_readAhead, CACHE_RECORDS - slot, records - r});
          m_handle.read(r * RECORD_BYTES, buf, (size_t)std::min<uint64_t>(n * RECORD_BYTES, m_size - r * RECORD_BYTES));
          for (size_t i = 0; i < n; i++) m_cachedRecord[slot + i] = (int64_t)(r + i);
          m_nextRecord = r + n;
        }
        return buf;
      }

      void readBytes(uint64_t offset, char *dst, size_t n) const {
        if (offset + n > m_size) {
          throw std::runtime_error("Short read from DAF [" + m_path + "]");
        }
        if (n >= DIRECT_READ_BYTES) {
          m_handle.read(offset, dst, n);
          return;
        }
        while (n > 0) {
          uint64_t r = offset / RECORD_BYTES;
          size_t within = (size_t)(offset % RECORD_BYTES);
          size_t take = std::min(n, (size_t)RECORD_BYTES - within);
          std::memcpy(dst, record(r) + within, take);
          dst += take;
          offset += take;
          n -= take;
        }
      }

      void readSummaries(int32_t record) {
        const int ss = summarySize();
        // The list can't have more records than the file does, which stops a
        // corrupt NEXT pointer from looping forever.
        const uint64_t maxRecords = m_size / RECORD_BYTES;
        uint64_t visited = 0;

        char rec[RECORD_BYTES];
        while (record > 0) {
          if ((uint64_t)record > maxRecords || ++visited > maxRecords) {
            throw std::runtime_error("DAF [" + m_path + "] has a broken summary record list");
          }
          readBytes((uint64_t)(record - 1) * RECORD_BYTES, rec, RECORD_BYTES);

          double next = decodeDouble(rec);
          int nsum = (int)decodeDouble(rec + 16);
          if (nsum < 0 || 3 + nsum * ss > 128) {
            throw std::runtime_error("DAF [" + m_path + "] has a summary record with " +
                                     std::to_string(nsum) + " summaries");
          }

          for (int i = 0; i < nsum; i++) {
            const char *s = rec + (3 + i * ss) * 8;
            Summary summary;
            summary.dc.resize(m_nd);
            summary.ic.resize(m_ni);
            for (int j = 0; j < m_nd; j++) summary.dc[j] = decodeDouble(s + j * 8);
            for (int j = 0; j < m_ni; j++) summary.ic[j] = decodeInt(s + m_nd * 8 + j * 4);
            m_summaries.push_back(std::move(summary));
          }
          record = (int32_t)next;
        }
      }
  };


  namespace detail {

    inline int nint(double d) { return (int)std::lround(d); }

    inline std::runtime_error badSegment(const File &f, int type, const std::string &why) {
      return std::runtime_error("CK type " + std::to_string(type) + " segment in [" + f.path() +
                                "] is malformed: " + why);
    }

    template<class Tick, class Add>
    void walkInterpolationIntervals(Tick &&tick, size_t ntick,
                                    const std::vector<double> &starts, Add add) {
      if (ntick == 0 || starts.empty()) return;

      // First epoch at or after value, searching [lo, ntick)
      auto firstAtOrAfter = [&](size_t lo, double value) {
        // Interval starts are normally epochs themselves, so check the
        // current epoch before searching.
        if (tick(lo) >= value) return lo;
        // Double the step until an epoch at or after value, or the end
        size_t below = lo, step = 1, hi = lo + 1;
        while (hi < ntick && tick(hi) < value) {
          below = hi;
          step *= 2;
          hi = below + std::min(step, ntick - below);
        }
        lo = below + 1;
        while (lo < hi) {
          size_t mid = lo + (hi - lo) / 2;
          if (tick(mid) < value) lo = mid + 1;
          else hi = mid;
        }
        return lo;
      };

      // Epochs before the first interval are skipped, and an interval that
      // starts after the last epoch ends the walk.
      size_t ti = firstAtOrAfter(0, starts[0]);
      if (ti == ntick) return;

      for (size_t si = 0; si < starts.size(); si++) {
        double begin = starts[si];
        if (si + 1 == starts.size()) {
          add(begin, tick(ntick - 1));
          return;
        }

        // An interval with no epochs in it is where zzckcv03 would reuse a
        // stale or unset finish
        size_t next = firstAtOrAfter(ti, starts[si + 1]);
        add(begin, next > ti ? tick(next - 1) : NAN);
        if (next == ntick) return;
        ti = next;
      }
    }

  }


  inline Window ckSegmentCoverage(const File &f, const Summary &s) {
    if (f.nd() != 2 || f.ni() != 6) {
      throw std::runtime_error("[" + f.path() + "] does not have CK summaries (ND/NI " +
                               std::to_string(f.nd()) + "/" + std::to_string(f.ni()) + ")");
    }

    const int type = s.ic[2];
    const int64_t beg = s.ic[4];
    const int64_t end = s.ic[5];
    const int64_t seglen = end - beg + 1;
    const double dcBeg = s.dc[0];
    const double dcEnd = s.dc[1];
    Window w;

    if (type == 1) {
      const int nrec = detail::nint(f.read(end));
      if (nrec < 1) throw detail::badSegment(f, 1, "record count is " + std::to_string(nrec));
      const int64_t ndir = (nrec - 1) / 100;
      int psiz;
      if (seglen == nrec * 5 + ndir + 1) psiz = 4;
      else if (seglen == nrec * 8 + ndir + 1) psiz = 7;
      else throw detail::badSegment(f, 1, "length doesn't match its record count");

      const int64_t at = beg + (int64_t)nrec * psiz;
      for (double t : f.read(at, at + nrec - 1)) w.emplace_back(t, t);
    }
    else if (type == 2) {
      const int nrec = detail::nint((seglen * 100.0 + 1.0) / 1001.0);
      if (nrec < 1) throw detail::badSegment(f, 2, "record count is " + std::to_string(nrec));
      std::vector<double> starts = f.read(beg + (int64_t)nrec * 8, beg + (int64_t)nrec * 9 - 1);
      std::vector<double> stops = f.read(beg + (int64_t)nrec * 9, beg + (int64_t)nrec * 10 - 1);
      for (int i = 0; i < nrec; i++) w.emplace_back(starts[i], stops[i]);
    }
    else if (type == 3 || type == 5) {
      int nrec, invls, rsize;
      if (type == 3) {
        std::vector<double> meta = f.read(end - 1, end);
        invls = detail::nint(meta[0]);
        nrec = detail::nint(meta[1]);
        if (nrec < 1 || invls < 1) throw detail::badSegment(f, 3, "empty record or interval list");
        const int64_t tail = (nrec - 1) / 100 + invls + (invls - 1) / 100 + 2;
        if (seglen == (int64_t)nrec * 5 + tail) rsize = 4;
        else if (seglen == (int64_t)nrec * 8 + tail) rsize = 7;
        else throw detail::badSegment(f, 3, "length doesn't match its record and interval counts");
      }
      else {
        std::vector<double> meta = f.read(end - 3, end);
        const int subtype = detail::nint(meta[0]);
        invls = detail::nint(meta[2]);
        nrec = detail::nint(meta[3]);
        if (subtype == 0) rsize = 8;
        else if (subtype == 1) rsize = 4;
        else if (subtype == 2) rsize = 14;
        else if (subtype == 3) rsize = 7;
        else throw detail::badSegment(f, 5, "unknown subtype " + std::to_string(subtype));
        if (nrec < 1 || invls < 1) throw detail::badSegment(f, 5, "empty record or interval list");
      }

      const int64_t tickAt = beg + (int64_t)rsize * nrec;
      const int64_t intAt = tickAt + nrec + (nrec - 1) / 100;
      std::vector<double> starts = f.read(intAt, intAt + invls - 1);
      auto tick = [&](size_t i) { return f.read(tickAt + (int64_t)i); };

      detail::walkInterpolationIntervals(tick, (size_t)nrec, starts, [&](double b, double e) {
        if (type == 3) {
          // ckcov_c signals an error when an interval has no epochs in it
          if (std::isnan(e) || e < b) throw detail::badSegment(f, 3, "interpolation interval with no epochs");
          w.emplace_back(b, e);
        }
        else {
          if (std::isnan(e)) return;
          b = std::max(b, dcBeg);
          e = std::min(e, dcEnd);
          if (b <= e) w.emplace_back(b, e);
        }
      });
    }
    else if (type == 4) {
      // Type 4 is a generic segment. Its meta data sits at the end, with the
      // count of meta items in the last word; see sgmeta and sgfpkt.
      const int count = detail::nint(f.read(end));
      if (count < 15) throw detail::badSegment(f, 4, "too few generic segment meta items");
      int metasz = count, stored = count;
      if (count == 15) metasz = stored = 16;
      else if (count > 17) metasz = 17;

      std::vector<double> raw = f.read(end - stored + 1, end - stored + metasz);
      int meta[17] = {0};
      for (int i = 0; i < metasz; i++) meta[i] = detail::nint(raw[i]);
      for (int i = metasz; i <= 16; i++) meta[i - 1] = 0;

      // Bases are relative to the segment
      const int64_t base = beg - 1;
      const int64_t pdrBase = meta[7] + base;
      const int pdrType = meta[9];
      const int64_t pktBase = meta[10] + base;
      const int npkt = meta[11];
      const int pktSize = meta[14];
      const int pktOff = meta[15];

      // Each packet starts with the record's midpoint and radius
      if (pdrType == 0) {
        for (int i = 1; i <= npkt; i++) {
          int64_t at = pktOff == 0 ? pktBase + (int64_t)(i - 1) * pktSize + 1
                                   : pktBase + (int64_t)(i - 1) * (pktSize + pktOff) + 1 + pktOff;
          std::vector<double> mr = f.read(at, at + 1);
          w.emplace_back(mr[0] - mr[1], mr[0] + mr[1]);
        }
      }
      else {
        std::vector<double> pdr = f.read(pdrBase + 1, pdrBase + npkt);
        for (int i = 0; i < npkt; i++) {
          int64_t at = pktBase + (int64_t)pdr[i];
          std::vector<double> mr = f.read(at, at + 1);
          w.emplace_back(mr[0] - mr[1], mr[0] + mr[1]);
        }
      }
    }
    else if (type == 6) {
      const int nintvl = detail::nint(f.read(end));
      if (nintvl < 1) throw detail::badSegment(f, 6, "no mini-segments");
      const int64_t ptrBase = end - 2 - (nintvl + 1);
      const int64_t ivlBase = ptrBase - nintvl / 100 - (nintvl + 1);
      std::vector<double> bounds = f.read(ivlBase + 1, ivlBase + nintvl + 1);
      std::vector<double> ptrs = f.read(ptrBase + 1, ptrBase + nintvl + 1);

      for (int i = 0; i < nintvl; i++) {
        // The mini-segment's last word is its record count; its last epoch sits
        // just before its epoch directory and four control words.
        const int64_t miniEnd = beg - 1 + detail::nint(ptrs[i + 1]) - 1;
        const int nrec = detail::nint(f.read(miniEnd));
        const double lastEpoch = f.read(miniEnd - 4 - (nrec - 1) / 100);

        double b = std::max(bounds[i], dcBeg);
        double e = std::min(std::min(lastEpoch, bounds[i + 1]), dcEnd);
        if (b <= e) w.emplace_back(b, e);
      }
    }
    else {
      throw std::runtime_error("CK segment in [" + f.path() + "] has unsupported data type " +
                               std::to_string(type));
    }
    return w;
  }

}
}
```


