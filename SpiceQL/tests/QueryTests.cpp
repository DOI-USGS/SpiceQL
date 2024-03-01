#include <fstream>
#include <algorithm>

#include <HippoMocks/hippomocks.h>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "Fixtures.h"

#include "query.h"
#include "utils.h"


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

TEST(QueryTests, getKernelStringValue){
  unique_ptr<Kernel> k(new Kernel("data/msgr_mdis_v010.ti"));
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
  unique_ptr<Kernel> k(new Kernel("data/msgr_mdis_v010.ti"));

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

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  SPDLOG_INFO("res: {}", res.dump());
  vector<string> s = SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["reconstructed"]["kernels"]);
  SPDLOG_INFO("CK Reconstructed: {}", fmt::join(s, ", "));

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ck"]["smithed"]["kernels"]).size(), 4);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["tspk"]["kernels"]).size(), 1);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["fk"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["ik"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["iak"]["kernels"]).size(), 2);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis"]["pck"]["na"]["kernels"]).size(), 2);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["mdis_att"]["ck"]["reconstructed"]["kernels"]).size(), 4);

  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["messenger"]["ck"]["reconstructed"]["kernels"]).size(), 5);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(res["messenger"]["sclk"]["kernels"]).size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsClem1) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

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

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["ck"]["reconstructed"]["kernels"]).size(), 4);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["ck"]["smithed"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["spk"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["iak"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["pck"]["smithed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["pck"]["na"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["galileo"]["sclk"]["kernels"]).size(), 1);
}



TEST_F(KernelDataDirectories, FunctionalTestListMissionKernelsCassini) {
  fs::path dbPath = getMissionConfigFile("cassini");
  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = listMissionKernels("/isis_data/", conf);

  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["ck"]["reconstructed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["ck"]["smithed"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["fk"]["kernels"]).size(), 2);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["iak"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["pck"]["kernels"]).size(), 3);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["pck"]["smithed"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["sclk"]["kernels"]).size(), 1);
  ASSERT_EQ(SpiceQL::getKernelsAsVector(res["cassini"]["spk"]["kernels"]).size(), 3);
}



// test for apollo 17 kernels 
TEST_F(IsisDataDirectory, FunctionalTestApollo17Conf) {
  fs::path dbPath = getMissionConfigFile("apollo17");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> expectedKernels = missionMap.at("apollo17");
  set<string> diff; 
  
  set_difference(expectedKernels.begin(), expectedKernels.end(), kernels.begin(), kernels.end(), inserter(diff, diff.begin()));
  
  if (diff.size() != 0) {
    FAIL() << "Kernel sets are not equal, diff: " << fmt::format("{}", fmt::join(diff, " ")) << endl;
  }
}


TEST_F(IsisDataDirectory, FunctionalTestLroConf) {
  compareKernelSets("lro");

  nlohmann::json conf = getMissionConfig("lro");
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  
  nlohmann::json res = listMissionKernels("doesn't matter", conf);
  
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  SpiceQL::getKernelsAsVector(res.at("moc").at("ck").at("reconstructed").at("kernels"));
  vector<string> expected = {"moc42r_2016305_2016336_v01.bc"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
  
  kernelToCheck = getKernelsAsVector(res.at("moc").at("spk").at("reconstructed")); 
  expected = {"fdf29r_2018305_2018335_v01.bsp", "fdf29_2021327_2021328_b01.bsp"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }


  kernelToCheck = getKernelsAsVector(res.at("moc").at("spk").at("smithed")); 
  expected = {"LRO_ES_05_201308_GRGM660PRIMAT270.bsp", "LRO_ES_16_201406_GRGM900C_L600.BSP"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
}


TEST_F(IsisDataDirectory, FunctionalTestJunoConf) {
  set<string> expectedDiff = {"jup260.bsp",
                              "jup310.bsp",
                              "jup329.bsp",
                              "vgr1_jup230.bsp",
                              "vgr2_jup204.bsp",
                              "vgr2_jup230.bsp"};
  compareKernelSets("juno", expectedDiff);
} 


TEST_F(IsisDataDirectory, FunctionalTestMroConf) {
  compareKernelSets("mro");

  nlohmann::json conf = getMissionConfig("mro");
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  
  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  // check a kernel from each regex exists in there quality groups
  vector<string> kernelToCheck =  getKernelsAsVector(res.at("mro").at("spk").at("reconstructed").at("kernels"));
  vector<string> expected = {"mro_cruise.bsp", "mro_ab.bsp", "mro_psp_rec.bsp", 
                             "mro_psp1.bsp", "mro_psp10.bsp", "mro_psp_rec.bsp", 
                             "mro_psp1_ssd_mro95a.bsp", "mro_psp27_ssd_mro110c.bsp"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }

  kernelToCheck = getKernelsAsVector(res.at("mro").at("spk").at("predicted")); 
  expected = {"mro_psp.bsp"};
  EXPECT_EQ(kernelToCheck, expected);

  kernelToCheck = getKernelsAsVector(res.at("mro").at("ck").at("reconstructed"));
  expected = {"mro_sc_psp_160719_160725.bc", "mro_sc_cru_060301_060310.bc", 
              "mro_sc_ab_060801_060831.bc", "mro_sc_psp_150324_150330_v2.bc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
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

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("mgs");
 
  // check a kernel from each regex exists in their quality groups
  vector<string> kernelToCheck =  getKernelsAsVector(res.at("mgs").at("ck").at("reconstructed"));
  vector<string> expected = {"mgs_sc_ab1.bc"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
  
  kernelToCheck = getKernelsAsVector(res.at("mgs").at("iak")); 
  expected = {"mocAddendum001.ti"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }


  kernelToCheck = getKernelsAsVector(res.at("mgs").at("ik")); 
  expected = {"moc20.ti", "tes12.ti", "mola25.ti"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }

  kernelToCheck = getKernelsAsVector(res.at("mgs").at("sclk")); 
  expected = {"MGS_SCLKSCET.00032.tsc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    if (it == kernelToCheck.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }

  kernelToCheck = getKernelsAsVector(res.at("mgs").at("spk").at("reconstructed")); 
  expected = {"mgs_ext24.bsp"};
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

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("odyssey");

  vector<string> expected = {"m01_sc_ab0110.bc", 
                             "m01_sc_map3_rec_nadir.bc", 
                             "m01_sc_map10_rec_nadir.bc", 
                             "m01_sc_map1_v2.bc",
                             "m01_sc_map10.bc",
                             "m01_sc_ext7_rec_nadir.bc",
                             "m01_sc_ext36_rec_nadir.bc",
                             "m01_sc_ext22_rec_roto_v2.bc",
                             "m01_sc_ext7.bc",
                             "m01_sc_ext42.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("ck").at("reconstructed")), expected); 

  expected = {"themis_nightir_merged_2018Mar02_ck.bc", 
              "themis_dayir_merged_2018Jul13_ck.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("ck").at("smithed")), expected); 
  
  expected = {"m01_map.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("spk").at("predicted")), expected);  
  
  expected = {"m01_ab_v2.bsp", 
              "m01_map1_v2.bsp",
              "m01_map2.bsp",
              "m01_ext8.bsp",
              "m01_ext23.bsp",
              "m01_map_rec.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("spk").at("reconstructed")), expected); 

  expected = {"themis_nightir_merged_2018Mar02_spk.bsp", 
              "themis_dayir_merged_2018Jul13_spk.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("odyssey").at("spk").at("smithed")), expected);  
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsKaguya) {
  fs::path dbPath = getMissionConfigFile("kaguya");
  
  compareKernelSets("kaguya");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("kaguya");
  
  vector<string> expected = {"SEL_M_ALL_D_V02.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("kaguya").at("ck").at("reconstructed")), expected); 

  expected = {"SEL_M_071020_081226_SGMI_05.BSP",
              "SELMAINGRGM900CL660DIRALT2008103020090610.bsp",
              "SEL_M_071020_090610_SGMH_02.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("kaguya").at("spk").at("smithed")), expected); 
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsTgo) {
  fs::path dbPath = getMissionConfigFile("tgo");
  
  compareKernelSets("tgo");
  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("tgo");
  
  vector<string> expected = {"em16_tgo_sc_fmp_026_01_20200321_20200418_f20180215_v01.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("tgo").at("ck").at("predicted")), expected); 

  expected = {"em16_tgo_sc_ssm_20190210_20190303_s20190208_v01.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("tgo").at("ck").at("reconstructed")), expected); 

  expected = {"em16_tgo_fap_167_01_20160314_20180203_v01.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("tgo").at("spk").at("predicted")), expected); 

  expected = {"em16_tgo_cassis_ipp_tel_20160407_20170309_s20170116_v01.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("cassis").at("ck").at("predicted")), expected); 

  expected = {"cassis_ck_p_160312_181231_180609.bc",
              "em16_tgo_cassis_tel_20160407_20201231_s20200803_v04.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("cassis").at("ck").at("reconstructed")), expected); 
}


TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsMex) {
  fs::path dbPath = getMissionConfigFile("mex");
  
  compareKernelSets("mex");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("mex");
  
  vector<string> expected = {"ATNM_P060401000000_00780.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("ck").at("predicted")), expected); 

  expected = {"ATNM_MEASURED_030602_040101_V03.BC",
              "ATNM_RECONSTITUTED_00004.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("ck").at("reconstructed")), expected);

  expected = {"ATNM_P060401000000_00780.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("ck").at("predicted")), expected); 

  expected = {"ORMF_______________00720.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("spk").at("predicted")), expected);

  expected = {"ORHM_______________00038.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("mex").at("spk").at("reconstructed")), expected);  

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsLo) {
  fs::path dbPath = getMissionConfigFile("lo");
  
  compareKernelSets("lo");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("lo");
  
  vector<string> expected = {"lo3_photo_support_ME.bsp", "lo4_photo_support_ME.bsp", "lo5_photo_support_ME.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("lo").at("spk").at("reconstructed")), expected);

  expected = {"lo3_photo_support_ME.bc", "lo4_photo_support_ME.bc", "lo5_photo_support_ME.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("lo").at("ck").at("reconstructed")), expected); 

  expected = {"lo01.ti", "lo02.ti"};
  CompareKernelSets(getKernelsAsVector(res.at("lo").at("ik")), expected);

  expected = {"lunarOrbiterAddendum001.ti", "lunarOrbiterAddendum002.ti"};
  CompareKernelSets(getKernelsAsVector(res.at("lo").at("iak")), expected);

  expected = {"lo_fict.tsc", "lo_fict1.tsc","lo_fict2.tsc","lo_fict3.tsc","lo_fict4.tsc","lo_fict5.tsc"};
  CompareKernelSets(getKernelsAsVector(res.at("lo").at("sclk")), expected);

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsSmart1) {

  fs::path dbPath = getMissionConfigFile("smart1");
  
  compareKernelSets("smart1");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);set<string> mission = missionMap.at("smart1");
  
  vector<string> expected = {"ATNS_P050930150947_00220.BC", "ATNS_P030929010023_00188.BC", "ATNS_P060301004212_00233.BC"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("ck").at("reconstructed")), expected); 

  expected = {"SMART1_070227_STEP.TSC"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("sclk")), expected);

  expected = {""};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("sclk")), expected);

  expected = {"SMART1_AMIE_V01.TI"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("ik")), expected); 

  expected = {"ORMS_______________00233.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("spk").at("predicted")), expected);

  expected = {"ORMS__041111020517_00206.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("spk").at("reconstructed")), expected);

  expected = {"ORHM_______________00038.BSP"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("spk").at("reconstructed")), expected);  

  expected = {"SMART1_V1.TF"};
  CompareKernelSets(getKernelsAsVector(res.at("smart1").at("fk")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsHayabusa2) {
  fs::path dbPath = getMissionConfigFile("hayabusa2");
  
  compareKernelSets("hayabusa2");
  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("hayabusa2");
  
  vector<string> expected = { "hyb2_hk_2015_v01.bc",
                              "hyb2_hk_2016_v02.bc",
                              "hyb2_hkattrpt_2017_v02.bc",
                              "hyb2_aocsc_2015_v01.bc",
                              "hyb2_aocsc_2017_v02.bc",
                              "hyb2_hkattrpt_2016_v02.bc",
                              "hyb2_hk_2014_v01.bc",
                              "hyb2_hk_2017_v02.bc",
                              "hyb2_hk_2015_v02.bc",
                              "hyb2_hkattrpt_2015_v02.bc",
                              "hyb2_aocsc_2016_v02.bc",
                              "hyb2_aocsc_2018_v02.bc",
                              "hyb2_hkattrpt_2018_v02.bc",
                              "hyb2_aocsc_2015_v02.bc",
                              "hyb2_hk_2014_v02.bc",
                              "hyb2_aocsc_2014_v02.bc",
                              "hyb2_aocsc_2014_v01.bc"
                            };
  CompareKernelSets(getKernelsAsVector(res.at("hayabusa2").at("ck").at("reconstructed")), expected); 

  expected = {"hyb2_v06.tf",
              "hyb2_v14.tf",
              "hyb2_v10.tf",
              "hyb2_ryugu_v01.tf",
              "hyb2_v09.tf",
              "hyb2_hp_v01.tf"
             };
  CompareKernelSets(getKernelsAsVector(res.at("hayabusa2").at("fk")), expected); 

  expected = {"2162173_ryugu_20180601-20191230_0060_20181221.bsp",
              "sat375.bsp",
              "jup329.bsp",
              "de430.bsp",
              "2162173_Ryugu.bsp"
             };

  CompareKernelSets(getKernelsAsVector(res.at("hayabusa2").at("tspk")), expected); 

  expected = {"hyb2_20141203-20161231_v01.tsc",
              "hyb2_20141203-20171231_v01.tsc",
              "hyb2_20141203-20191231_v01.tsc"
             };
  CompareKernelSets(getKernelsAsVector(res.at("hayabusa2").at("sclk")), expected); 

//@TODO lidar derived?
  expected = { "lidar_derived_trj_20191114_20180630053224_20190213030000_v02.bsp",
               "hyb2_20151123-20151213_0001m_final_ver1.oem.bsp",
               "hyb2_20141203-20161119_0001h_final_ver1.oem.bsp",
               "hyb2_20141203-20151231_0001h_final_ver1.oem.bsp",
               "hyb2_20141203-20141214_0001m_final_ver1.oem.bsp",
               "hyb2_hpk_20180627_20190213_v01.bsp",
               "hyb2_approach_od_v20180811114238.bsp"
             };
  CompareKernelSets(getKernelsAsVector(res.at("onc").at("spk").at("reconstructed")), expected); 

  expected = {"hyb2_onc_v00.ti",
              "hyb2_onc_v05.ti"
             };
  CompareKernelSets(getKernelsAsVector(res.at("onc").at("ik")), expected); 

  expected = {"hyb2oncAddendum0001.ti"};
  CompareKernelSets(getKernelsAsVector(res.at("onc").at("iak")), expected); 

}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsVoyager1) {

  fs::path dbPath = getMissionConfigFile("voyager1");
  
  compareKernelSets("voyager1");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);

  set<string> mission = missionMap.at("voyager1");
  
  vector<string> expected = {"vg1_jup_qmw_wa_fc-31100_t2.bc", "vg1_jup_qmw_na_fc-31100_t2.bc", "vg1_sat_qmw_wa_fc-31100_t2.bc", "vg1_sat_qmw_na_fc-31100_t2.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager1").at("ck").at("reconstructed")), expected); 

  expected = {"vg1_eur_usgs2020.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager1").at("ck").at("smithed")), expected); 

  expected = {"vg100010.tsc", "vg100008.tsc"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager1").at("sclk")), expected);

  expected = {"vg1_issna_v02.ti", "vg1_isswa_v01.ti"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager1").at("ik")), expected); 

  expected = {"vg1_sat.bsp", "vgr1_jup230.bsp", "vg1_sat.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager1").at("spk").at("reconstructed")), expected);  

  expected = {"vg1_v02.tf"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager1").at("fk")), expected);
}

TEST_F(IsisDataDirectory, FunctionalTestListMissionKernelsVoyager2) {

  fs::path dbPath = getMissionConfigFile("voyager2");
  
  compareKernelSets("voyager2");

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> mission = missionMap.at("voyager2");
  
  vector<string> expected = { "vg2_jup_qmw_wa_fc-32100_t2.bc",
                              "vg2_nep_version1_type1_iss_sedr.bc",
                              "vg2_ura_version1_type2_iss_sedr.bc",
                              "vg2_sat_qmw_na_fc-32100_t2.bc",
                              "vg2_jup_version1_type1_iss_sedr.bc",
                              "vg2_ura_version1_type1_iss_sedr.bc",
                              "vg2_jup_version1_type2_iss_sedr.bc",
                              "vg2_sat_version1_type1_iss_sedr.bc",
                              "vg2_jup_qmw_na_fc-32100_t2.bc",
                              "vg2_sat_version1_type2_iss_sedr.bc",
                              "vg2_sat_qmw_wa_fc-32100_t2.bc",
                              "vg2_nep_version1_type2_iss_sedr.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager2").at("ck").at("reconstructed")), expected); 

  expected = {"vg2_eur_usgs2020.bc"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager2").at("ck").at("smithed")), expected); 

  expected = {"vg200010.tsc", "vg200011.tsc" "vg200008.tsc"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager2").at("sclk")), expected);

  expected = {"vg1_issna_v02.ti", "vg1_isswa_v01.ti"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager2").at("ik")), expected); 

  expected = {"vg2_sat.bsp", "vgr2_jup230.bsp", "vg2_sat.bsp", "vg2_nep.bsp", "vg2_ura.bsp" "vgr2_nep081.bsp", "vgr2_sat336.bsp"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager2").at("spk").at("reconstructed")), expected);  

  expected = {"vg2_v02.tf"};
  CompareKernelSets(getKernelsAsVector(res.at("voyager2").at("fk")), expected);

}
