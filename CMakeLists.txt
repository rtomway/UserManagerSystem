cmake_minimum_required(VERSION 3.14)

project(UserPrivilege VERSION 1.0 LANGUAGES CXX)

add_subdirectory(UserPrivilegeManger-Server)
add_subdirectory(UserPrivilegeManger-Client)

add_subdirectory(Util)

add_subdirectory(SWidget)

add_subdirectory(SJwt)

add_subdirectory(ThiryPart/QXlsx)

