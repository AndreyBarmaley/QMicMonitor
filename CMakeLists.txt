cmake_minimum_required(VERSION 3.16)

project(qmicmonitor)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3 -O0 -Wall -Werror -Wno-sign-compare -Wno-unused-function -Wno-unused-variable")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -Wno-sign-compare -Wno-unused-function -Wno-unused-variable")

include(FindPkgConfig)
set(CMAKE_FIND_FRAMEWORK LAST)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp micmonitor.cpp resources.qrc)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qmicmonitor MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(qmicmonitor ${PROJECT_SOURCES})
endif()

pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
pkg_check_modules(SPA REQUIRED libspa-0.2)
    
target_compile_options(qmicmonitor PRIVATE ${PIPEWIRE_CFLAGS} ${SPA_CFLAGS})
target_include_directories(qmicmonitor PRIVATE ${PIPEWIRE_INCLUDE_DIR} ${SPA_INCLUDE_DIR})
target_link_libraries(qmicmonitor PRIVATE ${PIPEWIRE_LIBRARIES} ${SPA_LIBRARIES})
target_link_libraries(qmicmonitor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(qmicmonitor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include(GNUInstallDirs)
install(TARGETS qmicmonitor BUNDLE DESTINATION . LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qmicmonitor)
endif()
