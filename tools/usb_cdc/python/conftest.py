def pytest_configure(config):
    config.addinivalue_line(
        "usj_device", "marks tests that require the USJ devic"
    )
