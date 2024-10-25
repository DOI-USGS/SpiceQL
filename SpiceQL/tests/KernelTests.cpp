#include <gtest/gtest.h>
#include <fmt/format.h>

#include <HippoMocks/hippomocks.h>

#include "utils.h"
#include "Fixtures.h"
#include "spice_types.h"
#include "query.h"
#include "inventory.h"

#include <SpiceUsr.h>

#include "spdlog/spdlog.h"

using namespace SpiceQL;

TEST_F(LroKernelSet, UnitTestTranslateFrame) {
  MockRepository mocks;
  nlohmann::json translationKernels;
  translationKernels["fk"]["kernels"] = {{fkPath}};
  mocks.OnCallFunc(loadTranslationKernels).Return(translationKernels);

  string expectedFrameName = "LRO_LROCWAC";
  int frameCode = translateNameToCode(expectedFrameName, "lroc");
  EXPECT_EQ(frameCode, -85620);

  string frameName = translateCodeToName(frameCode, "lroc");
  EXPECT_EQ(frameName, expectedFrameName);
}


TEST_F(LroKernelSet, UnitTestStackedKernelConstructorDestructor) {
  int nkernels;

  // This should create local kernels that get unfurnished when the stack is popped
  {
    Kernel k(lskPath);

    // should match what spice counts
    ktotal_c("text", &nkernels);
    EXPECT_EQ(nkernels, 1);
  }

  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 0);
}


TEST_F(LroKernelSet, UnitTestStackedKernelCopyConstructor) {
  int nkernels;

  // Same kernel that is furnished three times is still one kernel
  {
    Kernel k(lskPath);
    Kernel k2 = k;
    Kernel k3(k2);

    // should match what spice counts
    ktotal_c("text", &nkernels);
    EXPECT_EQ(nkernels, 1);
  }

  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 0);
}


TEST_F(LroKernelSet, UnitTestStackedKernelSetConstructorDestructor) {
  // load all available kernels
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"lsk", "sclk", "ck", "spk", "ik", "fk"}, 110000000, 120000001);
  SPDLOG_DEBUG("Kernels after search: {} ", kernels.dump());

  // all the kernels in the group are now furnished.
  KernelSet ks(kernels);

  int nkernels;

  // load kernels in a closed call stack
  {
    // kernels are now loaded twice
    KernelSet k(kernels);

    // should match what spice counts
    ktotal_c("text", &nkernels);
    EXPECT_EQ(nkernels, 6);
    ktotal_c("ck", &nkernels);
    EXPECT_EQ(nkernels, 2);
    ktotal_c("spk", &nkernels);
    EXPECT_EQ(nkernels, 4);
  }

  // All kernels in previous stack should be unfurnished
  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 3);
  ktotal_c("ck", &nkernels);
  EXPECT_EQ(nkernels, 1);
  ktotal_c("spk", &nkernels);
  EXPECT_EQ(nkernels, 2);
}


TEST_F(LroKernelSet, UnitTestStrSclkToEt) {
  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}, {sclkPath}, {lskPath}};
  KernelSet testSet(testKernelJson);
  
  double et = strSclkToEt(-85, "1/281199081:48971", "lro");

  EXPECT_DOUBLE_EQ(et, 312778347.97478431);
}


TEST_F(LroKernelSet, UnitTestDoubleSclkToEt) {
  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}, {sclkPath}, {lskPath}};
  KernelSet testSet(testKernelJson);

  double et = doubleSclkToEt(-85, 922997380.174174, "lro");

  EXPECT_DOUBLE_EQ(et, 31593348.006268278);
}


TEST_F(LroKernelSet, UnitTestUtcToEt) {
  double et = utcToEt("2016-11-26 22:32:14.582000");

  EXPECT_DOUBLE_EQ(et, 533471602.76499087);
}


TEST_F(LroKernelSet, UnitTestGetFrameInfo) {
  MockRepository mocks;
  nlohmann::json translationKernels;
  translationKernels["fk"]["kernels"] = {{fkPath}};
  mocks.OnCallFunc(loadTranslationKernels).Return(translationKernels);

  vector<int> res = getFrameInfo(-85620, "lroc");
  EXPECT_EQ(res[0], -85);
  EXPECT_EQ(res[1], 3);
  EXPECT_EQ(res[2], -85620);
}


TEST_F(LroKernelSet, UnitTestFindMissionKeywords) {
  nlohmann::json keywords = findMissionKeywords("INS-85600_CCD_CENTER", "lro");

  nlohmann::json expectedResults;
  expectedResults["INS-85600_CCD_CENTER"] = {2531.5, 0.5};

  EXPECT_EQ(keywords, expectedResults);
}


TEST_F(LroKernelSet, UnitTestGetTargetFrameInfo) {
  nlohmann::json frameInfo = getTargetFrameInfo(499, "lroc");

  nlohmann::json expectedResults;
  expectedResults["frameCode"] = 10014;
  expectedResults["frameName"] = "IAU_MARS";

  EXPECT_EQ(frameInfo, expectedResults);
}
