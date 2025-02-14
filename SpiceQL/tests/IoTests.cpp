#include <gtest/gtest.h>
#include <vector>

#include "Fixtures.h"
#include "io.h"
#include "utils.h"

using namespace SpiceQL;


TEST(IoTests, UnitTestWriteCkTest) {
  fs::path path;
  path = static_cast<fs::path>(getenv("SPICEROOT")) / "test_ck.bsp";

  fs::path lskPath = fs::path("data") / "naif0012.tls"; 
  fs::path sclkPath = fs::path("data") / "lro_clkcor_2020184_v00.tsc";

  std::vector<std::vector<double>> orientations = {{0.2886751, 0.2886751, 0.5773503, 0.7071068 }, {0.4082483, 0.4082483, 0.8164966, 0 }};
  std::vector<std::vector<double>> av = {{1,1,1}, {1,2,3}};
  std::vector<double> times = {110000000, 120000001};
  int bodyCode = -85000; 
  std::string referenceFrame = "j2000";
  std::string segmentId = "CKCKCK";

  writeCk(path, orientations, times, bodyCode, referenceFrame, segmentId, sclkPath, lskPath, av);
}



TEST(IOTests, CreateSPKSegmentTest) {
  std::string comment = "This is a comment for \n a test SPK segment";
  int body = 1;
  int center = 1;
  std::string refFrame = "B1950";
  std::string id = "a test segment";
  int degree = 1;
  std::vector<std::vector<double>> pos = {{1.0, 2.0, 3.0}};
  std::vector<std::vector<double>> vel = {{0.1, 0.2, 0.3}};
  std::vector<double> et = {0.0};

  SpkSegment seg(pos, et, body, center, refFrame, id, degree, vel, comment);
  EXPECT_EQ(seg.comment, comment);

  // Add other member tests
}


TEST(IoTests, WriteSPKSegmentTest) {
  fs::path tpath;
  tpath = static_cast<fs::path>(getenv("SPICEROOT")) / "test_spk.bsp";

  std::string comment = "This is a comment for \n a test SPK segment";
  int body = 1;
  int center = 2;
  std::string refFrame = "J2000";
  std::string id = "a test segment";
  int degree = 1;
  std::vector<std::vector<double>> pos = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  std::vector<std::vector<double>> vel = {{0.1, 0.2, 0.3}, {0.4, 0.5, 0.6}};
  std::vector<double> et = {0.0, 1.0};

  std::vector<SpkSegment> segments;
  segments.push_back(SpkSegment(pos, et, body, center, refFrame, id, degree, vel, comment));

  writeSpk (tpath, segments);

  // TODO: Once we can read add a read and tests here
}


TEST(UnitTest, WriteTextKernel) { 
  fs::path tpath = static_cast<fs::path>(getenv("SPICEROOT")) / "test_ik.ti";

  nlohmann::json j = {
    {"test_pi", 3.141},
    {"test_happy", true},
    {"test_name", "Niels"},
    {"test_nothing", nullptr},
    {"test_array", {1, 2, 3, 5.0}}
    }; 

  writeTextKernel(tpath, "ik", j, "This is a IK kernel");

  // furnsh the new kernel
  Kernel k(tpath);

  // get all test keys in the variable pool
  nlohmann::json j2 = findKeywords("test_*");

  // diff should return an empty json array as input and output should be exactly the same
  ASSERT_EQ(nlohmann::json::diff(j, j2), nlohmann::json::array());
}


TEST(UnitTest, WriteTextKernelArrayAppend) { 
  // NAIF_BODY_CODE can be defined in multiple kernels, if and multiple kernels 
  // are furnished, the kernels should not overwrite the array but append to it instead.
  fs::path tpath1 = static_cast<fs::path>(getenv("SPICEROOT")) / "test_ik1.ti";
  fs::path tpath2 = static_cast<fs::path>(getenv("SPICEROOT")) / "test_ik2.ti";
  
  nlohmann::json j = {
    {"NAIF_BODY_NAME", {"body", "instrument1", "instrument2"}},
    {"NAIF_BODY_CODE", {-90, -90101, -90102}}
  }; 

  writeTextKernel(tpath1, "ik", j, "This is a IK kernel");

  j["NAIF_BODY_NAME"] = {"planet", "instrument3"};
  j["NAIF_BODY_CODE"] = {10, -90103};
 
  writeTextKernel(tpath2, "ik", j, "This one is different");

  // furnsh the new kernel
  Kernel k1(tpath1);
  Kernel k2(tpath2);

  EXPECT_EQ(translateNameToCode("body").first, -90);
  EXPECT_EQ(translateNameToCode("planet").first, 10);
  EXPECT_EQ(translateNameToCode("instrument1").first, -90101);
  EXPECT_EQ(translateNameToCode("instrument2").first, -90102);
  EXPECT_EQ(translateNameToCode("instrument3").first, -90103);
}

