
add_executable(samples_xutil samples_xutil.cpp)
add_executable(samples_xmemory samples_xmemory.cpp samples_xmemory_link.cpp)
add_executable(samples_xarray samples_xarray.cpp)
add_executable(samples_xdatastream samples_xdatastream.cpp)

add_executable(example_file example_file.cpp)

add_executable(samples_xsocket_server samples_xsocket_server.cpp)
add_executable(samples_xsocket_client samples_xsocket_client.cpp)

# ----------------
# Isn't actual now
# ----------------
# add_executable(samples_xnetwork_server samples_xnetwork_server.cpp)
# add_executable(samples_xnetwork_client samples_xnetwork_client.cpp)

if (WIN32)
    target_link_libraries(samples_xsocket_server ws2_32)
    target_link_libraries(samples_xsocket_client ws2_32)
    
    # target_link_libraries(samples_xnetwork_server ws2_32)
    # target_link_libraries(samples_xnetwork_client ws2_32)
endif()
