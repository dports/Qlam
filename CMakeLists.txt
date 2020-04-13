cmake_minimum_required(VERSION 3.10)

# set the project name
project(Qlam LANGUAGES CXX VERSION 1.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)
find_package(Threads REQUIRED)

# add the executable
add_executable(qlam
    src/main.cpp
    src/infectedfile.cpp
    src/mainwindow.cpp
    src/scanwidget.cpp
    src/scanner.cpp
    src/scanprofile.cpp
    src/scanprofilechooser.cpp
    src/elidinglabel.cpp
    src/treeitem.cpp
    src/application.cpp
    src/databaseinfo.cpp
    src/updatewidgetdatabaseinfohelperthread.cpp
    src/updatewidget.cpp
    src/updater.cpp
    src/settings.cpp
    src/virusdatabasedownloader.cpp
    src/settingswidget.cpp
    src/settingsdialogue.cpp
    src/scanreport.cpp
    src/timedactiondialogue.cpp

    src/resources/application.qrc
    src/resources/mainwindow.qrc
    src/resources/settingsdialogue.qrc
)

target_compile_features(qlam PRIVATE cxx_std_17)
target_link_libraries(qlam clamav Qt5::Core Qt5::Widgets Qt5::Network ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(qlam PROPERTIES PROJECT_LABEL QLam)

install(TARGETS qlam
        CONFIGURATIONS Release
        RUNTIME
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        )

install(TARGETS qlam
        CONFIGURATIONS Debug
        RENAME qlam.dbg
        RUNTIME
        PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        )

install(DIRECTORY dist/icons
        DESTINATION share
        FILES_MATCHING PATTERN "qlam.png"
        )

install(DIRECTORY dist/man
        DESTINATION share
        )

install(DIRECTORY dist/doc
        DESTINATION share
        )

install(FILES dist/qlam.desktop
        DESTINATION share/applications/
        )

install(FILES dist/qlamscan.desktop
        DESTINATION share/kservices5/
        )
