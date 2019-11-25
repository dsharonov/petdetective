cmake_minimum_required(VERSION 3.5)

project(PetDetective LANGUAGES CXX)

set(BOOST_ROOT "/usr/local/boost/1_71")
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)  # only find static libsset( BOOST_ROOT "/usr/local/boost/1_71" CACHE PATH "Boost library path" )

find_package(Boost 1.71 REQUIRED COMPONENTS thread fiber)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


add_executable(${PROJECT_NAME}
    main.cpp
    readlines.cpp
    readlines.h
    definitions.h
    state.cpp
    state.h
    pet.cpp
    pet.h
)

find_package(PkgConfig)

#pkg_check_modules(LIBS REQUIRED gstreamermm-1.0 gtk+-3.0 gstreamer-webrtc-1.0 gstreamer-sdp-1.0 libmicrohttpd jsoncpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        pthread
        ${LIBS_LIBRARIES}
        ${Boost_LIBRARIES}
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
#        -D TEST
#        -DGST_USE_UNSTABLE_API
#        -DVERSION_MAJOR=${VERSION_MAJOR}
#        -DVERSION_MINOR1=${VERSION_MINOR1}
#        -DVERSION_MINOR2=${VERSION_MINOR2}
#        "-DVERSION_TIMESTAMP=${VERSION_TIMESTAMP}"
#        "-DVERSION=\"${VERSION}\""
#        "-DG_LOG_DOMAIN=\"${PROJECT_NAME}\""
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${LIBS_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIR}
)
