#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "Fixtures.h"
#include <SpiceQL/alias_map.h>

using namespace SpiceQL;
using json = nlohmann::json;

TEST_F(AliasMapTest, DefaultLoad) {
	// Test the default load from source tree
	load_aliases(defaultAliasMapFile.string());
	EXPECT_EQ(aliasMap.getSpiceqlName("LRO_LROCNACL"), "lroc");
}

TEST_F(AliasMapTest, LoadFromSpecificPath) {
	// Verify test file exists
	ASSERT_TRUE(fs::exists(testAliasMapFile))
			<< "Test data file not found at: " << testAliasMapFile;
	EXPECT_NO_THROW(load_aliases(testAliasMapFile.string()));
	EXPECT_EQ(aliasMap.getSpiceqlName("Fake"), "test_mission");
	EXPECT_EQ(aliasMap.getSpiceqlName("MRO"), "mro");
	EXPECT_EQ(aliasMap.getAliasMap().size(), 1);
}

TEST_F(AliasMapTest, CaseInsensitiveFrameListFallback) {
	EXPECT_EQ(aliasMap.getSpiceqlName("LRO"), "lro");
	EXPECT_EQ(aliasMap.getSpiceqlName("LROC"), "lroc");
	// Existing lowercase behavior is preserved.
	EXPECT_EQ(aliasMap.getSpiceqlName("lro"), "lro");
}

TEST_F(AliasMapTest, InvalidAlias) {
	// Test error handling for missing alias
	load_aliases(testAliasMapFile.string());
	EXPECT_EQ(aliasMap.getSpiceqlName("bad_alias"), "");
}

TEST_F(AliasMapTest, EnsureInit) {
	// Get mission without explicit loading, will load default automatically
	EXPECT_EQ(aliasMap.getSpiceqlName("CH2_OHRC"), "ohrc");
}

TEST_F(AliasMapTest, PathParamOverridesEnvironment) {
	EXPECT_EQ(aliasMap.getSpiceqlName("CH2_TMC_NADIR"), "tmc2");

	// load test alias map
	EXPECT_NO_THROW(load_aliases(testAliasMapFile.string()));
	EXPECT_EQ(aliasMap.getSpiceqlName("CH2_TMC_NADIR"), "");
}

TEST_F(AliasMapTest, SetAliases) {
	// Test setting aliases
	json newAliasMap = { 
    {"test1", {"alias1", "alias2"}},
		{"test2", {"alias3"}} 
	};
	aliasMap.setAliasMap(newAliasMap);

	json updatedAliasMap = aliasMap.getAliasMap();
	EXPECT_EQ(updatedAliasMap.size(), 2);
	EXPECT_EQ(updatedAliasMap["test1"][1], "ALIAS1");
}

TEST_F(AliasMapTest, GetAliases) {
	// Verify dev alias map is valid
	json aliasMapJson = aliasMap.getAliasMap();
	
	EXPECT_FALSE(aliasMapJson.empty());
	EXPECT_GT(aliasMapJson.size(), 46); //currently 47 db keys
}

