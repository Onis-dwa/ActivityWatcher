cmake_minimum_required(VERSION 3.13)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(ActivityWatcher VERSION 1)
add_definitions(-DWATCHER_VERSION="1.0")


# libs
find_package(SQLite3 3.36 REQUIRED)
#find_package(sqlite  3.36 REQUIRED) TODO write a config.cmake in sqlite proj
find_package(pugixml 1.11 REQUIRED)
find_package(Boost   1.77 REQUIRED)


# global include (because .h .hpp in /src/global don't have target)
include_directories(
	PUBLIC ${SQLite3_INCLUDE_DIR}
	#PUBLIC sqlite TODO config.cmake
	PUBLIC pugixml
	PUBLIC ${Boost_INCLUDE_DIR}
) 


# outputs
add_executable(watcher)


# outputs link
if (TARGET watcher)
	set_property(TARGET watcher PROPERTY CXX_STANDARD 20)
	target_link_libraries(watcher
		LINK_PUBLIC ${SQLite3_LIBRARY}
		#LINK_PUBLIC sqlite TODO config.cmake
		LINK_PUBLIC pugixml
		#LINK_PUBLIC ${Boost_LIBRARIES} # precompiled?
	)
	endif()
if (TARGET watcher)
	endif()


# compiler options
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	add_compile_options("/O2")
	#add_compile_options("/std:c++20")
	message("setting /favor:${CMAKE_HOST_SYSTEM_PROCESSOR}")
	add_compile_options("/favor:${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()


add_subdirectory(src)


# installs
#install(TARGETS watcher DESTINATION bin)
#install(FILES ${SQLite3_LIBRARY} DESTINATION ~/)
