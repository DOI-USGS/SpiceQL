#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>

#include "TestUtilities.h"
#include "Fixtures.h"

#include "inventory.h"
#include "inventoryimpl.h"

#include <fstream>
#include <spdlog/spdlog.h>
#include <highfive/highfive.hpp>

TEST_F(LroKernelSet, TestInventorySmithed) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000);
  SPDLOG_DEBUG("Returned Kernels {} ", kernels.dump());
  
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc"); 
  
  EXPECT_EQ(kernels["spk"].size(), 1);
  
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed");  
  EXPECT_EQ(kernels["lroc_spk_quality"], "smithed");  
}

TEST_F(LroKernelSet, TestInventoryRecon) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"reconstructed"}, {"reconstructed"});
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc");
  EXPECT_EQ(kernels["lroc_ck_quality"], "reconstructed"); 
}


TEST_F(LroKernelSet, TestInventoryPredicted) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, {"predicted"}, {"predicted"});
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_TRUE(!kernels.contains("cks")); // no cks
}


TEST_F(LroKernelSet, TestInventoryEmpty) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("mgs", {"fk", "sclk", "spk", "ck"});
  EXPECT_TRUE(kernels.empty());
}


TEST_F(LroKernelSet, TestInventoryPortability) { 
  fs::path dbfile = Inventory::getDbFilePath();
  HighFive::File file(dbfile, HighFive::File::ReadOnly);
  
  auto dataset = file.getDataSet("/spice/lroc/sclk");
  vector<string> data = dataset.read<vector<string>>();   
  dataset.read(data);

  // assert that the path in the db is relative 
  EXPECT_EQ(data.at(0), "clocks/lro_clkcor_2020184_v00.tsc");

  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"});

  // these paths should be expanded
  EXPECT_TRUE(kernels["sclk"][0].get<string>().size() > data.at(0).size());
}


TEST_F(KernelsWithQualities, TestUnenforcedQuality) { 
  nlohmann::json kernels = Inventory::search_for_kernelset("odyssey", {"spk"}, 130000000, 140000000, {"smithed", "reconstructed"}, {"smithed", "reconstructed"}, false, false);
  // smithed kernels should not exist so it should return reconstructed
  EXPECT_EQ(kernels["odyssey_spk_quality"].get<string>(), "reconstructed");
}


TEST_F(KernelsWithQualities, TestEnforcedQuality) { 
  nlohmann::json kernels = Inventory::search_for_kernelset("odyssey", {"spk"}, 130000000, 140000000, {"smithed"}, {"smithed"}, false, true);
  // Should be empty since we are enforcing smithed
  EXPECT_TRUE(kernels.is_null());
}

TEST_F(LroKernelSet, TestInventorySearch) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"lsk", "sclk", "ck", "spk", "iak", "fk"}, 110000000, 140000001);
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["ck"].size(), 2);
  EXPECT_EQ(fs::path(kernels["ck"][0].get<string>()).filename(), "soc31_1111111_1111111_v21.bc" );
  EXPECT_EQ(fs::path(kernels["iak"][0].get<string>()).filename(), "lro_instrumentAddendum_v11.ti");
  EXPECT_EQ(fs::path(kernels["fk"][0].get<string>()).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0].get<string>()).filename(), "lro_clkcor_2020184_v00.tsc");
}

TEST_F(LroKernelSet, TestInventorySearchSetsNoOverwrite) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelsets({"moon", "base"}, {"pck"}, 110000000, 140000001, {"reconstructed"}, {"reconstructed"}, false, false, false);
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["pck"].size(), 2);
  EXPECT_EQ(fs::path(kernels["pck"][0].get<string>()).filename(), "moon_080317.tf");
  EXPECT_EQ(fs::path(kernels["pck"][1].get<string>()).filename(), "pck00009.tpc");
}


TEST_F(TempTestingFiles, SpiceQLPerformenceInventory) { 
	 std::ofstream outfile;
        outfile.open("/home/ec2-user/spiceqltimes_themis.txt", std::ios_base::app); // append instead of overwrite
    for(int i = 0; i < 1; i++) {
        const clock_t begin_time = clock();
	cout << Inventory::search_for_kernelsets({"odyssey", "mars"}, KERNEL_TYPES, 715662878.32324, 715663065.2303) << endl;
        float seconds = float( clock () - begin_time ) /  CLOCKS_PER_SEC;
	cout << seconds << endl;
	outfile << seconds << endl;

    }
}


TEST_F(LroKernelSet, TestInventorySearchSetsOverwrite) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelsets({"moon", "base"}, {"pck"}, 110000000, 140000001, {"reconstructed"}, {"reconstructed"}, false, false, true);
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["pck"].size(), 1);
  EXPECT_EQ(fs::path(kernels["pck"][0].get<string>()).filename(), "pck00009.tpc");
}


TEST_F(LroKernelSet, TestInventorySearchSetFromRegex) {
  // do a time query
  nlohmann::json kernels = Inventory::search_for_kernelset_from_regex({"/lroc/sclk/.*", "/lroc/spk/smithed/LRO_TEST_GRGM660MAT[0-9]{3}.bsp", "/iau_moon/pck/moon.*"});
  SPDLOG_DEBUG("TEST KERNELS: {}", kernels.dump(4));
  EXPECT_EQ(kernels["spk"].size(), 3);
  EXPECT_EQ(kernels["spk_quality"].get<string>(), "smithed"); 
  EXPECT_EQ(fs::path(kernels["pck"][0].get<string>()).filename(), "moon_080317.tf");
}