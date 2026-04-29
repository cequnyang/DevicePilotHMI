add_library(test_common INTERFACE)

target_link_libraries(test_common INTERFACE
    Qt6::Test
    DevicePilotHMICore
    project_options
    project_warnings
)

function(configure_test_runtime target_name test_name)
    if (WIN32)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target_name}>
                $<TARGET_FILE_DIR:${target_name}>
            COMMAND_EXPAND_LISTS
        )
    endif()

    set_tests_properties(${test_name} PROPERTIES
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${target_name}>
    )
endfunction()

function(add_devicepilothmi_test target_name source_file)
    add_executable(${target_name}
        ${source_file}
    )

    target_link_libraries(${target_name} PRIVATE
        test_common
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
    configure_test_runtime(${target_name} ${target_name})
endfunction()

add_devicepilothmi_test(settings_validation_tests settings_validation_test.cpp)
add_devicepilothmi_test(settings_json_codec_tests settings_json_codec_test.cpp)
add_devicepilothmi_test(settings_file_store_tests settings_file_store_test.cpp)
add_devicepilothmi_test(settings_apply_service_tests settings_apply_service_test.cpp)
add_devicepilothmi_test(machine_runtime_tests machine_runtime_test.cpp)
add_devicepilothmi_test(settings_manager_tests settings_manager_test.cpp)
add_devicepilothmi_test(settings_draft_tests settings_draft_test.cpp)
add_devicepilothmi_test(settings_session_tests settings_session_test.cpp)
add_devicepilothmi_test(alarm_manager_tests alarm_manager_test.cpp)
add_devicepilothmi_test(simulated_machine_backend_tests simulated_machine_backend_test.cpp)

set_target_properties(settings_manager_tests PROPERTIES
    AUTOMOC_COMPILER_PREDEFINES OFF
)
