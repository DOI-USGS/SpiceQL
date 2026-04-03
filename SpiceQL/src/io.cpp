#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

#include <fmt/format.h>
#include <ghc/fs_std.hpp>
#include <SpiceUsr.h>

#include "spdlog/spdlog.h"

#include "io.h"
#include "utils.h"

#include <spdlog/spdlog.h>


using namespace std;

using json = nlohmann::json;

namespace SpiceQL {

  SpkSegment::SpkSegment (vector<vector<double>> statePositions,
                          vector<double> stateTimes,
                          int bodyCode,
                          int centerOfMotion,
                          string referenceFrame,
                          string segmentId, int degree,
                          vector<vector<double>> stateVelocities,
                          string comment) {

    this->comment         = comment;
    this->bodyCode        = bodyCode;
    this->centerOfMotion  = centerOfMotion;
    this->referenceFrame  = referenceFrame;
    this->segmentId       = segmentId;
    this->polyDegree      = degree;
    this->statePositions  = statePositions;
    this->stateVelocities = stateVelocities;
    this->stateTimes      = stateTimes;
    return;
  }


  vector<vector<double>> concatStates (vector<vector<double>> statePositions, vector<vector<double>> stateVelocities) {

    if (statePositions.size() != stateVelocities.size()) {
      throw invalid_argument("Both statePositions and stateVelocities need to match in size.");
    }

    vector<vector<double>> states;
    for (int i=0; i<statePositions.size(); i++) {
      states.push_back({statePositions[i][0],
                        statePositions[i][1],
                        statePositions[i][2],
                        stateVelocities[i][0],
                        stateVelocities[i][1],
                        stateVelocities[i][2]});
    }
    return states;
  }


  CkSegment::CkSegment(vector<vector<double>> quats,
                       vector<double> times,
                       int bodyCode,
                       string referenceFrame,
                       string segmentId,
                       vector<vector<double>> angularVelocities,
                       string comment) {

    this->quats              = quats;
    this->times              = times;
    this->bodyCode           = bodyCode;
    this->referenceFrame     = referenceFrame;
    this->segmentId                 = segmentId;
    this->angularVelocities  = angularVelocities;
    this->comment            = comment;
  }
  

  void writeCk(string path,
               vector<vector<double>> quats,
               vector<double> times,
               int bodyCode,
               string referenceFrame,
               string segmentId,
               string sclk,
               string lsk,
               vector<vector<double>> angularVelocities,
               string comment) {

    SpiceInt handle;
    
    // convert times, but first, we need SCLK+LSK kernels

    // allow and furnish multiple sclks
    std::vector<std::unique_ptr<Kernel>> sclkKernels;
    if (!sclk.empty()) {
      for (const std::string& sclkPath : split(sclk, ','))
        sclkKernels.push_back(std::make_unique<Kernel>(sclkPath));
    }
    Kernel lskKernel(lsk);
    
    for(auto &et : times) {
      double sclkdp;
      checkNaifErrors();
      sce2c_c(bodyCode/1000, et, &sclkdp);
      checkNaifErrors();
      et = sclkdp;
    }
    checkNaifErrors();

    if(comment.empty()) {
      comment = "CK Kernel";
    }

    ckopn_c(path.c_str(), "CK", comment.size(), &handle);
    checkNaifErrors();

    // Flatten the nested quaternions into a single contiguous array.
    // CSPICE functions (like ckw03_c) are C-based and expect a pointer to a 
    // contiguous block of memory (SpiceDouble quats[][4]). 
    // A std::vector<std::vector<double>> stores inner vectors in fragmented 
    // locations on the heap; we must "flatten" them into a single ribbon 
    // of doubles so that pointer arithmetic works correctly inside SPICE.
    vector<double> flatQuats;
    flatQuats.reserve(quats.size() * 4);
    for (const auto& q : quats) {
        for (double d : q) {
            flatQuats.push_back(d);
        }
    }

    // Flatten angular velocities if they exist.
    // Similar to quaternions, AV data must be contiguous (SpiceDouble av[][3]).
    // We check if the input nested vector is non-empty before proceeding.
    vector<double> flatAv;
    if (!angularVelocities.empty()) {
        flatAv.reserve(angularVelocities.size() * 3);
        for (const auto& a : angularVelocities) {
            for (double d : a) {
                flatAv.push_back(d);
            }
        }
    }

    ckw03_c (handle,
             times.at(0),
             times.at(times.size()-1),
             bodyCode,
             referenceFrame.c_str(),
             !angularVelocities.empty(),
             segmentId.c_str(),
             times.size(),
             times.data(),
             flatQuats.data(),
            (!angularVelocities.empty()) ? flatAv.data() : nullptr,
             times.size(),
             times.data());
    checkNaifErrors();
    ckcls_c(handle);
    checkNaifErrors();

    writeComment(path, comment);
  }


  void writeSpk(string fileName,
                 vector<vector<double>> statePositions,
                 vector<double> stateTimes,
                 int bodyCode,
                 int centerOfMotion,
                 string referenceFrame,
                 string segmentId,
                 int polyDegree,
                 vector<vector<double>> stateVelocities,
                 string segmentComment) {

    if (stateTimes.empty() || statePositions.empty()) {
      throw runtime_error("writeSpk: stateTimes and statePositions must be non-empty.");
    }

    // NAIF spkw13_c requires segment start time < end time. Single-epoch (e.g. from ISD) has start == end.
    if (stateTimes.size() == 1) {
      stateTimes.push_back(stateTimes.front() + 1E-6);
      statePositions.push_back(statePositions.front());
      if (!stateVelocities.empty()) {
        stateVelocities.push_back(stateVelocities.front());
      }
    } else if (stateTimes.front() >= stateTimes.back()) {
      throw runtime_error(
          "writeSpk: segment start time must be less than end time (got start == end or reversed order).");
    }

    vector<vector<double>> states;

    if (stateVelocities.empty()) {
      // init a 0 velocity array
      vector<vector<double>> velocities;
      for (int i = 0; i < statePositions.size(); i++) {
        velocities.push_back({0,0,0});
      }
      stateVelocities = velocities;
    }

    states = concatStates(statePositions, stateVelocities);

    SpiceInt handle;
    checkNaifErrors();
    spkopn_c(fileName.c_str(), "SPK", 512, &handle);
    checkNaifErrors();

    spkw13_c(handle,
             bodyCode,
             centerOfMotion,
             referenceFrame.c_str(),
             stateTimes[0],
             stateTimes[stateTimes.size() - 1],
             segmentId.c_str(),
             polyDegree,
             stateTimes.size(),
             states.data(),
             stateTimes.data());
    checkNaifErrors();
    spkcls_c(handle);
    checkNaifErrors();

    // Write comment to header
    writeComment(fileName, segmentComment);

    return;
  }


  void writeSpk(string fileName, vector<SpkSegment> segments) {

    // TODO:
    //   write all segments not just first
    //   trap naif errors and do ????
    //   if file exists do something (delete it, error out)
    //   Add convience function to SpkSegment to combine the pos and vel into a single array

    // Combine positions and velocities for spicelib call

    writeSpk(fileName,
             segments[0].statePositions,
             segments[0].stateTimes,
             segments[0].bodyCode,
             segments[0].centerOfMotion,
             segments[0].referenceFrame,
             segments[0].segmentId,
             segments[0].polyDegree,
             segments[0].stateVelocities,
             segments[0].comment);

    return;
  }

  void writeCk(string fileName, string sclk, string lsk, vector<CkSegment> segments) {

    // TODO:
    //   write all segments not just first
    //   trap naif errors and do ????
    //   if file exists do something (delete it, error out)
    //   Add convience function to SpkSegment to combine the pos and vel into a single array

    // Combine positions and velocities for spicelib call

    writeCk(fileName,
            segments[0].quats,
            segments[0].times,
            segments[0].bodyCode,
            segments[0].referenceFrame,
            segments[0].segmentId,
            sclk,
            lsk,
            segments[0].angularVelocities,
            segments[0].comment);

  }

  void writeComment(string fileName, string comment) {
    SpiceInt handle;
    dafopw_c(fileName.c_str(), &handle);
    checkNaifErrors();

    // Trap errors so they are not fatal if the comment section fills up.
    // Calling environments can decide how to handle it.

    string commOut;
    checkNaifErrors();
    for ( int i = 0 ; i < comment.size() ; i++ ) {
        if ( comment[i] == '\n' ) {
          while ( commOut.size() < 2 ) { commOut.append(" "); }
          dafac_c(handle, 1, commOut.size(), commOut.data());
          checkNaifErrors();
          commOut.clear();
        }
        else {
          commOut.push_back(comment[i]);
        }
    }

    // See if there is residual to write
    if ( commOut.size() > 0 ) {
      while ( commOut.size() < 2 ) { commOut.append(" "); }
      dafac_c(handle, 1, commOut.size(), commOut.data());
      checkNaifErrors();
    }

    // Close file handle
    dafcls_c(handle);
  }

  void writeTextKernel(string fileName, string type, json &keywords, string comment) {

    /**
     * @brief Return an adjusted string such that it multi-lined if longer than some max length.
     *
     * This generates a string that is compatible with the requirements my NAIF text kernels
     * to be multi-line.
     */
    auto adjustStringLengths = [](string s, size_t maxLen = 100, bool isComment = true) -> string {
      // first, escape single quotes
      s = replaceAll(s, "'", "''");

      if (s.size() <= maxLen && isComment) {
        return s;
      }
      else if (s.size() <= maxLen - 5 && !isComment) {
        return "( '" + s + "' )";
      }

      string newString;
      string formatString = (isComment) ? "{}\n" : "( '{}' // )";

      for(int i = 0; i < s.size()/maxLen; i++) {
        newString.append(fmt::format(fmt::runtime(formatString), s.substr(i*maxLen, maxLen)) + "\n");
      }
      newString.append(fmt::format(fmt::runtime(formatString), s.substr(s.size()-(s.size()%maxLen), s.size()%maxLen)));

      return newString;
    };

    /**
     * @brief Given a JSON primitive, return a text kernel freindly string representation
     */
    auto json2String = [&](json keyword, size_t maxStrLen) -> string {
        if (!keyword.is_primitive()) {
          throw invalid_argument("Input JSON must be a primitive, not an array or object.");
        }

        if(keyword.is_string()) {
          return adjustStringLengths(keyword.get<string>(), maxStrLen, false);
        }
        if(keyword.is_number_integer()) {
          return to_string(keyword.get<int>());
        }
        if(keyword.is_number_float()) {
          return to_string(keyword.get<double>());
        }
        if(keyword.is_boolean()) {
          return (keyword.get<bool>()) ? "'true'" : "'false'";
        }
        if(keyword.is_null()) {
          return "'null'";
        }

        // theoritically should never throw
        throw invalid_argument("in json2string, input keyword is not a viable primitive type.");
    };

    static unsigned int MAX_TEXT_KERNEL_LINE_LEN = 132;
    
    fs::path kernelPath = fileName;
    fs::path parentDir = kernelPath.parent_path();
    if (!fs::exists(parentDir)) {
      throw std::runtime_error(fmt::format("Parent directory '{}' does not exist for output file '{}'", parentDir.string(), fileName));
    }

    ofstream textKernel;
    textKernel.open(fileName);
    string typeUpper = toUpper(type);
    vector<string> supportedTextKernels = {"FK", "IK", "IAK", "LSK", "MK", "PCK", "SCLK"};

    if (std::find(supportedTextKernels.begin(), supportedTextKernels.end(), typeUpper) == supportedTextKernels.end()) {
      throw invalid_argument(fmt::format("{} is not a valid text kernel type", type));
    }

    textKernel << "KPL/" + toUpper(type) << endl << endl;
    textKernel << "\\begintext" << endl << endl;
    textKernel << comment << endl << endl;
    textKernel << "\\begindata" << endl << endl;

    for(auto it = keywords.begin(); it != keywords.end(); it++) {

      if (it.value().is_array()) {
        for(auto ar = it.value().begin(); ar!=it.value().end();ar++) {
          textKernel << fmt::format("{} += {}", it.key(), json2String(ar.value(),  MAX_TEXT_KERNEL_LINE_LEN - it.key().size() + 5)) << endl;
        }
      }
      else if(it.value().is_primitive()) {
        textKernel << fmt::format("{} = {}",  it.key(), json2String(it.value(),  MAX_TEXT_KERNEL_LINE_LEN - it.key().size() + 5)) << endl;
      }
      else {
        // must be another object, skip.
        continue;
      }
    }

    textKernel.close();
    SPDLOG_TRACE("Text kernel written to {}", fileName);
  }

}
