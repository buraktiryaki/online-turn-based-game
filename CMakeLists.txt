cmake_minimum_required(VERSION 3.3)
project(OnlineSpaceGame)

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_DEBUG_LIBS        OFF)
set(Boost_USE_RELEASE_LIBS       ON)
set(Boost_USE_MULTITHREADED      ON)
find_package(Boost 1.74 REQUIRED COMPONENTS date_time thread)
message(STATUS "Boost version: ${Boost_VERSION}")

find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(ASSIMP REQUIRED)
message(STATUS "Assimp version: ${ASSIMP_VERSION}")

if (MSVC)
	set(ASSIMP_LINK assimp::assimp)
endif (MSVC)
if (UNIX)
	set(ASSIMP_LINK assimp)
endif (UNIX)


set(COMMON_SOURCES 
			${CMAKE_SOURCE_DIR}/src/net/tsqueue.cpp
			${CMAKE_SOURCE_DIR}/src/net/message.cpp
			${CMAKE_SOURCE_DIR}/src/net/connection.cpp
			${CMAKE_SOURCE_DIR}/src/game/gameCommon.cpp
			)

set(SERVER_SOURCES 
			${CMAKE_SOURCE_DIR}/src/main_s.cpp
			${CMAKE_SOURCE_DIR}/src/net/baseServer.cpp
			${CMAKE_SOURCE_DIR}/src/netServer.cpp
			${CMAKE_SOURCE_DIR}/src/game/gameServer.cpp
			${COMMON_SOURCES}
			)
			
set(CLIENT_SOURCES 
			${CMAKE_SOURCE_DIR}/src/glad/glad.c
			${CMAKE_SOURCE_DIR}/src/main_c.cpp
			${CMAKE_SOURCE_DIR}/src/mainClient.cpp
			${CMAKE_SOURCE_DIR}/src/render/textureImporter.cpp
			${CMAKE_SOURCE_DIR}/src/render/picking.cpp
			${CMAKE_SOURCE_DIR}/src/render/framebuffer.cpp
			${CMAKE_SOURCE_DIR}/src/net/baseClient.cpp
			${CMAKE_SOURCE_DIR}/src/netClient.cpp
			${CMAKE_SOURCE_DIR}/src/game/gameClient.cpp
			${COMMON_SOURCES}
)

file(GLOB IMGUI "${CMAKE_SOURCE_DIR}/imgui/imgui/*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/imgui)
include_directories(${CMAKE_SOURCE_DIR}/include/render/glad)

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server Boost::date_time Boost::thread)
target_compile_features(server PRIVATE cxx_std_17)

add_executable(client ${CLIENT_SOURCES} ${IMGUI})
target_link_libraries(client Boost::date_time Boost::thread glfw glm::glm ${ASSIMP_LINK})
target_compile_features(client PRIVATE cxx_std_17)
