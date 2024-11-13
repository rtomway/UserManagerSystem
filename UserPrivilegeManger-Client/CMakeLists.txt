cmake_minimum_required(VERSION 3.8)

project(UserPrivilege-Client)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

file(GLOB USERPRIVILEGE_CLIENT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB USERPRIVILEGE_CLIENT_QRC ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)

add_subdirectory(LoginRegister)
add_subdirectory(SubPage)

add_executable(${PROJECT_NAME} ${USERPRIVILEGE_CLIENT_SOURCES} ${USERPRIVILEGE_CLIENT_QRC})


target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets  Qt6::Network LoginRegister SWidget SubPage QXlsx)


