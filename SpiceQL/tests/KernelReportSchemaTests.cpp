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

    const std::set<std::string> validKernelTypes = {
        "ck", "spk", "tspk", "fk", "sclk", "lsk", "ik", "iak", "dsk", "pck", "ek"
    };

    for (const auto& [key, value] : report.items()) {
        if (validKernelTypes.count(key)) {
            if (!value.is_array()) {
                return false;
            }
            for (const auto& item : value) {
                if (!item.is_string()) {
                    return false;
                }
            }
        }
        else if (key.find("_ck_quality") != std::string::npos ||
                 key.find("_spk_quality") != std::string::npos) {
            if (!value.is_string()) {
                return false;
            }
            std::string qualityStr = value.get<std::string>();
            if (qualityStr != "reconstructed" && qualityStr != "predicted" &&
                qualityStr != "smithed" && qualityStr != "noquality") {
                return false;
            }
        }
        else {
            return false;
        }
    }
    return true;
}

TEST(KernelReportSchemaTest, ValidateMessengerExample) {
    nlohmann::json messengerReport = {
        {"ck", {"msgr_1234_v01.bc", "msgr_1235_v02.bc", "msgr_mdis_sc0001_usgs_v3.bc"}},
        {"spk", {"msgr_20040803_v1.bsp"}},
        {"fk", {"msgr_v001.tf", "msgr_v002.tf"}},
        {"ik", {"msgr_mdis_v001.ti"}},
        {"pck", {"pck00010_msgr_v02.tpc"}},
        {"sclk", {"messenger_0001.tsc"}},
        {"mdis_ck_quality", "smithed"},
        {"mdis_spk_quality", "reconstructed"}
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
        {"fk", nlohmann::json::array()}
    };
    if (!matchesSchema(emptyKernels)) {
        FAIL() << "Empty kernels array doesn't match schema";
    }

    nlohmann::json withQuality = {
        {"fk", {"test.tf"}},
        {"spk", {"test.bsp"}},
        {"lroc_spk_quality", "smithed"}
    };
    if (!matchesSchema(withQuality)) {
        FAIL() << "Report with quality field doesn't match schema";
    }
}

TEST(KernelReportSchemaTest, RejectInvalidStructures) {
    nlohmann::json notObject = nlohmann::json::array();

    if (matchesSchema(notObject)) {
        FAIL() << "Array should not match schema (expected object)";
    }

    nlohmann::json unknownKey = {{"unknown_key", {"test.tf"}}};
    if (matchesSchema(unknownKey)) {
        FAIL() << "Report with unknown key should not match schema";
    }

    nlohmann::json badKernelsType = {{"fk", 123}};
    if (matchesSchema(badKernelsType)) {
        FAIL() << "Report with numeric kernels value should not match schema";
    }

    nlohmann::json badQualityValue = {
        {"spk", {"test.bsp"}},
        {"lroc_spk_quality", "invalid_quality"}
    };
    if (matchesSchema(badQualityValue)) {
        FAIL() << "Report with invalid quality value should not match schema";
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
