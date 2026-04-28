def pytest_configure(config):
    config.addinivalue_line(
        "markers", "usb_device: marks tests that require USB device"
    )
