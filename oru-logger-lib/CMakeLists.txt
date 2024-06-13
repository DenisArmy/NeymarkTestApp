project(logger VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)

find_package(Threads REQUIRED)

include_directories(include/)
add_library(${PROJECT_NAME} SHARED src/Logger.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
