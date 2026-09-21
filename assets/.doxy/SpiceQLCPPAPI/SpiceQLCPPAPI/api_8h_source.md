

# File api.h

[**File List**](files.md) **>** [**include**](dir_e40c79f4f542c4f82d8820babbacb572.md) **>** [**SpiceQL**](dir_a0f0e7e2de0975b11733b3563ad43b12.md) **>** [**api.h**](api_8h.md)

[Go to the documentation of this file](api_8h.md)


```C++
#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace SpiceQL {

    std::string getSpiceqlName(const std::string& name);

    void addAliasKey(const std::string& key, const std::string& value);

    nlohmann::json getAliasMap();

    void setAliasMap(const nlohmann::json& newAliasMap);

    std::string url_encode(const std::string &value);

    nlohmann::json spiceAPIQuery(std::string functionName, nlohmann::json args, std::string method="GET");
    
    std::pair<std::vector<std::vector<double>>, nlohmann::json> getTargetStates(
        std::vector<double> ets,
        std::string target,
        std::string observer,
        std::string frame,
        std::string abcorr,
        std::string mission="",
        std::vector<std::string> ckQualities={"smithed", "reconstructed"},
        std::vector<std::string> spkQualities={"smithed", "reconstructed"},
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<std::vector<std::vector<double>>, nlohmann::json> getTargetStatesRanged(
        double startEt,
        double stopEt,
        int numRecords,
        std::string target,
        std::string observer,
        std::string frame,
        std::string abcorr,
        std::string mission="",
        std::vector<std::string> ckQualities={"smithed", "reconstructed"},
        std::vector<std::string> spkQualities={"smithed", "reconstructed"},
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});
    
    std::pair<std::vector<std::vector<double>>, nlohmann::json> getTargetOrientations(
        std::vector<double> ets,
        int toFrame,
        int refFrame,
        std::string mission="",
        std::vector<std::string> ckQualities={"smithed", "reconstructed"},
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<std::vector<std::vector<double>>, nlohmann::json> getTargetOrientationsRanged(
        double startEt, 
        double stopEt, 
        int numRecords, 
        int toFrame, 
        int refFrame, 
        std::string mission="", 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"},
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});
    
    std::pair<double, nlohmann::json> strSclkToEt(
        int frameCode,
        std::string sclk,
        std::string mission="",
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<double, nlohmann::json> doubleSclkToEt(
        int frameCode,
        double sclk,
        std::string mission="",
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    std::pair<std::string, nlohmann::json> doubleEtToSclk(
        int frameCode,
        double et,
        std::string mission="",
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    std::pair<double, nlohmann::json> utcToEt(
        std::string utc,
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<std::string, nlohmann::json> etToUtc(
        double et, 
        std::string format="", 
        double precision=0, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});
    
    std::pair<std::vector<double>, nlohmann::json> doubleEtsToSclkTicks(
        int frameCode,
        std::vector<double> ets,
        std::string mission="",
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1,
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<int, nlohmann::json> translateNameToCode(
        std::string frame, 
        std::string mission="", 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1, 
        std::vector<std::string> kernelList={});

    std::pair<std::string, nlohmann::json> translateCodeToName(
        int frame, 
        std::string mission="", 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<std::vector<int>, nlohmann::json> getFrameInfo(
        int frame, 
        std::string mission="", 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<nlohmann::json, nlohmann::json> getTargetFrameInfo(
        int targetId, 
        std::string mission="", 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    std::pair<nlohmann::json, nlohmann::json> findMissionKeywords(
        std::string key, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    std::pair<nlohmann::json, nlohmann::json> findTargetKeywords(
        std::string key, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    std::pair<std::vector<std::vector<int>>, nlohmann::json> frameTrace(
        double et, 
        int initialFrame, 
        std::string mission="", 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        std::vector<std::string> spkQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool searchKernels=true,
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1, 
        std::vector<std::string> kernelList={});

    std::pair<std::vector<double>, nlohmann::json> extractExactCkTimes(
        double observStart, 
        double observEnd, 
        int targetFrame, 
        std::string mission="", 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    std::pair<std::vector<std::vector<double>>, nlohmann::json> getExactTargetOrientations(
        double startEt, 
        double stopEt, 
        int toFrame, 
        int refFrame, 
        int exactCkFrame, 
        std::string mission="", 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList = {});

    std::pair<std::string, nlohmann::json> searchForKernelsets(
        std::vector<std::string> spiceqlNames, 
        std::vector<std::string> types={"ck", "spk", "tspk", "lsk", "mk", "sclk", "iak", "ik", "fk", "dsk", "pck", "ek"}, 
        double startTime=-std::numeric_limits<double>::max(), 
        double stopTime=std::numeric_limits<double>::max(), 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        std::vector<std::string> spkQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1, 
        bool overwrite=false);
}
```


