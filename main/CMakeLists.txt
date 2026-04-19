set(src "main.c")
set(include ".")

idf_component_register(SRCS "${src}"
                    INCLUDE_DIRS "${include}"
                    )
