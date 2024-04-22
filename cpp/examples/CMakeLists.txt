
add_executable(samples_xutil samples_xutil.cpp)
add_executable(samples_xmemory samples_xmemory.cpp samples_xmemory_link.cpp)
add_executable(samples_xarray samples_xarray.cpp)
add_executable(samples_xstream samples_xstream.cpp)

add_executable(example_file example_file.cpp)

if (UNIX)
  add_executable(example_xsocket_server example_xsocket_server.cpp)
  add_executable(example_xsocket_client example_xsocket_client.cpp)

  add_executable(samples_xnetwork_server samples_xnetwork_server.cpp)
  add_executable(samples_xnetwork_client samples_xnetwork_client.cpp)
endif()
