# ---------- Projeto ----------
cmake_minimum_required(VERSION 3.16)
project(Sala3D C CXX)
set(CMAKE_CXX_STANDARD 17)

# ---------- Dependencias (baixadas automaticamente) ----------
include(FetchContent)

# GLFW sem docs, testes e exemplos
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# GLFW (janela/entrada) e GLM (matematica)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm  GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glfw glm)

# ---------- Executavel ----------
add_executable(sala src/main.cpp external/glad/src/glad.c)
target_include_directories(sala PRIVATE external/glad/include external/stb)
target_link_libraries(sala PRIVATE glfw glm::glm)

# Copia shaders e assets para a pasta do executavel
add_custom_command(TARGET sala POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:sala>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:sala>/assets)