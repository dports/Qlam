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
add_executable(Qlam
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

    src/resources/application.qrc
    src/resources/mainwindow.qrc
    src/resources/settingsdialogue.qrc
        src/timedactiondialogue.cpp src/timedactiondialogue.h)

target_compile_features(Qlam PRIVATE cxx_std_17)
target_link_libraries(Qlam clamav Qt5::Core Qt5::Widgets Qt5::Network ${CMAKE_THREAD_LIBS_INIT})
