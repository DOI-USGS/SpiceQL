"""Module providing SpiceQL endpoints"""

from ast import literal_eval
from typing import Annotated, Any
from fastapi import FastAPI, Query
from pydantic import BaseModel, Field
import numpy as np
import os
import pyspiceql
import logging

logger = logging.getLogger('uvicorn.error')

SEARCH_KERNELS_BOOL = True

# Models
class MessageItem(BaseModel):
    message: str

class ResultModel(BaseModel):
    result: Any = Field(serialization_alias='return')

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
    ckQuality: str = "predicted"
    spkQuality: str = "predicted"

# Create FastAPI instance
app = FastAPI()

@app.get("/")
async def message():
    try: 
      data_dir_exists = os.path.exists(pyspiceql.getDataDirectory())
      return {"data_content": os.listdir(pyspiceql.getDataDirectory()),
              "data_dir_exists": data_dir_exists, 
              "is_healthy": data_dir_exists}
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
    ckQuality: str = "smithed",
    spkQuality: str = "smithed"):
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
        result = pyspiceql.getTargetStates(ets, target, observer, frame, abcorr, mission, ckQuality, spkQuality, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
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
    ckQuality = params.ckQuality
    spkQuality = params.spkQuality
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
        result = pyspiceql.getTargetStates(ets, target, observer, frame, abcorr, mission, ckQuality, spkQuality, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
    
@app.get("/getTargetOrientations")
async def getTargetOrientations(
    toFrame: int,
    refFrame: int,
    mission: str,
    ets: str | None = None,
    startEts: Annotated[list[float], Query()] | float | str | None = None,
    stopEts: Annotated[list[float], Query()] | float | str | None = None,
    exposureDuration: Annotated[list[float], Query()] | float | str | None = None,
    ckQuality: str = "smithed"):
    try:
        if ets is not None:
            if isinstance(ets, str):
                ets = literal_eval(ets)
        else:
            ets = calculate_ets(startEts, stopEts, exposureDuration)
        result = pyspiceql.getTargetOrientations(ets, toFrame, refFrame, mission, ckQuality, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)  
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    

@app.get("/strSclkToEt")
async def strSclkToEt(
    frameCode: int,
    sclk: str,
    mission: str):
    try:
        result = pyspiceql.strSclkToEt(frameCode, sclk, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/doubleSclkToEt")
async def doubleSclkToEt(
    frameCode: int,
    sclk: float,
    mission: str):
    try:
        result = pyspiceql.doubleSclkToEt(frameCode, sclk, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/doubleEtToSclk")
async def doubleEtToSclk(
    frameCode: int,
    et: float,
    mission: str):
    try:
        result = pyspiceql.doubleEtToSclk(frameCode, et, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/utcToEt")
async def utcToEt(
    utc: str):
    try:
        result = pyspiceql.utcToEt(utc, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/translateNameToCode")
async def translateNameToCode(
    frame: str,
    mission: str):
    try:
        result = pyspiceql.translateNameToCode(frame, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/translateCodeToName")
async def translateCodeToName(
    frame: int,
    mission: str):
    try:
        result = pyspiceql.translateCodeToName(frame, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/getFrameInfo")
async def getFrameInfo(
    frame: int,
    mission: str):
    try:
        result = pyspiceql.getFrameInfo(frame, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/getTargetFrameInfo")
async def getTargetFrameInfo(
    targetId: int,
    mission: str):
    try:
        result = pyspiceql.getTargetFrameInfo(targetId, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/findMissionKeywords")
async def findMissionKeywords(
    key: str,
    mission: str):
    try:
        result = pyspiceql.findMissionKeywords(key, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/findTargetKeywords")
async def findTargetKeywords(
    key: str,
    mission: str):
    try:
        result = pyspiceql.findTargetKeywords(key, mission, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/frameTrace")
async def frameTrace(
    et: float,
    initialFrame: int,
    mission: str,
    ckQuality: str = "smithed"):
    try:
        result = pyspiceql.frameTrace(et, initialFrame, mission, ckQuality, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
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
    ckQuality: str = "smithed"):
    try:
        result = pyspiceql.extractExactCkTimes(observStart, observEnd, targetFrame, mission, ckQuality, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
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
    print("start_times=" + str(start_times))
    print("exposure_times=" + str(exposure_times))
    print("stop_times=" + str(stop_times))
    result = zip(start_times, stop_times, exposure_times)
    print("result=" + str(list(result)))
    times = []
    for start, stop, exposure_time in zip(start_times, stop_times, exposure_times):
        print("start=" + str(start))
        print("stop=" + str(stop))
        print("exposure_time=" + str(exposure_time))
        interp_times = np.arange(start, stop, exposure_time, dtype=float)
        print("interp_times=" + str(interp_times))
        times.extend(interp_times.tolist())
        
    return np.asarray(times)
    