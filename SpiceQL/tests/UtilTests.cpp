#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>
#include <chrono>

using namespace std::chrono;

#include "TestUtilities.h"

#include "utils.h"
#include "Fixtures.h"
#include "spice_types.h"
#include "config.h"
#include <SpiceUsr.h>
#include "memo.h"
#include "query.h"

#include <spdlog/spdlog.h>

using namespace SpiceQL;


TEST(UtilTests, findKeywords) {
  Kernel k("data/msgr_mdis_v010.ti");

  nlohmann::json res = findKeywords("*");
  EXPECT_EQ(res.at("INS-236810_FOV_SHAPE"), "RECTANGLE");
  EXPECT_EQ(res.at("INS-236800_WAVELENGTH_RANGE")[1], 1040);
  EXPECT_EQ(res.at("INS-236800_IFOV"), 179.6);
}


TEST(UtilTests, findKeyInJson) {
  nlohmann::ordered_json j = R"(
    {
      "me" : "test",
      "l1a" : {
        "l2a" : 1,
        "me" : 2, 
        "l2b" : {
          "l3a" : "yay", 
          "me" : 1
        }
      }
    })"_json;

  std::vector<nlohmann::json::json_pointer> res = findKeyInJson(j, "me", true);

  EXPECT_EQ(res.at(0).to_string(), "/l1a/l2b/me");
  EXPECT_EQ(res.at(1).to_string(), "/l1a/me");
  EXPECT_EQ(res.at(2).to_string(), "/me");
}


TEST(UtilTests, resolveConfigDependencies) {
  nlohmann::json baseConfig = R"(
  {
    "mission_1" : {
      "mission_key" : {
        "array_key" : ["array_1", "array_2"]
      },
      "deps" : ["/mission_2/bad_key"]
    },
    "mission_2" : {
      "nested_key" : {
        "value_key" : "value_1"
      },
      "bad_key" : {
        "bad_nested_key" : "bad_value"
      }
    },
    "instrument" : {
      "instrument_key" : {
        "dummy_key" : "dummy_1"
      },
      "nested_key" : {
        "value_key" : "value_2",
        "deps" : ["/mission_2/nested_key"]
      },
      "deps" : ["/mission_1"]
    }
  })"_json;

  nlohmann::json resolvedConfig = baseConfig["instrument"];
  resolveConfigDependencies(resolvedConfig, baseConfig);

  nlohmann::json expectedConfig = R"(
  {
    "instrument_key" : {
      "dummy_key" : "dummy_1"
    },
    "mission_key" : {
      "array_key" : ["array_1", "array_2"]
    },
    "bad_nested_key" : "bad_value",
    "nested_key" : {
      "value_key" : ["value_2", "value_1"]
    }
  })"_json;
  EXPECT_EQ(resolvedConfig, expectedConfig);
}


TEST(UtilTests, resolveConfigDependenciesLoop) {
  nlohmann::json baseConfig = R"(
  {
    "Thing_1" : {
      "deps" : ["/Thing_2"]
    },
    "Thing_2" : {
      "deps" : ["/Thing_1"]
    }
  })"_json;

  nlohmann::json resolvedConfig = baseConfig["Thing_1"];
  EXPECT_THROW(resolveConfigDependencies(resolvedConfig, baseConfig), invalid_argument);
}


TEST(UtilTests, mergeConfigs) {
  nlohmann::json baseConfig = R"({
    "ck" : {
      "predict" : {
        "kernels" : "predict_ck_1.bc"
      },
      "reconstructed" : {
        "kernels" : "recon_ck_3.bs"
      },
      "smithed" : {
        "kernels" : ["smithed_ck_2.bs", "smithed_ck_3.bs"]
      }
    },
    "ik" : {
      "kernels" : "ik_1.ti"
    }
  })"_json;

  nlohmann::json mergeConfig = R"({
    "ck" : {
      "reconstructed" : {
        "kernels" : ["recon_ck_1.bc", "recon_ck_2.bc"]
      },
      "smithed" : {
        "kernels" : "smithed_ck_1.bs"
      }
    },
    "spk" : {
      "predict" : {
        "kernels" : "predict_spk_1.bsp"
      }
    },
    "sclk" : {
      "kernels" : "sclk_1.tsc"
    },
    "fk" : {
      "kernels" : "fk_1.tsc"
    }
  })"_json;

  mergeConfigs(baseConfig, mergeConfig);

  nlohmann::json expectedConfig = R"({
    "ck" : {
      "predict" : {
        "kernels" : "predict_ck_1.bc"
      },
      "reconstructed" : {
        "kernels" : ["recon_ck_3.bs", "recon_ck_1.bc", "recon_ck_2.bc"]
      },
      "smithed" : {
        "kernels" : ["smithed_ck_2.bs", "smithed_ck_3.bs", "smithed_ck_1.bs"]
      }
    },
    "spk" : {
      "predict" : {
        "kernels" : "predict_spk_1.bsp"
      }
    },
    "sclk" : {
      "kernels" : "sclk_1.tsc"
    },
    "fk" : {
      "kernels" : "fk_1.tsc"
    },
    "ik" : {
      "kernels" : "ik_1.ti"
    }
  })"_json;

  EXPECT_EQ(baseConfig, expectedConfig);

  nlohmann::json objectConfig = R"({
    "testkey" : {}
  })"_json;

  nlohmann::json valueConfig = R"({
    "testkey" : "testvalue"
  })"_json;

  EXPECT_THROW(mergeConfigs(objectConfig, valueConfig), std::invalid_argument);
  EXPECT_THROW(mergeConfigs(valueConfig, objectConfig), std::invalid_argument);
}

TEST(UtilTests, eraseAtPointer) {
  nlohmann::json baseJ = R"(
  {
    "outer_key" : {
      "middle_key" : {
        "inner_key" : {
          "value_to_erase" : "dummy"
        },
        "extra_inner_key" : "dummy"
      },
      "extra_middle_key" : "dummy"
    },
    "extra_outer_key" : "dummy"
  })"_json;

  nlohmann::json j = baseJ;
  size_t numErased = eraseAtPointer(j, nlohmann::json::json_pointer("/outer_key/middle_key/inner_key/value_to_erase"));
  EXPECT_EQ(numErased, 1);
  nlohmann::json expected = R"(
  {
    "outer_key" : {
      "middle_key" : {
        "inner_key" : {
        },
        "extra_inner_key" : "dummy"
      },
      "extra_middle_key" : "dummy"
    },
    "extra_outer_key" : "dummy"
  })"_json;
  EXPECT_EQ(j, expected);

  j = baseJ;
  numErased = eraseAtPointer(j, nlohmann::json::json_pointer("/outer_key/middle_key/bad_key"));
  EXPECT_EQ(numErased, 0);
  EXPECT_EQ(j, baseJ);
}


TEST(UtilTests, getRootDependency) {
  std::string folder = getenv("SPICEROOT");
  folder += "/mission_2";
  ASSERT_TRUE(fs::create_directory(folder));

  nlohmann::json baseConfig = R"(
  {
    "mission_1" : {
      "deps" : ["/mission_2"]
    },
    "mission_2" : {
    },
    "instrument" : {
      "deps" : ["/mission_1"]
    }
  })"_json;
  
  std::string rootPointer = getRootDependency(baseConfig, "/instrument");
  ASSERT_TRUE(fs::remove(folder));
  EXPECT_EQ(rootPointer, "/mission_2");
}


TEST(UtilTests, checkNaifErrors) {
  checkNaifErrors();
  furnsh_c("does/not/exist");

  // furnsh has errored, but we wont get anything until we check for it.
  try { 
    checkNaifErrors();
    FAIL() << "Should throw" << std::endl;
  }
  catch (runtime_error &e) {
    EXPECT_PRED_FORMAT2(spiceql::AssertExceptionMessage, e, "The attempt to load \"does/not/exist\" by the routine FURNSH failed.");
  }
}


TEST(UtilTests, testGetPathsFromRegex) {
  // create a temp work area 
  fs::path temp = fs::temp_directory_path() / SpiceQL::gen_random(10); 
  fs::create_directories(temp);

  // create some files 
  fstream fs;
  
  fs.open((temp/"test12345.ti").string(), ios::out);
  fs.close();
  fs.open((temp/"test44444.ti").string(), ios::out);
  fs.close();
  fs.open((temp/"test55555.ti").string(), ios::out);
  fs.close();

  fs.open((temp/"test1.spk").string(), ios::out);
  fs.close();
  fs.open((temp/"test2.spk").string(), ios::out);
  fs.close();


  nlohmann::json kernels = R"( ["test[0-9]{5}.ti", "test[0-9].spk"] )"_json;
  std::vector<std::vector<std::string>> res; 
  res = getPathsFromRegex(temp, jsonArrayToVector(kernels));

  // just enforce the shape
  EXPECT_EQ(res.size(), 2); 
  EXPECT_EQ(res.at(0).size(), 3); 
  EXPECT_EQ(res.at(1).size(), 2); 
 
}


TEST(UtilTests, testGetPathsFromRegexSingleRegex) {
  // create a temp work area 
  fs::path temp = fs::temp_directory_path() / SpiceQL::gen_random(10); 
  fs::create_directories(temp);

  // create some files 
  fstream fs;
  fs.open((temp/"test1.spk").string(), ios::out);
  fs.close();
  fs.open((temp/"test2.spk").string(), ios::out);
  fs.close();

  nlohmann::json kernels = R"( ["test[0-9].spk"] )"_json;
  std::vector<std::vector<std::string>> res; 
  res = getPathsFromRegex(temp, jsonArrayToVector(kernels));

  // just enforce the shape, this should still be "2D"
  EXPECT_EQ(res.size(), 1); 
  EXPECT_EQ(res.at(0).size(), 2); 
}


TEST(UtilTests, testJson2DArrayTo2DVector) { 
  nlohmann::json arrays = R"({
      "2D Array" : [["1.bc", "2.bc", "3.bc"], ["1.bc", "2.bc"]],
      "string" : "1.bc",
      "1D Array" : ["1.bc", "2.bc", "3.bc"]
  })"_json; 

  vector<vector<string>> res = json2DArrayTo2DVector(arrays["2D Array"]);
  ASSERT_EQ(res.size(), 2);
  ASSERT_EQ(res.at(0).size(), 3);
  ASSERT_EQ(res.at(1).size(), 2);

  vector<string> truth = {"1.bc", "2.bc", "3.bc"}; 
  EXPECT_THAT(res.at(0), truth);
  truth = {"1.bc", "2.bc"}; 
  EXPECT_THAT(res.at(1), truth);

  res = json2DArrayTo2DVector(arrays["string"]);
  ASSERT_EQ(res.size(), 1);
  ASSERT_EQ(res.at(0).size(), 1);
  ASSERT_EQ(res.at(0).at(0), "1.bc"); 


  try { 
    res = json2DArrayTo2DVector(arrays["1D Array"]);
    FAIL() << "Should throw" << std::endl;
  }
  catch (invalid_argument &e) {
    EXPECT_PRED_FORMAT2(spiceql::AssertExceptionMessage, e, "Input json is not a valid 2D Json array:");
  } 
}

TEST(PluralSuit, UnitTestGetTargetStates) {
  MockRepository mocks;

  nlohmann::json getLatestKernelsJson;
  getLatestKernelsJson["kernels"] = {{"/Some/Path/to/someKernel.x"}};
  mocks.OnCallFunc(SpiceQL::getLatestKernels).Return(getLatestKernelsJson);

  nlohmann::json searchMissionKernelsJson;
  searchMissionKernelsJson["ck"]["reconstructed"]["kernels"] = {{"/Path/to/some/ck.bc"}};
  searchMissionKernelsJson["spk"]["reconstructed"]["kernels"] = {{"/Path/to/some/spk.bsp"}};
  mocks.OnCallFunc(SpiceQL::searchEphemerisKernels).Return(searchMissionKernelsJson);

  vector<double> state = {0, 0, 0, 0, 0, 0, 0};
  mocks.OnCallFunc(getTargetState).Return(state);

  mocks.OnCallFunc(furnsh_c);

  vector<double> ets = {110000000};
  vector<vector<double>> resStates = getTargetStates(ets, "LRO", "LRO", "J2000", "NONE", "lro");

  EXPECT_EQ(resStates.size(), 1);
  ASSERT_EQ(resStates.at(0).size(), 7);
  EXPECT_DOUBLE_EQ(resStates.at(0)[0], 0.0);
  EXPECT_DOUBLE_EQ(resStates.at(0)[1], 0.0);
  EXPECT_DOUBLE_EQ(resStates.at(0)[2], 0.0);
  EXPECT_DOUBLE_EQ(resStates.at(0)[3], 0.0);
  EXPECT_DOUBLE_EQ(resStates.at(0)[4], 0.0);
  EXPECT_DOUBLE_EQ(resStates.at(0)[5], 0.0);
  EXPECT_DOUBLE_EQ(resStates.at(0)[6], 0.0);
}

TEST_F(LroKernelSet, UnitTestGetTargetState) {
  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}};
  KernelSet testSet(testKernelJson);

  double et = 110000000;
  vector<double> resStates = getTargetState(et, "LRO", "MOON", "J2000", "NONE");

  EXPECT_EQ(resStates.size(), 7);
  EXPECT_NEAR(resStates[0], 0.0, 1e-14);
  EXPECT_NEAR(resStates[1], 0.0, 1e-14);
  EXPECT_NEAR(resStates[2], 0.0, 1e-14);
  EXPECT_NEAR(resStates[3], 0.0, 1e-14);
  EXPECT_NEAR(resStates[4], 0.0, 1e-14);
  EXPECT_NEAR(resStates[5], 0.0, 1e-14);
  EXPECT_NEAR(resStates[6], 0.0, 1e-14);
}


TEST(PluralSuit, UnitTestGetTargetOrientations) {
  MockRepository mocks;

  nlohmann::json getLatestKernelsJson;
  getLatestKernelsJson["kernels"] = {{"/Some/Path/to/someKernel.x"}};
  mocks.OnCallFunc(SpiceQL::getLatestKernels).Return(getLatestKernelsJson);

  nlohmann::json searchMissionKernelsJson;
  searchMissionKernelsJson["ck"]["reconstructed"]["kernels"] = {{"/Path/to/some/ck.bc"}};
  searchMissionKernelsJson["spk"]["reconstructed"]["kernels"] = {{"/Path/to/some/spk.bsp"}};
  mocks.OnCallFunc(SpiceQL::searchEphemerisKernels).Return(searchMissionKernelsJson);

  vector<double> orientation = {0, 0, 0, 0, 0, 0, 0};
  mocks.OnCallFunc(getTargetOrientation).Return(orientation);

  mocks.OnCallFunc(furnsh_c);

  vector<double> ets = {110000000};
  vector<vector<double>> resOrientations = getTargetOrientations(ets, 1, -85620, "lro");

  EXPECT_EQ(resOrientations.size(), 1);
  ASSERT_EQ(resOrientations.at(0).size(), 7);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[0], 0.0);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[1], 0.0);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[2], 0.0);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[3], 0.0);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[4], 0.0);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[5], 0.0);
  EXPECT_DOUBLE_EQ(resOrientations.at(0)[6], 0.0);
}


TEST_F(LroKernelSet, UnitTestGetTargetOrientation) {
  nlohmann::json testKernelJson;
  testKernelJson["kernels"] = {{ckPath1}, {ckPath2}, {spkPath1}, {spkPath2}, {spkPath3}, {ikPath2}, {fkPath}, {sclkPath}};
  KernelSet testSet(testKernelJson);

  double et = 110000000;
  vector<double> resStates = getTargetOrientation(et, -85000, 1);

  EXPECT_EQ(resStates.size(), 7);
  EXPECT_NEAR(resStates[0], 1.0, 1e-14);
  EXPECT_NEAR(resStates[1], 0.0, 1e-14);
  EXPECT_NEAR(resStates[2], 0.0, 1e-14);
  EXPECT_NEAR(resStates[3], 0.0, 1e-14);
  EXPECT_NEAR(resStates[4], 0.0, 1e-14);
  EXPECT_NEAR(resStates[5], 0.0, 1e-14);
  EXPECT_NEAR(resStates[6], 0.0, 1e-14);
}
