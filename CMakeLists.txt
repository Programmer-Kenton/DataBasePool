cmake_minimum_required(VERSION 3.29)
project(DataBasePool)

set(CMAKE_CXX_STANDARD 20)

# 指定mysql的安装路径
set(MYSQL_INCLUDE_DIR "D:/Sql_Study/Mysql/include")
set(MYSQL_LIB_DIR "D:/Sql_Study/Mysql/lib")

include_directories(${MYSQL_INCLUDE_DIR})
link_directories(${MYSQL_LIB_DIR})

add_executable(
        DataBasePool main.cpp
        head/CommonConnectionPool.h
        head/Connection.h
        head/public.h
        source/CommonConnectionPool.cpp
        source/Connection.cpp
)

target_link_libraries(
        DataBasePool
        libmysql.lib
        mysqlclient.lib
        libmysql.dll
)