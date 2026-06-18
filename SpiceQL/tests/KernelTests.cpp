#include <algorithm>

#include <gtest/gtest.h>
#include <fmt/format.h>

#include <SpiceQL/utils.h>
#include "Fixtures.h"
#include <SpiceQL/spice_types.h>
#include <SpiceQL/query.h>
#include <SpiceQL/inventory.h>
#include <SpiceQL/api.h>
#include <SpiceQL/io.h>

#include <SpiceUsr.h>

#include <SpiceQL/spiceql_logging.h>

using namespace SpiceQL;

TEST_F(LroKernelSet, UnitTestTranslateFrame) {

  nlohmann::json translationKernels;
  translationKernels["fk"]["kernels"] = {{fkPath}};

  string expectedFrameName = "LRO_LROCWAC";
  auto [frameCode, kernels1] = translateNameToCode(expectedFrameName, "lroc");
  EXPECT_EQ(frameCode, -85620);

  auto [frameName, kernels2] = translateCodeToName(frameCode, "lroc");
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
  
  int nkernels;
  
  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 0);
  ktotal_c("ck", &nkernels);
  EXPECT_EQ(nkernels, 0);
  ktotal_c("spk", &nkernels);
  EXPECT_EQ(nkernels, 0);

  // all the kernels in the group are now furnished.
  KernelSet ks(kernels);

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
    EXPECT_EQ(nkernels, 2);
  }

  // All kernels in previous stack should be unfurnished
  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 3);
  ktotal_c("ck", &nkernels);
  EXPECT_EQ(nkernels, 1);
  ktotal_c("spk", &nkernels);
  EXPECT_EQ(nkernels, 1);
}


TEST_F(LroKernelSet, UnitTestStrSclkToEt) {
  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}, {sclkPath}, {lskPath}};
  KernelSet testSet(testKernelJson);
  
  auto [et, kernels] = strSclkToEt(-85, "1/281199081:48971", "lro");

  EXPECT_DOUBLE_EQ(et, 312778347.97478431);
}


TEST_F(LroKernelSet, UnitTestDoubleSclkToEt) {
  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}, {sclkPath}, {lskPath}};
  KernelSet testSet(testKernelJson);

  auto [et, kernels] = doubleSclkToEt(-85, 922997380.174174, "lro");

  EXPECT_DOUBLE_EQ(et, 31593348.006268278);
}


TEST_F(LroKernelSet, UnitTestUtcToEt) {
  auto [et, kernels] = utcToEt("2016-11-26 22:32:14.582000");

  EXPECT_DOUBLE_EQ(et, 533471602.76499087);
}


TEST_F(LroKernelSet, TestUtcToEtNoSearch) {
  const std::string testUtc = "2016-11-26T22:32:14.582000";
  const double expectedEt = 533471602.76499087;

  // Test with no search (uses utcet)
  auto [et_nosearch, kernels_nosearch] = utcToEt(testUtc, false, false);
  EXPECT_DOUBLE_EQ(et_nosearch, expectedEt);

  // Test with search (uses LSK kernel)
  auto [et_search, kernels_search] = utcToEt(testUtc, false, true);

  // Both methods should produce the same result
  EXPECT_NEAR(et_nosearch, et_search, 1e-6);
}


TEST_F(LroKernelSet, TestEtToUTCNoSearch) {
  const double testEt = 533471602.76499087;
  const std::string expectedUtc = "2016-11-26T22:32:14.582000";
  const std::string format = "ISOC";
  const int precision = 6;

  // Test with no search (uses utcet)
  auto [utc_nosearch, kernels_nosearch] = etToUtc(testEt, format, precision, false, false);
  EXPECT_STREQ(utc_nosearch.c_str(), (expectedUtc).c_str());

  // Test with search (uses LSK kernel)
  auto [utc_search, kernels_search] = etToUtc(testEt, format, precision, false, true);
  EXPECT_STREQ(utc_search.c_str(), expectedUtc.c_str());

  // Both paths must produce byte-identical output.
  EXPECT_EQ(utc_nosearch, expectedUtc);
  EXPECT_EQ(utc_search, expectedUtc);
}

TEST_F(LroKernelSet, UnitTestGetLoadedKernels) {
  // Nothing furnished yet, so the pool reports no kernels.
  EXPECT_TRUE(getLoadedKernels().empty());

  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {lskPath}, {sclkPath}};
  std::vector<std::string> expected = {ckPath1, ckPath2, spkPath1, lskPath, sclkPath};

  {
    KernelSet testSet(testKernelJson);

    std::vector<std::string> loaded = getLoadedKernels();
    for (const auto &path : loaded) {
      SPDLOG_DEBUG("Got Loaded kernel: {}", path);
    }
    
    // Every kernel that was furnished is reported as loaded, regardless of type.
    EXPECT_EQ(loaded.size(), expected.size());
    for (const auto &path : expected) {
      EXPECT_NE(std::find(loaded.begin(), loaded.end(), path), loaded.end())
          << "expected " << path << " to be reported as loaded";
    }
  }

  // Once the set is unfurnished the pool is empty again.
  EXPECT_TRUE(getLoadedKernels().empty());
}


TEST_F(LroKernelSet, UnitTestIsLskLoaded) {
  // No LSK furnished yet, so the leapseconds variable is absent from the pool.
  EXPECT_FALSE(isLskLoaded());

  {
    // Furnishing a kernel set that has no LSK must not flip the result.
    nlohmann::json noLskJson;
    noLskJson["kernels"] = {{ckPath1}, {spkPath1}, {sclkPath}};
    KernelSet noLskSet(noLskJson);
    EXPECT_FALSE(isLskLoaded());
  }

  {
    // Furnishing an LSK loads DELTET/DELTA_T_A into the pool.
    Kernel lsk(lskPath);
    EXPECT_TRUE(isLskLoaded());
  }

  // Once the LSK is unfurnished the pool no longer reports it.
  EXPECT_FALSE(isLskLoaded());
}


TEST_F(LroKernelSet, UnitTestGetFrameInfo) {

  nlohmann::json translationKernels;
  translationKernels["fk"]["kernels"] = {{fkPath}};

  auto [res, kernels] = getFrameInfo(-85620, "lroc");
  EXPECT_EQ(res[0], -85);
  EXPECT_EQ(res[1], 3);
  EXPECT_EQ(res[2], -85620);
}


TEST_F(LroKernelSet, InferMissionGetFrameInfo) {
  Inventory::create_database();

  // mission inferred from the frame code: -85620 -> bus -85 -> "LRO" -> "lro"
  auto [resInferred, kernelsInferred] = getFrameInfo(-85620, "");
  auto [resExplicit, kernelsExplicit] = getFrameInfo(-85620, "lroc");
  EXPECT_EQ(resInferred, resExplicit);
  EXPECT_EQ(resInferred[0], -85);
  EXPECT_EQ(resInferred[1], 3);
  EXPECT_EQ(resInferred[2], -85620);
}


TEST_F(LroKernelSet, InferMissionStrSclkToEt) {
  Inventory::create_database();

  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}, {sclkPath}, {lskPath}};
  KernelSet testSet(testKernelJson);

  auto [etInferred, kernelsInferred] = strSclkToEt(-85, "1/281199081:48971", "");
  auto [etExplicit, kernelsExplicit] = strSclkToEt(-85, "1/281199081:48971", "lro");
  EXPECT_DOUBLE_EQ(etInferred, etExplicit);
  EXPECT_DOUBLE_EQ(etInferred, 312778347.97478431);
}


TEST_F(LroKernelSet, InferMissionFromFrameName) {
  Inventory::create_database();

  // mission inferred from the frame name "LRO_LROCNACL" -> alias -> "lroc"
  auto [codeInferred, kernelsInferred] = translateNameToCode("LRO_LROCNACL", "");
  auto [codeExplicit, kernelsExplicit] = translateNameToCode("LRO_LROCNACL", "lroc");
  EXPECT_EQ(codeInferred, codeExplicit);
  EXPECT_EQ(codeInferred, -85600);
}


TEST_F(LroKernelSet, UnitTestFindMissionKeywords) {
  auto [keywords, kernels] = findMissionKeywords("INS-85600_CCD_CENTER", "lro");

  nlohmann::json expectedResults;
  expectedResults["INS-85600_CCD_CENTER"] = {2531.5, 0.5};

  EXPECT_EQ(keywords, expectedResults);
}


TEST_F(LroKernelSet, UnitTestGetTargetFrameInfo) {
  auto [frameInfo, kernels] = getTargetFrameInfo(499, "lroc");

  nlohmann::json expectedResults;
  expectedResults["frameCode"] = 10014;
  expectedResults["frameName"] = "IAU_MARS";

  EXPECT_EQ(frameInfo, expectedResults);
}


TEST_F(TempTestingFiles, UnitTestIAKKernelSetConstructor) {
  nlohmann::json kernels;
  nlohmann::json iakData = {
    {"IK_KEY", { 100 }},
    {"INS-85600_CCD_CENTER", { 2531.5 , 0.5 }}
  };
  nlohmann::json ikData = {
    {"IK_KEY", { 200 }},
    {"INS-85600_CCD_CENTER", { 2531.5 , 0.5 }}
  };
  fs::path iakPath = tempDir / "iak.ti";
  fs::path ikPath = tempDir / "ik.ti";

  writeTextKernel(iakPath, "iak", iakData);
  writeTextKernel(ikPath, "ik", ikData);

  kernels["iak"] = {iakPath.string()};
  kernels["ik"] = {ikPath.string()};

  KernelSet ks(kernels);

  EXPECT_EQ(ks.m_loadedKernels.size(), 2);

  // iak should be loaded second
  EXPECT_EQ(ks.m_loadedKernels[0]->path, ikPath.string());
  EXPECT_EQ(ks.m_loadedKernels[1]->path, iakPath.string());
  EXPECT_EQ(findKeywords("IK_KEY")["IK_KEY"][1], 100);
}


TEST(FormatKernelsTest, FormatKernelsBasicTypes) {
  std::vector<std::string> kernels = {
    "k1.tf",
    "k2.ti",
    "k3.bsp"
  };

  nlohmann::json result = formatKernels(kernels);

  EXPECT_TRUE(result.contains("fk"));
  EXPECT_TRUE(result.contains("ik"));
  EXPECT_TRUE(result.contains("spk"));

  EXPECT_EQ(result["fk"].size(), 1);
  EXPECT_EQ(result["ik"].size(), 1);
  EXPECT_EQ(result["spk"].size(), 1);

  EXPECT_EQ(result["fk"][0], "k1.tf");
  EXPECT_EQ(result["ik"][0], "k2.ti");
  EXPECT_EQ(result["spk"][0], "k3.bsp");
}


TEST(FormatKernelsTest, FormatKernelsMultipleSameType) {
  std::vector<std::string> kernels = {
    "k1.tf",
    "k2.tf",
    "k3.bsp",
    "k4.bsp"
  };

  nlohmann::json result = formatKernels(kernels);

  EXPECT_TRUE(result.contains("fk"));
  EXPECT_TRUE(result.contains("spk"));

  EXPECT_EQ(result["fk"].size(), 2);
  EXPECT_EQ(result["spk"].size(), 2);

  EXPECT_EQ(result["fk"][0], "k1.tf");
  EXPECT_EQ(result["fk"][1], "k2.tf");
  EXPECT_EQ(result["spk"][0], "k3.bsp");
  EXPECT_EQ(result["spk"][1], "k4.bsp");
}


TEST(FormatKernelsTest, FormatKernelsAllTypes) {
  std::vector<std::string> kernels = {
    "attitude.bc",
    "ephemeris.bsp",
    "frames.tf",
    "instrument.ti",
    "leapsec.tls",
    "metakernel.tm",
    "constants.tpc",
    "clock.tsc",
    "addendum.iak",
    "shape.bds",
    "events.bes"
  };

  nlohmann::json result = formatKernels(kernels);

  EXPECT_TRUE(result.contains("ck"));
  EXPECT_TRUE(result.contains("spk"));
  EXPECT_TRUE(result.contains("fk"));
  EXPECT_TRUE(result.contains("ik"));
  EXPECT_TRUE(result.contains("lsk"));
  EXPECT_TRUE(result.contains("mk"));
  EXPECT_TRUE(result.contains("pck"));
  EXPECT_TRUE(result.contains("sclk"));
  EXPECT_TRUE(result.contains("iak"));
  EXPECT_TRUE(result.contains("dsk"));
  EXPECT_TRUE(result.contains("ek"));

  EXPECT_EQ(result["ck"][0], "attitude.bc");
  EXPECT_EQ(result["spk"][0], "ephemeris.bsp");
  EXPECT_EQ(result["fk"][0], "frames.tf");
}


TEST(FormatKernelsTest, FormatKernelsEmptyInput) {
  std::vector<std::string> kernels = {};

  nlohmann::json result = formatKernels(kernels);

  EXPECT_TRUE(result.is_object());
  EXPECT_EQ(result.size(), 0);
}


TEST(FormatKernelsTest, FormatKernelsMixedCase) {
  std::vector<std::string> kernels = {
    "kernel.TF",
    "KERNEL.BSP",
    "Kernel.Ti"
  };

  nlohmann::json result = formatKernels(kernels);

  EXPECT_TRUE(result.contains("fk"));
  EXPECT_TRUE(result.contains("spk"));
  EXPECT_TRUE(result.contains("ik"));
}


TEST_F(LroKernelSet, FormatKernelsWithRealFiles) {
  std::vector<std::string> kernels = {
    lskPath,
    fkPath,
    ikPath1
  };

  nlohmann::json result = formatKernels(kernels);

  EXPECT_TRUE(result.contains("lsk"));
  EXPECT_TRUE(result.contains("fk"));
  EXPECT_TRUE(result.contains("ik"));

  EXPECT_EQ(result["lsk"][0], lskPath);
  EXPECT_EQ(result["fk"][0], fkPath);
  EXPECT_EQ(result["ik"][0], ikPath1);
}
