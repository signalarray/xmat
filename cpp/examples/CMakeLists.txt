
add_executable(samples_xarr samples_xarr.cpp)
add_executable(samples_xmat samples_xmat.cpp)

if (UNIX)
  add_executable(example_xsocket_server example_xsocket_server.cpp)
  add_executable(example_xsocket_client example_xsocket_client.cpp)
endif()
