#include <fstream>

#include <gtest/gtest.h>

#include "Fixtures.h"

#include "config.h"
#include "utils.h"
#include "query.h"
#include "memo.h"

using namespace std;
using json = nlohmann::json;
using namespace SpiceQL;


TEST_F(IsisDataDirectory, FunctionalTestConfigEval) {
  Config testConfig;
  json config_eval_res = testConfig.get();
  json pointer_eval_res = testConfig.get("/clementine1");

  json::json_pointer pointer = "/ck/reconstructed/kernels"_json_pointer;
  int expected_number = 0;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res["clementine1"][pointer]).size(), expected_number);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(pointer_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig["clementine1"][pointer].size(), 1);

  pointer = "/ck/smithed/kernels"_json_pointer;
  expected_number = 1;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res["clementine1"][pointer]).size(), expected_number);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(pointer_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig["clementine1"][pointer].size(), expected_number);

  pointer = "/spk/reconstructed/kernels"_json_pointer;
  expected_number = 0;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res["clementine1"][pointer]).size(), expected_number);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(pointer_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig["clementine1"][pointer].size(), 1);

  pointer = "/fk/kernels"_json_pointer;
  expected_number = 3;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res["clementine1"][pointer]).size(), expected_number);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(pointer_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig["clementine1"][pointer].size(), 1);

  pointer = "/sclk/kernels"_json_pointer;
  expected_number = 1;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res["clementine1"][pointer]).size(), expected_number);
  EXPECT_EQ(SpiceQL::getKernelsAsVector(pointer_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig["clementine1"][pointer].size(), 1);

  pointer = "/uvvis/ik/kernels"_json_pointer;
  expected_number = 1;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/uvvis/iak/kernels"_json_pointer;
  expected_number = 4;
  EXPECT_EQ(SpiceQL::getKernelsAsVector(config_eval_res[pointer]).size(), expected_number);
  EXPECT_EQ(testConfig[pointer].size(), 1);
}

