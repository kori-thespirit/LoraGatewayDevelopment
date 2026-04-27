# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(unit-test_mqtt_tls_ssl_vps_broker)

set(CERTIFICATE_PATH components/network/mqtts/certificates)

target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "${CERTIFICATE_PATH}/client.crt" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "${CERTIFICATE_PATH}/client.key" TEXT)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "${CERTIFICATE_PATH}/kolabori.crt" TEXT)




