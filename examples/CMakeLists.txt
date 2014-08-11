# Example 1
add_executable( "example1" "example1.cpp" )
target_link_libraries( "example1" ${WEBSOCKETPP_PLATFORM_LIBS} ${Boost_LIBRARIES})

# Example 2
add_executable( "example2" "example2.cpp" )
target_link_libraries( "example2" ${WEBSOCKETPP_PLATFORM_LIBS} ${Boost_LIBRARIES})

# Example 3
add_executable( "example3" "example3.cpp" "test_server.cpp" )
target_link_libraries( "example3" ${WEBSOCKETPP_PLATFORM_LIBS} ${Boost_LIBRARIES})
