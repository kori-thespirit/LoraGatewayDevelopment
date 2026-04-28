def pytest_configure(config):
    config.addinivalue_line(
        "usb_device", "marks tests that require the USJ device"
    )
