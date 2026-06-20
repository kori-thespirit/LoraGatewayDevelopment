# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

# Set path(s) to extra components
set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../middleware/common/lora_protocol/
    ${CMAKE_CURRENT_SOURCE_DIR}/../middleware/common/modbus_payload_handle/
/
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(core_gateway)


set(CERTIFICATE_PATH components/network/mqtts/certificates)
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "${CERTIFICATE_PATH}/kolabori.pem" TEXT)



