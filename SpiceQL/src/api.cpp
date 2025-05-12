#include <exception>
#include <fstream>

#include <SpiceUsr.h>
#include <SpiceZfc.h>
#include <SpiceZmc.h>

#include <ghc/fs_std.hpp>

#include <fmt/format.h>
#include <fmt/compile.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "query.h"
#include "spice_types.h"
#include "utils.h"
#include "inventory.h"
#include "api.h"
#include "restincurl.h"

using json = nlohmann::json;
using namespace std;

namespace SpiceQL {

  map<string, string> spiceql_mission_map = {
      {"AMICA", "amica"},
      {"CHANDRAYAAN-1_M3", "m3"},
      {"CHANDRAYAAN-1_MRFFR", "mrffr"},
      {"CASSINI_ISS_NAC", "cassini"},
      {"CASSINI_ISS_WAC", "cassini"},
      {"DAWN_FC2_FILTER_1", "fc2"},
      {"DAWN_FC2_FILTER_2", "fc2"},
      {"DAWN_FC2_FILTER_3", "fc2"},
      {"DAWN_FC2_FILTER_4", "fc2"},
      {"DAWN_FC2_FILTER_5", "fc2"},
      {"DAWN_FC2_FILTER_6", "fc2"},
      {"DAWN_FC2_FILTER_7", "fc2"},
      {"DAWN_FC2_FILTER_8", "fc2"},
      {"GLL_SSI_PLATFORM", "galileo"},
      {"HAYABUSA_AMICA", "amica"},
      {"HAYABUSA_NIRS", "nirs"},
      {"HAYABUSA2_ONC-W2", ""},
      {"JUNO_JUNOCAM", "juno"},
      {"JUPITER", "voyager1"},
      {"LRO_LROCNACL", "lroc"},
      {"LRO_LROCNACR", "lroc"},
      {"LRO_LROCWAC_UV", "lroc"},
      {"LRO_LROCWAC_VIS", "lroc"},
      {"LRO_MINIRF", ""},
      {"M10_VIDICON_A", "m10_vidicon_a"},
      {"M10_VIDICON_B", "m10_vidicon_b"},
      {"MARS", "mro"},
      {"MSGR_MDIS_WAC", "mdis"},
      {"MSGR_MDIS_NAC", "mdis"},
      {"MEX_HRSC_SRC", "src"},
      {"MEX_HRSC_IR", "hrsc"},
      {"MGS_MOC_NA", "mgs"},
      {"MGS_MOC_WA_RED", "mgs"},
      {"MGS_MOC_WA_BLUE", "mgs"},
      {"MOON", "apollo15"},
      {"MRO_MARCI_VIS", "marci"},
      {"MRO_MARCI_UV", "marci"},
      {"MRO_CTX", "ctx"},
      {"MRO_HIRISE", "hirise"},
      {"MRO_CRISM_VNIR", "crism"},
      {"NEAR EARTH ASTEROID RENDEZVOUS", ""},
      {"MSL_MASTCAM_RIGHT", ""},
      {"MSL_MASTCAM_LEFT", ""},
      {"NH_LORRI", "lorri"},
      {"NH_RALPH_LEISA", "leisa"},
      {"NH_MVIC", "mvic_tdi"},
      {"ISIS_NH_RALPH_MVIC_METHANE", "mvic_framing"},
      {"THEMIS_IR", "odyssey"},
      {"THEMIS_VIS", "odyssey"},
      {"ORX_OCAMS_MAPCAM", ""},
      {"ORX_OCAMS_POLYCAM", ""},
      {"ORX_OCAMS_SAMCAM", ""},
      {"LISM_MI-VIS1", "kaguya"},
      {"LISM_MI-VIS2", "kaguya"},
      {"LISM_MI-VIS3", "kaguya"},
      {"LISM_MI-VIS4", "kaguya"},
      {"LISM_MI-VIS5", "kaguya"},
      {"LISM_MI-NIR1", "kaguya"},
      {"LISM_MI-NIR2", "kaguya"},
      {"LISM_MI-NIR3", "kaguya"},
      {"LISM_MI-NIR4", "kaguya"},
      {"LISM_TC1_WDF", "kaguya"},
      {"LISM_TC1_WTF", "kaguya"},
      {"LISM_TC1_SDF", "kaguya"},
      {"LISM_TC1_STF", "kaguya"},
      {"LISM_TC1_WDN", "kaguya"},
      {"LISM_TC1_WTN", "kaguya"},
      {"LISM_TC1_SDN", "kaguya"},
      {"LISM_TC1_STN", "kaguya"},
      {"LISM_TC1_WDH", "kaguya"},
      {"LISM_TC1_WTH", "kaguya"},
      {"LISM_TC1_SDH", "kaguya"},
      {"LISM_TC1_STH", "kaguya"},
      {"LISM_TC1_SSH", "kaguya"},
      {"LO1_HIGH_RESOLUTION_CAMERA", ""},
      {"LO2_HIGH_RESOLUTION_CAMERA", ""},
      {"LO3_HIGH_RESOLUTION_CAMERA", ""},
      {"LO4_HIGH_RESOLUTION_CAMERA", ""},
      {"LO5_HIGH_RESOLUTION_CAMERA", ""},
      {"NEPTUNE", "voyager1"}, 
      {"SATURN", "voyager1"},
      {"TGO_CASSIS", "cassis"},
      {"VIKING ORBITER 1", "viking1"},
      {"VIKING ORBITER 2", "viking2"},
      {"VG1_ISSNA", ""},
      {"VG1_ISSWA", ""},
      {"VG2_ISSNA", ""},
      {"VG2_ISSWA", ""},
      {"ULTRAVIOLET/VISIBLE CAMERA", "uvvis"},
      {"Near Infrared Camera", "nir"},
      {"High Resolution Camera", "clementine1"},
      {"Long Wave Infrared Camera", "clementine1"},
      {"Visual and Infrared Spectrometer", "vir"}
    };

    std::string url_encode(const std::string &value) {
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
            std::string::value_type c = (*i);

            // Keep alphanumeric and other accepted characters intact
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '&' || c == '/' || c == '?' || c == '=' || c == ':') {
                escaped << c;
                continue;
            }

            if (c == '"'){
                continue;
            }

            // Any other characters are percent-encoded
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char) c);
            escaped << std::nouppercase;
        }

        return escaped.str();
    }

    json spiceAPIQuery(std::string functionName, json args, std::string method){
        restincurl::Client client;
        // Need to be able to set URL externally
        std::string queryString = getRestUrl() + functionName + "?";

        json j;

        if (method == "GET"){
            SPDLOG_TRACE("spiceAPIQuery GET");
            for (auto x : args.items()) {
                if (x.value().is_null()) {
                    continue;
                }
                queryString+= x.key();
                queryString+= "=";
                queryString+= x.value().dump();
                queryString+= "&";
            }
            SPDLOG_TRACE("queryString = {}", queryString);
            std::string encodedString = url_encode(queryString);
            SPDLOG_TRACE("encodedString = {}", encodedString);
            client.Build()->Get(encodedString).Option(CURLOPT_FOLLOWLOCATION, 1L).AcceptJson().WithCompletion([&](const restincurl::Result& result) {
                if (result.http_response_code != 200) {
                    SPDLOG_DEBUG("[Failed HTTP request] HTTP Code: {}, Message: {}, Payload: {}", result.http_response_code, result.msg, result.body);
                }
                SPDLOG_TRACE("GET result body = {}", result.body);
                j = json::parse(result.body);
            }).ExecuteSynchronous();
        } else {
            SPDLOG_TRACE("POST");
            client.Build()->Post(queryString).Option(CURLOPT_FOLLOWLOCATION, 1L).AcceptJson().WithJson(args.dump()).WithCompletion([&](const restincurl::Result& result) {
                if (result.http_response_code != 200) {
                    SPDLOG_DEBUG("[Failed HTTP request] HTTP Code: {}, Message: {}, Payload: {}", result.http_response_code, result.msg, result.body);
                }
                SPDLOG_TRACE("POST result = {}", result.body);
                j = json::parse(result.body);
            }).ExecuteSynchronous();
        }
        
        client.CloseWhenFinished();
        client.WaitForFinish();

        // Check is JSON is valid
        if (j.is_null() || !json::accept(j.dump())) {
            throw runtime_error("REST API Response is not a valid JSON.");
        }

        // Check for successful call
        if (!(j["statusCode"] == 200)) {
            throw runtime_error("REST API Error Response: [" + j["body"].dump() + "]");
        }

        return j;
    }


    pair<vector<vector<double>>, json> getTargetStates(vector<double> ets, string target, string observer, string frame, string abcorr, string mission, vector<string> ckQualities, vector<string> spkQualities, bool useWeb, bool searchKernels, vector<string> kernelList) {
        SPDLOG_TRACE("Calling getTargetStates with {}, {}, {}, {}, {}, {}, {}, {}, {}, {}", ets.size(), target, observer, frame, abcorr, mission, ckQualities.size(), spkQualities.size(), useWeb, searchKernels, kernelList.size());

        if (useWeb) {
            // @TODO validity checks
            json args = json::object({
                {"target", target},
                {"observer", observer},
                {"frame", frame},
                {"abcorr", abcorr},
                {"ets", ets},
                {"mission", mission},
                {"ckQualities", ckQualities},
                {"spkQualities", spkQualities},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
                });
            // @TODO check that json exists / contains what we're looking for
            json out  = spiceAPIQuery("getTargetStates", args);
            vector<vector<double>> kvect = json2DFloatArrayTo2DVector(out["body"]["return"]);
            return make_pair(kvect, out["body"]["kernels"]);
        }

        if (ets.size() < 1) {
            throw invalid_argument("No ephemeris times given."); 
        }

        json ephemKernels = {};

        if (searchKernels) {
            ephemKernels = Inventory::search_for_kernelsets({mission, target, observer, "base"}, {"sclk", "ck", "spk", "pck", "tspk", "lsk", "fk"}, ets.front(), ets.back(), ckQualities, spkQualities);
            SPDLOG_DEBUG("{} Kernels : {}", mission, ephemKernels.dump(4));
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(ephemKernels, regexk);
        }

        auto start = std::chrono::high_resolution_clock::now();
        KernelSet ephemSet(ephemKernels);

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        SPDLOG_INFO("Time in std::chrono::microseconds to furnish kernel sets: {}", duration.count());

        start = std::chrono::high_resolution_clock::now();
        vector<vector<double>> lt_stargs;
        vector<double> lt_starg;
        for (auto et: ets) {
            lt_starg = getTargetState(et, target, observer, frame, abcorr);
            lt_stargs.push_back(lt_starg);
        }

        stop = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        SPDLOG_INFO("Time in std::chrono::microseconds to get data results: {}", duration.count());

        return {lt_stargs, ephemKernels};
    }


    pair<vector<vector<double>>, json> getTargetOrientations(vector<double> ets, int toFrame, int refFrame, string mission, vector<string> ckQualities, bool useWeb, bool searchKernels, vector<string> kernelList) {
        SPDLOG_TRACE("Calling getTargetOrientations with {}, {}, {}, {}, {}, {}, {}, {}", ets.size(), toFrame, refFrame, mission, ckQualities.size(), useWeb, searchKernels, kernelList.size());

        if (useWeb){
            json args = json::object({
                {"ets", ets},
                {"toFrame", toFrame},
                {"refFrame", refFrame},
                {"mission", mission},
                {"ckQualities", ckQualities},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("getTargetOrientations", args);
            vector<vector<double>> kvect = json2DFloatArrayTo2DVector(out["body"]["return"]);
            return make_pair(kvect, out["body"]["kernels"]);
        }

        if (ets.size() < 1) {
            throw invalid_argument("No ephemeris times given.");
        }

        json ephemKernels = {};

        if (searchKernels) {
            ephemKernels = Inventory::search_for_kernelsets({mission, "base"}, {"sclk", "ck", "pck", "fk", "lsk", "tspk"}, ets.front(), ets.back(), ckQualities, {"noquality"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(ephemKernels, regexk);
        }

        auto start = std::chrono::high_resolution_clock::now();
        KernelSet ephemSet(ephemKernels);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        SPDLOG_INFO("Time in std::chrono::microseconds to furnish kernel sets: {}", duration.count());

        start = std::chrono::high_resolution_clock::now();
        vector<vector<double>> orientations = {};
        vector<double> orientation;
        for (auto et: ets) {
            orientation = getTargetOrientation(et, toFrame, refFrame);
            orientations.push_back(orientation);
        }
        stop = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        SPDLOG_INFO("Time in std::chrono::microseconds to get data results: {}", duration.count());

        return {orientations, ephemKernels};
    }


    pair<double, json> strSclkToEt(int frameCode, string sclk, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        SPDLOG_TRACE("calling strSclkToEt({}, {}, {}, {}, {}, {})", frameCode, sclk, mission, useWeb, searchKernels, kernelList.size());

        if (useWeb) {
            json args = json::object({
                {"frameCode", frameCode},
                {"sclk", sclk},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("strSclkToEt", args);
            double result = out["body"]["return"].get<double>();
            return make_pair(result, out["body"]["kernels"]);
        }

        json ephemKernels;
        if (searchKernels) {
            ephemKernels = Inventory::search_for_kernelsets({"base", mission}, {"lsk", "fk", "sclk"}); 
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(ephemKernels, regexk);
        }

        KernelSet kSet(ephemKernels);
        
        SpiceDouble et;
        checkNaifErrors();
        try {
            scs2e_c(frameCode, sclk.c_str(), &et);
            checkNaifErrors();
            SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);
        }
        catch(exception &e) { 
            // we want the platforms code, if they passs in an instrument code (e.g. -85600), truncate it to (-85)
            frameCode = (abs(frameCode / 1000) > 0) ? frameCode/1000 : frameCode;
            scs2e_c(frameCode, sclk.c_str(), &et);
            checkNaifErrors();
            SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et); 
        }
        return {et, ephemKernels};
    }


   pair<string, json> doubleEtToSclk(int frameCode, double et, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        SPDLOG_TRACE("calling doubleEtToSclk({}, {}, {}, {}, {}, {})", frameCode, et, mission, useWeb, searchKernels, kernelList.size());

        json ephemKernels;

        if (useWeb) {
            json args = json::object({
                {"frameCode", frameCode},
                {"et", et},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("doubleEtToSclk", args);
            string result = out["body"]["return"].get<string>();
            return make_pair(result, out["body"]["kernels"]);
        }

        if (searchKernels) {
          ephemKernels = Inventory::search_for_kernelsets({"base", mission}, {"fk", "lsk", "sclk"}); 
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(ephemKernels, regexk);
        }

        KernelSet sclkSet(ephemKernels);

        SpiceChar sclk[100];
        checkNaifErrors();
        sce2s_c(frameCode, et, 100, sclk);
        checkNaifErrors();
        SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);

        return make_pair(string(sclk), ephemKernels);
   }


    pair<double, json> doubleSclkToEt(int frameCode, double sclk, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {

        if (useWeb){
            json args = json::object({
                {"frameCode", frameCode},
                {"sclk", sclk},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("doubleSclkToEt", args);
            double result = out["body"]["return"].get<double>();
            return make_pair(result, out["body"]["kernels"]);
        }
        
        json sclks;

        if (searchKernels) {
            sclks = Inventory::search_for_kernelsets({"base", mission}, {"lsk", "fk", "sclk"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(sclks, regexk);
        }

        KernelSet sclkSet(sclks);
        
        // we want the platforms code, if they passs in an instrument code (e.g. -85600), truncate it to (-85)
        frameCode = (abs(frameCode / 1000) > 0) ? frameCode/1000 : frameCode; 

        SpiceDouble et;
        checkNaifErrors();
        sct2e_c(frameCode, sclk, &et);
        checkNaifErrors();
        SPDLOG_DEBUG("strsclktoet({}, {}, {}) -> {}", frameCode, mission, sclk, et);

        return {et, sclks};
    }


    pair<double, json> utcToEt(string utc, bool useWeb, bool searchKernels, vector<string> kernelList) {
        
        if (useWeb){
            json args = json::object({
                {"utc", utc},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("utcToEt", args);
            double result = out["body"]["return"].get<double>();
            return make_pair(result, out["body"]["kernels"]);
        }

        json lsks = {};

        // get lsk kernel
        if (searchKernels) {
            lsks = Inventory::search_for_kernelset("base", {"lsk"});
        }
        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(lsks, regexk);
        }
        
        KernelSet lsk(lsks);

        SpiceDouble et;
        checkNaifErrors();
        str2et_c(utc.c_str(), &et);
        checkNaifErrors();

        return {et, lsks};
    }


    pair<string, json> etToUtc(double et, string format, double precision, bool useWeb, bool searchKernels, vector<string> kernelList) {
    
        if (useWeb){
            json args = json::object({
                {"et", et},
                {"format", format},
                {"precision", precision},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("etToUtc", args);
            string result = out["body"]["return"].get<string>();
            return make_pair(result, out["body"]["kernels"]);
        }   
       
        json lsks = {};

        // get lsk kernel
        if (searchKernels) {
            lsks = Inventory::search_for_kernelset("base", {"lsk"});
        }
        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(lsks, regexk);
        }

        KernelSet lsk(lsks);

        SpiceChar utc_spice[100];
        checkNaifErrors();
        et2utc_c(et, format.c_str(), precision, 100, utc_spice);
        checkNaifErrors();
        string utc_string(utc_spice);
        return {utc_string, lsks};
    }


    pair<int, json> translateNameToCode(string frame, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {    
        
        if (useWeb){
            json args = json::object({
                {"frame", frame},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("translateNameToCode", args);
            int result = out["body"]["return"].get<int>();
            return make_pair(result, out["body"]["kernels"]);
        }
        
        SpiceInt code;
        SpiceBoolean found;
        json kernelsToLoad = {};

        if (mission != "" && searchKernels) {
            kernelsToLoad = Inventory::search_for_kernelset(mission, {"fk", "ik"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(kernelsToLoad, regexk);
        }

        KernelSet kset(kernelsToLoad);

        checkNaifErrors();
        bodn2c_c(frame.c_str(), &code, &found);
        checkNaifErrors();

        if (!found) {
            namfrm_c(frame.c_str(), &code);
            checkNaifErrors();
        }

        if (code == 0) {
            throw invalid_argument(fmt::format("Frame code for frame name [{}] not found.", frame));
        }

        return {code, kernelsToLoad};
    }


    pair<string, json> translateCodeToName(int frame, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        
        if (useWeb){
            json args = json::object({
                {"frame", frame},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("translateCodeToName", args);
            string result = out["body"]["return"].get<string>();
            return make_pair(result, out["body"]["kernels"]);
        }

        SpiceChar name[128];
        SpiceBoolean found;
        json kernelsToLoad = {};

        if (mission != "" && searchKernels){
            kernelsToLoad = Inventory::search_for_kernelset(mission, {"fk"});
        }
        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(kernelsToLoad, regexk);
        }

        KernelSet kset(kernelsToLoad);

        checkNaifErrors();
        bodc2n_c(frame, 128, name, &found);
        checkNaifErrors();

        if(!found) {  
            frmnam_c(frame, 128, name);
            checkNaifErrors();
        }

        if(strlen(name) == 0) {
            throw invalid_argument(fmt::format("Frame name for code {} not found.", frame));
        }

        return {string(name), kernelsToLoad};
    }


    pair<vector<int>, json> getFrameInfo(int frame, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        
        if (useWeb){
            json args = json::object({
                {"frame", frame},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("getFrameInfo", args);
            vector<int> result = jsonIntArrayToVector(out["body"]["return"]);
            return make_pair(result, out["body"]["kernels"]);
        }

        SpiceInt cent;
        SpiceInt frclss;
        SpiceInt clssid;
        SpiceBoolean found;

        json kernelsToLoad = {};

        if (mission != "" && searchKernels) {
            // Load only the FKs
            kernelsToLoad = Inventory::search_for_kernelset(mission, {"fk"});
        }
        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(kernelsToLoad, regexk);
        }
        KernelSet kset(kernelsToLoad);

        checkNaifErrors();
        frinfo_c(frame, &cent, &frclss, &clssid, &found);
        checkNaifErrors();
        SPDLOG_TRACE("RETURN FROM FRINFO: {}, {}, {}, {}", cent, frclss, clssid, found);

        if (!found) {
            throw invalid_argument(fmt::format("Frame info for code {} not found.", frame));
        }

        return {{cent, frclss, clssid}, kernelsToLoad};
    }


    pair<json, json> getTargetFrameInfo(int targetId, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        
        if (useWeb){
            json args = json::object({
                {"targetId", targetId},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("getTargetFrameInfo", args);
            json result = out["body"]["return"];
            return make_pair(result, out["body"]["kernels"]);
        }

        SpiceInt frameCode;
        SpiceChar frameName[128];
        SpiceBoolean found;

        json frameInfo;
        json kernelsToLoad = {};

        if (mission != "" && searchKernels) {
            kernelsToLoad = Inventory::search_for_kernelsets({mission, "base"}, {"fk"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(kernelsToLoad, regexk);
        }

        KernelSet kSet(kernelsToLoad);

        checkNaifErrors();
        cidfrm_c(targetId, 128, &frameCode, frameName, &found);
        checkNaifErrors();

        if(!found) {  
            throw invalid_argument(fmt::format("Frame info for target id {} not found.", targetId));
        }

        frameInfo["frameCode"] = frameCode;
        frameInfo["frameName"] = frameName;

        return {frameInfo, kernelsToLoad};
    }


    pair<json, json> findMissionKeywords(string key, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        
        if (useWeb){
            json args = json::object({
                {"key", key},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("findMissionKeywords", args);
            json result = out["body"]["return"];
            return make_pair(result, out["body"]["kernels"]);
        }

        json translationKernels = {};

        if (mission != "" && searchKernels) {
            translationKernels = Inventory::search_for_kernelset(mission, {"iak", "fk", "ik"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(translationKernels, regexk);
        }

        KernelSet kset(translationKernels);

        return {findKeywords(key), translationKernels};
    }


    pair<json, json> findTargetKeywords(string key, string mission, bool useWeb, bool searchKernels, vector<string> kernelList) {
        
        if (useWeb){
            json args = json::object({
                {"key", key},
                {"mission", mission},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("findTargetKeywords", args);
            json result = out["body"]["return"];
            return make_pair(result, out["body"]["kernels"]);
        }
        
        json kernelsToLoad = {};

        if (mission != "" && searchKernels) {
            kernelsToLoad = Inventory::search_for_kernelsets({mission, "base"}, {"pck"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(kernelsToLoad, regexk);
        }

        KernelSet kSet(kernelsToLoad);
        return {findKeywords(key), kernelsToLoad};
    }


    pair<vector<vector<int>>, json> frameTrace(double et, int initialFrame, string mission, vector<string> ckQualities, bool useWeb, bool searchKernels, vector<string> kernelList) {
        checkNaifErrors();

        if (useWeb){
            json args = json::object({
                {"et", et},
                {"initialFrame", initialFrame},
                {"mission", mission},
                {"ckQualities", ckQualities},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("frameTrace", args);
            vector<vector<int>> kvect = json2DIntArrayTo2DVector(out["body"]["return"]);
            return make_pair(kvect, out["body"]["kernels"]);
        }

        json ephemKernels;

        if (searchKernels) {
            ephemKernels = Inventory::search_for_kernelsets({mission, "base"}, {"sclk", "ck", "pck", "fk", "lsk", "tspk"}, et, et, ckQualities, {"noquality"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(ephemKernels, regexk);
        }
        KernelSet ephemSet(ephemKernels);

        checkNaifErrors();
        // The code for this method was extracted from the Naif routine rotget written by N.J. Bachman &
        //   W.L. Taber (JPL)
        int           center;
        int           type;
        int           typid;
        SpiceBoolean  found;
        int           frmidx;  // Frame chain index for current frame
        SpiceInt      nextFrame;   // Naif frame code of next frame
        int           J2000Code = 1;
        checkNaifErrors();
        vector<int> frameCodes;
        vector<int> frameTypes;
        vector<int> constantFrames;
        vector<int> timeFrames;
        frameCodes.push_back(initialFrame);
        frinfo_c((SpiceInt)frameCodes[0],
                    (SpiceInt *)&center,
                    (SpiceInt *)&type,
                    (SpiceInt *)&typid, &found);
        frameTypes.push_back(type);

        while (frameCodes[frameCodes.size() - 1] != J2000Code) {
            frmidx  =  frameCodes.size() - 1;
            // First get the frame type  (Note:: we may also need to save center if we use dynamic frames)
            // (Another note:  the type returned is the Naif from type.  This is not quite the same as the
            // SpiceRotation enumerated FrameType.  The SpiceRotation FrameType differentiates between
            // pck types.  FrameTypes of 2, 6, and 7 will all be considered to be Naif frame type 2.  The
            // logic for FrameTypes in this method is correct for all types except type 7.  Current pck
            // do not exercise this option.  Should we ever use pck with a target body not referenced to
            // the J2000 frame and epoch, both this method and loadPCFromSpice will need to be modified.
            frinfo_c((SpiceInt) frameCodes[frmidx],
                    (SpiceInt *) &center,
                    (SpiceInt *) &type,
                    (SpiceInt *) &typid, &found);

            if (!found) {
            string msg = "The frame " + to_string(frameCodes[frmidx]) + " is not supported by Naif";
            throw logic_error(msg);
            }

            double matrix[3][3];

            // To get the next link in the frame chain, use the frame type
            // 1 = INTERNAL, 2 = PCK
            if (type == 1 ||  type == 2) {
            nextFrame = J2000Code;
            }
            // 3 = CK
            else if (type == 3) {
            ckfrot_((SpiceInt *) &typid, &et, (double *) matrix, &nextFrame, (logical *) &found);

            if (!found) {
                string msg = "The ck rotation from frame " + to_string(frameCodes[frmidx]) + " can not "
                    + "be found due to no pointing available at requested time or a problem with the "
                    + "frame";
                throw logic_error(msg);
            }
            }
            // 4 = TK
            else if (type == 4) {
            tkfram_((SpiceInt *) &typid, (double *) matrix, &nextFrame, (logical *) &found);
            if (!found) {
                string msg = "The tk rotation from frame " + to_string(frameCodes[frmidx]) +
                            " can not be found";
                throw logic_error(msg);
            }
            }
            // 5 = DYN
            else if (type == 5) {
            //
            //        Unlike the other frame classes, the dynamic frame evaluation
            //        routine ZZDYNROT requires the input frame ID rather than the
            //        dynamic frame class ID. ZZDYNROT also requires the center ID
            //        we found via the FRINFO call.

            zzdynrot_((SpiceInt *) &typid, (SpiceInt *) &center, &et, (double *) matrix, &nextFrame);
            }

            else {
            string msg = "The frame " + to_string(frameCodes[frmidx]) +
                            " has a type " + to_string(type) + " not supported by your version of Naif Spicelib. " + 
                            "You need to update.";
            throw logic_error(msg);
            }
            frameCodes.push_back(nextFrame);
            frameTypes.push_back(type);
        }
        SPDLOG_TRACE("All Frame Chain Codes: {}", fmt::join(frameCodes, ", "));

        constantFrames.clear();
        // 4 = TK
        while (frameCodes.size() > 0) {
            if (frameTypes[0] == 4) {
            constantFrames.push_back(frameCodes[0]);
            frameCodes.erase(frameCodes.begin());
            frameTypes.erase(frameTypes.begin());
            }
            else {
            break;
            }
        }

        if (constantFrames.size() != 0) {
            timeFrames.push_back(constantFrames[constantFrames.size() - 1]);
        }

        for (int i = 0;  i < (int) frameCodes.size(); i++) {
            timeFrames.push_back(frameCodes[i]);
        }
        SPDLOG_TRACE("Time Dependent Frame Chain Codes: {}", fmt::join(timeFrames, ", "));
        SPDLOG_TRACE("Constant Frame Chain Codes: {}", fmt::join(constantFrames, ", "));
        checkNaifErrors();

        vector<vector<int>> res = {timeFrames, constantFrames};
        return {res, ephemKernels};
    }


    pair<vector<double>, json> extractExactCkTimes(double observStart, double observEnd, int targetFrame, string mission, vector<string> ckQualities, bool useWeb, bool searchKernels, vector<string> kernelList) {
        SPDLOG_TRACE("Calling extractExactCkTimes with {}, {}, {}, {}, {}, {}, {}", observStart, observEnd, targetFrame, mission, ckQualities.size(), useWeb, searchKernels);
        
        if (useWeb){
            json args = json::object({
                {"observStart", observStart},
                {"observEnd", observEnd},
                {"targetFrame",targetFrame},
                {"mission", mission},
                {"ckQualities", ckQualities},
                {"searchKernels", searchKernels},
                {"kernelList", kernelList}
            });
            json out = spiceAPIQuery("extractExactCkTimes", args);
            vector<double> kvect = jsonDoubleArrayToVector(out["body"]["return"]);
            return make_pair(kvect, out["body"]["kernels"]);
        }

        json missionJson;
        json ephemKernels = {};

        if (searchKernels) {
            ephemKernels = Inventory::search_for_kernelsets({mission, "base"}, {"ck", "sclk", "lsk"}, observStart, observEnd, ckQualities, {"noquality"});
        }

        if (!kernelList.empty()) {
            json regexk = Inventory::search_for_kernelset_from_regex(kernelList);
            // merge them into the ephem kernels overwriting anything found in the query
            merge_json(ephemKernels, regexk);
        }

        KernelSet ephemSet(ephemKernels);

        int count = 0;

        //  Next line added 12-03-2009 to allow observations to cross segment boundaries
        double currentTime = observStart;
        bool timeLoaded = false;

        // Get number of ck loaded for this rotation.  This method assumes only one SpiceRotation
        // object is loaded.
        checkNaifErrors();
        ktotal_c("ck", (SpiceInt *)&count);

        if (count > 1) {
            std::string msg = "Unable to get exact CK record times when more than 1 CK is loaded, Aborting";
            throw std::runtime_error(msg);
        }
        else if (count < 1) {
            std::string msg = "No CK kernels loaded, Aborting";
            throw std::runtime_error(msg);
        }

        // case of a single ck -- read instances and data straight from kernel for given time range
        SpiceInt handle;

        // Define some Naif constants
        int FILESIZ = 128;
        int TYPESIZ = 32;
        int SOURCESIZ = 128;
        //      double DIRSIZ = 100;

        SpiceChar file[FILESIZ];
        SpiceChar filtyp[TYPESIZ]; // kernel type (ck, ek, etc.)
        SpiceChar source[SOURCESIZ];

        SpiceBoolean found;
        bool observationSpansToNextSegment = false;

        double segStartEt;
        double segStopEt;

        kdata_c(0, "ck", FILESIZ, TYPESIZ, SOURCESIZ, file, filtyp, source, &handle, &found);
        dafbfs_c(handle);
        daffna_c(&found);
        int spCode = ((int)(targetFrame / 1000)) * 1000;
        std::vector<double> cacheTimes = {};

        while (found) {
            observationSpansToNextSegment = false;
            double sum[10]; // daf segment summary
            double dc[2];   // segment starting and ending times in tics
            SpiceInt ic[6]; // segment summary values:
            // instrument code for platform,
            // reference frame code,
            // data type,
            // velocity flag,
            // offset to quat 1,
            // offset to end.
            dafgs_c(sum);
            dafus_c(sum, (SpiceInt)2, (SpiceInt)6, dc, ic);

            // Don't read type 5 ck here
            if (ic[2] == 5)
                break;

            // Check times for type 3 ck segment if spacecraft matches
            if (ic[0] == spCode && ic[2] == 3) {
                sct2e_c((int)spCode / 1000, dc[0], &segStartEt);
                sct2e_c((int)spCode / 1000, dc[1], &segStopEt);
                checkNaifErrors();
                double et;

                // Get times for this segment
                if (currentTime >= segStartEt && currentTime <= segStopEt) {

                    // Check for a gap in the time coverage by making sure the time span of the observation
                    //  does not cross a segment unless the next segment starts where the current one ends
                    if (observationSpansToNextSegment && currentTime > segStartEt) {
                        std::string msg = "Observation crosses segment boundary--unable to interpolate pointing";
                        throw std::runtime_error(msg);
                    }
                    if (observEnd > segStopEt) {
                        observationSpansToNextSegment = true;
                    }

                    // Extract necessary header parameters
                    int dovelocity = ic[3];
                    int end = ic[5];
                    double val[2];
                    dafgda_c(handle, end - 1, end, val);
                    //            int nints = (int) val[0];
                    int ninstances = (int)val[1];
                    int numvel = dovelocity * 3;
                    int quatnoff = ic[4] + (4 + numvel) * ninstances - 1;
                    //            int nrdir = (int) (( ninstances - 1 ) / DIRSIZ); /* sclkdp directory records */
                    int sclkdp1off = quatnoff + 1;
                    int sclkdpnoff = sclkdp1off + ninstances - 1;
                    //            int start1off = sclkdpnoff + nrdir + 1;
                    //            int startnoff = start1off + nints - 1;
                    int sclkSpCode = spCode / 1000;

                    // Now get the times
                    std::vector<double> sclkdp(ninstances);
                    dafgda_c(handle, sclkdp1off, sclkdpnoff, (SpiceDouble *)&sclkdp[0]);

                    int instance = 0;
                    sct2e_c(sclkSpCode, sclkdp[0], &et);

                    while (instance < (ninstances - 1) && et < currentTime) {
                        instance++;
                        sct2e_c(sclkSpCode, sclkdp[instance], &et);
                    }

                    if (instance > 0)
                        instance--;
                    sct2e_c(sclkSpCode, sclkdp[instance], &et);

                    while (instance < (ninstances - 1) && et < observEnd) {
                        cacheTimes.push_back(et);
                        instance++;
                        sct2e_c(sclkSpCode, sclkdp[instance], &et);
                    }
                    cacheTimes.push_back(et);

                    if (!observationSpansToNextSegment) {
                        timeLoaded = true;
                        break;
                    }
                    else {
                        currentTime = segStopEt;
                    }
                }
            }
            dafcs_c(handle);  // Continue search in daf last searched
            daffna_c(&found); // Find next forward array in current daf
        }

        return {cacheTimes, ephemKernels};
    }
}
