function(infra_add_test cpp_name)
    add_executable(${cpp_name} ${cpp_name}.cpp)

    if(MSVC)
        target_compile_options(${cpp_name} PRIVATE /utf-8 /W4 /WX)
        target_link_options(${cpp_name} PRIVATE /WX)
        target_compile_definitions(${cpp_name} PRIVATE UNICODE _UNICODE)
    else()
        target_compile_options(${cpp_name} PRIVATE -Werror -Wall)
    endif()

    target_link_libraries(${cpp_name} PRIVATE infra)
    target_include_directories(${cpp_name} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME ${cpp_name} COMMAND $<TARGET_FILE:${cpp_name}>)
endfunction()

configure_file(test_config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test_config.hpp)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_files/test_file.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

infra_add_test(compile_succeed)
infra_add_test(x86_64)
infra_add_test(x86_32)
infra_add_test(binary_serialization)
infra_add_test(enum_util)
