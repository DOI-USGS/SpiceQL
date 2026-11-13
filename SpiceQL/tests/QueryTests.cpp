#include <fstream>
#include <algorithm>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <SpiceQL/spiceql_logging.h>

#include "Fixtures.h"

#include <SpiceQL/query.h>
#include <SpiceQL/utils.h>

using namespace std;
using namespace SpiceQL;

TEST(QueryTests, UnitTestGetLatestKernel) {
  vector<string> kernels = {
    "iak.0001.ti",
    "iak.0003.ti",
    "different/place/iak.0002.ti",
    "test/iak.0004.ti"
  };

  EXPECT_EQ(getLatestKernel(kernels)[0],  "test/iak.0004.ti");
}


TEST(QueryTests, UnitTestGetLatestKernelDuplicateFileNames) {
  vector<string> kernels = {
                    "/base/kernels/spk/mar080.bsp",
                    "/base/kernels/spk/mar097.bsp",
                    "/mro/kernels/spk/mar063.bsp",
                    "/odyssey/kernels/spk/mar063.bsp",
                    "/tgo/kernels/spk/mar085.bsp",
                    "/tgo/kernels/spk/mar097.bsp",
                    "/tgo/kernels/tspk/mar085.bsp",
                    "/tgo/kernels/tspk/mar097.bsp",
                    "/viking1/kernels/spk/mar033.bsp",
                    "/viking2/kernels/spk/mar033.bsp", 
                    "/base/kernels/spk/mar097.bsp"
  };

  // which one doesn't matter, as long as we get mar097.bsp
  EXPECT_EQ(static_cast<fs::path>(getLatestKernel(kernels)[0]).filename(),  "mar097.bsp");
}


TEST(QueryTests, getKernelStringValue){
  unique_ptr<Kernel> k(new Kernel(fs::absolute("data/msgr_mdis_v010.ti")));
  // INS-236810_CCD_CENTER        =  (  511.5, 511.5 )
  string res = getKernelStringValue("INS-236810_FOV_SHAPE");
  EXPECT_EQ(res, "RECTANGLE");

  res = getKernelStringValue("INS-236810_FOV_REF_ANGLE");
  EXPECT_EQ(res, "0.7465");

  try {
    getKernelStringValue("aKeyThatWillNotBeInTheResults");
    FAIL() << "Expected std::invalid_argument";
  }
  catch(std::invalid_argument const & err) {
      EXPECT_EQ(err.what(),std::string("key not in results"));
  }
  catch(...) {
      FAIL() << "Expected std::invalid_argument";
  }
}


TEST(QueryTests, getKernelVectorValue){
  unique_ptr<Kernel> k(new Kernel(fs::absolute("data/msgr_mdis_v010.ti")));

  vector<string> actualResultsOne = getKernelVectorValue("INS-236810_CCD_CENTER");
  std::vector<string> expectedResultsOne{"511.5", "511.5"};

  vector<string> actualResultsTwo = getKernelVectorValue("INS-236800_FOV_REF_VECTOR");
  std::vector<string> expectedResultsTwo{"1.0", "0.0", "0.0"};

  for (int i = 0; i < actualResultsOne.size(); ++i) {
    EXPECT_EQ(actualResultsOne[i], expectedResultsOne[i]) << "Vectors x and y differ at index " << i;
  }

  for (int j = 0; j < actualResultsTwo.size(); ++j) {
    EXPECT_EQ(actualResultsTwo[j], expectedResultsTwo[j]) << "Vectors x and y differ at index " << j;
  }

   try {
        getKernelVectorValue("aKeyThatWillNotBeInTheResults");
        FAIL() << "Expected std::invalid_argument";
    }
    catch(std::invalid_argument const & err) {
        EXPECT_EQ(err.what(),std::string("key not in results"));
    }
    catch(...) {
        FAIL() << "Expected std::invalid_argument";
    }
}

TEST(QueryTests, UnitTestGetLatestKernelError) {
  vector<string> kernels = {
    "iak.0001.ti",
    "iak.0003.ti",
    "different/place/iak.0002.ti",
    "test/iak.4.ti",
    // different extension means different filetype and therefore error
    "test/error.tf"
  };

  try {
    getLatestKernel(kernels);
    FAIL() << "expected invalid argument error";
  }
  catch(invalid_argument &e) {
    SUCCEED();
  }
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsAllMess) {
  string dbPath = getMissionConfigFile("mess");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  nlohmann::json res = listMissionKernels(tempDir, conf);

  SPDLOG_INFO("res: {}", res.dump());
  vector<string> s = SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["reconstructed"]["kernels"]);
  SPDLOG_INFO("CK Reconstructed: {}", fmt::join(s, ", "));

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["smithed"]["kernels"]).size(), 4);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["tspk"]["kernels"]).size(), 0);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["fk"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ik"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["iak"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["pck"]["noquality"]["kernels"]).size(), 2);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis_att"]["ck"]["reconstructed"]["kernels"]).size(), 4);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["messenger"]["ck"]["reconstructed"]["kernels"]).size(), 5);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["messenger"]["sclk"]["kernels"]).size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsClem1) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  nlohmann::json res = listMissionKernels(tempDir, conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["ck"]["smithed"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["fk"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["clementine1"]["sclk"]["kernels"]).size(), 2);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["uvvis"]["ik"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["uvvis"]["iak"]["kernels"]).size(), 2);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["uvvis"]["iak"]["kernels"]).size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsGalileo) {
  fs::path dbPath = getMissionConfigFile("galileo");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["ck"]["smithed"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["iak"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["pck"]["smithed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["pck"]["noquality"]["kernels"]).size(), 0);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["sclk"]["kernels"]).size(), 1);
}



TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsCassini) {
  fs::path dbPath = getMissionConfigFile("cassini");
  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  nlohmann::json res = listMissionKernels(tempDir/"cassini", conf);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["ck"]["reconstructed"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["ck"]["smithed"]["kernels"]).size(), 3);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["fk"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["iak"]["kernels"]).size(), 3);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["pck"]["kernels"]).size(), 0);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["pck"]["smithed"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["sclk"]["kernels"]).size(), 1);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["spk"]["reconstructed"]["kernels"]).size(), 2);
}


// test for apollo 17 kernels 
TEST_F(IsisDataDirectory, FunctionalTestApollo17Conf) {
  fs::path dbPath = getMissionConfigFile("apollo17");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);
  
  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> expectedKernels = missionMap.at("apollo17");
  set<string> diff; 
  set_difference(expectedKernels.begin(), expectedKernels.end(), kernels.begin(), kernels.end(), inserter(diff, diff.begin()), 
    [](const string& a, const string& b) { return fs::path(a).filename() == fs::path(b).filename(); });
  if (diff.size() != 0) {
    FAIL() << "Kernel sets are not equal, diff: " << fmt::format("{}", fmt::join(diff, " ")) << endl;
  }
}


TEST_F(IsisDataDirectory, FunctionalTestsLroKernelList) { 
  compareKernelSets("lro", {"de421.bsp"});
}


TEST_F(IsisDataDirectory, FunctionalTestsCH2KernelList) { 
  compareKernelSets("chandrayaan2", {});

  nlohmann::json conf = getMissionConfig("chandrayaan2");
  nlohmann::json res = listMissionKernels(tempDir, conf);
  SPDLOG_DEBUG("Kernel Results: {}", res.dump(4));
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("chandrayaan2").at("spk").at("reconstructed").at("kernels"));
  vector<string> expected = {
    tempDir/"chandrayaan2"/"kernels"/ "spk" / "ch2_orb_31Mar2020_02May2020_v1.bsp",
    tempDir/"chandrayaan2"/"kernels"/ "spk" / "ch2_eph_30Sep2025_02Nov2025_v1.bsp"};
  SPDLOG_DEBUG("Checking TMC2 SPKs");
  for (auto &e : expected) {
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("chandrayaan2").at("ck").at("reconstructed").at("kernels"));
  expected = {
    tempDir/"chandrayaan2"/"kernels"/ "ck" / "ch2_att_01Sep2019_02Oct2019_v1.bc",
    tempDir/"chandrayaan2"/"kernels"/ "ck" / "ch2_att_27Mar2021_04May2021_v1.bc",
    tempDir/"chandrayaan2"/"kernels"/ "ck" / "ch2_att_29Nov2020_04Jan2021_v1.bc"};
  SPDLOG_DEBUG("Checking OHRC CKs");
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("tmc2").at("iak").at("kernels"));
  expected = {tempDir/"chandrayaan2"/"kernels"/ "iak" / "tmc2Addendum001.ti"};
  SPDLOG_DEBUG("Checking TMC2 IAKs");
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("ohrc").at("iak").at("kernels"));
  expected = {tempDir/"chandrayaan2"/"kernels"/ "iak" / "ohrcAddendum001.ti"};
  SPDLOG_DEBUG("Checking OHRC IAKs");
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("chandrayaan2").at("fk").at("kernels"));
  expected = {tempDir/"chandrayaan2"/"kernels"/ "fk" / "ch2_v01.tf"};
  SPDLOG_DEBUG("Checking CH2 FKs");
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("chandrayaan2").at("sclk").at("kernels"));
  expected = {tempDir/"chandrayaan2"/"kernels"/ "sclk" / "ch2_sclk_v1.tsc"};
  SPDLOG_DEBUG("Checking CH2 SCLKs");
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }
}

TEST_F(IsisDataDirectory, FunctionalTestLroConf) {
  nlohmann::json conf = getMissionConfig("lro");
  
  nlohmann::json res = listMissionKernels(tempDir, conf);
  SPDLOG_DEBUG("Kernel Results: {}", res.dump(4));
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("moc").at("ck").at("reconstructed").at("kernels"));
  vector<string> expected = {tempDir/"lro"/"kernels"/ "ck" / "moc42r_2016305_2016336_v01.bc"};
  SPDLOG_DEBUG("Checking Recon CKs");
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  SPDLOG_DEBUG("Checking Smithed CKs");
  kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("moc").at("ck").at("smithed").at("kernels"));
  expected = {
    tempDir/"lro"/"kernels"/ "ck" /"LRONAC_SPole_LE_2009_ck.bc"
  };
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }
  
  SPDLOG_DEBUG("Checking Recon SPKs");
  kernelToCheck = getKernelsAsVector(res.at("moc").at("spk").at("reconstructed")); 
  expected = {tempDir/"lro"/"kernels"/ "spk" /"fdf29r_2018305_2018335_v01.bsp"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  SPDLOG_DEBUG("Checking Smithed SPKs");
  kernelToCheck = getKernelsAsVector(res.at("moc").at("spk").at("smithed")); 
  expected = {tempDir/"lro"/"kernels"/ "spk" /"LRO_ES_05_201308_GRGM660PRIMAT270.bsp", 
              tempDir/"lro"/"kernels"/ "spk" /"LRO_ES_16_201406_GRGM900C_L600.BSP",
              tempDir/"lro"/"kernels"/ "spk" /"LRONAC_SPole_RE_2013_spk.bsp"
  };
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

  SPDLOG_DEBUG("Checking IAKs");
  kernelToCheck = getKernelsAsVector(res.at("minirf").at("iak"));
  expected = {tempDir/"lro"/"kernels"/ "iak" / "mrflroAddendum002.ti"};
  EXPECT_EQ(kernelToCheck, expected);
}


TEST_F(IsisDataDirectory, FunctionalTestJunoConf) {
  set<string> expectedDiff = {
                              tempDir / "juno" / "kernels" / "tspk" / "de436s.bsp",
                              tempDir / "juno" / "kernels" / "tspk" / "de438s.bsp",
                              tempDir / "juno" / "kernels" / "tspk" / "de440s.bsp"};
  compareKernelSets("juno", expectedDiff);
} 


TEST_F(IsisDataDirectory, FunctionalTestMroConf) {
  compareKernelSets("mro");

  nlohmann::json conf = getMissionConfig("mro");
  nlohmann::json res = listMissionKernels(tempDir, conf);

  // check a kernel from each regex exists in there quality groups
  vector<string> kernelToCheck =  getKernelsAsVector(res.at("mro").at("spk").at("reconstructed").at("kernels"));
  vector<string> expected = {
                            tempDir / "mro" / "kernels" / "spk" / "mro_cruise.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_ab.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_psp_rec.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_psp1.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_psp10.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_psp_rec.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_psp1_ssd_mro95a.bsp", 
                            tempDir / "mro" / "kernels" / "spk" / "mro_psp27_ssd_mro110c.bsp"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }

  kernelToCheck = getKernelsAsVector(res.at("mro").at("spk").at("predicted")); 
  expected = {tempDir / "mro" / "kernels" / "spk" / "mro_psp.bsp"};
  ASSERT_EQ(kernelToCheck, expected);

  kernelToCheck = getKernelsAsVector(res.at("mro").at("ck").at("reconstructed"));
  expected = {tempDir / "mro" / "kernels" / "ck" / "mro_sc_psp_160719_160725.bc",
              tempDir / "mro" / "kernels" / "ck" / "mro_sc_cru_060301_060310.bc",
              tempDir / "mro" / "kernels" / "ck" / "mro_sc_ab_060801_060831.bc",
              tempDir / "mro" / "kernels" / "ck" / "mro_sc_psp_150324_150330_v2.bc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }
}

TEST_F(IsisDataDirectory, FunctionalTestOsirisRexConf) {

  set<string> unsupportedOrexKernels = {
    tempDir / "osirisrex" / "kernels" / "iak" / "orex_ocams_addendum_v07_draft.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_lidar_v00.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_ola_v00.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_ola_v01.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_otes_v00.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_ovirs_v00.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_rexis_v00.ti",
    tempDir / "osirisrex" / "kernels" / "ik" / "orx_rexis_v01.ti"
  };

  compareKernelSets("osirisrex", unsupportedOrexKernels);

  nlohmann::json conf = getMissionConfig("osirisrex");
  nlohmann::json res = listMissionKernels(tempDir, conf);
  SPDLOG_DEBUG("Kernel Results: {}", res.dump(4));

  vector<string> kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("osirisrex").at("spk").at("reconstructed").at("kernels"));
  vector<string> expected = {
    tempDir / "osirisrex" / "kernels" / "spk" / "orx_200305_200421_200410_od239_v1.bsp",
    tempDir / "osirisrex" / "kernels" / "spk" / "orx_230809_231001_230930_od383a_v1.bsp"
  };

  SPDLOG_DEBUG("Checking OREX SPKs");
  for (auto &e : expected) {
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      throw runtime_error(e+" was not found in the kernel results");
    }
  }

}


TEST_F(IsisDataDirectory, FunctionalTestViking1Conf) {
  compareKernelSets("viking1");
  // skip specific tests since viking images are mostly literals and without mixed qualities
}


TEST_F(IsisDataDirectory, FunctionalTestViking2Conf) {
  compareKernelSets("viking2");
}


TEST_F(IsisDataDirectory, FunctionalTestMgsConf) {
  fs::path dbPath = getMissionConfigFile("mgs");
  compareKernelSets("mgs");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("mgs");
 
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  getKernelsAsVector(res.at("mgs").at("ck").at("reconstructed"));
  vector<string> expected = {tempDir / "mgs" / "kernels" / "ck" / "mgs_sc_ab1.bc"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
  
  kernelToCheck = getKernelsAsVector(res.at("mgs").at("iak")); 
  expected = {tempDir / "mgs" / "kernels" / "iak" / "mocAddendum001.ti"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }


  kernelToCheck = getKernelsAsVector(res.at("mgs").at("ik")); 
  expected = {tempDir / "mgs" / "kernels" / "ik" / "moc20.ti", 
              tempDir / "mgs" / "kernels" / "ik" / "tes12.ti", 
              tempDir / "mgs" / "kernels" / "ik" / "mola25.ti"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }

  kernelToCheck = getKernelsAsVector(res.at("mgs").at("sclk")); 
  expected = {tempDir / "mgs" / "kernels" / "sclk" / "MGS_SCLKSCET.00032.tsc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }

  kernelToCheck = getKernelsAsVector(res.at("mgs").at("spk").at("reconstructed")); 
  expected = {tempDir / "mgs" / "kernels" / "spk" / "mgs_ext24.bsp"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
}

TEST_F(IsisDataDirectory, FunctionalTestOdysseyConf) {
  fs::path dbPath = getMissionConfigFile("odyssey");
  
  compareKernelSets("odyssey");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);
  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("odyssey");

  fs::path ckDir = tempDir / "odyssey" / "kernels" / "ck";
  fs::path spkDir = tempDir / "odyssey" / "kernels" / "spk";

  vector<string> expected = {ckDir / "m01_sc_ab0110.bc",
                             ckDir / "m01_sc_map3_rec_nadir.bc",
                             ckDir / "m01_sc_map10_rec_nadir.bc",
                             ckDir / "m01_sc_map1_v2.bc",
                             ckDir / "m01_sc_map10.bc",
                             ckDir / "m01_sc_ext7_rec_nadir.bc",
                             ckDir / "m01_sc_ext36_rec_nadir.bc",
                             ckDir / "m01_sc_ext22_rec_roto_v2.bc",
                             ckDir / "m01_sc_ext7.bc",
                             ckDir / "m01_sc_ext42.bc"};
  compareKernelVector(getKernelsAsVector(res.at("odyssey").at("ck").at("reconstructed")), expected);

  expected = {ckDir / "themis_nightir_merged_2018Mar02_ck.bc",
              ckDir / "themis_dayir_merged_2018Jul13_ck.bc"};
  compareKernelVector(getKernelsAsVector(res.at("odyssey").at("ck").at("smithed")), expected);

  expected = {spkDir / "m01_map.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("odyssey").at("spk").at("predicted")), expected);

  expected = {spkDir / "m01_ab_v2.bsp",
              spkDir / "m01_map1_v2.bsp",
              spkDir / "m01_map2.bsp",
              spkDir / "m01_ext8.bsp",
              spkDir / "m01_ext23.bsp",
              spkDir / "m01_map_rec.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("odyssey").at("spk").at("reconstructed")), expected);

  expected = {spkDir / "themis_nightir_merged_2018Mar02_spk.bsp",
              spkDir / "themis_dayir_merged_2018Jul13_spk.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("odyssey").at("spk").at("smithed")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestKploConf) {
  fs::path dbPath = getMissionConfigFile("kplo");
  
  compareKernelSets("kplo");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);
  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("kplo");

  fs::path kDir = tempDir / "kplo" / "kernels";

  vector<string> expected = {kDir / "ck" / "kplo_sc_20220805_20220806_v00.bc",
                             kDir / "ck" / "kplo_sc_20220805_20220809_v01.bc",
                             kDir / "ck" / "kplo_sc_20220805_20220811_v03.bc",
                             kDir / "ck" / "kplo_sc_20220805_20220901_v05.bc",
                             kDir / "ck" / "kplo_sc_20220806_20220807_v00.bc",
                             kDir / "ck" / "kplo_sc_20220807_20220808_v02.bc",
                             kDir / "ck" / "kplo_sc_20220808_20220809_v00.bc",
                             kDir / "ck" / "kplo_sc_20220809_20220810_v00.bc",
                             kDir / "ck" / "kplo_sc_20220809_20220816_v01.bc"};
  compareKernelVector(getKernelsAsVector(res.at("kplo").at("ck").at("reconstructed")), expected);

  expected = {kDir / "spk" / "kplo_d_20220805_20220806_v00.bsp",
              kDir / "spk" / "kplo_d_20220805_20220808_v00.bsp",
              kDir / "spk" / "kplo_d_20220805_20220901_v01.bsp",
              kDir / "spk" / "kplo_d_20220807_20220808_v00.bsp",
              kDir / "spk" / "kplo_d_20220808_20220809_v00.bsp",
              kDir / "spk" / "kplo_d_20220809_20220810_v00.bsp",
              kDir / "spk" / "kplo_d_20220809_20220813_v00.bsp",
              kDir / "spk" / "kplo_d_20220810_20220811_v00.bsp",
              kDir / "spk" / "kplo_d_20220812_20220815_v00.bsp",
              kDir / "spk" / "kplo_d_20220813_20220816_v00.bsp",
              kDir / "spk" / "kplo_d_20220814_20220817_v00.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("kplo").at("spk").at("reconstructed")), expected);

  expected = {kDir / "fk" / "kplo_v00.tf",
              kDir / "fk" / "kplo_v01.tf"};
  compareKernelVector(getKernelsAsVector(res.at("kplo").at("fk")), expected);

  expected = {kDir / "sclk" / "kplo_sclkscet_v000.tsc",
              kDir / "sclk" / "kplo_sclkscet_v004.tsc",
              kDir / "sclk" / "kplo_sclkscet_v005.tsc",
              kDir / "sclk" / "kplo_sclkscet_v006.tsc",
              kDir / "sclk" / "kplo_sclkscet_v007.tsc",
              kDir / "sclk" / "kplo_sclkscet_v008.tsc",
              kDir / "sclk" / "kplo_sclkscet_v010.tsc",
              kDir / "sclk" / "kplo_sclkscet_v011.tsc",
              kDir / "sclk" / "kplo_sclkscet_v012.tsc",
              kDir / "sclk" / "kplo_sclkscet_v013.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("kplo").at("sclk")), expected);

  expected = {kDir / "ik" / "kplo_shadowcam_v00.ti"};
  compareKernelVector(getKernelsAsVector(res.at("shadowcam").at("ik")), expected);

  expected = {kDir / "iak" / "kplo_instrumentAddendum_v01.ti",
              kDir / "iak" / "kplo_instrumentAddendum_v02.ti"};
  compareKernelVector(getKernelsAsVector(res.at("shadowcam").at("iak")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsKaguya) {
  fs::path dbPath = getMissionConfigFile("kaguya");
  
  compareKernelSets("kaguya");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("kaguya");
  
  fs::path kDir = tempDir / "kaguya" / "kernels";

  vector<string> expected = {kDir / "ck" / "SEL_M_ALL_D_V02.BC"};
  compareKernelVector(getKernelsAsVector(res.at("kaguya").at("ck").at("reconstructed")), expected);

  expected = {kDir / "spk" / "SEL_M_071020_090610_SGMH_02.BSP"};
  compareKernelVector(getKernelsAsVector(res.at("kaguya").at("spk").at("reconstructed")), expected);

  expected = {kDir / "spk" / "SEL_M_071020_081226_SGMI_05.BSP",
              kDir / "spk" / "SELMAINGRGM900CL660DIRALT2008103020090610.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("kaguya").at("spk").at("smithed")), expected);
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsTgo) {
  fs::path dbPath = getMissionConfigFile("tgo");
  
  compareKernelSets("tgo");
  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("tgo");
  
  fs::path kDir = tempDir / "tgo" / "kernels";

  vector<string> expected = {kDir / "ck" / "em16_tgo_sc_fmp_026_01_20200321_20200418_f20180215_v01.bc"};
  compareKernelVector(getKernelsAsVector(res.at("tgo").at("ck").at("predicted")), expected);

  expected = {kDir / "ck" / "em16_tgo_sc_ssm_20190210_20190303_s20190208_v01.bc"};
  compareKernelVector(getKernelsAsVector(res.at("tgo").at("ck").at("reconstructed")), expected);

  expected = {kDir / "spk" / "em16_tgo_fap_167_01_20160314_20180203_v01.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("tgo").at("spk").at("predicted")), expected);

  expected = {kDir / "ck" / "em16_tgo_cassis_ipp_tel_20160407_20170309_s20170116_v01.bc"};
  compareKernelVector(getKernelsAsVector(res.at("cassis").at("ck").at("predicted")), expected);

  expected = {kDir / "ck" / "cassis_ck_p_160312_181231_180609.bc",
              kDir / "ck" / "em16_tgo_cassis_tel_20160407_20201231_s20200803_v04.bc"};
  compareKernelVector(getKernelsAsVector(res.at("cassis").at("ck").at("reconstructed")), expected);
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsMex) {
  fs::path dbPath = getMissionConfigFile("mex");
  
  compareKernelSets("mex");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("mex");
  
  fs::path kDir = tempDir / "mex" / "kernels";

  vector<string> expected = {kDir / "ck" / "ATNM_P060401000000_00780.BC"};
  compareKernelVector(getKernelsAsVector(res.at("mex").at("ck").at("predicted")), expected);

  expected = {kDir / "ck" / "ATNM_MEASURED_030602_040101_V03.BC",
              kDir / "ck" / "ATNM_RECONSTITUTED_00004.BC"};
  compareKernelVector(getKernelsAsVector(res.at("mex").at("ck").at("reconstructed")), expected);

  expected = {kDir / "ck" / "ATNM_P060401000000_00780.BC"};
  compareKernelVector(getKernelsAsVector(res.at("mex").at("ck").at("predicted")), expected);

  expected = {kDir / "spk" / "ORMF_______________00720.BSP"};
  compareKernelVector(getKernelsAsVector(res.at("mex").at("spk").at("predicted")), expected);

  expected = {kDir / "spk" / "ORHM_______________00038.BSP"};
  compareKernelVector(getKernelsAsVector(res.at("mex").at("spk").at("reconstructed")), expected);

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsLo) {
  fs::path dbPath = getMissionConfigFile("lo");
  
  compareKernelSets("lo");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("lo");
  
  fs::path kDir = tempDir / "lo" / "kernels";

  vector<string> expected = {kDir / "spk" / "lo3_photo_support_ME.bsp",
                             kDir / "spk" / "lo4_photo_support_ME.bsp",
                             kDir / "spk" / "lo5_photo_support_ME.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("lo").at("spk").at("reconstructed")), expected);

  expected = {kDir / "ck" / "lo3_photo_support_ME.bc",
              kDir / "ck" / "lo4_photo_support_ME.bc",
              kDir / "ck" / "lo5_photo_support_ME.bc"};
  compareKernelVector(getKernelsAsVector(res.at("lo").at("ck").at("reconstructed")), expected);

  expected = {kDir / "ik" / "lo01.ti",
              kDir / "ik" / "lo02.ti"};
  compareKernelVector(getKernelsAsVector(res.at("lo").at("ik")), expected);

  expected = {kDir / "iak" / "lunarOrbiterAddendum001.ti",
              kDir / "iak" / "lunarOrbiterAddendum002.ti"};
  compareKernelVector(getKernelsAsVector(res.at("lo").at("iak")), expected);

  expected = {kDir / "sclk" / "lo_fict.tsc",
              kDir / "sclk" / "lo1_fict.tsc",
              kDir / "sclk" / "lo2_fict.tsc",
              kDir / "sclk" / "lo3_fict.tsc",
              kDir / "sclk" / "lo4_fict.tsc",
              kDir / "sclk" / "lo5_fict.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("lo").at("sclk")), expected);

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsSmart1) {

  fs::path dbPath = getMissionConfigFile("smart1");
  
  compareKernelSets("smart1");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("smart1");
  
  fs::path kDir = tempDir / "smart1" / "kernels";

  vector<string> expected = {kDir / "ck" / "ATNS_P050930150947_00220.BC",
                             kDir / "ck" / "ATNS_P030929010023_00188.BC",
                             kDir / "ck" / "ATNS_P060301004212_00233.BC"};
  compareKernelVector(getKernelsAsVector(res.at("smart1").at("ck").at("reconstructed")), expected);

  expected = {kDir / "sclk" / "SMART1_070227_STEP.TSC"};
  compareKernelVector(getKernelsAsVector(res.at("smart1").at("sclk")), expected);

  expected = {kDir / "ik" / "SMART1_AMIE_V01.TI"};
  compareKernelVector(getKernelsAsVector(res.at("smart1").at("ik")), expected);

  expected = {kDir / "spk" / "ORMS_______________00233.BSP"};
  compareKernelVector(getKernelsAsVector(res.at("smart1").at("spk").at("predicted")), expected);

  expected = {kDir / "spk" / "ORMS__041111020517_00206.BSP"};
  compareKernelVector(getKernelsAsVector(res.at("smart1").at("spk").at("reconstructed")), expected);

  expected = {kDir / "fk" / "SMART1_V1.TF"};
  compareKernelVector(getKernelsAsVector(res.at("smart1").at("fk")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsHayabusa2) {
  fs::path dbPath = getMissionConfigFile("hayabusa2");
  
  compareKernelSets("hayabusa2");
  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("hayabusa2");
  
  fs::path kDir = tempDir / "hayabusa2" / "kernels";

  vector<string> expected = { kDir / "ck" / "hyb2_hk_2015_v01.bc",
                              kDir / "ck" / "hyb2_hk_2016_v02.bc",
                              kDir / "ck" / "hyb2_hkattrpt_2017_v02.bc",
                              kDir / "ck" / "hyb2_aocsc_2015_v01.bc",
                              kDir / "ck" / "hyb2_aocsc_2017_v02.bc",
                              kDir / "ck" / "hyb2_hkattrpt_2016_v02.bc",
                              kDir / "ck" / "hyb2_hk_2014_v01.bc",
                              kDir / "ck" / "hyb2_hk_2017_v02.bc",
                              kDir / "ck" / "hyb2_hk_2015_v02.bc",
                              kDir / "ck" / "hyb2_hkattrpt_2015_v02.bc",
                              kDir / "ck" / "hyb2_aocsc_2016_v02.bc",
                              kDir / "ck" / "hyb2_aocsc_2018_v02.bc",
                              kDir / "ck" / "hyb2_hkattrpt_2018_v02.bc",
                              kDir / "ck" / "hyb2_aocsc_2015_v02.bc",
                              kDir / "ck" / "hyb2_hk_2014_v02.bc",
                              kDir / "ck" / "hyb2_aocsc_2014_v02.bc",
                              kDir / "ck" / "hyb2_aocsc_2014_v01.bc"
                            };
  compareKernelVector(getKernelsAsVector(res.at("hayabusa2").at("ck").at("reconstructed")), expected);

  expected = {kDir / "fk" / "hyb2_v06.tf",
              kDir / "fk" / "hyb2_v14.tf",
              kDir / "fk" / "hyb2_v10.tf",
              kDir / "fk" / "hyb2_ryugu_v01.tf",
              kDir / "fk" / "hyb2_v09.tf",
              kDir / "fk" / "hyb2_hp_v01.tf"
             };
  compareKernelVector(getKernelsAsVector(res.at("hayabusa2").at("fk")), expected);

  expected = {kDir / "tspk" / "2162173_ryugu_20180601-20191230_0060_20181221.bsp",
              kDir / "tspk" / "sat375.bsp",
              kDir / "tspk" / "jup329.bsp",
              kDir / "tspk" / "de430.bsp",
              kDir / "tspk" / "2162173_Ryugu.bsp"
             };

  compareKernelVector(getKernelsAsVector(res.at("hayabusa2").at("tspk")), expected);

  expected = {kDir / "sclk" / "hyb2_20141203-20161231_v01.tsc",
              kDir / "sclk" / "hyb2_20141203-20171231_v01.tsc",
              kDir / "sclk" / "hyb2_20141203-20191231_v01.tsc"
             };
  compareKernelVector(getKernelsAsVector(res.at("hayabusa2").at("sclk")), expected);

//@TODO lidar derived?
  expected = { kDir / "spk" / "lidar_derived_trj_20191114_20180630053224_20190213030000_v02.bsp",
               kDir / "spk" / "hyb2_20151123-20151213_0001m_final_ver1.oem.bsp",
               kDir / "spk" / "hyb2_20141203-20161119_0001h_final_ver1.oem.bsp",
               kDir / "spk" / "hyb2_20141203-20151231_0001h_final_ver1.oem.bsp",
               kDir / "spk" / "hyb2_20141203-20141214_0001m_final_ver1.oem.bsp",
               kDir / "spk" / "hyb2_hpk_20180627_20190213_v01.bsp",
               kDir / "spk" / "hyb2_approach_od_v20180811114238.bsp"
             };
  compareKernelVector(getKernelsAsVector(res.at("onc").at("spk").at("reconstructed")), expected);

  expected = {kDir / "ik" / "hyb2_onc_v00.ti",
              kDir / "ik" / "hyb2_onc_v05.ti"
             };
  compareKernelVector(getKernelsAsVector(res.at("onc").at("ik")), expected);

  expected = {kDir / "iak" / "hyb2oncAddendum0001.ti"};
  compareKernelVector(getKernelsAsVector(res.at("onc").at("iak")), expected);

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsVoyager1) {

  fs::path dbPath = getMissionConfigFile("voyager1");
  
  compareKernelSets("voyager1");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("voyager1");
  
  fs::path kDir = tempDir / "voyager1" / "kernels";

  vector<string> expected = {kDir / "ck" / "vg1_jup_qmw_wa_fc-31100_t2.bc",
                             kDir / "ck" / "vg1_jup_qmw_na_fc-31100_t2.bc",
                             kDir / "ck" / "vg1_sat_qmw_wa_fc-31100_t2.bc",
                             kDir / "ck" / "vg1_sat_qmw_na_fc-31100_t2.bc"};
  compareKernelVector(getKernelsAsVector(res.at("voyager1").at("ck").at("reconstructed")), expected);

  expected = {kDir / "ck" / "vg1_eur_usgs2020.bc"};
  compareKernelVector(getKernelsAsVector(res.at("voyager1").at("ck").at("smithed")), expected);

  expected = {kDir / "sclk" / "vg100010.tsc",
              kDir / "sclk" / "vg100008.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("voyager1").at("sclk")), expected);

  expected = {kDir / "ik" / "vg1_issna_v02.ti",
              kDir / "ik" / "vg1_isswa_v01.ti"};
  compareKernelVector(getKernelsAsVector(res.at("voyager1").at("ik")), expected);

  expected = {kDir / "spk" / "vg1_sat.bsp",
              kDir / "spk" / "vgr1_jup230.bsp",
              kDir / "spk" / "vg1_sat.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("voyager1").at("spk").at("reconstructed")), expected);

  expected = {kDir / "fk" / "vg1_v02.tf"};
  compareKernelVector(getKernelsAsVector(res.at("voyager1").at("fk")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsVoyager2) {

  fs::path dbPath = getMissionConfigFile("voyager2");
  
  compareKernelSets("voyager2");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("voyager2");
  
  fs::path kDir = tempDir / "voyager2" / "kernels";

  vector<string> expected = { kDir / "ck" / "vg2_jup_qmw_wa_fc-32100_t2.bc",
                              kDir / "ck" / "vg2_nep_version1_type1_iss_sedr.bc",
                              kDir / "ck" / "vg2_ura_version1_type2_iss_sedr.bc",
                              kDir / "ck" / "vg2_sat_qmw_na_fc-32100_t2.bc",
                              kDir / "ck" / "vg2_jup_version1_type1_iss_sedr.bc",
                              kDir / "ck" / "vg2_ura_version1_type1_iss_sedr.bc",
                              kDir / "ck" / "vg2_jup_version1_type2_iss_sedr.bc",
                              kDir / "ck" / "vg2_sat_version1_type1_iss_sedr.bc",
                              kDir / "ck" / "vg2_jup_qmw_na_fc-32100_t2.bc",
                              kDir / "ck" / "vg2_sat_version1_type2_iss_sedr.bc",
                              kDir / "ck" / "vg2_sat_qmw_wa_fc-32100_t2.bc",
                              kDir / "ck" / "vg2_nep_version1_type2_iss_sedr.bc"};
  compareKernelVector(getKernelsAsVector(res.at("voyager2").at("ck").at("reconstructed")), expected);

  expected = {kDir / "ck" / "vg2_eur_usgs2020.bc"};
  compareKernelVector(getKernelsAsVector(res.at("voyager2").at("ck").at("smithed")), expected);

  expected = {kDir / "sclk" / "vg200010.tsc",
              kDir / "sclk" / "vg200011.tsc",
              kDir / "sclk" / "vg200008.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("voyager2").at("sclk")), expected);

  expected = {kDir / "ik" / "vg2_issna_v02.ti",
              kDir / "ik" / "vg2_isswa_v01.ti"};
  compareKernelVector(getKernelsAsVector(res.at("voyager2").at("ik")), expected);

  expected = {kDir / "spk" / "vg2_sat.bsp",
              kDir / "spk" / "vgr2_jup230.bsp",
              kDir / "spk" / "vg2_sat.bsp",
              kDir / "spk" / "vg2_nep.bsp",
              kDir / "spk" / "vg2_ura.bsp",
              kDir / "spk" / "vgr2_nep081.bsp",
              kDir / "spk" / "vgr2_sat336.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("voyager2").at("spk").at("reconstructed")), expected);

  expected = {kDir / "fk" / "vg2_v02.tf"};
  compareKernelVector(getKernelsAsVector(res.at("voyager2").at("fk")), expected);

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsMsl) {
  fs::path dbPath = getMissionConfigFile("msl");
  
  compareKernelSets("msl");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("msl");
  
  fs::path kDir = tempDir / "msl" / "kernels";

  vector<string> expected = {kDir / "ck" / "msl_ra_toolsref_v1.bc",
                             kDir / "ck" / "msl_cruise_recon_rawrt_v2.bc",
                             kDir / "ck" / "msl_cruise_recon_raweng_v1.bc",
                             kDir / "ck" / "msl_edl_v01.bc",
                             kDir / "ck" / "msl_surf_hga_tlm.bc",
                             kDir / "ck" / "msl_surf_ra_tlmenc.bc",
                             kDir / "ck" / "msl_surf_ra_tlmres.bc",
                             kDir / "ck" / "msl_surf_rsm_tlmenc.bc",
                             kDir / "ck" / "msl_surf_rsm_tlmres.bc",
                             kDir / "ck" / "msl_surf_rover_tlm.bc"};

  compareKernelVector(getKernelsAsVector(res.at("msl").at("ck").at("reconstructed")), expected);


  expected = {kDir / "fk" / "msl.tf"};
  compareKernelVector(getKernelsAsVector(res.at("msl").at("fk")), expected);

  expected = {kDir / "ik" / "msl_aux_v00.ti",
              kDir / "ik" / "msl_chrmi_20120731_c03.ti",
              kDir / "ik" / "msl_hbla_20120731_c03.ti",
              kDir / "ik" / "msl_hblb_20120731_c03.ti",
              kDir / "ik" / "msl_hbra_20120731_c03.ti",
              kDir / "ik" / "msl_hbrb_20120731_c03.ti",
              kDir / "ik" / "msl_hfla_20120731_c03.ti",
              kDir / "ik" / "msl_hflb_20120731_c03.ti",
              kDir / "ik" / "msl_hfra_20120731_c03.ti",
              kDir / "ik" / "msl_hfrb_20120731_c03.ti",
              kDir / "ik" / "msl_mahli_20120731_c02.ti",
              kDir / "ik" / "msl_mardi_20120731_c02.ti",
              kDir / "ik" / "msl_ml_20120731_c03.ti",
              kDir / "ik" / "msl_mr_20120731_c03.ti",
              kDir / "ik" / "msl_nla_20120731_c04.ti",
              kDir / "ik" / "msl_nlb_20120731_c04.ti",
              kDir / "ik" / "msl_nlb_20130530_c05.ti",
              kDir / "ik" / "msl_nra_20120731_c04.ti",
              kDir / "ik" / "msl_nrb_20120731_c04.ti",
              kDir / "ik" / "msl_nrb_20130530_c05.ti",
              kDir / "ik" / "msl_struct_v01.ti"};

  compareKernelVector(getKernelsAsVector(res.at("msl").at("ik")), expected);

  expected = {kDir / "lsk" / "msl.tls"};
  compareKernelVector(getKernelsAsVector(res.at("msl").at("lsk")), expected);

  expected = {kDir / "sclk" / "msl.tsc",
              kDir / "sclk" / "msl_lmst_ops120808_v1.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("msl").at("sclk")), expected);

  expected = {kDir / "spk" / "msl_struct_v02.bsp",
              kDir / "spk" / "msl_cruise_v1.bsp",
              kDir / "spk" / "msl_edl_v01.bsp",
              kDir / "spk" / "msl_ls_ops120808_iau2000_v1.bsp",
              kDir / "spk" / "msl_surf_rover_loc.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("msl").at("spk")), expected);
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsMer1) {

  fs::path dbPath = getMissionConfigFile("mer1");
  
  compareKernelSets("mer1");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("mer1");

  fs::path kDir = tempDir / "mer1" / "kernels";

  vector<string> expected = {kDir / "ck" / "mer1_surf_hga_ext10_v1.bc",
                             kDir / "ck" / "mer1_cruise.bc",
                             kDir / "ck" / "mer1_hga_stowed.bc",
                             kDir / "ck" / "mer1_surf_rover_prim_v1.bc"};
  compareKernelVector(getKernelsAsVector(res.at("mer1").at("ck").at("reconstructed")), expected);

  expected = {kDir / "spk" / "mar033_2000-2025.bsp",
              kDir / "spk" / "mer1_cruise.bsp",
              kDir / "spk" / "de410_de910.bsp",
              kDir / "spk" / "mer1_surf_iddg_ext11_v1.bsp",
              kDir / "spk" / "mer1_struct_ver11.bsp",
              kDir / "spk" / "mer1_edl_rcb_v1.bsp",
              kDir / "spk" / "mer1_surf_iddg.bsp",
              kDir / "spk" / "mer1_surf_roverrl_0001_3240_v1.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("mer1").at("spk").at("reconstructed")), expected);

  expected = {kDir / "fk" / "mer1_tp_tm20b3_iau2000_v1.tf",
              kDir / "fk" / "mer1.tf"};
  compareKernelVector(getKernelsAsVector(res.at("mer1").at("fk")), expected);

  expected = {kDir / "ik" / "mer1_nl_20040125_c196.ti"};
  compareKernelVector(getKernelsAsVector(res.at("mer1").at("ik")), expected);

  expected = {kDir / "sclk" / "MER_253_SCLKSCET.00001.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("mer1").at("sclk")), expected);

  expected = {kDir / "pck" / "mars_iau2000_v0.tpc"};
  compareKernelVector(getKernelsAsVector(res.at("mer1").at("pck")), expected);

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsMer2) {

  fs::path dbPath = getMissionConfigFile("mer2");
  
  compareKernelSets("mer2");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);


  

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("mer2");
  
  fs::path kDir = tempDir / "mer2" / "kernels";

  vector<string> expected = {kDir / "spk" / "spk_b_s_071029-160101_110624.bsp",
                             kDir / "spk" / "mer2_surf_roverrl_ro_v01.bsp",
                             kDir / "spk" / "mer2_ls_040108_iau2000_v1.bsp",
                             kDir / "spk" / "de410_de910.bsp",
                             kDir / "spk" / "mer2_still_at_ls_v1.bsp",
                             kDir / "spk" / "mer2_struct_ver11.bsp",
                             kDir / "spk" / "mar033_2000-2025.bsp",
                             kDir / "spk" / "mer2_edl_rcb_v1.bsp",
                             kDir / "spk" / "mer2_surf_rover_all_v01.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("mer2").at("spk").at("reconstructed")), expected);

  expected = {kDir / "fk" / "mer2.tf",
              kDir / "fk" / "mer2_tp_ep78a3p_iau2000_v1.tf"};
  compareKernelVector(getKernelsAsVector(res.at("mer2").at("fk")), expected);

  expected = {kDir / "ik" / "mer2_rl_20031204_c60.ti"};
  compareKernelVector(getKernelsAsVector(res.at("mer2").at("ik")), expected);

  expected = {kDir / "sclk" / "MER_254_SCLKSCET.00032.tsc",
              kDir / "sclk" / "mer2.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("mer2").at("sclk")), expected);

  expected = {kDir / "pck" / "mars_iau2000_v0.tpc"};
  compareKernelVector(getKernelsAsVector(res.at("mer2").at("pck")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsClipper) {

  fs::path dbPath = getMissionConfigFile("clipper");
  
  compareKernelSets("clipper");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  nlohmann::json res = listMissionKernels(tempDir, conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("clipper");
  
  fs::path kDir = tempDir / "clipper" / "kernels";

  vector<string> expected = {kDir / "ck" / "clipper_sc_rq_260510_260516_26130_v01.bc",
                             kDir / "ck" / "clipper_sc_rq_241215_241221_24day350_24day356_v01.bc"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("ck").at("reconstructed")), expected);

  expected = {kDir / "ck" / "clipper_sc_pb_250918_251030_v01.bc",
              kDir / "ck" / "clipper_sc_pb_250807_250918_v03.bc",
              kDir / "ck" / "clipper_sc_pb_250807_250918_v02.bc"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("ck").at("predicted")), expected);

  expected = {kDir / "spk" / "trj_250515-250626-dco2507090443-cruise006-reconstruct-OD067-v1.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("spk").at("reconstructed")), expected);

  expected = {kDir / "spk" / "trj_250601-251129-dco2507310113-cruise009-predict-OD070-v1.bsp"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("spk").at("predicted")), expected);

  expected = {kDir / "fk" / "clipper_v16.tf",
              kDir / "fk" / "clipper_v17.tf"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("fk")), expected);

  expected = {kDir / "ik" / "clipper_eis_v06.ti",
              kDir / "ik" / "clipper_eis_v07.ti"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("ik")), expected);

  expected = {kDir / "iak" / "clipperAddendum001.ti"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("iak")), expected);

  expected = {kDir / "sclk" / "europaclipper_00290.tsc",
              kDir / "sclk" / "europaclipper_00291.tsc"};
  compareKernelVector(getKernelsAsVector(res.at("clipper").at("sclk")), expected);
}

