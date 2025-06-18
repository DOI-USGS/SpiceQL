"""Module providing SpiceQL endpoints"""

from ast import literal_eval
from typing import Annotated, Any
from fastapi import FastAPI, Query
from pydantic import BaseModel, Field
import numpy as np
import os
import pyspiceql
import logging
import h5py
import sys

logger = logging.getLogger('uvicorn.error')

# Models
class MessageItem(BaseModel):
    message: str

class ResultModel(BaseModel):
    result: Any = Field(serialization_alias='return')
    kernels: Any = Field(serialization_alias='kernels')

class ErrorModel(BaseModel):
    error: str

class ResponseModel(BaseModel):
    statusCode: int
    body: ResultModel | ErrorModel

class TargetStatesRequestModel(BaseModel):
    target: str
    observer: str
    frame: str
    abcorr: str
    mission: str
    ets: Annotated[list[float], Query()] | float | str | None = None
    startEts: Annotated[list[float], Query()] | float | str | None = None
    stopEts: Annotated[list[float], Query()] | float | str | None = None
    exposureDuration: Annotated[list[float], Query()] | float | str | None = None
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"]
    spkQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"]
    kernelList: Annotated[list[str], Query()] | str | None = []
    searchKernels: bool = True
    fullKernelPath: bool = False
    limitQuality: bool = True

# Create FastAPI instance
app = FastAPI()

@app.get("/")
async def message():
    try: 
      data_dir_exists = os.path.exists(pyspiceql.getDataDirectory())
      db_exists = os.path.exists(pyspiceql.getDbFilePath())
      if db_exists:
        try:
            hdf_db = h5py.File(pyspiceql.getDbFilePath(), 'r')
            spiceql_version = hdf_db.attrs['SPICEQL_VERSION']
        except Exception as e:
            raise Exception("Could not read SpiceQL version from HDF file.")
      else:
        logger.error(f"SpiceQL DB not found at : {pyspiceql.getDbFilePath()}")
        raise Exception("SpiceQL DB could not be found.")
        
      return {"data_content": os.listdir(pyspiceql.getDataDirectory()),
              "data_dir_exists": data_dir_exists, 
              "db_exists": db_exists,
              "is_healthy": data_dir_exists,
              "spiceql_version" : spiceql_version}
    except Exception as e:
        logger.error(f"ERROR: {e}")
        return {"is_healthy": False}


# SpiceQL endpoints
@app.get("/getTargetStates")
async def getTargetStates(
    target: str,
    observer: str,
    frame: str,
    abcorr: str,
    mission: str,
    ets: str = None,
    startEts: Annotated[list[float], Query()] | float | str | None = None,
    stopEts: Annotated[list[float], Query()] | float | str | None = None,
    exposureDuration: Annotated[list[float], Query()] | float | str | None = None,
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    spkQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        if ets is not None:
            if isinstance(ets, str):
                ets = literal_eval(ets)
            else:
                # getTargetStates requires an iterable ets.  If not iterable, make it a list.
                try:
                    iter(ets)
                except TypeError:
                    ets = [ets]
        else:
            ets = calculate_ets(startEts, stopEts, exposureDuration)
        ckQualities = strToList(ckQualities)
        spkQualities = strToList(spkQualities)
        kernelList = strToList(kernelList)
        print("getTargetStates")
        result, kernels = pyspiceql.getTargetStates(ets, target, observer, frame, abcorr, mission, ckQualities, spkQualities, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
    
@app.post("/getTargetStates")
async def getTargetStates(params: TargetStatesRequestModel):
    target = params.target
    observer = params.observer
    frame =  params.frame
    abcorr = params.abcorr
    mission = params.mission
    ets = params.ets
    startEts = params.startEts
    stopEts = params.stopEts
    exposureDuration = params.exposureDuration
    ckQualities = params.ckQualities
    spkQualities = params.spkQualities
    searchKernels = params.searchKernels
    fullKernelPath = params.fullKernelPath
    limitQuality = params.limitQuality
    kernelList = params.kernelList
    try:
        if ets is not None:
            if isinstance(ets, str):
                ets = literal_eval(ets)
            else:
                # getTargetStates requires an iterable ets.  If not iterable, make it a list.
                try:
                    iter(ets)
                except TypeError:
                    ets = [ets]
        else:
            ets = calculate_ets(startEts, stopEts, exposureDuration)
        ckQualities = strToList(ckQualities)
        spkQualities = strToList(spkQualities)
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.getTargetStates(ets, target, observer, frame, abcorr, mission, ckQualities, spkQualities, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
    
@app.get("/getTargetOrientations")
async def getTargetOrientations(
    toFrame: int,
    refFrame: int,
    mission: str,
    ets: Annotated[list[float], Query()] | float | str | None = [],
    startEts: Annotated[list[float], Query()] | float | str | None = None,
    stopEts: Annotated[list[float], Query()] | float | str | None = None,
    exposureDuration: Annotated[list[float], Query()] | float | str | None = None,
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        if ets is not None:
            if isinstance(ets, str):
                ets = literal_eval(ets)
            else:
                # getTargetStates requires an iterable ets.  If not iterable, make it a list.
                try:
                    iter(ets)
                except TypeError:
                    ets = [ets]
        else:
            ets = calculate_ets(startEts, stopEts, exposureDuration)
        ckQualities = strToList(ckQualities)
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.getTargetOrientations(ets, toFrame, refFrame, mission, ckQualities, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    

@app.get("/strSclkToEt")
async def strSclkToEt(
    frameCode: int,
    sclk: str,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.strSclkToEt(frameCode, sclk, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/doubleSclkToEt")
async def doubleSclkToEt(
    frameCode: int,
    sclk: float,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.doubleSclkToEt(frameCode, sclk, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/doubleEtToSclk")
async def doubleEtToSclk(
    frameCode: int,
    et: float,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.doubleEtToSclk(frameCode, et, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/utcToEt")
async def utcToEt(
    utc: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.utcToEt(utc, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/etToUtc")
async def etToUtc(
    et: float,
    format: str,
    precision: float,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.etToUtc(et, format, precision, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/translateNameToCode")
async def translateNameToCode(
    frame: str,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.translateNameToCode(frame, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/translateCodeToName")
async def translateCodeToName(
    frame: int,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.translateCodeToName(frame, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/getFrameInfo")
async def getFrameInfo(
    frame: int,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.getFrameInfo(frame, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/getTargetFrameInfo")
async def getTargetFrameInfo(
    targetId: int,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.getTargetFrameInfo(targetId, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/findMissionKeywords")
async def findMissionKeywords(
    key: str,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.findMissionKeywords(key, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/findTargetKeywords")
async def findTargetKeywords(
    key: str,
    mission: str,
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.findTargetKeywords(key, mission, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/frameTrace")
async def frameTrace(
    et: float,
    initialFrame: int,
    mission: str,
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    spkQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        ckQualities = strToList(ckQualities)
        spkQualities = strToList(spkQualities)
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.frameTrace(et, initialFrame, mission, ckQualities, spkQualities, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
@app.get("/extractExactCkTimes")
async def extractExactCkTimes(
    observStart: float,
    observEnd: float,
    targetFrame: int,
    mission: str,
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    searchKernels: bool = True,
    fullKernelPath: bool = False,
    limitQuality: bool = True,
    kernelList: Annotated[list[str], Query()] | str | None = []):
    try:
        ckQualities = strToList(ckQualities)
        kernelList = strToList(kernelList)
        result, kernels = pyspiceql.extractExactCkTimes(observStart, observEnd, targetFrame, mission, ckQualities, False, searchKernels, fullKernelPath, limitQuality, kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/searchForKernelsets")
async def searchForKernelsets(
    spiceqlNames: Annotated[list[str], Query()] | str = [],
    types: Annotated[list[str], Query()] | str | None = ["ck", "spk", "tspk", "lsk", "mk", "sclk", "iak", "ik", "fk", "dsk", "pck", "ek"],
    startTime: float = -sys.float_info.max,
    stopTime: float = sys.float_info.max,
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    spkQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"],
    fullKernelPath: bool = False,
    overwrite: bool = False):
    try:
        spiceqlNames = strToList(spiceqlNames)
        types = strToList(types)
        ckQualities = strToList(ckQualities)
        spkQualities = strToList(spkQualities)
        result, kernels = pyspiceql.searchForKernelsets(spiceqlNames, types, startTime, stopTime, ckQualities, spkQualities, False, fullKernelPath, overwrite)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


def calculate_ets(startEts, stopEts, exposureDuration) -> list:
    ets = []
    etsCalculationParams = [startEts, stopEts, exposureDuration]
    if all(v is not None for v in etsCalculationParams):
        if (all(isinstance(i, list) for i in etsCalculationParams)
            and (len(startEts) == len (stopEts) == len(exposureDuration))):
            ets = interpolate_times(startEts, stopEts, exposureDuration)
        elif (all(isinstance(i, str) for i in etsCalculationParams)
                or all(isinstance(i, float) for i in etsCalculationParams)):
            startEts = literal_eval(startEts)
            stopEts = literal_eval(stopEts)
            exposureDuration = literal_eval(exposureDuration)
            etsCalculationParams = [startEts, stopEts, exposureDuration]
            if all(isinstance(i, float) for i in etsCalculationParams):
                etsNpArray = np.arange(startEts, stopEts, exposureDuration)
                # If ets is a single value, np.arange yields an empty array
                ets = list(etsNpArray)
            elif (all(isinstance(i, tuple) for i in etsCalculationParams)
                    or all(isinstance(i, list) for i in etsCalculationParams)):
                ets = interpolate_times(startEts, stopEts, exposureDuration)
        else:
            raise Exception("Params startEts, stopEts, and exposureDuration must be either all floats or lists of the same length.")
    else:
        raise Exception("Verify that either params ets or startEts, stopEts, and exposureDuration are being passed correctly.")
    return ets

def interpolate_times(start_times, stop_times, exposure_times) -> np.ndarray:
    # Convert lists to numpy arrays for easy manipulation
    start_times = np.asarray(start_times)
    exposure_times = np.asarray(exposure_times)
    stop_times = np.asarray(stop_times)
    times = []
    for start, stop, exposure_time in zip(start_times, stop_times, exposure_times):
        interp_times = np.arange(start, stop, exposure_time, dtype=float)
        times.extend(interp_times.tolist())
    return np.asarray(times)

def strToList(value: str) -> list:
    # Converts a string into a list or its literal value
    if value is not None:
        if isinstance(value, str):
            value = value.replace("[", "").replace("]", "").split(",")
        else:
            try:
                iter(value)
            except TypeError:
                value = [value]
    return value
    