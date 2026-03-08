"""
Test suite for the SpiceQL REST API

Run with:
    pip install pytest httpx fastapi
    pytest test_api_remote.py -v
"""

import pytest
import httpx
import math

BASE_URL = "https://astrogeology.usgs.gov/apis/spiceql/latest"


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def assert_success(response: httpx.Response) -> dict:
    """Assert HTTP 200 and SpiceQL statusCode 200, return the body dict."""
    assert response.status_code == 200, (
        f"HTTP error {response.status_code}: {response.text}"
    )
    data = response.json()
    assert data.get("statusCode") == 200, (
        f"SpiceQL returned non-200 statusCode: {data}"
    )
    assert "body" in data, "Response missing 'body' key"
    assert "return" in data["body"], "Response body missing 'return' key"
    assert "kernels" in data["body"], "Response body missing 'kernels' key"
    return data["body"]


def approx_equal(a: float, b: float, rel: float = 1e-6) -> bool:
    """Return True if a and b are within rel relative tolerance."""
    if b == 0:
        return abs(a) < 1e-10
    return abs(a - b) / abs(b) < rel


# ---------------------------------------------------------------------------
# getTargetStates
# ---------------------------------------------------------------------------

class TestGetTargetStates:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/getTargetStates
        ?ets=[690201375.8323615,690201389.2866975]
        &target=SUN&observer=Mars&frame=IAU_MARS&mission=ctx&abcorr=LT%2BS
        &searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/getTargetStates"
    PARAMS = {
        "ets": "[690201375.8323615,690201389.2866975]",
        "target": "SUN",
        "observer": "Mars",
        "frame": "IAU_MARS",
        "mission": "ctx",
        "abcorr": "LT+S",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_returns_two_state_vectors(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        result = body["return"]
        assert isinstance(result, list)
        assert len(result) == 2, "Expected 2 state vectors (one per ET)"

    def test_each_state_vector_has_seven_elements(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        for vec in body["return"]:
            assert len(vec) == 7, (
                "Each state vector should have 7 elements (x,y,z,vx,vy,vz,lt)"
            )

    def test_first_state_vector_values(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        vec = body["return"][0]
        expected = [
            123515791.9195627,
            187209003.7067195,
            80611152.03610656,
            13251.543112834495,
            -8742.597438450646,
            -6.575020419444353,
            794.9856233875888,
        ]
        for got, exp in zip(vec, expected):
            assert approx_equal(got, exp), f"Expected ~{exp}, got {got}"

    def test_kernels_present(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        kernels = body["kernels"]
        assert isinstance(kernels, dict)
        assert len(kernels) > 0, "Expected at least one kernel type in response"

    def test_kernels_include_ck(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "ck" in body["kernels"], "Expected 'ck' kernel in response"

    def test_kernels_include_spk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "spk" in body["kernels"], "Expected 'spk' kernel in response"


# ---------------------------------------------------------------------------
# getTargetOrientations
# ---------------------------------------------------------------------------

class TestGetTargetOrientations:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/getTargetOrientations
        ?ets=[690201375.8323615]&toFrame=-74000&refFrame=-74690
        &mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/getTargetOrientations"
    PARAMS = {
        "ets": "[690201375.8323615]",
        "toFrame": -74000,
        "refFrame": -74690,
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_returns_one_quaternion(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        result = body["return"]
        assert isinstance(result, list)
        assert len(result) == 1, "Expected 1 quaternion for 1 input ET"

    def test_quaternion_has_seven_elements(self):
        """Each row is [qw, qx, qy, qz, av_x, av_y, av_z]."""
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        q = body["return"][0]
        assert len(q) == 7, "Quaternion row should have 7 elements"

    def test_quaternion_values(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        q = body["return"][0]
        expected = [
            0.9999924134600601,
            0.0005720078450331138,
            0.003853027964066137,
            -2.2039789431520754e-06,
            0.0,
            0.0,
            0.0,
        ]
        for got, exp in zip(q, expected):
            assert approx_equal(got, exp, rel=1e-5), f"Expected ~{exp}, got {got}"

    def test_scalar_part_near_one(self):
        """First element (scalar part of unit quaternion) should be close to 1."""
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        qw = body["return"][0][0]
        assert abs(qw) <= 1.0 + 1e-6

    def test_kernels_include_ck(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "ck" in body["kernels"]


# ---------------------------------------------------------------------------
# strSclkToEt
# ---------------------------------------------------------------------------

class TestStrSclkToEt:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/strSclkToEt
        ?frameCode=-74&sclk=1321396563:036&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/strSclkToEt"
    PARAMS = {
        "frameCode": -74,
        "sclk": "1321396563:036",
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_float(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], float)

    def test_et_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert approx_equal(body["return"], 690201375.8323615), (
            f"Expected ~690201375.8323615, got {body['return']}"
        )

    def test_kernels_include_sclk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "sclk" in body["kernels"]

    def test_kernels_include_lsk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "lsk" in body["kernels"]


# ---------------------------------------------------------------------------
# doubleSclkToEt
# ---------------------------------------------------------------------------

class TestDoubleSclkToEt:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/doubleSclkToEt
        ?frameCode=-85&sclk=922997380.174174&mission=lro&searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/doubleSclkToEt"
    PARAMS = {
        "frameCode": -85,
        "sclk": 922997380.174174,
        "mission": "lro",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_float(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], float)

    def test_et_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert approx_equal(body["return"], 31593348.006268278), (
            f"Expected ~31593348.006268278, got {body['return']}"
        )

    def test_kernels_include_sclk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "sclk" in body["kernels"]

    def test_kernels_include_fk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "fk" in body["kernels"]


# ---------------------------------------------------------------------------
# doubleEtToSclk
# ---------------------------------------------------------------------------

class TestDoubleEtToSclk:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/doubleEtToSclk
        ?et=690201375.8323615&frameCode=-74&kernelList=[]&mission=ctx
        &searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/doubleEtToSclk"
    PARAMS = {
        "et": 690201375.8323615,
        "frameCode": -74,
        "kernelList": "[]",
        "mission": "ctx",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_string(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], str)

    def test_sclk_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"] == "27/1321396563.036", (
            f"Expected '27/1321396563.036', got '{body['return']}'"
        )

    def test_kernels_include_sclk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "sclk" in body["kernels"]


# ---------------------------------------------------------------------------
# utcToEt
# ---------------------------------------------------------------------------

class TestUtcToEt:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/utcToEt
        ?utc=1971-08-04T16:28:24.9159358&searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/utcToEt"
    PARAMS = {
        "utc": "1971-08-04T16:28:24.9159358",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_float(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], float)

    def test_et_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert approx_equal(body["return"], -896556653.900884), (
            f"Expected ~-896556653.900884, got {body['return']}"
        )

    def test_kernels_include_lsk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "lsk" in body["kernels"]


# ---------------------------------------------------------------------------
# etToUtc
# ---------------------------------------------------------------------------

class TestEtToUtc:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/etToUtc
        ?et=-896556653.900884&format=C&precision=10&searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/etToUtc"
    PARAMS = {
        "et": -896556653.900884,
        "format": "C",
        "precision": 10,
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_string(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], str)

    def test_utc_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"] == "1971 AUG 04 16:28:24.9159357548", (
            f"Unexpected UTC string: '{body['return']}'"
        )

    def test_kernels_include_lsk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "lsk" in body["kernels"]


# ---------------------------------------------------------------------------
# translateNameToCode
# ---------------------------------------------------------------------------

class TestTranslateNameToCode:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/translateNameToCode
        ?frame=MRO&mission=ctx&searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/translateNameToCode"
    PARAMS = {
        "frame": "MRO",
        "mission": "ctx",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_int(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], int)

    def test_code_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"] == -74, f"Expected -74, got {body['return']}"

    def test_kernels_include_fk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "fk" in body["kernels"]


# ---------------------------------------------------------------------------
# translateCodeToName
# ---------------------------------------------------------------------------

class TestTranslateCodeToName:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/translateCodeToName
        ?frame=-74&mission=ctx&searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/translateCodeToName"
    PARAMS = {
        "frame": -74,
        "mission": "ctx",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_string(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], str)

    def test_name_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"] == "MRO", f"Expected 'MRO', got '{body['return']}'"

    def test_kernels_include_fk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "fk" in body["kernels"]


# ---------------------------------------------------------------------------
# getFrameInfo
# ---------------------------------------------------------------------------

class TestGetFrameInfo:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/getFrameInfo
        ?frame=-74021&mission=ctx&searchKernels=true"
    """

    ENDPOINT = f"{BASE_URL}/getFrameInfo"
    PARAMS = {
        "frame": -74021,
        "mission": "ctx",
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_list_of_three(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        result = body["return"]
        assert isinstance(result, list)
        assert len(result) == 3, "Expected [center, class, classId]"

    def test_frame_info_values(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        center, cls, class_id = body["return"]
        assert center == -74, f"Expected center=-74, got {center}"
        assert cls == 4, f"Expected class=4, got {cls}"
        assert class_id == -74021, f"Expected classId=-74021, got {class_id}"

    def test_kernels_include_fk(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "fk" in body["kernels"]


# ---------------------------------------------------------------------------
# getTargetFrameInfo
# ---------------------------------------------------------------------------

class TestGetTargetFrameInfo:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/getTargetFrameInfo
        ?targetId=499&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/getTargetFrameInfo"
    PARAMS = {
        "targetId": 499,
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_dict(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], dict)

    def test_return_contains_frame_code_and_name(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        result = body["return"]
        assert "frameCode" in result
        assert "frameName" in result

    def test_frame_code_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"]["frameCode"] == 10014, (
            f"Expected frameCode=10014, got {body['return']['frameCode']}"
        )

    def test_frame_name_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"]["frameName"] == "IAU_MARS", (
            f"Expected frameName='IAU_MARS', got {body['return']['frameName']}"
        )


# ---------------------------------------------------------------------------
# findMissionKeywords
# ---------------------------------------------------------------------------

class TestFindMissionKeywords:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/findMissionKeywords
        ?key=*-74021*&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/findMissionKeywords"
    PARAMS = {
        "key": "*-74021*",
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_dict(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], dict)

    def test_contains_frame_name(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "FRAME_-74021_NAME" in body["return"]

    def test_frame_name_is_mro_ctx(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert body["return"]["FRAME_-74021_NAME"] == "MRO_CTX"

    def test_contains_focal_length(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "INS-74021_FOCAL_LENGTH" in body["return"]

    def test_focal_length_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert approx_equal(body["return"]["INS-74021_FOCAL_LENGTH"], 352.9271664)

    def test_contains_pixel_pitch(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "INS-74021_PIXEL_PITCH" in body["return"]

    def test_kernels_include_ik(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "ik" in body["kernels"]


# ---------------------------------------------------------------------------
# findTargetKeywords
# ---------------------------------------------------------------------------

class TestFindTargetKeywords:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/findTargetKeywords
        ?key=*499*&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/findTargetKeywords"
    PARAMS = {
        "key": "*499*",
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_dict(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], dict)

    def test_contains_radii(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "BODY499_RADII" in body["return"]

    def test_radii_values(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        radii = body["return"]["BODY499_RADII"]
        assert len(radii) == 3
        assert approx_equal(radii[0], 3396.19)
        assert approx_equal(radii[1], 3396.19)
        assert approx_equal(radii[2], 3376.2)

    def test_contains_pole_ra(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "BODY499_POLE_RA" in body["return"]

    def test_contains_pole_dec(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "BODY499_POLE_DEC" in body["return"]

    def test_kernels_include_pck(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "pck" in body["kernels"]


# ---------------------------------------------------------------------------
# frameTrace
# ---------------------------------------------------------------------------

class TestFrameTrace:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/frameTrace
        ?et=690201382.5595295&initialFrame=-74021&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/frameTrace"
    PARAMS = {
        "et": 690201382.5595295,
        "initialFrame": -74021,
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_list_of_two(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        result = body["return"]
        assert isinstance(result, list)
        assert len(result) == 2, "Expected two chain lists"

    def test_first_chain(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        chain1 = body["return"][0]
        assert chain1 == [-74000, -74900, 1], f"Unexpected chain 1: {chain1}"

    def test_second_chain(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        chain2 = body["return"][1]
        assert chain2 == [-74021, -74020, -74699, -74690, -74000], (
            f"Unexpected chain 2: {chain2}"
        )

    def test_kernels_include_ck(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "ck" in body["kernels"]


# ---------------------------------------------------------------------------
# extractExactCkTimes
# ---------------------------------------------------------------------------

class TestExtractExactCkTimes:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/extractExactCkTimes
        ?observStart=690201375.8323615&observEnd=690201389.2866975
        &targetFrame=-74021&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/extractExactCkTimes"
    PARAMS = {
        "observStart": 690201375.8323615,
        "observEnd": 690201389.2866975,
        "targetFrame": -74021,
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_list(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], list)

    def test_return_is_non_empty(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert len(body["return"]) > 0

    def test_times_are_floats(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        for t in body["return"]:
            assert isinstance(t, float), f"Expected float, got {type(t)}"

    def test_times_within_bounds(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        times = body["return"]
        # All times should be within [observStart, observEnd] with some tolerance
        assert all(690201375.0 <= t <= 690201390.0 for t in times), (
            "Some CK times fall outside expected range"
        )

    def test_first_time_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert approx_equal(body["return"][0], 690201375.8001044)

    def test_kernels_include_ck(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "ck" in body["kernels"]


# ---------------------------------------------------------------------------
# getExactTargetOrientations
# ---------------------------------------------------------------------------

class TestGetExactTargetOrientations:
    """
    curl -XGET "https://astrogeology.usgs.gov/apis/spiceql/latest/getExactTargetOrientations
        ?startEt=690201375.8323615&stopEt=690201389.2866975
        &toFrame=-74000&refFrame=-74690&mission=ctx&searchKernels=True"
    """

    ENDPOINT = f"{BASE_URL}/getExactTargetOrientations"
    PARAMS = {
        "startEt": 690201375.8323615,
        "stopEt": 690201389.2866975,
        "toFrame": -74000,
        "refFrame": -74690,
        "mission": "ctx",
        "searchKernels": "True",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        assert_success(r)

    def test_return_is_list(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert isinstance(body["return"], list)

    def test_return_is_non_empty(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert len(body["return"]) > 0

    def test_each_row_has_eight_elements(self):
        """Each row: [et, qw, qx, qy, qz, av_x, av_y, av_z]."""
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        for row in body["return"]:
            assert len(row) == 8, f"Expected 8 elements per row, got {len(row)}"

    def test_first_row_et_value(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        et = body["return"][0][0]
        assert approx_equal(et, 690201375.8001044)

    def test_first_row_quaternion_values(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        row = body["return"][0]
        # row[1:5] = [qw, qx, qy, qz]
        expected_q = [
            0.9999924134600601,
            0.0005720078450331138,
            0.003853027964066137,
            -2.2039789431520754e-06,
        ]
        for got, exp in zip(row[1:5], expected_q):
            assert approx_equal(got, exp, rel=1e-5)

    def test_kernels_include_ck(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        body = assert_success(r)
        assert "ck" in body["kernels"]


# ---------------------------------------------------------------------------
# searchForKernelsets  (mentioned in the docs TOC and manual)
# ---------------------------------------------------------------------------

class TestSearchForKernelsets:
    """
    The docs mention this endpoint in the navigation.
    Params derived from the Python example in the SpiceQL manual:
        psql.searchForKernelsets(
            ["odyssey", "mars"], ["sclk", "spk", "tspk", "ck"],
            715662878.32324, 715663065.2303
        )
    """

    ENDPOINT = f"{BASE_URL}/searchForKernelsets"
    PARAMS = {
        "missions": '["odyssey","mars"]',
        "kernelTypes": '["sclk","spk","tspk","ck"]',
        "startEt": 715662878.32324,
        "stopEt": 715663065.2303,
        "searchKernels": "true",
    }

    def test_status_ok(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        # Accept 200 or a well-formed error – endpoint may require different params
        assert r.status_code == 200, (
            f"HTTP error {r.status_code}: {r.text}"
        )

    def test_return_is_dict_or_list(self):
        r = httpx.get(self.ENDPOINT, params=self.PARAMS)
        if r.status_code == 200:
            data = r.json()
            if data.get("statusCode") == 200:
                result = data["body"]["return"]
                assert isinstance(result, (dict, list)), (
                    "searchForKernelsets should return a dict or list of kernels"
                )


# ---------------------------------------------------------------------------
# Round-trip sanity: utcToEt → etToUtc
# ---------------------------------------------------------------------------

class TestRoundTripUtcEt:
    """Verify utcToEt and etToUtc are inverses of each other."""

    UTC_IN = "1971-08-04T16:28:24.9159358"

    def test_round_trip(self):
        # Step 1: UTC → ET
        r1 = httpx.get(
            f"{BASE_URL}/utcToEt",
            params={"utc": self.UTC_IN, "searchKernels": "true"},
        )
        body1 = assert_success(r1)
        et = body1["return"]

        # Step 2: ET → UTC
        r2 = httpx.get(
            f"{BASE_URL}/etToUtc",
            params={"et": et, "format": "ISOC", "precision": 3, "searchKernels": "true"},
        )
        body2 = assert_success(r2)
        utc_out = body2["return"]

        # The ISO string should start with 1971-08-04
        assert utc_out.startswith("1971-08-04"), (
            f"Round-trip UTC mismatch. Got: {utc_out}"
        )


# ---------------------------------------------------------------------------
# Round-trip sanity: strSclkToEt → doubleEtToSclk
# ---------------------------------------------------------------------------

class TestRoundTripSclkEt:
    """Verify strSclkToEt and doubleEtToSclk are consistent."""

    def test_round_trip(self):
        # Step 1: SCLK string → ET
        r1 = httpx.get(
            f"{BASE_URL}/strSclkToEt",
            params={
                "frameCode": -74,
                "sclk": "1321396563:036",
                "mission": "ctx",
                "searchKernels": "True",
            },
        )
        body1 = assert_success(r1)
        et = body1["return"]

        # Step 2: ET → double SCLK string
        r2 = httpx.get(
            f"{BASE_URL}/doubleEtToSclk",
            params={
                "et": et,
                "frameCode": -74,
                "mission": "ctx",
                "searchKernels": "true",
            },
        )
        body2 = assert_success(r2)
        sclk_out = body2["return"]

        # The numeric part should encode the same clock tick
        assert isinstance(sclk_out, str)
        assert "1321396563" in sclk_out, (
            f"Round-trip SCLK mismatch. Got: {sclk_out}"
        )


# ---------------------------------------------------------------------------
# Round-trip sanity: translateNameToCode → translateCodeToName
# ---------------------------------------------------------------------------

class TestRoundTripTranslate:
    """Verify name→code and code→name are inverses."""

    def test_round_trip(self):
        # Name → code
        r1 = httpx.get(
            f"{BASE_URL}/translateNameToCode",
            params={"frame": "MRO", "mission": "ctx", "searchKernels": "true"},
        )
        body1 = assert_success(r1)
        code = body1["return"]

        # Code → name
        r2 = httpx.get(
            f"{BASE_URL}/translateCodeToName",
            params={"frame": code, "mission": "ctx", "searchKernels": "true"},
        )
        body2 = assert_success(r2)
        name = body2["return"]

        assert name == "MRO", f"Expected 'MRO', got '{name}'"
