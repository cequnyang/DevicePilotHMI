cmake_minimum_required(VERSION 3.25)

project(DevicePilotHMI VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

if (NOT CMAKE_PREFIX_PATH AND DEFINED ENV{QT_ROOT_DIR} AND NOT "$ENV{QT_ROOT_DIR}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{QT_ROOT_DIR}" DEVICEPILOTHMI_QT_ROOT_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "${DEVICEPILOTHMI_QT_ROOT_DIR}")
endif()

if (NOT CMAKE_PREFIX_PATH AND UNIX AND NOT APPLE)
    file(GLOB DEVICEPILOTHMI_LINUX_QT_CANDIDATES
        "$ENV{HOME}/Qt/6.*/gcc_64"
        "/opt/Qt/6.*/gcc_64"
    )
    if (DEVICEPILOTHMI_LINUX_QT_CANDIDATES)
        list(SORT DEVICEPILOTHMI_LINUX_QT_CANDIDATES COMPARE NATURAL ORDER DESCENDING)
        list(GET DEVICEPILOTHMI_LINUX_QT_CANDIDATES 0 DEVICEPILOTHMI_QT_ROOT_DIR)
        list(PREPEND CMAKE_PREFIX_PATH "${DEVICEPILOTHMI_QT_ROOT_DIR}")
        message(STATUS "Using detected Qt root: ${DEVICEPILOTHMI_QT_ROOT_DIR}")
    endif()
endif()

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
)

if (MINGW)
    target_compile_options(project_warnings INTERFACE
        -Wa,-mbig-obj
    )
endif()

find_package(Qt6 QUIET COMPONENTS Core Quick Qml QuickControls2 Test)

if (NOT Qt6_FOUND)
    message(FATAL_ERROR
        "Qt 6 with the required Quick, QML, QuickControls2, and Test components was not found.\n"
        "\n"
        "How to fix it:\n"
        "  - Command line on Linux: run ./scripts/build.sh --preset linux-ninja-debug --qt-root /path/to/Qt/6.11.0/gcc_64\n"
        "  - Qt Creator on Linux/WSL: use build/linux-ninja-debug as the build directory, not build/linux-ninja-debug/stage.\n"
        "  - Qt Creator on Linux/WSL: set QT_ROOT_DIR or CMAKE_PREFIX_PATH to your Qt 6 root before opening the project,\n"
        "    or create a local CMakeUserPresets.json based on CMakeUserPresets.json.example and select linux-ninja-debug-local.\n"
        "  - If Qt is installed through apt, verify the distro package includes Qt Quick Controls 2 and Qt Test development files.\n"
        "  - Windows: run ./scripts/setup_windows_qt_env.ps1 first.\n"
        "\n"
        "Expected Qt root examples:\n"
        "  - Linux: /home/<user>/Qt/6.11.0/gcc_64\n"
        "  - Windows MinGW: C:/Qt/6.11.0/mingw_64\n"
        "  - Windows MSVC: C:/Qt/6.11.0/msvc2022_64\n"
    )
endif()

qt_standard_project_setup(REQUIRES 6.11)

set(DEVICEPILOTHMI_CORE_SOURCES
    src/backend/machine_backend.h
    src/backend/machine_backend.cpp
    src/backend/simulated_machine_backend.h
    src/backend/simulated_machine_backend.cpp
    src/backend/simulation_scenario.h
    src/backend/simulation_scenario.cpp
    src/backend/simulation_control.h
    src/backend/simulation_control.cpp
    src/backend/simulation_strategy.h
    src/backend/simulation_strategy.cpp
    src/log/log_event.h
    src/log/app_logging.h
    src/log/app_logging.cpp
    src/log/log_model.h
    src/log/log_model.cpp
    src/log/log_filter_proxy_model.h
    src/log/log_filter_proxy_model.cpp
    src/log/log_interface.h
    src/log/log_interface.cpp
    src/settings/settings_defined.h
    src/settings/settings_presets.h
    src/settings/settings_presets.cpp
    src/settings/settings_manager.h
    src/settings/settings_manager.cpp
    src/settings/settings_draft.h
    src/settings/settings_draft.cpp
    src/settings/settings_session.h
    src/settings/settings_session.cpp
    src/runtime/machine_runtime.h
    src/runtime/machine_runtime.cpp
    src/runtime/machine_types.h
    src/alarm/alarm_manager.h
    src/alarm/alarm_manager.cpp
    src/settings/settings_file_store.cpp
    src/settings/settings_file_store.h
    src/settings/settings_json_codec.cpp
    src/settings/settings_json_codec.h
    src/settings/settings_validation.cpp
    src/settings/settings_validation.h
    src/settings/settings_apply_service.cpp
    src/settings/settings_apply_service.h
)

add_library(DevicePilotHMICore STATIC
    ${DEVICEPILOTHMI_CORE_SOURCES}
)

target_include_directories(DevicePilotHMICore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/log
        ${CMAKE_CURRENT_SOURCE_DIR}/src/alarm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
        ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime
        ${CMAKE_CURRENT_SOURCE_DIR}/src/settings
)

target_link_libraries(DevicePilotHMICore
    PUBLIC
        Qt6::Core
        Qt6::Qml
    PRIVATE
        project_options
        project_warnings
)

qt_add_executable(DevicePilotHMI
    src/main.cpp
)

qt_add_qml_module(DevicePilotHMI
    URI DevicePilotHMI
    VERSION 1.0

    QML_FILES
        qml/Main.qml
        qml/pages/DashboardPage.qml
        qml/pages/LogPage.qml
        qml/pages/SettingsPage.qml
        qml/components/StatusBanner.qml
        qml/components/MetricCard.qml
        qml/components/ControlPanel.qml
        qml/components/StateContextCard.qml
        qml/components/TrendChart.qml
)

set_source_files_properties(assets/icons/app_icon_normal.png PROPERTIES
    QT_RESOURCE_ALIAS icons/app_icon_normal.png
)
set_source_files_properties(assets/icons/app_icon_warning.png PROPERTIES
    QT_RESOURCE_ALIAS icons/app_icon_warning.png
)
set_source_files_properties(assets/icons/app_icon_fault.png PROPERTIES
    QT_RESOURCE_ALIAS icons/app_icon_fault.png
)

qt_add_resources(DevicePilotHMI devicepilothmi_assets
    PREFIX "/"
    FILES
        assets/icons/app_icon_normal.png
        assets/icons/app_icon_warning.png
        assets/icons/app_icon_fault.png
)

if (WIN32)
    target_sources(DevicePilotHMI PRIVATE
        resources/windows/app_icon.rc
    )
endif()

set_target_properties(DevicePilotHMI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(DevicePilotHMI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alarm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings
)

target_link_libraries(DevicePilotHMI
    PRIVATE
        DevicePilotHMICore
        Qt6::Quick
        Qt6::QuickControls2
        project_options
        project_warnings
)

if (WIN32)
    find_program(DEVICEPILOTHMI_WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt
        HINTS "${Qt6_DIR}/../../../bin"
    )

    if (DEVICEPILOTHMI_WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET DevicePilotHMI POST_BUILD
            COMMAND "${DEVICEPILOTHMI_WINDEPLOYQT_EXECUTABLE}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                "$<TARGET_FILE:DevicePilotHMI>"
            COMMAND_EXPAND_LISTS
        )
    else()
        add_custom_command(TARGET DevicePilotHMI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:DevicePilotHMI>
                $<TARGET_FILE_DIR:DevicePilotHMI>
            COMMAND_EXPAND_LISTS
        )

        if (TARGET Qt6::QWindowsIntegrationPlugin)
            add_custom_command(TARGET DevicePilotHMI POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:DevicePilotHMI>/platforms"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>"
                    "$<TARGET_FILE_DIR:DevicePilotHMI>/platforms"
                COMMAND_EXPAND_LISTS
            )
        else()
            message(WARNING
                "windeployqt and Qt6::QWindowsIntegrationPlugin were not found. "
                "Running DevicePilotHMI from the build tree may require deploying Qt runtime files manually."
            )
        endif()
    endif()
endif()

install(TARGETS DevicePilotHMI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET DevicePilotHMI
    OUTPUT_SCRIPT devicepilothmi_deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)

install(SCRIPT ${devicepilothmi_deploy_script})

include(CTest)
enable_testing()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
