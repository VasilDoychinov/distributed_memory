cmake_minimum_required(VERSION 3.14)

project(Distributed_memory LANGUAGES C CXX)

set(MY_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(MY_LIB_DIR ${PROJECT_SOURCE_DIR}/../lib)
set(MY_LOGGER_DIR "~/src/Logger")

# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(DM_sim
  ${PROJECT_SOURCE_DIR}/test_dm.cpp
  ${MY_SOURCE_DIR}/dm_task.cpp  ${MY_SOURCE_DIR}/dm_communication.cpp
  ${MY_LIB_DIR}/tcp_sockets.cpp ${MY_LIB_DIR}/udp_sockets.cpp ${MY_LIB_DIR}/sockets_basic.cpp
  ${MY_LOGGER_DIR}/lib/Logger_impl.cpp
)

target_link_libraries(DM_sim Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(DM_sim PUBLIC
   ${PROJECT_SOURCE_DIR}/../include ${MY_LOGGER_DIR}/include
)



