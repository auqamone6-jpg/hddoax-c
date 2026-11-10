cmake_minimum_required(VERSION 3.20)
project(FortniteUtilityMod VERSION 1.0.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "FortniteUtilityMod must be compiled on Windows (x64).")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)

# ---- ImGui (1.90.4) ----
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ---- MinHook ----
FetchContent_Declare(minhook
    GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(minhook)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

set(MOD_SOURCES
    src/dllmain.cpp
    src/core/Core.cpp
    src/core/HookManager.cpp
    src/render/Renderer.cpp
    src/render/Draw.cpp
    src/menu/Menu.cpp
    src/modules/ESP.cpp
    src/modules/Aimbot.cpp
    src/modules/Movement.cpp
    src/modules/Visual.cpp
    src/modules/Weapon.cpp
    src/modules/Misc.cpp
)

add_library(FortniteUtilityMod SHARED
    ${MOD_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(FortniteUtilityMod PRIVATE
    src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${minhook_SOURCE_DIR}/include
)

target_link_libraries(FortniteUtilityMod PRIVATE
    minhook
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    dwmapi.lib
)

target_compile_definitions(FortniteUtilityMod PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
)

target_compile_options(FortniteUtilityMod PRIVATE
    /W3
    /permissive-
    $<$<CONFIG:Release>:/O2 /Oi /GL>
)

target_link_options(FortniteUtilityMod PRIVATE
    $<$<CONFIG:Release>:/LTCG>
    /SUBSYSTEM:WINDOWS
)

set_target_properties(FortniteUtilityMod PROPERTIES
    OUTPUT_NAME  "FortniteUtility"
    PREFIX       ""
    SUFFIX       ".dll"
)
