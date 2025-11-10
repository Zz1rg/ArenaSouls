cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

# CRITICAL FIX: Enable both CXX (C++) and C compilers for the project.
# This ensures that both .cpp files (like stb_image.cpp) and .c files (like glad.c)
# can be compiled by the generated Visual Studio solution.
project(ArenaSouls CXX C)

## --- Configuration ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer standard compliance

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF()


## --- Library Paths and Includes ---
# We still need link_directories for the pre-compiled .lib files
link_directories(${CMAKE_SOURCE_DIR}/Libraries/lib)

# Define common external libraries needed
if(WIN32)
    set(EXTERNAL_LIBS
        glfw3
        opengl32
        assimp
        freetype
        irrKlang
    )
    # Add a definition often needed on MSVC for safety warnings
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    
elseif(UNIX AND NOT APPLE)
    set(EXTERNAL_LIBS )
    
elseif(APPLE)
    set(EXTERNAL_LIBS )
    
endif()

## --- Internal Source Files as Static Libraries ---

# 1. GLAD (C source file)
add_library(GLAD STATIC glad.c)
# We still need this to tell CMake it's a C-language target
set_target_properties(GLAD PROPERTIES LINKER_LANGUAGE "C")
# Tell the GLAD target where to find its own headers
target_include_directories(GLAD
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/Libraries/includes
)

# 2. STB_IMAGE (C++ source file)
add_library(STB_IMAGE STATIC stb_image.cpp)
# Tell the STB_IMAGE target where to find its headers
target_include_directories(STB_IMAGE
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/Libraries/includes
)


## --- Executable Creation ---
# Define source files for the main executable
set(MAIN_SOURCES
    main.cpp
    player.cpp
    TrainingDummy.cpp
    PlayerCamera.cpp
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

# Set output directory properties for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
)

# Explicitly enforce build order.
add_dependencies(${PROJECT_NAME} GLAD STB_IMAGE)


# Link the executable against all dependencies.
# Because GLAD and STB_IMAGE have PUBLIC includes,
# ArenaSouls will automatically inherit them.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    GLAD
    STB_IMAGE
    ${EXTERNAL_LIBS}
)

# Set C++ standard options for MSVC compiler
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17 /MP)
    target_link_options(${PROJECT_NAME} PUBLIC /ignore:4099)
endif()

# --- Post-Build Operations ---

# Automatically copy required DLLs on Windows
if(WIN32)
    # Find all .dll files in the lib directory
    # This assumes all .dlls in /Libraries/lib are needed for runtime.
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
    else()
        message(WARNING "No .dll files found in ${CMAKE_SOURCE_DIR}/Libraries/lib to copy. "
                        "The application may fail to start if it requires runtime libraries "
                        "(like glfw3.dll, assimp.dll, etc.).")
    endif()
endif()

# Copy Shaders
file(GLOB SHADERS_TO_COPY
    "${CMAKE_SOURCE_DIR}/*.vs"
    "${CMAKE_SOURCE_DIR}/*.fs"
)

# Copy shaders to the executable's output directory
foreach(SHADER ${SHADERS_TO_COPY})
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SHADER}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying shader: ${SHADER}"
    )
endforeach()