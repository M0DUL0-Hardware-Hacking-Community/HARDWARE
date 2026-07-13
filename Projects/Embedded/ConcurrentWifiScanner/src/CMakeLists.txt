idf_component_register(SRCS "main.c" "network_catalog.c" "security_assessment.c" INCLUDE_DIRS ".")

target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wshadow
    -Wstrict-prototypes
    -fanalyzer
    -std=gnu11
)
