idf_component_register(SRCS "main.cpp" "network_catalog.cpp" "security_assessment.cpp" INCLUDE_DIRS ".")

target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_20)
target_compile_options(
  ${COMPONENT_LIB}
  PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wconversion
    -Wshadow
    -fanalyzer
    -fno-exceptions
    -fno-rtti
)
