cmake_minimum_required (VERSION 3.19)

project(DNS CXX)

set(PROJECT_DIRECTORY          "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROJECT_OUTPUT_DIRECTORY   "${PROJECT_DIRECTORY}/x64")


list(PREPEND CMAKE_MODULE_PATH "${PROJECT_DIRECTORY}/cmake")
include(cmake_settings)
include(compiler_settings)

include(3rd-party/fmt)

set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_LIBS ON)


find_package(Boost 1.76.0 REQUIRED COMPONENTS serialization filesystem date_time)
if(NOT Boost_FOUND)
	message(SEND_ERROR "Failed to find boost")
	return()
else()
    message("BOOST ROOT: " ${Boost_ROOT})
	message("BOOST INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
	message("BOOST LIBRARIES LOCATION: " ${Boost_LIBRARIES})
endif()


set(INCLUDE 
	include/dns/dns.hpp
	include/dns/dns_cache.hpp
	include/dns/dns_detail.hpp
	include/dns/server.h
)

set(SOURCE 
	src/main.cpp
	src/server.cpp
)

source_group("include" FILES ${INCLUDE})
source_group("source" FILES  ${SOURCE})

add_executable(DNS ${INCLUDE} ${SOURCE})
target_link_libraries(DNS PRIVATE Boost::filesystem Boost::serialization Boost::date_time fmt)
target_include_directories(DNS PRIVATE "include")

set_target_properties(DNS PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIRECTORY}")