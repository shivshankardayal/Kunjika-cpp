cmake_minimum_required(VERSION 2.8.11)

project(kunjika)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(include)
include(FindPkgConfig)
include(CPack)



# Options 

option(STATIC_VIEW "Build Static View instead of dynamically loaded one" ON)


# Dependencies

find_library(LIB_BOOSTER booster)

if(LIB_BOOSTER)
    MESSAGE("-- booster library found at " ${LIB_BOOSTER})
else()
    MESSAGE(FATAL ERROR "booster library not found.")
endif()

find_library(LIB_CPPCMS cppcms)

if(LIB_CPPCMS)
    MESSAGE("-- booster library found at " ${LIB_CPPCMS})
else()
    MESSAGE(FATAL ERROR "cppcms library not found.")
endif()

find_library(LIB_PTHREAD pthread)
find_library(LIB_CURL curl)
find_library(LIB_SSL ssl)
find_library(LIB_COUCHBASE couchbase)
find_library(LIB_SCRYPT scrypt)
find_library(LIB_CRYPTO crypto)

find_program(EXE_TMPL_CC cppcms_tmpl_cc)

find_path(INC_BOOSTER booster/config.h)
find_path(INC_CPPCMS cppcms/config.h)
find_path(INC_PTHREAD pthread.h)
find_path(INC_CURL curl/curl.h)
find_path(INC_SSL openssl/ssl.h)
find_path(INC_COUCHBASE libcouchbase/couchbase.h)
find_path(INC_PTHREAD libscrypt.h)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++14")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Sources

set(SKINS contendend orangesky)

set(TEMPLATES
    templates/index.html
    templates/login.html
	)


set(USDIGI_SRC
	src/kunjika.cpp
    src/main.cpp
    src/config.cpp
    src/login.cpp
    src/base64.cpp
    src/registration.cpp
    src/globals.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/templates.cpp"
	)


# Actual Build

include_directories(${INC_BOOSTER})
include_directories(${INC_CPPCMS})
include_directories(${INC_CASS})
include_directories(${INC_PTHREAD})
include_directories(${INC_CURL})
include_directories(${INC_SSL})
include_directories(${INC_COUCHBASE})
include_directories(${INC_SCRYPT})

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/templates.cpp"
	COMMAND ${EXE_TMPL_CC}
		${TMPL_CC_PARAMS}
		${TEMPLATES}
		-o "${CMAKE_CURRENT_BINARY_DIR}/templates.cpp"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${TEMPLATES})

add_executable(kunjika ${USDIGI_SRC})
target_link_libraries(kunjika ${LIB_CPPCMS})
target_link_libraries(kunjika ${LIB_BOOSTER})
target_link_libraries(kunjika ${LIB_CURL})
target_link_libraries(kunjika ${LIB_PTHREAD})
target_link_libraries(kunjika ${LIB_SSL})
target_link_libraries(kunjika ${LIB_COUCHBASE})
target_link_libraries(kunjika ${LIB_SCRYPT})
target_link_libraries(kunjika ${LIB_CRYPTO})

set(EXTRACT_SRC ${CMAKE_CURRENT_BINARY_DIR}/templates.cpp)

#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/sample_config.js DESTINATION share/cppblog)
