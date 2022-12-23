# SPDX-License-Identifier: BSD-3-Clause
cmake_minimum_required(VERSION 3.5)

project(SelfTrack VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "ui")

enable_language("RC")
set(app_icon_resource_windows ${CMAKE_CURRENT_SOURCE_DIR}/src/app.rc)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools Sql Charts Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql Charts Core Gui)

if(NOT DEFINED SELFTRACK_INSTALL_RUNTIME_DESTINATION)
    set(SELFTRACK_INSTALL_RUNTIME_DESTINATION "release")
endif()

set(TS_FILES SelfTrack_en_US.ts)

qt6_add_resources(WIN32_RESOURCES app.qrc)

set(PROJECT_SOURCES
        ui/timetracker.ui
        src/app.qrc
        src/main.cc
        src/db.cc
        src/tray.cc
        src/log.cc
        src/charts.cc
        src/events.cc
        src/notifications.cc
        src/tracking.cc
        src/settings.cc
        src/timetracker.cc
        src/timetracker.hpp
        src/clock/digitalclock.hpp src/clock/digitalclock.cc
        src/timer/timer.hpp src/timer/timer.cc
        ${TS_FILES}
)

if(Qt6_FOUND AND WIN32 AND TARGET Qt6::qmake AND NOT TARGET Qt6::windeployqt)
    get_target_property(_qt6_qmake_location Qt6::qmake IMPORTED_LOCATION)

    execute_process(
        COMMAND "${_qt6_qmake_location}" -query QT_INSTALL_PREFIX
        RESULT_VARIABLE return_code
        OUTPUT_VARIABLE qt6_install_prefix
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(imported_location "${qt6_install_prefix}/bin/windeployqt.exe")

    if(EXISTS ${imported_location})
        add_executable(Qt6::windeployqt IMPORTED)

        set_target_properties(Qt6::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()
endif()

add_compile_definitions(APP_NAME="TimeTracker")

qt6_add_resources(WIN32_RESOURCES src/app.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SelfTrack
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_resource_windows}
        ${WIN32_RESOURCES}
        src/app.qrc
    )

    if(TARGET Qt6::windeployqt)
        # execute windeployqt in a tmp directory after build
        add_custom_command(TARGET SelfTrack
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/release"
            COMMAND set PATH=%PATH%$<SEMICOLON>${qt6_install_prefix}/bin
            COMMAND Qt6::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/release" "$<TARGET_FILE_DIR:SelfTrack>/$<TARGET_FILE_NAME:SelfTrack>"
        )

        # copy deployment directory during installation
        install(
            DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/" DESTINATION ${SELFTRACK_INSTALL_RUNTIME_DESTINATION}
        )
    endif()
endif()

target_link_libraries(SelfTrack PUBLIC Qt6::Widgets Qt6::Sql Qt6::Charts User32 wtsapi32 Kernel32)

target_include_directories(SelfTrack INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/ui"

)

set_target_properties(SelfTrack PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SelfTrack)
endif()
