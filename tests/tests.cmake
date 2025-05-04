
target_compile_definitions(${PROJECT_NAME} PRIVATE
    HAGOROMO_UNIT_TEST
)

if(LINUX)
    set_tests_properties(${PROJECT_NAME} PROPERTIES
        "--platform" "offscreen"
    )
endif()
