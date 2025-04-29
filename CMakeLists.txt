cmake_minimum_required(VERSION 3.30)

project(dodo C)

include(FetchContent)
FetchContent_Declare(silky
        GIT_REPOSITORY https://github.com/arctangerines/silky.git
        GIT_TAG dev)
FetchContent_MakeAvailable(silky)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if (CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
            ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif ()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED on)

#set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra -fsanitize=address -fsanitize=leak -fsanitize=undefined -fdiagnostics-color=always")
#set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address -fsanitize=leak -fsanitize=undefined -static-libasan -fdiagnostics-color=always")
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "-fdiagnostics-color=always")
set(CMAKE_C_FLAGS_DEBUG "-g -Wall -Wextra -Wpedantic -fdiagnostics-color=always")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -s -Wall -Wextra -Wpedantic -fdiagnostics-color=always")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Og -Wall -Wextra -Wpedantic -fdiagnostics-color=always")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -Wpedantic -fdiagnostics-color=always")
set(CMAKE_BUILD_TYPE Debug)

include_directories(src)
SET(SRCS
        src/dodo.c
        src/term_colors.h
        src/trie.h
        src/trie.c
)

add_executable(dodo ${SRCS})
target_link_libraries(dodo PRIVATE silky)
