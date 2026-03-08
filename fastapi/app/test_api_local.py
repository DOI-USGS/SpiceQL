"""
SpiceQL FastAPI – local CI tests
=================================
Tests every endpoint in main.py using FastAPI's TestClient.

pyspiceql is mocked so no SPICE kernels or compiled C++ binaries are
needed. Each test patches the relevant pql.* function to return the
exact value documented at:
https://astrogeology.usgs.gov/docs/getting-started/using-spiceql/exploring-spiceqls-rest-python-and-cpp-apis/

Install:
    pip install fastapi httpx pytest

Run:
    pytest test_spiceql_api.py -v
"""

from unittest.mock import MagicMock, patch
import sys
from fastapi import FastAPI

# ---------------------------------------------------------------------------
# Stub out pyspiceql before main.py imports it, so CI doesn't need the
# compiled C++ extension installed.
# ---------------------------------------------------------------------------
pql_stub = MagicMock()
sys.modules["pyspiceql"] = pql_stub

from fastapi.testclient import TestClient  # noqa: E402

from .main import pyspiceql
from .main import app
client = TestClient(app)

# Convenience kernels reused across fixtures (mirrors the docs responses)
CK_KERNELS = {"ck": ["/mro/kernels/ck/mro_sc_psp_211109_211115.bc"]}
FK_KERNELS = {"fk": ["/mro/kernels/fk/mro_v16.tf"]}
LSK_KERNELS = {"lsk": ["/base/kernels/lsk/naif0012.tls"]}
SCLK_KERNELS = {
    "fk": ["/mro/kernels/fk/mro_v16.tf"],
    "lsk": ["/base/kernels/lsk/naif0012.tls"],
    "sclk": [
        "/mro/kernels/sclk/MRO_SCLKSCET.00112.65536.tsc",
        "/mro/kernels/sclk/MRO_SCLKSCET.00112.tsc",
    ],
}


# ---------------------------------------------------------------------------
# getTargetStates
# ---------------------------------------------------------------------------

def test_getTargetStates_returns_expected_state_vectors():
    expected_return = [
        [
            123515791.9195627,
            187209003.7067195,
            80611152.03610656,
            13251.543112834495,
            -8742.597438450646,
            -6.575020419444353,
            794.9856233875888,
        ],
        [
            123694026.59723282,
            187091292.98278633,
            80611063.57350075,
            13243.211405493359,
            -8755.21373709343,
            -6.575031051390966,
            794.985539001637,
        ],
    ]
    with patch("pyspiceql.getTargetStates", return_value=(expected_return, CK_KERNELS)):
        response = client.get("/getTargetStates", params={
            "ets": "[690201375.8323615,690201389.2866975]",
            "target": "SUN",
            "observer": "Mars",
            "frame": "IAU_MARS",
            "mission": "ctx",
            "abcorr": "LT+S",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# getTargetOrientations
# ---------------------------------------------------------------------------

def test_getTargetOrientations_returns_expected_quaternion():
    expected_return = [
        [
            0.9999924134600601,
            0.0005720078450331138,
            0.003853027964066137,
            -2.2039789431520754e-06,
            0.0,
            0.0,
            0.0,
        ]
    ]
    with patch("pyspiceql.getTargetOrientations", return_value=(expected_return, CK_KERNELS)):
        response = client.get("/getTargetOrientations", params={
            "ets": "[690201375.8323615]",
            "toFrame": -74000,
            "refFrame": -74690,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# strSclkToEt
# ---------------------------------------------------------------------------

def test_strSclkToEt_returns_expected_et():
    expected_return = 690201375.8323615
    with patch("pyspiceql.strSclkToEt", return_value=(expected_return, SCLK_KERNELS)):
        response = client.get("/strSclkToEt", params={
            "frameCode": -74,
            "sclk": "1321396563:036",
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# doubleSclkToEt
# ---------------------------------------------------------------------------

def test_doubleSclkToEt_returns_expected_et():
    expected_return = 31593348.006268278
    lro_kernels = {
        "fk": ["/lro/kernels/fk/lro_frames_2014049_v01.tf"],
        "lsk": ["/base/kernels/lsk/naif0012.tls"],
        "sclk": ["/lro/kernels/sclk/lro_clkcor_2024262_v00.tsc"],
    }
    with patch("pyspiceql.doubleSclkToEt", return_value=(expected_return, lro_kernels)):
        response = client.get("/doubleSclkToEt", params={
            "frameCode": -85,
            "sclk": 922997380.174174,
            "mission": "lro",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# doubleEtToSclk
# ---------------------------------------------------------------------------

def test_doubleEtToSclk_returns_expected_sclk_string():
    expected_return = "27/1321396563.036"
    with patch("pyspiceql.doubleEtToSclk", return_value=(expected_return, SCLK_KERNELS)):
        response = client.get("/doubleEtToSclk", params={
            "et": 690201375.8323615,
            "frameCode": -74,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# utcToEt
# ---------------------------------------------------------------------------

def test_utcToEt_returns_expected_et():
    expected_return = -896556653.900884
    with patch("pyspiceql.utcToEt", return_value=(expected_return, LSK_KERNELS)):
        response = client.get("/utcToEt", params={
            "utc": "1971-08-04T16:28:24.9159358",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# etToUtc
# ---------------------------------------------------------------------------

def test_etToUtc_returns_expected_utc_string():
    expected_return = "1971 AUG 04 16:28:24.9159357548"
    with patch("pyspiceql.etToUtc", return_value=(expected_return, LSK_KERNELS)):
        response = client.get("/etToUtc", params={
            "et": -896556653.900884,
            "format": "C",
            "precision": 10,
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# translateNameToCode
# ---------------------------------------------------------------------------

def test_translateNameToCode_returns_expected_code():
    expected_return = -74
    with patch("pyspiceql.translateNameToCode", return_value=(expected_return, FK_KERNELS)):
        response = client.get("/translateNameToCode", params={
            "frame": "MRO",
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# translateCodeToName
# ---------------------------------------------------------------------------

def test_translateCodeToName_returns_expected_name():
    expected_return = "MRO"
    with patch("pyspiceql.translateCodeToName", return_value=(expected_return, FK_KERNELS)):
        response = client.get("/translateCodeToName", params={
            "frame": -74,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# getFrameInfo
# ---------------------------------------------------------------------------

def test_getFrameInfo_returns_expected_frame_info():
    expected_return = [-74, 4, -74021]
    with patch("pyspiceql.getFrameInfo", return_value=(expected_return, FK_KERNELS)):
        response = client.get("/getFrameInfo", params={
            "frame": -74021,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# getTargetFrameInfo
# ---------------------------------------------------------------------------

def test_getTargetFrameInfo_returns_expected_frame_code_and_name():
    expected_return = {"frameCode": 10014, "frameName": "IAU_MARS"}
    with patch("pyspiceql.getTargetFrameInfo", return_value=(expected_return, FK_KERNELS)):
        response = client.get("/getTargetFrameInfo", params={
            "targetId": 499,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# findMissionKeywords
# ---------------------------------------------------------------------------

def test_findMissionKeywords_returns_expected_ctx_keywords():
    expected_return = {
        "FRAME_-74021_CENTER": -74.0,
        "FRAME_-74021_CLASS": 4.0,
        "FRAME_-74021_CLASS_ID": -74021.0,
        "FRAME_-74021_NAME": "MRO_CTX",
        "INS-74021_BORESIGHT": [0.0, 0.0, 1.0],
        "INS-74021_BORESIGHT_LINE": 0.430442527,
        "INS-74021_BORESIGHT_SAMPLE": 2543.46099,
        "INS-74021_CCD_CENTER": [2500.5, 0.5],
        "INS-74021_CK_FRAME_ID": -74000.0,
        "INS-74021_CK_REFERENCE_ID": -74900.0,
        "INS-74021_F/RATIO": 3.25,
        "INS-74021_FOCAL_LENGTH": 352.9271664,
        "INS-74021_FOV_ANGLE_UNITS": "DEGREES",
        "INS-74021_FOV_ANGULAR_SIZE": [5.73, 0.001146],
        "INS-74021_FOV_CLASS_SPEC": "ANGLES",
        "INS-74021_FOV_CROSS_ANGLE": 0.00057296,
        "INS-74021_FOV_FRAME": "MRO_CTX",
        "INS-74021_FOV_REF_ANGLE": 2.86478898,
        "INS-74021_FOV_REF_VECTOR": [0.0, 1.0, 0.0],
        "INS-74021_FOV_SHAPE": "RECTANGLE",
        "INS-74021_IFOV": [2e-05, 2e-05],
        "INS-74021_ITRANSL": [0.0, 142.85714285714, 0.0],
        "INS-74021_ITRANSS": [0.0, 0.0, 142.85714285714],
        "INS-74021_OD_K": [
            -0.0073433925920054505,
            2.8375878636241697e-05,
            1.2841989124027099e-08,
        ],
        "INS-74021_PIXEL_LINES": 1.0,
        "INS-74021_PIXEL_PITCH": 0.007,
        "INS-74021_PIXEL_SAMPLES": 5000.0,
        "INS-74021_PIXEL_SIZE": [0.007, 0.007],
        "INS-74021_PLATFORM_ID": -74000.0,
        "INS-74021_TRANSX": [0.0, 0.0, 0.007],
        "INS-74021_TRANSY": [0.0, 0.007, 0.0],
        "TKFRAME_-74021_ANGLES": [0.0, 0.0, 0.0],
        "TKFRAME_-74021_AXES": [1.0, 2.0, 3.0],
        "TKFRAME_-74021_RELATIVE": "MRO_CTX_BASE",
        "TKFRAME_-74021_SPEC": "ANGLES",
        "TKFRAME_-74021_UNITS": "DEGREES",
    }
    ik_kernels = {
        "fk": ["/mro/kernels/fk/mro_v16.tf"],
        "iak": ["/mro/kernels/iak/mroctxAddendum005.ti"],
        "ik": ["/mro/kernels/ik/mro_ctx_v11.ti"],
    }
    with patch("pyspiceql.findMissionKeywords", return_value=(expected_return, ik_kernels)):
        response = client.get("/findMissionKeywords", params={
            "key": "*-74021*",
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# findTargetKeywords
# ---------------------------------------------------------------------------

def test_findTargetKeywords_returns_expected_mars_keywords():
    expected_return = {
        "BODY499_PM": [176.63, 350.89198226, 0.0],
        "BODY499_POLE_DEC": [52.8865, -0.0609, 0.0],
        "BODY499_POLE_RA": [317.68143, -0.1061, 0.0],
        "BODY499_RADII": [3396.19, 3396.19, 3376.2],
    }
    pck_kernels = {"pck": ["/base/kernels/pck/pck00009.tpc"]}
    with patch("pyspiceql.findTargetKeywords", return_value=(expected_return, pck_kernels)):
        response = client.get("/findTargetKeywords", params={
            "key": "*499*",
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# frameTrace
# ---------------------------------------------------------------------------

def test_frameTrace_returns_expected_chains():
    expected_return = [
        [-74000, -74900, 1],
        [-74021, -74020, -74699, -74690, -74000],
    ]
    with patch("pyspiceql.frameTrace", return_value=(expected_return, CK_KERNELS)):
        response = client.get("/frameTrace", params={
            "et": 690201382.5595295,
            "initialFrame": -74021,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# extractExactCkTimes
# ---------------------------------------------------------------------------

def test_extractExactCkTimes_returns_expected_times():
    expected_return = [
        690201375.8001044, 690201375.9005225, 690201376.0001376,
        690201376.1005514, 690201376.2003593, 690201376.3007555,
        690201376.4001434, 690201376.5005605, 690201376.6001616,
        690201376.7006094, 690201376.8001995, 690201376.9005914,
        690201377.0002506, 690201377.1006265, 690201377.2001615,
        690201377.3006384, 690201377.4002154, 690201377.5006325,
        690201377.6001914, 690201377.7006304, 690201377.8002934,
        690201377.9007006, 690201378.0002494, 690201378.1006765,
        690201378.2002484, 690201378.3007184, 690201378.4002284,
        690201378.5006794, 690201378.6003283, 690201378.7007493,
        690201378.8002504, 690201378.9007055, 690201379.0002974,
        690201379.1006984, 690201379.2002794, 690201379.3007773,
        690201379.4004143, 690201379.5007674, 690201379.6002804,
        690201379.7012802, 690201379.8006243, 690201379.9007763,
        690201380.0003043, 690201380.1007553, 690201380.2004362,
        690201380.3008415, 690201380.4003323, 690201380.5007504,
        690201380.6003684, 690201380.7007734, 690201380.8003484,
        690201380.9007963, 690201381.0004884, 690201381.1008502,
        690201381.2003893, 690201381.3008813, 690201381.4004093,
        690201381.5008324, 690201381.6003863, 690201381.7008892,
        690201381.8004864, 690201381.9009064, 690201382.0004803,
        690201382.1008643, 690201382.2004443, 690201382.3009053,
        690201382.4004704, 690201382.5008882, 690201382.6009352,
        690201382.7011213, 690201382.8006132, 690201382.9009142,
        690201383.0005112, 690201383.1009102, 690201383.2005081,
        690201383.3010123, 690201383.4005923, 690201383.5009692,
        690201383.6004891, 690201383.7008853, 690201383.8005042,
        690201383.9009511, 690201384.0005193, 690201384.1009772,
        690201384.2006862, 690201384.3010433, 690201384.4005322,
        690201384.5009923, 690201384.6005883, 690201384.7010853,
        690201384.8005362, 690201384.9010303, 690201385.0006411,
        690201385.1010532, 690201385.2006001, 690201385.3010662,
        690201385.4006102, 690201385.5010152, 690201385.6006303,
        690201385.7010381, 690201385.8008661, 690201385.9011462,
        690201386.0006802, 690201386.1010602, 690201386.2006671,
        690201386.3011441, 690201386.4006581, 690201386.5010881,
        690201386.6007452, 690201386.7011452, 690201386.8006532,
        690201386.901147,  690201387.000686,  690201387.1011051,
        690201387.200734,  690201387.3011972, 690201387.4007832,
        690201387.5011872, 690201387.600747,  690201387.7011622,
        690201387.8007251, 690201387.9011792, 690201388.0007602,
        690201388.1012081, 690201388.2008182, 690201388.3012551,
        690201388.4008032, 690201388.5012511, 690201388.600788,
        690201388.701203,  690201388.800896,  690201388.9012221,
        690201389.000851,  690201389.1012791, 690201389.200796,
        690201389.301267,
    ]
    with patch("pyspiceql.extractExactCkTimes", return_value=(expected_return, CK_KERNELS)):
        response = client.get("/extractExactCkTimes", params={
            "observStart": 690201375.8323615,
            "observEnd": 690201389.2866975,
            "targetFrame": -74021,
            "mission": "ctx",
            "searchKernels": "true",
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return


# ---------------------------------------------------------------------------
# getExactTargetOrientations
# ---------------------------------------------------------------------------

def test_getExactTargetOrientations_returns_expected_first_row():
    # The docs show ~130 rows; we stub the full first-row value only.
    expected_first_row = [
        690201375.8001044,
        0.9999924134600601,
        0.0005720078450331138,
        0.003853027964066137,
        -2.2039789431520754e-06,
        0.0,
        0.0,
        0.0,
    ]
    expected_return = [expected_first_row]  # stub returns just the one row
    with patch("pyspiceql.getExactTargetOrientations", return_value=(expected_return, CK_KERNELS)):
        response = client.get("/getExactTargetOrientations", params={
            "startEt": 690201375.8323615,
            "stopEt": 690201389.2866975,
            "toFrame": -74000,
            "refFrame": -74690,
            "exactCkFrame" : 52,
            "mission": "ctx",
            "searchKernels": "true",
        })
    print(response)
    assert response.status_code == 200
    assert response.json()["body"]["return"][0] == expected_first_row


# ---------------------------------------------------------------------------
# searchForKernelsets
# ---------------------------------------------------------------------------

def test_searchForKernelsets_returns_expected_kernels():
    expected_return = {
        "ck": ["/odyssey/kernels/ck/odyssey_sc_ext67.bc"],
        "spk": ["/odyssey/kernels/spk/ody_ext67.bsp"],
    }
    with patch("pyspiceql.searchForKernelsets", return_value=(expected_return, {})):
        response = client.get("/searchForKernelsets", params={
            "missions": '["odyssey","mars"]',
            "kernelTypes": '["sclk","spk","tspk","ck"]',
            "startEt": 715662878.32324,
            "stopEt": 715663065.2303,
        })
    assert response.status_code == 200
    assert response.json()["body"]["return"] == expected_return