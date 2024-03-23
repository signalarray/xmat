
add_executable(samples_xutil samples_xutil.cpp samples_xutil_link.cpp)
add_executable(samples_xarray samples_xarray.cpp)
add_executable(samples_xmat samples_xmat.cpp)
add_executable(samples_xstream samples_xstream.cpp)

add_executable(example_file example_file.cpp)

if (UNIX)
  add_executable(example_xsocket_server example_xsocket_server.cpp)
  add_executable(example_xsocket_client example_xsocket_client.cpp)

  add_executable(samples_xnetwork_server samples_xnetwork_server.cpp)
  add_executable(samples_xnetwork_client samples_xnetwork_client.cpp)
endif()
