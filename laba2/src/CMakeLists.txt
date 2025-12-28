cmake_minimum_required(VERSION 3.10)
project(ConvolutionLab VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

option(WITH_SANITIZER "Enable thread sanitizer" ON)

if(WITH_SANITIZER)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif()

set(SOURCES
    main.c
    convolution.c
    utils.c
)

add_executable(convolution ${SOURCES})

target_link_libraries(convolution 
    PRIVATE 
    Threads::Threads
    m
)

set_target_properties(convolution PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

message(STATUS "Project configured successfully")
message(STATUS "Threads library: ${CMAKE_THREAD_LIBS_INIT}")
