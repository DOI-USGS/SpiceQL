#include <fstream>
#include <gtest/gtest.h>

#include <cstdlib>

#include "Fixtures.h"

#include "query.h"
#include "utils.h"

using namespace std;
using namespace SpiceQL;


TEST_F(LroKernelSet, FunctionalTestSearchMissionKernels) {
  setenv("SPICEROOT", root.c_str(), true);

  // load all available kernels
  nlohmann::json kernels = listMissionKernels(root, conf);

  // do a time query
  kernels = searchEphemerisKernels(kernels, {110000000, 120000001}, false);
  kernels = getLatestKernels(kernels);
  
  ASSERT_EQ(kernels["moc"]["spk"]["smithed"]["kernels"].size(), 2);
  EXPECT_EQ(kernels["moc"]["spk"]["smithed"]["kernels"][0].size(), 1);
  EXPECT_EQ(kernels["moc"]["spk"]["smithed"]["kernels"][1].size(), 1);
  EXPECT_EQ(fs::path(kernels["moc"]["ck"]["reconstructed"]["kernels"][0][0].get<string>()).filename(), "soc31.0001.bc" );
  EXPECT_EQ(fs::path(kernels["moc"]["ik"]["kernels"][0][0].get<string>()).filename(), "lro_instruments_v11.ti");
  EXPECT_EQ(fs::path(kernels["moc"]["fk"]["kernels"][0][0].get<string>()).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["moc"]["sclk"]["kernels"][0][0].get<string>()).filename(), "lro_clkcor_2020184_v00.tsc");
}