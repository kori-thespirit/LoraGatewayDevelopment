import time
import pytest
from pytest_embedded import Dut
from pytest_embedded_idf.utils import idf_parametrize


@pytest.mark.usj_device
@idf_parametrize("target", ["esp32s3"], indirect=["target"])
def test_cmd_parser(dut: Dut):

    time.sleep(2)

    def send_and_check(cmd):
        print(f"\nSend: {cmd}")

        dut.write(cmd + "\n")
        dut.expect(cmd, timeout=3)
    send_and_check("IDLE")
    send_and_check("START")
    send_and_check("RUN")
    send_and_check("STOP")
