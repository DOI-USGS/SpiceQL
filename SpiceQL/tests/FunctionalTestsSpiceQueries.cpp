#include <fstream>
#include <gtest/gtest.h>

#include <cstdlib>

#include "Fixtures.h"

#include "query.h"
#include "utils.h"
#include "inventory.h"

using namespace std;
using namespace SpiceQL;


TEST_F(LroKernelSet, FunctionalTestSearchMissionKernels) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"lsk", "sclk", "ck", "spk", "iak", "fk"}, 110000000, 140000001);
  cout << kernels.dump(4) << endl;
  ASSERT_EQ(kernels["ck"].size(), 2);
  EXPECT_EQ(fs::path(kernels["ck"][0].get<string>()).filename(), "soc31_1111111_1111111_v21.bc" );
  EXPECT_EQ(fs::path(kernels["iak"][0].get<string>()).filename(), "lro_instrumentAddendum_v11.ti");
  EXPECT_EQ(fs::path(kernels["fk"][0].get<string>()).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0].get<string>()).filename(), "lro_clkcor_2020184_v00.tsc");
}