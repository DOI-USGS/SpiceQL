from ast import literal_eval
from typing import Annotated, Any
from fastapi import Query
from pydantic import BaseModel, Field, field_validator, ValidationInfo
import numpy as np
import logging
import sys
import functools

logger = logging.getLogger(__name__)

# logger = logging.getLogger('uvicorn.error')
# logger.setLevel(logging.DEBUG)

#region DECORATOR
"""
Decorator that checks and validates any list-like inputs and ets calculations.
"""

def validate_params(init_func):
    @functools.wraps(init_func)
    def wrapper(self, *args, **kwargs):
        init_func(self, *args, **kwargs)

        logger.debug(f"Running validation of {self.__class__.__name__}!")

        # Iterate through instance variables (self.__dict__)
        for var_name, var_value in self.__dict__.items():
            logger.debug(f"  Checking '{var_name}': {var_value}, 'type': {str(type(var_value))}")
            
            # Check if variable names are possible list type
            if var_name in ['ckQualities', 'spkQualities', 'kernelList', 'spiceqlNames', 'types']:
                setattr(self, var_name, strToList(var_value))
            
            # Check if variable name is 'ets'
            if var_name == 'ets':
                ets = var_value
                ets = verify_ets(ets, self.__dict__['startEt'], self.__dict__['stopEt'], self.__dict__['numRecords'])
                setattr(self, var_name, ets)
        
        logger.debug(f"--- Validation successful for instance of {self.__class__.__name__} ---")
        logger.debug(f"AFTERMATH self: {str(self.__dict__.items())}")

    return wrapper
#endregion   


#region UTILS
"""
Utilities for param validation.
"""

def calculate_ets(startEt, stopEt, numRecords) -> list:
    return np.linspace(startEt, stopEt, numRecords)

def strToList(value: str) -> list:
    # Converts a string into a list or its literal value
    if value is not None:
        if isinstance(value, str):
            value = value.replace("[", "").replace("]", "").replace("\"", "").replace("\'", "").replace(" ", "").split(",")
        else:
            try:
                iter(value)
            except TypeError:
                value = [value]
    return value

def verify_ets(ets, startEts, stopEts, exposureDuration):
    if ets is not None:
        if isinstance(ets, str):
            ets = literal_eval(ets)
        else:
            # getTargetStates requires an iterable ets.  If not iterable, make it a list.
            try:
                ets = iter(ets)
            except TypeError:
                ets = [ets]
    else:
        ets = calculate_ets(startEts, stopEts, exposureDuration)
    return ets

#endregion


#region MODELS
"""
Models using Pydantic's BaseModel format, mainly for POST endpoints.
"""

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
    startEts: float | None = None
    stopEts: float | None = None
    numRecords: int | None = None
    ckQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"]
    spkQualities: Annotated[list[str], Query()] | str | None = ["smithed", "reconstructed"]
    kernelList: Annotated[list[str], Query()] | str | None = []
    searchKernels: bool = True
    fullKernelPath: bool = False
    limitCk: int = -1,
    limitSpk: int = 1

    @field_validator('ets', mode='before')
    @classmethod
    def validate_ets(cls, ets: Any, info: ValidationInfo) -> str:
        """Strips leading/trailing whitespace from the name."""
        startEts = info.data.get('startEts') 
        stopEts = info.data.get('stopEts') 
        numRecords = info.data.get('numRecords') 
        ets = verify_ets(ets, startEts, stopEts, numRecords)
        return ets

#endregion


#region PARAMS
"""
Query parameters for GET endpoints.
Alphabetical order.
"""

class AbcorrParam():
    def __init__(
            self,
            abcorr: Annotated[str, Query(
                description="Abcorr.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "LT+S"
                    }
                }
            )]):
        self.value = abcorr


class CommonParams():
    @validate_params
    def __init__(
            self,
            kernelList: Annotated[list[str], Query(
                description="This is a description.",
                openapi_examples={
                    "empty": {
                        "summary": "No list of kernels",
                        "value": []
                    },
                    "test1": {
                        "summary": "example1",
                        "value": ["123", "asd"]
                    }
                }
            )] = [],
            searchKernels: Annotated[bool, Query(
                description="Whether to search for kernels.",
                openapi_examples={
                    "noSearch": {
                        "summary": "No search",
                        "value": False
                    }
                }
            )] = True,
            fullKernelPath: Annotated[bool, Query(
                description="Whether to list full kernel paths.",
                openapi_examples={
                    "fullPath": {
                        "summary": "Show full kernel path",
                        "value": True
                    }
                }
            )] = False,
            limitCk: Annotated[int, Query(
                description="Limit CK kernels.",
                openapi_examples={
                    "one": {
                        "summary": "Return one CK kernel, if possible.",
                        "value": 1
                    },
                    "five": {
                        "summary": "Return five CK kernels, if possible.",
                        "value": 5
                    }
                }
            )] = -1,
            limitSpk: Annotated[int, Query(
                description="Limit SPK kernels.",
                openapi_examples={
                    "one": {
                        "summary": "Return all SPK kernels, if possible.",
                        "value": -1
                    }
                }
            )] = 1,
        ):
        self.kernelList = kernelList
        self.searchKernels = searchKernels
        self.fullKernelPath = fullKernelPath
        self.limitCk = limitCk
        self.limitSpk = limitSpk


class CkQualitiesParam():
    @validate_params
    def __init__(
            self,
            ckQualities: Annotated[list[str], Query(
                description="List of CK qualities.",
                openapi_examples={
                    "smithed": {
                        "summary": "Only smithed",
                        "value": ["smithed"]
                    },
                    "reconstructed": {
                        "summary": "Only reconstructed",
                        "value": ["reconstructed"]
                    },
                    "all": {
                        "summary": "All quality types",
                        "value": ["smithed", "reconstructed", "predicted", "nadir", "noquality"]
                    }
                }
            )] = ["smithed", "reconstructed"]):
        self.value = ckQualities


class EtParam():
    def __init__(
            self,
            et: Annotated[float, Query(
                description="Ephemeris time",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201375.8323615
                    },
                    "example1": {
                        "summary": "Example 1",
                        "value": -896556653.900884
                    },
                    "frameTrace example": {
                        "summary": "MRO CTX Example [frameTrace]",
                        "value": 690201382.5595295
                    }
                }
            )]):
        self.value = et 


class EtsParam():
    @validate_params
    def __init__(
            self,
            ets: Annotated[list[float], Query(
                description="Ephemeris times.",
                openapi_examples={
                    "empty": {
                        "summary": "No ephemeris time range, using start/stop ETs and exposure duration instead.",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": [690201375.8323615, 690201389.2866975]
                    }
                }
            )] = None,
            startEt: Annotated[float, Query(
                description="Start ephemeris times.",
                openapi_examples={
                    "empty": {
                        "summary": "No startEt, using 'ets' param instead.",
                        "value": None
                    }
                }
            )] = None,
            stopEt: Annotated[float, Query(
                description="Stop ephemeris times.",
                openapi_examples={
                    "empty": {
                        "summary": "No stopEt, using 'ets' param instead.",
                        "value": None
                    }
                }
            )] = None,
            numRecords: Annotated[int, Query(
                description="Number of records.",
                openapi_examples={
                    "empty": {
                        "summary": "No numRecords, using 'ets' param instead.",
                        "value": None
                    }
                }
            )] = None):
        self.value = ets


class ExactCkFrameParam():
    def __init__(
            self,
            exactCkFrame: Annotated[int, Query(
                description="Exact CK frame code.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "-74690"
                    }
                }
            )]):
        self.value = exactCkFrame


class FormatParam():
    @validate_params
    def __init__(
            self,
            format: Annotated[str, Query(
                description="UTC Format.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "example1": {
                        "summary": "Format C",
                        "value": "C"
                    }
                }
            )]):
        self.value = format 


class FrameCodeParam():
    def __init__(
            self,
            frameCode: Annotated[int, Query(
                description="Frame code",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": -74
                    },
                    "lro": {
                        "summary": "LRO Example",
                        "value": -85
                    }
                }
            )]):
        self.value = frameCode


class FrameIntParam():
    @validate_params
    def __init__(
            self,
            frame: Annotated[int, Query(
                description="Frame code",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": -74
                    }
                }
            )]):
        self.value = frame


class FrameStrParam():
    def __init__(
            self,
            frame: Annotated[str, Query(
                description="Frame code.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx1": {
                        "summary": "MRO CTX Example",
                        "value": "IAU_MARS"
                    },
                    "ctx2": {
                        "summary": "MRO CTX Example",
                        "value": "IAU_MARS"
                    }
                }
            )]):
        self.value = frame


class InitialFrameParam():
    @validate_params
    def __init__(
            self,
            initialFrame: Annotated[int, Query(
                description="Initial frame code",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": -74021
                    }
                }
            )]):
        self.value = initialFrame


class KeyParam():
    @validate_params
    def __init__(
            self,
            key: Annotated[str, Query(
                description="Key word",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx1": {
                        "summary": "MRO CTX Example",
                        "value": "*-74021*"
                    },
                    "ctx2": {
                        "summary": "MRO CTX Example",
                        "value": "*499*"
                    }
                }
            )]):
        self.value = key


class LimitCkOneParam():
    @validate_params
    def __init__(
            self,
            limitCk: Annotated[int, Query(
                description="Limit CK kernels.",
                openapi_examples={
                    "one": {
                        "summary": "Return one CK kernel, if possible.",
                        "value": 1
                    },
                    "five": {
                        "summary": "Return five CK kernels, if possible.",
                        "value": 5
                    }
                }
            )] = 1):
        self.value = limitCk


class MissionParam():
    @validate_params
    def __init__(
            self,
            mission: Annotated[str, Query(
                description="Mission name.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "ctx"
                    }
                }
            )]):
        self.value = mission


class ObserverParam():
    def __init__(
            self,
            observer: Annotated[str, Query(
                description="Observer name.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "mars"
                    }
                }
            )]):
        self.value = observer


class ObservEndParam():
    @validate_params
    def __init__(
            self,
            observEnd: Annotated[float, Query(
                description="Observed end ephemeris time",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201382.5595295
                    }
                }
            )]):
        self.value = observEnd


class ObservStartParam():
    @validate_params
    def __init__(
            self,
            observStart: Annotated[float, Query(
                description="Observed start ephemeris time",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201382.5595295
                    }
                }
            )]):
        self.value = observStart


class OverwriteParam():
    @validate_params
    def __init__(
            self,
            overwrite: Annotated[bool, Query(
                description="Whether to remote duplicate kernel matches.",
                openapi_examples={
                    "default": {
                        "summary": "Default",
                        "value": False
                    }
                }
            )] = False):
        self.value = overwrite


class PrecisionParam():
    @validate_params
    def __init__(
            self,
            precision: Annotated[float, Query(
                description="Precision.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "example1": {
                        "summary": "10",
                        "value": 10
                    }
                }
            )]):
        self.value = precision  


class RefFrameParam():
    def __init__(
            self,
            refFrame: Annotated[int, Query(
                description="Reference frame code.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "-74690"
                    }
                }
            )]):
        self.value = refFrame


class SclkDblParam():
    def __init__(
            self,
            sclk: Annotated[float, Query(
                description="Spacecraft clock time as double or float",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "LRO Example",
                        "value": 922997380.174174
                    }
                }
            )]):
        self.value = sclk


class SclkStrParam():
    def __init__(
            self,
            sclk: Annotated[str, Query(
                description="Spacecraft clock time as string",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "1321396563:036"
                    }
                }
            )]):
        self.value = sclk


class SpiceqlNamesParam():
    @validate_params
    def __init__(
            self,
            spiceqlNames: Annotated[list[str], Query(
                description="List of mission names.",
                openapi_examples={
                    "empty": {
                        "summary": "No mission names",
                        "value": []
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": ["ctx", "mars"]
                    }
                }
            )] = []):
        self.value = spiceqlNames


class SpkQualitiesParam():
    @validate_params
    def __init__(
            self,
            spkQualities: Annotated[list[str], Query(
                description="List of SPK qualities.",
                openapi_examples={
                    "smithed": {
                        "summary": "Only smithed",
                        "value": ["smithed"]
                    },
                    "reconstructed": {
                        "summary": "Only reconstructed",
                        "value": ["reconstructed"]
                    },
                    "all": {
                        "summary": "All quality types",
                        "value": ["smithed", "reconstructed", "predicted", "nadir", "noquality"]
                    }
                }
            )] = ["smithed", "reconstructed"]):
        self.value = spkQualities
        

class StartEtParam():
    @validate_params
    def __init__(
            self,
            startEt: Annotated[float, Query(
                description="Start ephemeris time",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201382.5595295
                    }
                }
            )]):
        self.value = startEt


class StopEtParam():
    @validate_params
    def __init__(
            self,
            stopEt: Annotated[float, Query(
                description="Stop ephemeris time",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201382.5595295
                    }
                }
            )]):
        self.value = stopEt


class StartTimeParam():
    @validate_params
    def __init__(
            self,
            startTime: Annotated[float, Query(
                description="Start ephemeris time.",
                openapi_examples={
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201375.8323615
                    }
                }
            )] = -sys.float_info.max):
        self.value = startTime


class StopTimeParam():
    @validate_params
    def __init__(
            self,
            stopTime: Annotated[float, Query(
                description="Stop ephemeris time.",
                openapi_examples={
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 690201389.2866975
                    }
                }
            )] = sys.float_info.max):
        self.value = stopTime


class TargetParam():
    def __init__(
            self,
            target: Annotated[str, Query(
                description="Target name.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "sun"
                    }
                }
            )]):
        self.value = target


class TargetFrameParam():
    @validate_params
    def __init__(
            self,
            targetFrame: Annotated[int, Query(
                description="Target frame code",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": -74021
                    }
                }
            )]):
        self.value = targetFrame


class TargetIdParam():
    @validate_params
    def __init__(
            self,
            targetId: Annotated[int, Query(
                description="Target ID code",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": 499
                    }
                }
            )]):
        self.value = targetId


class ToFrameParam():
    def __init__(
            self,
            toFrame: Annotated[int, Query(
                description="Target frame code.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": "-74000"
                    }
                }
            )]):
        self.value = toFrame


class TypesParam():
    @validate_params
    def __init__(
            self,
            types: Annotated[list[str], Query(
                description="List of kernel types.",
                openapi_examples={
                    "ctx": {
                        "summary": "MRO CTX Example",
                        "value": ["sclk", "ck", "pck", "fk", "ik", "iak", "lsk", "tspk", "spk"]
                    }
                }
            )] = ["ck", "spk", "tspk", "lsk", "mk", "sclk", "iak", "ik", "fk", "dsk", "pck", "ek"]):
        self.value = types


class UtcParam():
    @validate_params
    def __init__(
            self,
            utc: Annotated[str, Query(
                description="UTC Time.",
                openapi_examples={
                    "empty": {
                        "summary": "Default",
                        "value": None
                    },
                    "example1": {
                        "summary": "Example 1",
                        "value": "1971-08-04T16:28:24.9159358"
                    }
                }
            )]):
        self.value = utc   


#endregion