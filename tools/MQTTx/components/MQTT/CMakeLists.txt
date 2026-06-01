
set(srcs "MQTT.c")
set(include_dir ".")

idf_component_register(SRCS "${srcs}"
                     INCLUDE_DIRS "${include_dir}"
                     REQUIRES mqtt)
