cmake_minimum_required (VERSION 2.8.3)
project (RETRY_CLIENT)

# So all projects here will have this flag defined because we require C++11
add_definitions ("-D_WEBSOCKETPP_CPP11_STL_")

if(UNIX)
	# The websocket pp libraries that will need to be added, and this could vary depending on the os/compiler being used, hence why they are in here.
	set (WEBSOCKETPP_PLATFORM_LIBS pthread rt)
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -O0 -g")
else()
  # I have never tried it on any other OS. I suspect apple will work.
  message( FATAL_ERROR "I have never tried this on a non Linux machine, please try at your own risk")
endif()

# base path of where binaries should be built
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# components that we will search for with boost
set (WEBSOCKETPP_BOOST_COMPONENTS system ) # removed thread

#finding and linking boost
set(Boost_USE_STATIC_LIBS FALSE) 
set(Boost_USE_MULTITHREADED FALSE)  
set(Boost_USE_STATIC_RUNTIME FALSE) 
find_package(Boost 1.39.0 COMPONENTS ${WEBSOCKETPP_BOOST_COMPONENTS}) 

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
else()
	message( FATAL_ERROR "Couldn't find boost install" )
endif()

# setup the includes
include_directories("src")

include_directories("lib/websocketpp")

###########
# Examples
###########
add_subdirectory( examples )
