#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace SpiceQL {

    extern nlohmann::json aliasMap;
    
    /**
     * @brief Accessor for the aliasMap.
     * @return const reference to the aliasMap JSON object.
     */
    inline const nlohmann::json& getAliasMap() {
        return aliasMap;
    }


    /**
     * @brief Adds or updates a key-value pair in the aliasMap.
     * @param key The key to add or update.
     * @param value The value to associate with the key.
     */
    void addAliasKey(const std::string& key, const std::string& value);

    
    /**
     * @brief Setter for the aliasMap.
     * @param newAliasMap The new JSON object to set as the aliasMap.
     */
    inline void setAliasMap(const nlohmann::json& newAliasMap) {
        aliasMap = newAliasMap;
    }

    std::string getSpiceqlName(const std::string& name);
    
    std::string url_encode(const std::string &value);
    nlohmann::json spiceAPIQuery(std::string functionName, nlohmann::json args, std::string method="GET");
    
    /**
     * @brief Gives the positions and velocities for a given frame given a set of ephemeris times
     *
     * Mostly a C++ wrap for NAIF's spkezr_c
     *
     * @param ets ephemeris times at which you want to obtain the target state
     * @param target NAIF ID for the target frame
     * @param observer NAIF ID for the observing frame
     * @param frame The reference frame in which to get the positions in
     * @param abcorr aborration correction flag, default it NONE.
     *        This can set to:
     *           "NONE" - No correction
     *        For the "reception" case, i.e. photons from the target being recieved by the observer at the given time.
     *           "LT"   - One way light time correction
     *           "LT+S" - Correct for one-way light time and stellar aberration correction
     *           "CN"   - Converging Newtonian light time correction
     *           "CN+S" - Converged Newtonian light time correction and stellar aberration correction
     *        For the "transmission" case, i.e. photons emitted from the oberver hitting at target at the given time
     *           "XLT"   - One-way light time correction using a newtonian formulation
     *           "XLT+S" - One-way light time and stellar aberration correction using a newtonian formulation
     *           "XCN"   - converged Newtonian light time correction
     *           "XCN+S" - converged Newtonian light time correction and stellar aberration correction.
     * @param mission Config subset as it relates to the mission
     * @param ckQualities vector of strings describing the quality of cks to try and obtain
     * @param spkQualities string of strings describing the quality of spks to try and obtain
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     * 
     * @see SpiceQL::getTargetState
     * @see Kernel::Quality
     *
     * @return A vector of vectors with a Nx7 state vector of positions and velocities in x,y,z,vx,vy,vz format followed by the light time adjustment.
     **/
    std::pair<std::vector<std::vector<double>>, nlohmann::json> getTargetStates(
        std::vector<double> ets,
        std::string target,
        std::string observer,
        std::string frame,
        std::string abcorr,
        std::string mission,
        std::vector<std::string> ckQualities={"smithed", "reconstructed"},
        std::vector<std::string> spkQualities={"smithed", "reconstructed"},
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});
    
    /**
     * @brief Gives quaternion and angular velocity for a given frame at a set of ephemeris times
     *
     * Gets orientations for an input frame in some reference frame.
     * The orientations returned from this function can be used to transform a position
     * in the source frame to the ref frame.
     *
     * @param ets ephemeris times at which you want to optain the target pointing
     * @param toframe the source frame's NAIF code.
     * @param refframe the reference frame's NAIF code, orientations are relative to this reference frame
     * @param mission Config subset as it relates to the mission
     * @param ckQualities vector of string describing the quality of cks to try and obtain
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @see SpiceQL::getTargetOrientation
     *
     * @returns Vector of SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with)
     **/
    std::pair<std::vector<std::vector<double>>, nlohmann::json> getTargetOrientations(
        std::vector<double> ets,
        int toFrame,
        int refFrame,
        std::string mission,
        std::vector<std::string> ckQualities={"smithed", "reconstructed"},
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});
    
    /**
     * @brief Converts a given string spacecraft clock time to an ephemeris time
     *
     * Given a known frame code strSclkToEt converts a given spacecraft clock time as a string
     * to an ephemeris time. Call this function if your clock time looks something like:
     * 1/281199081:48971
     *
     * @param frameCode int Frame id to use
     * @param sclk string Spacecraft Clock formatted as a string
     * @param mission string Mission name as it relates to the config files
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load     
     * @return double
     */
    std::pair<double, nlohmann::json> strSclkToEt(
        int frameCode,
        std::string sclk,
        std::string mission,
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    /**
     * @brief Converts a given double spacecraft clock time to an ephemeris time
     *
     * Given a known frame code doubleSclkToEt converts a given spacecraft clock time as a double
     * to an ephemeris time. Call this function if your clock time looks something like:
     * 922997380.174174
     *
     * @param frameCode int Frame id to use
     * @param sclk int Spacecraft Clock formatted as an int
     * @param mission string Mission name as it relates to the config files
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     * @return double
     */
    std::pair<double, nlohmann::json> doubleSclkToEt(
        int frameCode,
        double sclk,
        std::string mission,
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    /**
     * @brief Converts a given double spacecraft clock time to an ephemeris time
     *
     * Given a known frame code doubleSclkToEt converts a given spacecraft clock time as a double
     * to an ephemeris time. Call this function if your clock time looks something like:
     * 922997380.174174
     *
     * @param frameCode int Frame id to use
     * @param et double Spacecraft ephemeris time to convert to an SCLK
     * @param mission string Mission name as it relates to the config files
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     * @return double
     */
    std::pair<std::string, nlohmann::json> doubleEtToSclk(
        int frameCode,
        double et,
        std::string mission,
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    /**
     * @brief convert a UTC string to an ephemeris time
     *
     * Basically a wrapper around NAIF's cspice str2et function except it also temporarily loads the required kernels.
     * See Also: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/str2et_c.html
     *
     * @param et UTC string, e.g. "1988 June 13, 12:29:48 TDB"
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @returns double precision ephemeris time
     **/
    std::pair<double, nlohmann::json> utcToEt(
        std::string utc,
        bool useWeb=false,
        bool searchKernels=true,
        bool fullKernelPath=false,
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    /**
     * @brief convert et string to a UTC string
     *
     * Basically a wrapper around NAIF's cspice et2utc_c function except it also temporarily loads the required kernels.
     * See Also: https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/et2utc_c.html
     *
     * @param et ephemeris time
     * @param precision number of decimal 
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @returns double precision ephemeris time
     **/
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

    /**
     * @brief Switch between NAIF frame string name to integer frame code
     *
     * See <a href="https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html">NAIF's Docs on frame codes</a> for more information
     *
     * @param frame String frame name to translate to a NAIF code
     * @param mission Mission name as it relates to the config files
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     * 
     * @return integer Naif frame code
     **/
    std::pair<int, nlohmann::json> translateNameToCode(
        std::string frame, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1, 
        std::vector<std::string> kernelList={});

    /**
     * @brief Switch between NAIF frame integer code to string frame name
     *
     * See <a href="https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html">NAIF's Docs on frame codes</a> for more information
     *
     * @param frame int NAIF frame code to translate
     * @param searchKernels bool Whether to search the kernels for the user
     * @param mission Mission name as it relates to the config files
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @return string Naif frame name
     **/
    std::pair<std::string, nlohmann::json> translateCodeToName(
        int frame, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    /**
     * @brief Get the center, class id, and class of a given frame
     *
     * See <a href="https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html">NAIF's Docs on frame codes</a> for more information
     *
     * @param frame String frame name to translate to a NAIF code
     * @param mission Mission name as it relates to the config files
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @return 3 element vector of the given frames center, class id, and class
     **/
    std::pair<std::vector<int>, nlohmann::json> getFrameInfo(
        int frame, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

     /**
    * @brief returns frame name and frame code associated to the target ID.
    *
    *  Takes in a target id and returns the frame name and frame code in json format
    * 
    * @param targetId target ID
    * @param mission mission name as it relates to the config files
    * @param searchKernels bool Whether to search the kernels for the user
    * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
    * @param limitCk int number of cks to limit to, default is -1 to retrieve all
    * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
    * @param kernelList vector<string> vector of additional kernels to load 
    * 
    * @returns json of frame name and frame code
    **/
    std::pair<nlohmann::json, nlohmann::json> getTargetFrameInfo(
        int targetId, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    /**
    * @brief returns kernel values for a specific mission in the form of a json
    *
    *  Takes in a kernel key and returns the value associated with the inputted mission as a json
    * 
    * @param key key - Kernel to get values from 
    * @param mission mission name
    * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
    * @param limitCk int number of cks to limit to, default is -1 to retrieve all
    * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
    * @param searchKernels bool Whether to search the kernels for the user
    * @returns json of values
    **/
    std::pair<nlohmann::json, nlohmann::json> findMissionKeywords(
        std::string key, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    /**
    * @brief returns Target values in the form of a vector
    *
    *  Takes in a target and key and returns the value associated in the form of vector.
    *  Note: This function is mainly for obtaining target keywords. For obtaining other values, use findMissionKeywords.
    * 
    * @param key keyword for desired values
    * @param mission mission name as it relates to the config files
    * @param searchKernels bool Whether to search the kernels for the user
    * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
    * @param limitCk int number of cks to limit to, default is -1 to retrieve all
    * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
    * @param kernelList vector<string> vector of additional kernels to load 
    *
    * @returns vector of values
    **/
    std::pair<nlohmann::json, nlohmann::json> findTargetKeywords(
        std::string key, 
        std::string mission, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});


    /**
     * @brief Given an ephemeris time and a starting frame, find the path from that starting frame to J2000 (1)
     *
     * This function uses NAIF routines and builds a path from the initalframe to J2000 making
     * note of all the in between frames
     *
     * @param et ephemeris times at which you want to optain the frame trace
     * @param initialFrame the initial frame's NAIF code.
     * @param mission Config subset as it relates to the mission
     * @param ckQualities vector of strings describing the quality of cks to try and obtain
     * @param spkQualities vector of strings describing the quality of spks to try and obtain
     * @param searchKernels bool Whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @returns A two element vector of vectors ints, where the first element is the sequence of time dependent frames
     * and the second is the sequence of constant frames
     **/
    std::pair<std::vector<std::vector<int>>, nlohmann::json> frameTrace(
        double et, 
        int initialFrame, 
        std::string mission, 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        std::vector<std::string> spkQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool searchKernels=true,
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1, 
        std::vector<std::string> kernelList={});

    /**
     * @brief Extracts all segment times between observStart and observeEnd
     *
     * Given an observation start and observation end, extract all times assocaited
     * with segments in a CK file. The times returned are all times assocaited with
     * concrete CK segment times with no interpolation. This function is limited to 
     * loading one CK file at a time, if a time window covers multiple CK files, 
     * the function will throw an error.
     *
     * @param observStart Ephemeris time to start searching at
     * @param observEnd Ephemeris time to stop searching at
     * @param targetFrame Target reference frame to get ephemeris data in
     * @param ckQualities vector of string describing the quality of cks to try and obtain
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @returns A list of times
     **/
    std::pair<std::vector<double>, nlohmann::json> extractExactCkTimes(
        double observStart, 
        double observEnd, 
        int targetFrame, 
        std::string mission, 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=1, 
        int limitSpk=1,
        std::vector<std::string> kernelList={});

    /**
     * @brief Returns exact target orientations for given time intervals and parameters
     *
     * Given a start and stop ephemeris time, extract all times assocaited with segments in a CK file. The times returned are all times assocaited with
     * concrete CK segment times with no interpolation. This function is limited to loading one CK file at a time, if a time window covers multiple CK files, 
     * the function will throw an error.
     *
     * @param startEts vector of start ephemeris times
     * @param stopEts vector of stop ephemeris times
     * @param exposureDuration vector of exposure durations
     * @param toFrame target frame
     * @param refFrame reference frame
     * @param mission mission name as it relates to the config files
     * @param ckQualities vector of string describing the quality of cks to try and obtain
     * @param useWeb bool whether to use web SpiceQL
     * @param searchKernels bool whether to search the kernels for the user
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitQuality bool only returns higher priority quality kernel if true
     * @param kernelList vector<string> vector of additional kernels to load 
     *
     * @returns Vector of SPICE-style quaternions (w,x,y,z) and optional angular velocity (4 element without angular velocity, 7 element with)
     **/
    std::pair<std::vector<std::vector<double>>, nlohmann::json> getExactTargetOrientations(
        double startEt, 
        double stopEt, 
        int toFrame, 
        int refFrame, 
        std::string mission, 
        std::vector<std::string> ckQualities={"smithed", "reconstructed"}, 
        bool useWeb=false, 
        bool searchKernels=true, 
        bool fullKernelPath=false, 
        int limitCk=-1, 
        int limitSpk=1,
        std::vector<std::string> kernelList = {});

    /**
     * @brief Searches for kernels given mission(s) and parameters.
     *
     * Searches for kernels given mission(s) and parameters.
     *
     * @param spiceqlNames mission names
     * @param types kernel types
     * @param startTime Ephemeris time to start searching at
     * @param stopTime Ephemeris time to stop searching at
     * @param ckQualities vector of string describing the quality of cks to try and obtain
     * @param spkQualities vector of string describing the quality of spks to try and obtain
     * @param useWeb whether to use web SpiceQL
     * @param fullKernelPath bool if true returns full kernel paths, default returns relative paths
     * @param limitCk limitCk int number of cks to limit to, default is -1 to retrieve all
     * @param limitSpk int number of spks to limit to, default is 1 to retrieve only one
     * @param overwrite subkernels will take precedent over parent mission kernels if true
     *
     * @returns An empty return and list of kernels
     **/
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