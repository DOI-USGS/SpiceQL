#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>

#include "TestUtilities.h"
#include "Fixtures.h"

#include "inventory.h"
#include "inventoryimpl.h"

#include <spdlog/spdlog.h>
#include <highfive/highfive.hpp>

TEST_F(LroKernelSet, TestInventorySmithed) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000);
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc"); 
  
  EXPECT_EQ(kernels["spk"].size(), 3);
  
  EXPECT_EQ(kernels["ckQuality"], "reconstructed");  
  EXPECT_EQ(kernels["spkQuality"], "smithed");  
}

TEST_F(LroKernelSet, TestInventoryRecon) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, "reconstructed", "reconstructed");
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_EQ(fs::path(kernels["ck"][0]).filename(), "soc31_1111111_1111111_v21.bc");
  EXPECT_EQ(kernels["ckQuality"], "reconstructed"); 
}


TEST_F(LroKernelSet, TestInventoryPredicted) { 
  Inventory::create_database();
  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, "predicted", "predicted");
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
  
  auto dataset = file.getDataSet("spice/lroc/sclk/kernels");
  vector<string> data = dataset.read<vector<string>>();   
  dataset.read(data);

  // assert that the path in the db is relative 
  EXPECT_EQ(data.at(0), "clocks/lro_clkcor_2020184_v00.tsc");

  nlohmann::json kernels = Inventory::search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"});

  // these paths should be expanded
  EXPECT_TRUE(kernels["sclk"][0].get<string>().size() > data.at(0).size());
}


TEST_F(KernelsWithQualities, TestUnenforcedQuality) { 
  nlohmann::json kernels = Inventory::search_for_kernelset("odyssey", {"spk"}, 130000000, 140000000, "smithed", "smithed", false);
  // smithed kernels should not exist so it should return reconstructed
  EXPECT_EQ(kernels["spkQuality"].get<string>(), "reconstructed");
}


TEST_F(KernelsWithQualities, TestEnforcedQuality) { 
  nlohmann::json kernels = Inventory::search_for_kernelset("odyssey", {"spk"}, 130000000, 140000000, "smithed", "smithed", true);
  // Should be empty since we are enforcing smithed
  EXPECT_TRUE(kernels.is_null());
}

