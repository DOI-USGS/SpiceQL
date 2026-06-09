#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <ghc/fs_std.hpp>

#include "Fixtures.h"
#include <SpiceQL/api.h>

using namespace std;
using namespace SpiceQL;

static bool matchesSchema(const nlohmann::json& report) {
    if (!report.is_object()) return false;

    for (const auto& [missionName, missionObj] : report.items()) {
        if (!missionObj.is_object()) return false;

        for (const auto& [kernelType, typeObj] : missionObj.items()) {
            if (kernelType == "deps") {
                if (!typeObj.is_array()) return false;
                continue;
            }

            if (!typeObj.is_object()) {
                return false;
            }

            bool hasQuality = typeObj.contains("reconstructed") ||
                                typeObj.contains("smithed") ||
                                typeObj.contains("predicted") ||
                                typeObj.contains("noquality");
            bool hasDirectKernels = typeObj.contains("kernels");

            if (!hasQuality && !hasDirectKernels) {
                return false;
            }
            for (const auto& [key, val] : typeObj.items()) {
                if (key == "kernels") {
                    if (!val.is_array() && !val.is_string()) {
                        return false;
                    } 
                } else {
                    if (!val.is_object() || !val.contains("kernels")) {
                        return false;
                    }
                    const auto& kernels = val["kernels"];
                    if (!kernels.is_array() && !kernels.is_string()) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

TEST(KernelReportSchemaTest, ValidateMessengerExample) {
    nlohmann::json messengerReport = {
        {"mdis", {
            {"ck", {
                {"reconstructed", {{"kernels", {"msgr_1234_v01.bc", "msgr_1235_v02.bc"}}}},
                {"smithed", {{"kernels", {"msgr_mdis_sc0001_usgs_v3.bc"}}}}
            }},
            {"spk", {
                {"reconstructed", {{"kernels", {"msgr_20040803_v1.bsp"}}}}
            }},
            {"fk", {{"kernels", {"msgr_v001.tf", "msgr_v002.tf"}}}},
            {"ik", {{"kernels", {"msgr_mdis_v001.ti"}}}},
            {"pck", {
                {"noquality", {{"kernels", {"pck00010_msgr_v02.tpc"}}}}
            }}
        }},
        {"messenger", {
            {"sclk", {{"kernels", {"messenger_0001.tsc"}}}}
        }}
    };

    if (!matchesSchema(messengerReport)) {
        FAIL() << "Real messenger kernel report doesn't match schema";
    }
}


TEST(KernelReportSchemaTest, ValidateEdgeCases) {
    nlohmann::json emptyReport = nlohmann::json::object();
    if (!matchesSchema(emptyReport)) {
        FAIL() << "Empty report doesn't match schema";
    }

    nlohmann::json emptyKernels = {
        {"lroc", {{"fk", {{"kernels", nlohmann::json::array()}}}}}
    };
    if (!matchesSchema(emptyKernels)) {
        FAIL() << "Empty kernels array doesn't match schema";
    }

    nlohmann::json singleKernel = {
        {"lroc", {{"fk", {{"kernels", "single.tf"}}}}}
    };
    if (!matchesSchema(singleKernel)) {
        FAIL() << "Single kernel string doesn't match schema";
    }

    nlohmann::json withDeps = {
        {"mdis", {
            {"fk", {{"kernels", {"test.tf"}}}},
            {"deps", {"/messenger", "/base"}}
        }}
    };
    if (!matchesSchema(withDeps)) {
        FAIL() << "Report with deps array doesn't match schema";
    }
}

TEST(KernelReportSchemaTest, RejectInvalidStructures) {
    nlohmann::json notObject = nlohmann::json::array();
    
    if (matchesSchema(notObject)) {
        FAIL() << "Array should not match schema (expected object)";
    }

    nlohmann::json missingKernels = {{"lroc", {{"fk", {{"files", {"test.tf"}}}}}}
};
    if (matchesSchema(missingKernels)) {
        FAIL() << "Report with 'files' instead of 'kernels' should not match schema";
    }

    nlohmann::json badKernelsType = {{"lroc", {{"fk", {{"kernels", 123}}}}}};

    if (matchesSchema(badKernelsType)) {
        FAIL() << "Report with numeric kernels value should not match schema";
    }
}

TEST_F(LroKernelSet, ValidateGetTargetStatesKernelReport) {
    vector<double> ets = {110000000, 110000001};
    std::pair<std::vector<std::vector<double>>, nlohmann::json> result = getTargetStates(
        ets, "LRO", "LRO", "J2000", "NONE", "lroc",
        {"smithed"}, {"smithed"}
    );
    std::vector<std::vector<double>> resStates = result.first;
    nlohmann::json kernels = result.second;

    if (!matchesSchema(kernels)) {
        FAIL() << "getTargetStates() kernel report doesn't match schema: " << kernels.dump(2);
    }

    if (kernels.empty()) {
        FAIL() << "getTargetStates() returned empty kernel report";
    }
}

TEST_F(LroKernelSet, ValidateUtcToEtKernelReport) {
    std::pair<double, nlohmann::json> result = utcToEt("2010-01-01T00:00:00");
    double et = result.first;
    nlohmann::json kernels = result.second;

    if (!matchesSchema(kernels)) {
        FAIL() << "utcToEt() kernel report doesn't match schema: " << kernels.dump(2);
    }

    if (kernels.empty()) {
        FAIL() << "utcToEt() returned empty kernel report";
    }
}
