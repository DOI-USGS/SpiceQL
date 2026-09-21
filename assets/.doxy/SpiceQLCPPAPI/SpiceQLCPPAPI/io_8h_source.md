

# File io.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**io.h**](io_8h.md)

[Go to the documentation of this file](io_8h.md)


```C++
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace SpiceQL {


  class SpkSegment {
    public:

      SpkSegment(std::vector<std::vector<double>> statePositions,
                 std::vector<double> stateTimes,
                 int bodyCode,
                 int centerOfMotion,
                 std::string referenceFrame,
                 std::string segmentId, int degree,
                 std::vector<std::vector<double>> stateVelocities = {},
                 std::string segmentComment = "");

      std::vector<double> stateTimes;
      int bodyCode;
      int centerOfMotion;
      std::string referenceFrame;
      std::string segmentId;
      int polyDegree;
      std::vector<std::vector<double>> statePositions;
      std::vector<std::vector<double>> stateVelocities;
      std::string comment;
  };


  class CkSegment {
      public:

        CkSegment(std::vector<std::vector<double>> quats, std::vector<double> times,  int bodyCode,
                  std::string referenceFrame, std::string segmentId,
                  std::vector<std::vector<double>> angularVelocities = {},
                  std::string comment = "");

        std::vector<double> times;
        std::vector<std::vector<double>> quats;
        int bodyCode;
        std::string referenceFrame;
        std::string segmentId;
        std::vector<std::vector<double>> angularVelocities;
        std::string comment;
    };


    std::vector<std::vector<double>> concatStates (std::vector<std::vector<double>> statePositions,
                                                   std::vector<std::vector<double>> stateVelocities);


    void writeSpk (std::string fileName,
                   std::vector<SpkSegment> segments);


    void writeSpk(std::string fileName,
                  std::vector<std::vector<double>> statePositions,
                  std::vector<double> stateTimes,
                  int bodyCode,
                  int centerOfMotion,
                  std::string referenceFrame,
                  std::string segmentId,
                  int polyDegree,
                  std::vector<std::vector<double>> stateVelocities = {},
                  std::string segmentComment = "");


    void writeCk(std::string fileName,
                 std::vector<std::vector<double>> quats,
                 std::vector<double> times,
                 int bodyCode,
                 std::string referenceFrame,
                 std::string segmentId,
                 std::vector<std::vector<double>> angularVelocity = {},
                 std::string comment = "");


  void writeCk(std::string fileName,
               std::vector<CkSegment> segments);

  void writeComment(std::string fileName,
                    std::string comment);

  void writeTextKernel(std::string fileName, std::string type, nlohmann::json &keywords, std::string comment = "");

  }
```


