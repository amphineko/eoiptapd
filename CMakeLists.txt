cmake_minimum_required(VERSION 3.9)
project(eoiptapd)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES eoiptapd.h eoip.h eoip.c socket.c socket.h tap.c tap.h utils.c utils.h)

add_executable(eoiptapd ${SOURCE_FILES} eoiptapd.c)
