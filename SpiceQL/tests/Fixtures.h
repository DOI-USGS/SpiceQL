#pragma once

#include <HippoMocks/hippomocks.h>

#include "config.h"

#include "gtest/gtest.h"
#include <ghc/fs_std.hpp>

#include "spice_types.h"

using namespace std;
using namespace SpiceQL;


class TempTestingFiles : public ::testing::Environment {
  protected:
    fs::path tempDir;

    void SetUp() override;
    void TearDown() override;
};

class IsisDataDirectory : public ::testing::Test {
  protected: 
    
    string base;
    vector<string> files; 

    unordered_map<string, set<string>> missionMap;
    unordered_map<string, set<string>> kernelTypeMap;
    
    void SetUp() override;
    void TearDown() override; 
    void compareKernelSets(string name, set<string> expectedDiff = {});
    void CompareKernelSets(vector<string> kVector, vector<string> expectedSubSet);
};

class KernelDataDirectories : public ::testing::Test  {
  protected:

    vector<string> paths;

    void SetUp() override;
    void TearDown() override;
};


class LroKernelSet : public ::testing::Test  {
  protected:
    fs::path root;
    string lskPath;
    string sclkPath;
    string ckPath1;
    string ckPath2;
    string spkPath1;
    string spkPath2;
    string spkPath3;
    string ikPath1;
    string ikPath2;
    string fkPath;

    nlohmann::json conf;

    void SetUp() override;
    void TearDown() override;
};

class KernelsWithQualities : public ::testing::Test  {
  protected:
    fs::path root;
    string spkPathPredict; 
    string spkPathRecon; 
    string spkPathRecon2; 
    string spkPathSmithed; 

    void SetUp() override;
    void TearDown() override;
};

class TestConfig : public KernelDataDirectories {
  protected:

    SpiceQL::Config testConfig;

    void SetUp() override;
    void TearDown() override;
};
