set(srcs "test.c")

set(include_dirs ".")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "${include_dirs}"
                    PRIV_REQUIRES unity)
