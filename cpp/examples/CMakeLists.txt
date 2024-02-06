
add_executable(samples_xarr samples_xarr.cpp)
add_executable(samples_xmat samples_xmat.cpp)

if (UNIX)
  add_executable(example_xsocket_server example_xsocket_server.cpp)
  add_executable(example_xsocket_client example_xsocket_client.cpp)

  add_executable(samples_xnetwork_server samples_xnetwork_server.cpp)
  add_executable(samples_xnetwork_client samples_xnetwork_client.cpp)
endif()
