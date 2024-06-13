#include <gtest/gtest.h>

#include <ghc/fs_std.hpp>

#include "TestUtilities.h"
#include "Fixtures.h"

#include "inventory.h"

#include <spdlog/spdlog.h>


TEST_F(LroKernelSet, TestInventorySmithed) { 
  Inventory inv(true);
  nlohmann::json kernels = inv.search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000);
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_EQ(fs::path(kernels["spk"][0]).filename(), "LRO_TEST_GRGM660MAT470.bsp");
  EXPECT_EQ(kernels["ck"].size(), 2);
  EXPECT_EQ(kernels["ckQuality"], "reconstructed");  
  EXPECT_EQ(kernels["spkQuality"], "smithed");  
}

TEST_F(LroKernelSet, TestInventoryRecon) { 
  Inventory inv(true);
  nlohmann::json kernels = inv.search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, "reconstructed", "reconstructed");
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_EQ(kernels["ck"].size(), 1);
  EXPECT_EQ(kernels["ckQuality"], "reconstructed"); 
}


TEST_F(LroKernelSet, TestInventoryPredicted) { 
  Inventory inv(true);
  nlohmann::json kernels = inv.search_for_kernelset("lroc", {"fk", "sclk", "spk", "ck"}, 110000000, 140000000, "predicted", "predicted");
  EXPECT_EQ(fs::path(kernels["fk"][0]).filename(), "lro_frames_1111111_v01.tf");
  EXPECT_EQ(fs::path(kernels["sclk"][0]).filename(), "lro_clkcor_2020184_v00.tsc");
  EXPECT_TRUE(!kernels.contains("spk")); // no spks
  EXPECT_TRUE(!kernels.contains("cks")); // no cks
}


TEST_F(LroKernelSet, TestInventoryEmpty) { 
  Inventory inv(true);
  nlohmann::json kernels = inv.search_for_kernelset("mgs", {"fk", "sclk", "spk", "ck"});
  EXPECT_TRUE(kernels.empty());
}

