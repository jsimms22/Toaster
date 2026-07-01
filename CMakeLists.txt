# root/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
cmake_policy(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(ToasterBot LANGUAGES CXX)

include(FetchContent)

## ----- fmt ----- ##
FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
    GIT_TAG         1be298e1bd68957e4cd352e1f676f00e07dcfb57 # 12.2.0
)
FetchContent_MakeAvailable(fmt)

## ----- dpp ----- ##
FetchContent_Declare(
    dpp
    GIT_REPOSITORY  https://github.com/brainboxdotcc/DPP.git
    GIT_TAG         59179f4ffc4f111411fb18d8da31258456768025 # 10.1.5
)
FetchContent_MakeAvailable(dpp)

## ----- spdlog ----- ##
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog.git
    GIT_TAG         79524ddd08a4ec981b7fea76afd08ee05f83755d # 1.17.0
)
FetchContent_MakeAvailable(spdlog)

## ----- mongodb ----- ##
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

FILE(GLOB Sources Toaster/*.cpp)
add_executable(ToasterBot ${Sources})

target_link_libraries(ToasterBot
    PRIVATE
        fmt::fmt-header-only
        spdlog::spdlog_header_only
        dpp

        $<IF:$<TARGET_EXISTS:mongo::bsoncxx_static>,mongo::bsoncxx_static,mongo::bsoncxx_shared>
        $<IF:$<TARGET_EXISTS:mongo::mongocxx_static>,mongo::mongocxx_static,mongo::mongocxx_shared>
)

if(WIN32)
add_custom_command(TARGET ToasterBot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:ToasterBot>
        $<TARGET_FILE_DIR:ToasterBot>
    COMMAND_EXPAND_LISTS
)

file(GLOB DPP_RUNTIME_DLLS
    "${CMAKE_SOURCE_DIR}/Deps/Windows/*.dll"
)

foreach(dll ${DPP_RUNTIME_DLLS})
    add_custom_command(TARGET ToasterBot POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${dll}
            $<TARGET_FILE_DIR:ToasterBot>
    )
endforeach()

add_custom_command(TARGET ToasterBot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/token.txt
        $<TARGET_FILE_DIR:ToasterBot>/token.txt
)
else()
add_custom_command(TARGET ToasterBot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/token.txt
        $<TARGET_FILE_DIR:ToasterBot>/token.txt
)
endif()