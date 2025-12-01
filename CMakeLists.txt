cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

project(ArenaSouls CXX C)

## --- Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

## --- Library Paths and Includes ---
link_directories(${CMAKE_SOURCE_DIR}/Libraries/lib)

# Find packages
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)
find_package(glfw3 REQUIRED)

# Find packages
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)
find_package(glfw3 REQUIRED)

if(WIN32)
    set(EXTERNAL_LIBS
        glfw3
        opengl32
        assimp
        freetype
        irrKlang
    )
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    
    # For SFML - it seems find_package has issues, so we will link it directly.
    # You may need to adjust the version number in the path.
    set(SFML_DIR "/opt/homebrew/Cellar/sfml/3.0.2/lib")

    set(EXTERNAL_LIBS
        ${OPENGL_LIBRARIES}
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        "${SFML_DIR}/libsfml-graphics.dylib"
        "${SFML_DIR}/libsfml-audio.dylib"
        "${SFML_DIR}/libsfml-network.dylib"
        "${SFML_DIR}/libsfml-window.dylib"
        "${SFML_DIR}/libsfml-system.dylib"
    )
    find_package(OpenGL REQUIRED)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    
    # For SFML - it seems find_package has issues, so we will link it directly.
    # You may need to adjust the version number in the path.
    set(SFML_DIR "/opt/homebrew/Cellar/sfml/3.0.2/lib")

    set(EXTERNAL_LIBS
        ${OPENGL_LIBRARIES}
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        "${SFML_DIR}/libsfml-graphics.dylib"
        "${SFML_DIR}/libsfml-audio.dylib"
        "${SFML_DIR}/libsfml-network.dylib"
        "${SFML_DIR}/libsfml-window.dylib"
        "${SFML_DIR}/libsfml-system.dylib"
    )
    
endif()

## --- Internal Source Files as Static Libraries ---

# 1. GLAD (C source file)
add_library(GLAD STATIC glad.c)
set_target_properties(GLAD PROPERTIES LINKER_LANGUAGE "C")
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/Libraries/includes)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/Libraries/includes)

# 2. STB_IMAGE (C++ source file)
add_library(STB_IMAGE STATIC stb_image.cpp)
target_include_directories(STB_IMAGE PUBLIC ${CMAKE_SOURCE_DIR}/Libraries/includes)
target_include_directories(STB_IMAGE PUBLIC ${CMAKE_SOURCE_DIR}/Libraries/includes)


## --- Executable Creation ---
set(MAIN_SOURCES
    main.cpp
    player.cpp
    TrainingDummy.cpp
    Boss.cpp
    PlayerCamera.cpp
    Menu.cpp
    Entity.cpp
    DebugDrawer.cpp
    Entity.cpp
    DebugDrawer.cpp
    SoundEngine.cpp
    UI.cpp
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

add_dependencies(${PROJECT_NAME} GLAD STB_IMAGE)

# Prefer Homebrew Assimp headers on macOS to avoid mismatch with vendored headers
if(APPLE)
    # Prefer Homebrew headers (Assimp, GLFW, etc.)
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE /opt/homebrew/include)
endif()

# Add specific project include subdirs (exclude vendored assimp to avoid ABI mismatch)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Libraries/includes
    ${CMAKE_SOURCE_DIR}/Libraries/includes/learnopengl
    ${CMAKE_SOURCE_DIR}/Libraries/includes/glm
    ${CMAKE_SOURCE_DIR}/Libraries/includes/glad
    ${CMAKE_SOURCE_DIR}/Libraries/includes/GL
    ${CMAKE_SOURCE_DIR}/Libraries/includes/GLFW
    ${CMAKE_SOURCE_DIR}/Libraries/includes/json
    ${CMAKE_SOURCE_DIR}/Libraries/includes/KHR
    ${CMAKE_SOURCE_DIR}/Libraries/includes/SFML
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    GLAD
    STB_IMAGE
    glfw
    assimp::assimp
    Freetype::Freetype
    glfw
    assimp::assimp
    Freetype::Freetype
    ${EXTERNAL_LIBS}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /MP)
    target_link_options(${PROJECT_NAME} PUBLIC /ignore:4099)
endif()

# --- Post-Build Operations ---
if(WIN32)
    file(GLOB RUNTIME_DLLS "${CMAKE_SOURCE_DIR}/Libraries/lib/*.dll")
    if(RUNTIME_DLLS)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${RUNTIME_DLLS}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying runtime DLLs..."
        )
    endif()
elseif(APPLE)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
        COMMENT "Copying resources..."
    )

    file(GLOB SHADERS
        "${CMAKE_SOURCE_DIR}/*.vs"
        "${CMAKE_SOURCE_DIR}/*.fs"
    )
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${SHADERS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying shaders..."
    )
elseif(APPLE)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
        COMMENT "Copying resources..."
    )

    file(GLOB SHADERS
        "${CMAKE_SOURCE_DIR}/*.vs"
        "${CMAKE_SOURCE_DIR}/*.fs"
    )
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${SHADERS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying shaders..."
    )
endif()