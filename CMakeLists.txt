cmake_minimum_required(VERSION 3.14)
project(StudentAttendanceSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB BACKEND_CORE_SOURCES
    "Backend/core/*.cpp"
    "Backend/middlewares/*.cpp"
    "Backend/utils/*.cpp"
    "Backend/controllers/*.cpp"
    "Backend/routes/*.cpp"
)

add_executable(attendance_server Backend/main.cpp ${BACKEND_CORE_SOURCES})
target_include_directories(attendance_server PRIVATE "${CMAKE_SOURCE_DIR}/Backend")

add_executable(integration_tests Backend/tests/IntegrationTests.cpp ${BACKEND_CORE_SOURCES})
target_include_directories(integration_tests PRIVATE "${CMAKE_SOURCE_DIR}/Backend")

if(WIN32)
    target_link_libraries(attendance_server PRIVATE ws2_32)
    target_link_libraries(integration_tests PRIVATE ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(attendance_server PRIVATE Threads::Threads)
    target_link_libraries(integration_tests PRIVATE Threads::Threads)
endif()
