#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "Fixtures.h"
#include "alias_map.h"

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
	EXPECT_EQ(aliasMap.getSpiceqlName("MRO"), "");
	EXPECT_EQ(aliasMap.getAliasMap().size(), 1);
}

TEST_F(AliasMapTest, InvalidAlias) {
	// Test error handling for missing alias
	load_aliases(testAliasMapFile.string());
	EXPECT_EQ(aliasMap.getSpiceqlName("bad_alias"), "");
}

TEST_F(AliasMapTest, EnsureInit) {
	// Get mission without explicit loading, will load default automatically
	setenv("SPICEQL_DEV_ALIAS", "true", 1);
	EXPECT_EQ(aliasMap.getSpiceqlName("CH2_OHRC"), "ohrc");
}

TEST_F(AliasMapTest, PathParamOverridesEnvironment) {
	setenv("SPICEQL_DEV_ALIAS", "true", 1);
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
	setenv("SPICEQL_DEV_ALIAS", "true", 1);
	json aliasMapJson = aliasMap.getAliasMap();
	
	EXPECT_FALSE(aliasMapJson.empty());
	EXPECT_GT(aliasMapJson.size(), 46); //currently 47 db keys
}

