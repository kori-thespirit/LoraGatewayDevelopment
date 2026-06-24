
set(src "MQTT.c")
set(include_dir ".")

idf_component_register(SRCS "${src}"
                     INCLUDE_DIRS "${include_dir}" REQUIRES mqtt)
