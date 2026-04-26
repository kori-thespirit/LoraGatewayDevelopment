idf_component_register(
    SRCS ""   # no source files, just headers
    INCLUDE_DIRS "include"
)
add_subdirectory(wifi_app) 
add_subdirectory(mqtts)


