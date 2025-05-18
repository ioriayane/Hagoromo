
target_compile_definitions(${PROJECT_NAME} PRIVATE
    HAGOROMO_UNIT_TEST
)

if(UNIX)
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} --platform offscreen)
else()
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
endif()
