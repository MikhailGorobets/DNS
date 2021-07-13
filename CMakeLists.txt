cmake_minimum_required (VERSION 3.19)

project(DNS CXX)

set(PROJECT_DIRECTORY          "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_OUTPUT_DIRECTORY   "${PROJECT_DIRECTORY}/x64")

list(PREPEND CMAKE_MODULE_PATH "${PROJECT_DIRECTORY}/cmake")
include(cmake_settings)
include(compiler_settings)

include(3rd-party/fmt)
include(3rd-party/argparse)

set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_LIBS ON)

find_package(Boost REQUIRED COMPONENTS serialization filesystem date_time)
if(NOT Boost_FOUND)
	message(SEND_ERROR "Failed to find boost")
	return()
endif()

set(INCLUDE 
	include/dns/dns.hpp
	include/dns/dns_cache.hpp
	include/dns/dns_server.hpp
)

set(SOURCE 
    src/dns.cpp
    src/dns_cache.cpp
    src/dns_server.cpp
    src/main.cpp
)

source_group("include" FILES ${INCLUDE})
source_group("source" FILES  ${SOURCE})

add_executable(DNS ${INCLUDE} ${SOURCE})
target_link_libraries(DNS PRIVATE Boost::filesystem Boost::serialization Boost::date_time fmt argparse)
target_include_directories(DNS PRIVATE "include")

set_target_properties(DNS PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIRECTORY}")
