cmake_minimum_required(VERSION 2.8.11)

project(kunjika-cpp)



include_directories(
    /home/shiv/Documents/ws/kunjika-cpp/
    /home/shiv/Documents/ws/kunjika-cpp/

)

add_definitions(
    -DNDEBUG
)

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ")

# Library path
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L\".\" ")

# Define the CXX sources
set ( CXX_SRCS
    main.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -O2 -Wall")

add_executable(kunjika-cpp ${CXX_SRCS} ${C_SRCS})

