idf_component_register(SRCS "main.c" "lora_task.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES network lora
)
