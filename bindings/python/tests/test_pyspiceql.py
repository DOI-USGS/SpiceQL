import pytest
from pyspiceql import getMissionConfig, Config, getKernelStringValue

def test_jsonConversion():
    lro_config = getMissionConfig('lro')
    assert isinstance(lro_config, dict)
    kernel_list = lro_config["lro"]["iak"]["kernels"]
    assert isinstance(kernel_list, list)

def test_config():
    global_config = Config()
    lro_config = global_config['lro']

def test_exception():
    with pytest.raises(RuntimeError):
        getKernelStringValue("bad_terrible_no_good_key")
