cmake_minimum_required(VERSION 3.20)
project(servo_tuning VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)

# GTKmm4 + Cairo
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.16)

# EtherCAT
find_library(ETHERCAT_LIB ethercat PATHS /usr/lib /usr/lib/x86_64-linux-gnu /opt/etherlab/lib)
if(ETHERCAT_LIB)
    message(STATUS "Found EtherCAT: ${ETHERCAT_LIB}")
    add_definitions(-DHAVE_ETHERCAT=1)
endif()
file(GLOB ETHERCAT_INCLUDE_DIR "/usr/src/ethercat-dkms-*/include")
if(ETHERCAT_INCLUDE_DIR)
    message(STATUS "Found EtherCAT headers: ${ETHERCAT_INCLUDE_DIR}")
    add_definitions(-DHAVE_ETHERCAT_H=1)
    set(ETHERCAT_INCLUDE ${ETHERCAT_INCLUDE_DIR})
endif()

# nlohmann-json
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann-json: ${nlohmann_json_VERSION}")
endif()

set(SOURCES
    src/main.cpp
    src/param_db.cpp
    src/sdo_client.cpp
    src/monitor.cpp
    src/gui_main_window.cpp
    src/gui_param_page.cpp
    src/gui_param_edit_dialog.cpp
    src/gui_monitor_page.cpp
    src/gui_graph.cpp
)

set(HEADERS
    src/param_db.h
    src/param_row.h
    src/sdo_client.h
    src/monitor.h
    src/gui_main_window.h
    src/gui_param_page.h
    src/gui_param_edit_dialog.h
    src/gui_monitor_page.h
    src/gui_graph.h
)

add_executable(servo_tuning ${SOURCES} ${HEADERS})

target_include_directories(servo_tuning PRIVATE src)
target_include_directories(servo_tuning PRIVATE ${GTKMM_INCLUDE_DIRS} ${CAIROMM_INCLUDE_DIRS})
if(ETHERCAT_INCLUDE)
    target_include_directories(servo_tuning PRIVATE ${ETHERCAT_INCLUDE})
endif()
if(nlohmann_json_FOUND)
    target_link_libraries(servo_tuning PRIVATE nlohmann_json::nlohmann_json)
endif()

target_link_libraries(servo_tuning PRIVATE ${GTKMM_LIBRARIES} ${CAIROMM_LIBRARIES})
if(ETHERCAT_LIB)
    target_link_libraries(servo_tuning PRIVATE ${ETHERCAT_LIB})
endif()
find_package(Threads REQUIRED)
target_link_libraries(servo_tuning PRIVATE Threads::Threads)

install(TARGETS servo_tuning DESTINATION /usr/local/bin)

message(STATUS "=== servo_tuning build ===")
message(STATUS "GTKmm4: ${GTKMM_CFLAGS}")
message(STATUS "Cairo: ${CAIROMM_CFLAGS}")
