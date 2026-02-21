cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")


########################################################################################################################
## Define project
########################################################################################################################
project(
        OFootBlock
        VERSION 1.0.0
        DESCRIPTION "SHYUE.CC SKSE plugin built with CommonLibSSE."
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

macro(set_from_environment VARIABLE)
        if (NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
                set(${VARIABLE} $ENV{${VARIABLE}})
        endif ()
endmacro()

set_from_environment(VCPKG_ROOT)
add_compile_definitions(SKYRIM)
set(CommonLibPath "extern/CommonLibVR")
set(CommonLibName "CommonLibSSE")
set(GameVersion "Skyrim")

if (DEFINED CommonLibPath AND NOT ${CommonLibPath} STREQUAL "")
        add_subdirectory(${CommonLibPath} ${CommonLibName} EXCLUDE_FROM_ALL)
else ()
        message(
                FATAL_ERROR
                "Variable ${CommonLibName}Path is not set."
        )
endif()

include(GNUInstallDirs)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY)

set(headers
        include/Header/Header.h)

set(sources

	src/Main.cpp
    src/Hook.cpp

    src/Utils/BoneUtils.cpp
    src/Utils/ActorUtils.cpp

    src/Events/OFootBlockEventListener.cpp
    src/Events/EquipEventSink.cpp
    src/Events/InputEventSink.cpp
    src/Events/InputChordDetector.cpp
    src/Events/CellWatcher.cpp

    src/Papyrus.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc)

source_group(
        TREE ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ${headers}
        ${sources})

########################################################################################################################
## Configure target DLL
########################################################################################################################
find_package(pugixml REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ryml CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_path(ARTICUNO_INCLUDE_DIRS "articuno/articuno.h")

add_library(
        ${PROJECT_NAME}
        SHARED
        ${headers}
        ${sources}
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>
        ${ARTICUNO_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/external/openvr/headers)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ryml::ryml
        pugixml
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        PUBLIC
        ${CommonLibName}::${CommonLibName}
)

target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        src/PCH.h)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/Header"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

set(OUTPUT_FOLDER "D:/GreatGAME/SkyrimSE/Data")

if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )
    
    # Copy the SKSE plugin .pdb files into the SKSE/Plugins/ folder
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )
endif()