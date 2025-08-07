"""Module providing SpiceQL endpoints"""

from .models import *

from typing import Annotated
from fastapi import FastAPI, Depends, Body
import os
import pyspiceql
import logging
import h5py


logger = logging.getLogger(__name__)

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
            logger.info(hdf_db.attrs)
            spiceql_version = hdf_db.attrs['SPICEQL_VERSION']
        except Exception as e:
            raise Exception(f"Could not read SpiceQL version from HDF file: {e}.")
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
    target: Annotated[TargetParam, Depends()],
    observer: Annotated[ObserverParam, Depends()],
    frame: Annotated[FrameStrParam, Depends()],
    abcorr: Annotated[AbcorrParam, Depends()],
    ets: Annotated[EtsParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()],
    ckQualities: Annotated[CkQualitiesParam, Depends()],
    spkQualities: Annotated[SpkQualitiesParam, Depends()],
    ):
    try:
        result, kernels = pyspiceql.getTargetStates(
            ets.value,
            target.value,
            observer.value,
            frame.value,
            abcorr.value,
            mission.value,
            ckQualities.value,
            spkQualities.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
    
@app.post("/getTargetStates")
async def getTargetStates(params: Annotated[TargetStatesRequestModel, Body(
    openapi_examples={
        "example": {
            "summary": "LROC Payload",
            "description": "Try getting target states using the POST endpoint with the LROC example body payload.",
            "value": {"ets": "[302228504.36824864]", "target": "LUNAR RECONNAISSANCE ORBITER", "observer": "MOON", "frame": "J2000", "abcorr": "None", "mission": "lroc", "searchKernels": "True"}
        }
    }
)]):
    target = params.target
    observer = params.observer
    frame =  params.frame
    abcorr = params.abcorr
    mission = params.mission
    ets = params.ets
    startEt = params.startEt
    stopEt = params.stopEt
    numRecords = params.numRecords
    ckQualities = params.ckQualities
    spkQualities = params.spkQualities
    searchKernels = params.searchKernels
    fullKernelPath = params.fullKernelPath
    limitCk = params.limitCk
    limitSpk = params.limitSpk
    kernelList = params.kernelList
    try:
        result, kernels = pyspiceql.getTargetStates(
            ets,
            target,
            observer,
            frame,
            abcorr,
            mission,
            ckQualities,
            spkQualities,
            False,
            searchKernels,
            fullKernelPath,
            limitCk,
            limitSpk,
            kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
    
@app.get("/getTargetOrientations")
async def getTargetOrientations(
    toFrame: Annotated[ToFrameParam, Depends()],
    refFrame: Annotated[RefFrameParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    ets: Annotated[EtsParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()],
    ckQualities: Annotated[CkQualitiesParam, Depends()]):
    try:
        result, kernels = pyspiceql.getTargetOrientations(
            ets.value,
            toFrame.value,
            refFrame.value,
            mission.value,
            ckQualities.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    

@app.get("/strSclkToEt")
async def strSclkToEt(
    frameCode: Annotated[FrameCodeParam, Depends()],
    sclk: Annotated[SclkStrParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.strSclkToEt(
            frameCode.value,
            sclk.value, 
            mission.value, 
            False, 
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/doubleSclkToEt")
async def doubleSclkToEt(
    frameCode: Annotated[FrameCodeParam, Depends()],
    sclk: Annotated[SclkDblParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.doubleSclkToEt(
            frameCode.value,
            sclk.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/doubleEtToSclk")
async def doubleEtToSclk(
    frameCode: Annotated[FrameCodeParam, Depends()],
    et: Annotated[EtParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.doubleEtToSclk(
            frameCode.value,
            et.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/utcToEt")
async def utcToEt(
    utc: Annotated[UtcParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.utcToEt(
            utc.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/etToUtc")
async def etToUtc(
    et: Annotated[EtParam, Depends()],
    format: Annotated[FormatParam, Depends()],
    precision: Annotated[PrecisionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.etToUtc(
            et.value,
            format.value,
            precision.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/translateNameToCode")
async def translateNameToCode(
    frame: Annotated[FrameStrParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.translateNameToCode(
            frame.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/translateCodeToName")
async def translateCodeToName(
    frame: Annotated[FrameIntParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.translateCodeToName(
            frame.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/getFrameInfo")
async def getFrameInfo(
    frame: Annotated[FrameIntParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.getFrameInfo(
            frame.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/getTargetFrameInfo")
async def getTargetFrameInfo(
    targetId: Annotated[TargetIdParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.getTargetFrameInfo(
            targetId.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/findMissionKeywords")
async def findMissionKeywords(
    key: Annotated[KeyParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.findMissionKeywords(
            key.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/findTargetKeywords")
async def findTargetKeywords(
    key: Annotated[KeyParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.findTargetKeywords(
            key.value,
            mission.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

@app.get("/frameTrace")
async def frameTrace(
    et: Annotated[EtParam, Depends()],
    initialFrame: Annotated[InitialFrameParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    ckQualities: Annotated[CkQualitiesParam, Depends()],
    spkQualities: Annotated[SpkQualitiesParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.frameTrace(
            et.value,
            initialFrame.value, 
            mission.value,
            ckQualities.value,
            spkQualities.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)
    
@app.get("/extractExactCkTimes")
async def extractExactCkTimes(
    observStart: Annotated[ObservStartParam, Depends()],
    observEnd: Annotated[ObservEndParam, Depends()],
    targetFrame: Annotated[TargetFrameParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    ckQualities: Annotated[CkQualitiesParam, Depends()],
    limitCk: Annotated[LimitCkOneParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]
    ):
    try:
        result, kernels = pyspiceql.extractExactCkTimes(
            observStart.value, 
            observEnd.value, 
            targetFrame.value, 
            mission.value, 
            ckQualities.value,
            False,
            commonParams.searchKernels,
            commonParams.fullKernelPath, 
            limitCk.value,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/getExactTargetOrientations")
async def getExactTargetOrientations(
    startEt: Annotated[StartEtParam, Depends()],
    stopEt: Annotated[StopEtParam, Depends()],
    toFrame: Annotated[ToFrameParam, Depends()],
    refFrame: Annotated[RefFrameParam, Depends()],
    exactCkFrame: Annotated[ExactCkFrameParam, Depends()],
    mission: Annotated[MissionParam, Depends()],
    ckQualities: Annotated[CkQualitiesParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()]):
    try:
        result, kernels = pyspiceql.getExactTargetOrientations(
            startEt.value, 
            stopEt.value, 
            toFrame.value, 
            refFrame.value, 
            exactCkFrame.value,
            mission.value, 
            ckQualities.value, 
            False, 
            commonParams.searchKernels,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            commonParams.kernelList)
        body = ResultModel(result=result, kernels=kernels)
        print(body)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)


@app.get("/searchForKernelsets")
async def searchForKernelsets(
    spiceqlNames: Annotated[SpiceqlNamesParam, Depends()],
    types: Annotated[TypesParam, Depends()],
    startTime: Annotated[StartTimeParam, Depends()],
    stopTime: Annotated[StopTimeParam, Depends()],
    ckQualities: Annotated[CkQualitiesParam, Depends()],
    spkQualities: Annotated[SpkQualitiesParam, Depends()],
    commonParams: Annotated[CommonParams, Depends()],
    overwrite: Annotated[OverwriteParam, Depends()]):
    try:
        logger.debug("Calling searchForKernelsets ....")
        result, kernels = pyspiceql.searchForKernelsets(
            spiceqlNames.value,
            types.value,
            startTime.value,
            stopTime.value,
            ckQualities.value,
            spkQualities.value,
            False,
            commonParams.fullKernelPath,
            commonParams.limitCk,
            commonParams.limitSpk,
            overwrite.value)
        body = ResultModel(result=result, kernels=kernels)
        return ResponseModel(statusCode=200, body=body)
    except Exception as e:
        body = ErrorModel(error=str(e))
        return ResponseModel(statusCode=500, body=body)

    