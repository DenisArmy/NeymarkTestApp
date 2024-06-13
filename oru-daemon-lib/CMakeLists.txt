project(daemon VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)

find_package(Threads)

set(DAEMON_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(DAEMON_LIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB DAEMON_LIB_HEADERS ${DAEMON_LIB_INCLUDE_DIR}/*.h)
file(GLOB DAEMON_LIB_SOURCE ${DAEMON_LIB_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} SHARED ${DAEMON_LIB_SOURCE})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall")

get_target_property(LOGGER_INCLUDES logger INCLUDE_DIRECTORIES)
get_target_property(CONSTANTS_INCLUDES constants INCLUDE_DIRECTORIES)

target_include_directories(${PROJECT_NAME} PUBLIC ${DAEMON_LIB_INCLUDE_DIR} ${LOGGER_INCLUDES} ${CONSTANTS_INCLUDES})

# Project dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC logger rt constants yang ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
