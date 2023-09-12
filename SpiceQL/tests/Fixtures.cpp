#include "Fixtures.h"
#include "Paths.h"

#include <HippoMocks/hippomocks.h>

#include <fmt/format.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

#include "utils.h"
#include "io.h"
#include "query.h"

using namespace std;
using namespace SpiceQL;


void TempTestingFiles::SetUp() {
  int max_tries = 10;
  auto tmp_dir = fs::temp_directory_path();
  unsigned long long i = 0;
  random_device dev;
  mt19937 prng(dev());
  uniform_int_distribution<uint64_t> rand(0);
  fs::path tpath;

  while (true) {
    stringstream ss;
    ss << "SQTESTS" << hex << rand(prng);
    tpath = tmp_dir / ss.str();

    // true if the directory was created.
    if (fs::create_directory(tpath)) {
        break;
    }
    if (i == max_tries) {
        throw runtime_error("could not find non-existing directory");
    }
    i++;
  }
  tempDir = tpath;

  setenv("SPICEROOT", tempDir.c_str(), true);
}


void TempTestingFiles::TearDown() {
    if(!fs::remove_all(tempDir)) {
      throw runtime_error("Could not delete temporary files");
    }
}


void KernelDataDirectories::SetUp() { 
  
  // combine multiple path lists here as we add more.
  paths = base_paths;
  paths.insert(paths.end(), mess_paths.begin(), mess_paths.end());
  paths.insert(paths.end(), apollo17_paths.begin(), apollo17_paths.end());
  paths.insert(paths.end(), clem1_paths.begin(), clem1_paths.end());
  paths.insert(paths.end(), galileo_paths.begin(), galileo_paths.end());
  paths.insert(paths.end(), cassini_paths.begin(), cassini_paths.end());
  paths.insert(paths.end(), lro_paths.begin(), lro_paths.end());
  paths.insert(paths.end(), apollo16_paths.begin(), apollo16_paths.end());
  paths.insert(paths.end(), juno_paths.begin(), juno_paths.end());
  paths.insert(paths.end(), viking1_paths.begin(), viking1_paths.end());
  paths.insert(paths.end(), viking2_paths.begin(), viking2_paths.end());
}


void KernelDataDirectories::TearDown() { }


void IsisDataDirectory::SetUp() { 
  base = "";

  ifstream infile("data/isisKernelList.txt");
  string line;

  while(getline(infile, line)) {
    fs::path p = line;
    string mission = p.parent_path().parent_path().parent_path().filename();
    string kernelType = p.parent_path().filename();

    files.emplace_back(base / p.filename());

    auto iter = missionMap.find(mission);
    if (iter == missionMap.end()) {
      set<string> s = {p.filename()};
      missionMap.emplace(mission, s);
    }
    else {
      missionMap[mission].emplace(p.filename());
    }

    iter = kernelTypeMap.find(kernelType);
    if (iter == kernelTypeMap.end()) {
      set<string> s = {p.filename()};
      kernelTypeMap.emplace(kernelType, s);
    }
    else {
      kernelTypeMap[kernelType].emplace(p.filename());
    }
  }
}


void IsisDataDirectory::TearDown() {}


void IsisDataDirectory::compareKernelSets(string name, set<string> expectedDiff) {
  fs::path dbPath = getMissionConfigFile(name);

  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);
  
  nlohmann::json res = listMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelsAsSet(res);
  set<string> expectedKernels = missionMap.at(name);
  set<string> diff;
  set<string> diffDiff;

  string diffFailMessage = "";

  set_difference(expectedKernels.begin(), expectedKernels.end(), kernels.begin(), kernels.end(), inserter(diff, diff.begin()));
  
  if (diff.size() != 0) {
    diffFailMessage = "Kernel sets are not equal, expected - retrieved diff: " + fmt::format("{}", fmt::join(diff, " "));
  }

  set_difference(kernels.begin(), kernels.end(), expectedKernels.begin(), expectedKernels.end(), inserter(diff, diff.begin()));
  if (diff.size() != 0) {
    diffFailMessage = "Kernel sets are not equal, retrieved - expected diff: " + fmt::format("{}", fmt::join(diff, " "));
  }

  if (expectedDiff.size() > 0) {
    set_difference(diff.begin(), diff.end(), expectedDiff.begin(), expectedDiff.end(), inserter(diffDiff, diffDiff.begin()));
    if (diffDiff.size() != 0) {
      diffFailMessage = "Kernel sets with expected diff are not equal, diff - expected diff: " + fmt::format("{}", fmt::join(diffDiff, " "));
    }
    else {
      diffFailMessage = "";
    }
  }

  if (diffFailMessage != "") {
    FAIL() << diffFailMessage << endl;
  }
}


void IsisDataDirectory::CompareKernelSets(vector<string> kVector, vector<string> expectedSubSet) {
  for (auto &e : kVector) { 
    auto it = find(kVector.begin(), kVector.end(), e);
    if (it == kVector.end()) {
      FAIL() << e << " was not found in the kernel results";
    }
  }
}


void LroKernelSet::SetUp() {
  root = getenv("SPICEROOT");
   
  // Move Clock kernels
  // TODO: Programmatic clock kernels
  lskPath = fs::path("data") / "naif0012.tls";
  sclkPath = fs::path("data") / "lro_clkcor_2020184_v00.tsc";
  create_directory(root / "clocks");

  fs::copy_file(lskPath, root / "clocks" / "naif0012.tls");
  fs::copy_file(sclkPath, root / "clocks" / "lro_clkcor_2020184_v00.tsc");

  // reassign member vars to temp dir
  lskPath = root / "clocks" / "naif0012.tls";
  sclkPath = root / "clocks" / "lro_clkcor_2020184_v00.tsc";

  Kernel sclk(sclkPath);

  // Write CK1 ------------------------------------------
  fs::create_directory(root / "ck");

  int bodyCode = -85000;
  std::string referenceFrame = "j2000";

  ckPath1 = root / "ck" / "soc31.0001.bc";
  std::vector<std::vector<double>> avs = {{1,1,1}, {2,2,2}};
  std::vector<std::vector<double>> quats = {{0.2886751, 0.2886751, 0.5773503, 0.7071068 }, {0.4082483, 0.4082483, 0.8164966, 0 }};
  std::vector<double> times1 = {110000000, 120000000};
  std::vector<double> times2 = {130000000, 140000000};

  writeCk(ckPath1, quats, times1, bodyCode, referenceFrame, "CK ID 1",  sclkPath, lskPath, avs, "CK1");

  // Write CK2 ------------------------------------------
  ckPath2 = root / "ck" / "lrolc.0002.bc";
  avs = {{3,4,5}, {6,5,5}};
  quats = {{0.3754439, 0.3754439, 0.3754439, -0.7596879}, {-0.5632779, -0.5632779, -0.5632779, 0.21944}};
  writeCk(ckPath2, quats, times2, bodyCode, referenceFrame, "CK ID 2", sclkPath, lskPath, avs, "CK2");

  // Write SPK1 ------------------------------------------
  fs::create_directory(root / "spk");
  spkPath1 = root / "spk" / "LRO_TEST_GRGM660MAT270.bsp";

  std::vector<std::vector<double>> velocities = {{1,1,1}, {2,2,2}};
  std::vector<std::vector<double>> positions = {{1, 1, 1}, {2, 2, 2}};
  writeSpk(spkPath1, positions, times1, bodyCode, 1, referenceFrame, "SPK ID 1", 1, velocities, "SPK 1");

  // Write SPK2 ------------------------------------------
  velocities = {{3, 3, 3}, {5, 5, 5}};
  positions = {{3, 3, 3}, {4, 4, 4}};
  spkPath2 = root / "spk" / "LRO_TEST_GRGM660MAT370.bsp";
  writeSpk(spkPath2, positions, times2, bodyCode, 1, referenceFrame, "SPK ID 2", 1, velocities, "SPK 2");

  // Write SPK3 ------------------------------------------
  fs::create_directory(root / "spk");
  spkPath3 = root / "spk" / "LRO_TEST_GRGM660MAT470.bsp";

  velocities = {{1, 1, 1}, {2, 2, 2}};
  positions = {{10, 10, 10}, {11, 11, 11}};
  writeSpk(spkPath3, positions, times1, bodyCode/1000, 301, referenceFrame, "SPK ID 3", 1, velocities, "SPK 3");

  // Write IK1 -------------------------------------------
  fs::create_directory(root / "ik");

  ikPath1 = root / "ik" / "lro_instruments_v10.ti";
  nlohmann::json jKeywords = {
    {"INS-85600_PIXEL_SAMPLES", { 5064 }},
    {"INS-85600_PIXEL_LINES", { 1 }},
    {"INS-85600_PIXEL_SIZE", { 7.0E-3 , 7.0E-3 }},
    {"INS-85600_CCD_CENTER", { 2531.5 , 0.5 }}
  };

  writeTextKernel(ikPath1, "ik", jKeywords);

  // Write IK2 -------------------------------------------
  ikPath2 = root / "ik" / "lro_instruments_v11.ti";
  jKeywords = {
    {"INS-85600_PIXEL_SAMPLES", { 5063 }},
    {"INS-85600_PIXEL_LINES", { 1 }},
    {"INS-85600_PIXEL_SIZE", { 7.0E-4 , 7.0E-2 }},
    {"INS-85600_CCD_CENTER", { 2531.3 , 0.4 }}
  };

  writeTextKernel(ikPath2, "ik", jKeywords);

  // Write FK ---------------------------------------------
  fs::create_directory(root / "fk");

  jKeywords = {
    {"FRAME_LRO_LROCWAC", -85620},
    {"FRAME_-85620_NAME", "LRO_LROCWAC"},
    {"FRAME_-85620_CLASS", 3},
    {"FRAME_-85620_CLASS_ID", -85620},
    {"FRAME_-85620_CENTER", -85},
    {"TKFRAME_-85620_RELATIVE", "LRO_SC_BUS"},
    {"CK_-85620_SCLK", -85},
    {"CK_-85620_SPK", -85},
    {"FRAME_LRO_SC_BUS", -85000},
    {"FRAME_-85000_NAME", "LRO_SC_BUS"},
    {"FRAME_-85000_CLASS", 3},
    {"FRAME_-85000_CLASS_ID", -85000},
    {"FRAME_-85000_CENTER", -85},
    {"CK_-85000_SCLK", -85},
    {"CK_-85000_SPK", -85},
    {"NAIF_BODY_NAME", {"LRO", "LUNAR RECONNAISSANCE ORBITER", "LRO_LROCWAC", "LRO_LROCNACL", "LRO_LROCNACR"}},
    {"NAIF_BODY_CODE", {-85, -85, -85620, -85600, -85610}}
  };

  fkPath = root / "fk" / "lro_frames_1111111_v01.tf";

  writeTextKernel(fkPath, "fk", jKeywords);

  conf = R"({
    "moc" : {
        "ck" : {
            "reconstructed" : {
                "kernels": ["soc31.*.bc", "lrolc.*.bc"]
            },
            "deps" : {
            "sclk" : ["lro_clkcor_[0-9]{7}_v[0-9]{2}.tsc"],
            "objs" : ["/base/lsk", "/moc/sclk"]
            }
        },
        "spk" : {
        "reconstructed" : {
            "kernels" : ["fdf29_[0-9]{7}_[0-9]{7}_[0-9]{3}.bsp", "fdf29r_[0-9]{7}_[0-9]{7}_[0-9]{3}.bsp"]
        },
        "smithed" : {
            "kernels" : ["LRO_.*_GRGM660.*.bsp", "LRO_.*_GRGM900C.*.BSP"]
        },
        "deps" : {
            "sclk" : ["lro_clkcor_[0-9]{7}_v[0-9]{2}.tsc"],
            "objs" : ["/base/lsk", "/moc/sclk"]
        }
        },
        "sclk" : {
        "kernels" : ["lro_clkcor_[0-9]{7}_v[0-9]{2}.tsc"]
        },
        "fk" : {
        "kernels" : ["lro_frames_[0-9]{7}_v[0-9]{2}.tf"]
        },
        "ik" : {
        "kernels" : ["lro_instruments_v[0-9]{2}.ti"]
        }
    }
})"_json;
}

void LroKernelSet::TearDown() {

}

void TestConfig::SetUp() {
  KernelDataDirectories::SetUp();
}

void TestConfig::TearDown() {

}
