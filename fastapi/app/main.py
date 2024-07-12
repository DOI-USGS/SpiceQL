"""Module providing SpiceQL endpoints"""

from ast import literal_eval
from typing import Annotated, Any
from fastapi import FastAPI, Query
from pydantic import BaseModel, Field
from starlette.responses import RedirectResponse
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

# Create FastAPI instance
app = FastAPI()

# General endpoints
@app.get("/", include_in_schema=False)
async def root():
    return RedirectResponse(url="/docs")

@app.get("/healthCheck")
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
    ets: Annotated[list[float], Query()] | str | None = None,
    startEts: float | None = None,
    exposureDuration: float | None = None,
    numOfExposures: int | None = None,
    ckQuality: str = "",
    spkQuality: str = ""):
    try:
        if ets is not None:
            if isinstance(ets, str):
                ets = literal_eval(ets)
        else:
            if all(v is not None for v in [startEts, exposureDuration, numOfExposures]):
                stopEts = (exposureDuration * numOfExposures) + startEts
                etsNpArray = np.arange(startEts, stopEts, exposureDuration)
                ets = list(etsNpArray)
            else:
                raise Exception("Verify that a startEts, exposureDuration, and numOfExposures are being passed correctly.")
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
    ets: Annotated[list[float], Query()] | str | None = None,
    startEts: float | None = None,
    exposureDuration: float | None = None,
    numOfExposures: int | None = None,
    ckQuality: str = ""):
    try:
        if ets is not None:
            if isinstance(ets, str):
                ets = literal_eval(ets)
        else:
            if all(v is not None for v in [startEts, exposureDuration, numOfExposures]):
                stopEts = (exposureDuration * numOfExposures) + startEts
                etsNpArray = np.arange(startEts, stopEts, exposureDuration)
                ets = list(etsNpArray)
            else:
                raise Exception("Verify that a startEts, exposureDuration, and numOfExposures are being passed correctly.")
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
async def strSclkToEt(
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
    ckQuality: str = ""):
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
    ckQuality: str = ""):
    try:
        result = pyspiceql.extractExactCkTimes(observStart, observEnd, targetFrame, mission, ckQuality, SEARCH_KERNELS_BOOL)
        body = ResultModel(result=result)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    