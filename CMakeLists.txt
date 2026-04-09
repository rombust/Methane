cmake_minimum_required(VERSION 3.16)

project(SuperMethaneBrothers
    VERSION 2026.1
    DESCRIPTION "Super Methane Brothers"
    LANGUAGES C CXX
)

# ============================================================
# C++20 — hard requirement
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Platform guard
# ============================================================
if(NOT WIN32 AND NOT (UNIX AND NOT APPLE))
    message(FATAL_ERROR "Only Windows and Linux (X11) are supported targets.")
endif()

if(UNIX AND NOT APPLE)
    set(METHANE_LINUX TRUE)
endif()

# ============================================================
# Linux: detect optional headers that gate conditional code
# ============================================================
if(METHANE_LINUX)
    include(CheckIncludeFile)

    # ALSA is the required sound backend on Linux.
    check_include_file("alsa/asoundlib.h" HAVE_ALSA_ASOUNDLIB_H)
    if(NOT HAVE_ALSA_ASOUNDLIB_H)
        message(FATAL_ERROR
            "ALSA development headers not found.\n"
            "  Debian/Ubuntu : sudo apt install libasound2-dev\n"
            "  Fedora/RHEL   : sudo dnf install alsa-lib-devel")
    endif()
    find_package(ALSA REQUIRED)

    # Xrender — conditionally included in vulkan_window_provider_x11.cpp
    # via #ifdef HAVE_X11_EXTENSIONS_XRENDER_H
    check_include_file("X11/extensions/Xrender.h" HAVE_X11_EXTENSIONS_XRENDER_H)

    # Linux joystick API — conditionally used in input_device_provider_linuxjoystick.cpp
    # via #ifdef HAVE_LINUX_JOYSTICK_H
    check_include_file("linux/joystick.h" HAVE_LINUX_JOYSTICK_H)
endif()

# ============================================================
# Find packages
# ============================================================

find_package(Vulkan)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR
        "Vulkan development files not found.\n"
        "  Debian/Ubuntu : sudo apt install libvulkan-dev\n"
        "  Fedora/RHEL   : sudo dnf install vulkan-devel\n"
        "  Arch Linux    : sudo pacman -S vulkan-devel\n"
    )
endif()


if(METHANE_LINUX)
    find_package(X11 REQUIRED)
    # Xinerama is called unconditionally in x11_window.cpp
    if(NOT X11_Xinerama_FOUND)
        message(FATAL_ERROR
            "Xinerama not found.\n"
            "  Debian/Ubuntu : sudo apt install libxinerama-dev\n"
            "  Fedora/RHEL   : sudo dnf install libXinerama-devel")
    endif()
endif()

# ============================================================
# Source file lists
# ============================================================

# --- Game layer ---
set(SOURCES_GAME
    Sources/Game/baddie.cpp
    Sources/Game/amiga_anim.cpp
    Sources/Game/bitdraw.cpp
    Sources/Game/bitgroup.cpp
    Sources/Game/bititem.cpp
    Sources/Game/boss.cpp
    Sources/Game/doc.cpp
    Sources/Game/game.cpp
    Sources/Game/game_render_batch_triangle.cpp
    Sources/Game/gasobj.cpp
    Sources/Game/gfxoff.cpp
    Sources/Game/global.cpp
    Sources/Game/goodie.cpp
    Sources/Game/mapdata.cpp
    Sources/Game/maps.cpp
    Sources/Game/methane.cpp
    Sources/Game/misc.cpp
    Sources/Game/objlist.cpp
    Sources/Game/player.cpp
    Sources/Game/power.cpp
    Sources/Game/sound.cpp
    Sources/Game/suck.cpp
    Sources/Game/target.cpp
    Sources/Game/weapon.cpp
)

# --- TinyClan Core ---
set(SOURCES_TC_CORE
    Sources/TinyClan/Core/core_iostream.cpp
    Sources/TinyClan/Core/ErrorReporting/exception_dialog.cpp
    Sources/TinyClan/Core/IOData/directory.cpp
    Sources/TinyClan/Core/IOData/directory_listing.cpp
    Sources/TinyClan/Core/IOData/directory_listing_entry.cpp
    Sources/TinyClan/Core/IOData/directory_scanner.cpp
    Sources/TinyClan/Core/IOData/endianess.cpp
    Sources/TinyClan/Core/IOData/file.cpp
    Sources/TinyClan/Core/IOData/file_help.cpp
    Sources/TinyClan/Core/IOData/file_system.cpp
    Sources/TinyClan/Core/IOData/file_system_provider_file.cpp
    Sources/TinyClan/Core/IOData/iodevice.cpp
    Sources/TinyClan/Core/IOData/iodevice_provider_file.cpp
    Sources/TinyClan/Core/IOData/iodevice_provider_memory.cpp
    Sources/TinyClan/Core/IOData/memory_device.cpp
    Sources/TinyClan/Core/IOData/path_help.cpp
    Sources/TinyClan/Core/Math/angle.cpp
    Sources/TinyClan/Core/Math/half_float.cpp
    Sources/TinyClan/Core/Math/mat2.cpp
    Sources/TinyClan/Core/Math/mat3.cpp
    Sources/TinyClan/Core/Math/mat4.cpp
    Sources/TinyClan/Core/Math/origin.cpp
    Sources/TinyClan/Core/Math/rect.cpp
    Sources/TinyClan/Core/Math/vec2.cpp
    Sources/TinyClan/Core/Math/vec3.cpp
    Sources/TinyClan/Core/Math/vec4.cpp
    Sources/TinyClan/Core/System/block_allocator.cpp
    Sources/TinyClan/Core/System/console_window.cpp
    Sources/TinyClan/Core/System/console_window_generic.cpp
    Sources/TinyClan/Core/System/databuffer.cpp
    Sources/TinyClan/Core/System/datetime.cpp
    Sources/TinyClan/Core/System/detect_cpu_ext.cpp
    Sources/TinyClan/Core/System/disposable_object.cpp
    Sources/TinyClan/Core/System/exception.cpp
    Sources/TinyClan/Core/System/game_time.cpp
    Sources/TinyClan/Core/System/setup_core.cpp
    Sources/TinyClan/Core/System/system.cpp
    Sources/TinyClan/Core/System/thread_local_storage.cpp
    Sources/TinyClan/Core/System/thread_local_storage_impl.cpp
    Sources/TinyClan/Core/System/tls_instance.cpp
    Sources/TinyClan/Core/Text/console.cpp
    Sources/TinyClan/Core/Text/console_logger.cpp
    Sources/TinyClan/Core/Text/logger.cpp
    Sources/TinyClan/Core/Text/string_format.cpp
    Sources/TinyClan/Core/Text/string_help.cpp
    Sources/TinyClan/Core/Zip/miniz.cpp
    Sources/TinyClan/Core/Zip/zlib_compression.cpp
)

# --- TinyClan Display ---
set(SOURCES_TC_DISPLAY
    Sources/TinyClan/Display/2D/canvas.cpp
    Sources/TinyClan/Display/2D/canvas_batcher.cpp
    Sources/TinyClan/Display/2D/canvas_impl.cpp
    Sources/TinyClan/Display/2D/color.cpp
    Sources/TinyClan/Display/display_target.cpp
    Sources/TinyClan/Display/Image/cpu_pixel_buffer_provider.cpp
    Sources/TinyClan/Display/Image/icon_set.cpp
    Sources/TinyClan/Display/Image/image_import_description.cpp
    Sources/TinyClan/Display/Image/pixel_buffer.cpp
    Sources/TinyClan/Display/Image/pixel_buffer_help.cpp
    Sources/TinyClan/Display/Image/pixel_buffer_impl.cpp
    Sources/TinyClan/Display/Image/pixel_buffer_set.cpp
    Sources/TinyClan/Display/Image/pixel_converter.cpp
    Sources/TinyClan/Display/ImageProviders/PNGLoader/png_loader.cpp
    Sources/TinyClan/Display/ImageProviders/png_provider.cpp
    Sources/TinyClan/Display/ImageProviders/provider_factory.cpp
    Sources/TinyClan/Display/ImageProviders/provider_type.cpp
    Sources/TinyClan/Display/Render/graphic_context.cpp
    Sources/TinyClan/Display/Render/graphic_context_impl.cpp
    Sources/TinyClan/Display/Render/graphic_context_state.cpp
    Sources/TinyClan/Display/Render/graphic_screen.cpp
    Sources/TinyClan/Display/Render/primitives_array.cpp
    Sources/TinyClan/Display/Render/program_object.cpp
    Sources/TinyClan/Display/Render/render_buffer.cpp
    Sources/TinyClan/Display/Render/shader_object.cpp
    Sources/TinyClan/Display/Render/shared_gc_data.cpp
    Sources/TinyClan/Display/Render/shared_gc_data_impl.cpp
    Sources/TinyClan/Display/Render/storage_buffer.cpp
    Sources/TinyClan/Display/Render/texture.cpp
    Sources/TinyClan/Display/Render/texture_2d.cpp
    Sources/TinyClan/Display/Render/texture_impl.cpp
    Sources/TinyClan/Display/Render/transfer_buffer.cpp
    Sources/TinyClan/Display/Render/uniform_buffer.cpp
    Sources/TinyClan/Display/Render/vertex_array_buffer.cpp
    Sources/TinyClan/Display/screen_info.cpp
    Sources/TinyClan/Display/setup_display.cpp
    Sources/TinyClan/Display/System/run_loop.cpp
    Sources/TinyClan/Display/System/timer.cpp
    Sources/TinyClan/Display/Window/display_window.cpp
    Sources/TinyClan/Display/Window/display_window_description.cpp
    Sources/TinyClan/Display/Window/input_device.cpp
    Sources/TinyClan/Display/Window/input_event.cpp
    Sources/TinyClan/Display/Window/keys.cpp
)

# --- TinyClan Sound core ---
set(SOURCES_TC_SOUND
    Sources/TinyClan/Sound/clan_sound.cpp
    Sources/TinyClan/Sound/Mixer/sound_format_conversion.cpp
    Sources/TinyClan/Sound/setupsound.cpp
    Sources/TinyClan/Sound/sound_sse.cpp
    Sources/TinyClan/Sound/soundbuffer.cpp
    Sources/TinyClan/Sound/soundbuffer_impl.cpp
    Sources/TinyClan/Sound/soundbuffer_session.cpp
    Sources/TinyClan/Sound/soundbuffer_session_impl.cpp
    Sources/TinyClan/Sound/soundfilter.cpp
    Sources/TinyClan/Sound/soundoutput.cpp
    Sources/TinyClan/Sound/soundoutput_description.cpp
    Sources/TinyClan/Sound/soundoutput_impl.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_factory.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_raw.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_raw_session.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_session.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_type.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis_session.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_wave.cpp
    Sources/TinyClan/Sound/SoundProviders/soundprovider_wave_session.cpp
)

# --- TinyClan Vulkan ---
set(SOURCES_TC_VK
    Sources/TinyClan/VK/setup_vulkan.cpp
    Sources/TinyClan/VK/volk.c
    Sources/TinyClan/VK/vulkan_context_description.cpp
    Sources/TinyClan/VK/vulkan_device.cpp
    Sources/TinyClan/VK/vulkan_vma.cpp
    Sources/TinyClan/VK/vulkan_window_provider_base.cpp
    Sources/TinyClan/VK/VK1/vulkan_buffer_object_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_graphic_context_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_primitives_array_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_program_object_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_render_buffer_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_shader_object_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_storage_buffer_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_texture_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_transfer_buffer_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_uniform_buffer_provider.cpp
    Sources/TinyClan/VK/VK1/vulkan_vertex_array_buffer_provider.cpp
)

# ============================================================
# Platform-specific sources
# ============================================================
if(WIN32)
    set(SOURCES_PLATFORM
        # App entry point (WinMain)
        Sources/TinyClan/App/Win32/clanapp.cpp
        # Core
        Sources/TinyClan/Core/IOData/Win32/directory_scanner_win32.cpp
        Sources/TinyClan/Core/System/Win32/system_win32.cpp
        # Display
        Sources/TinyClan/Display/Platform/Win32/display_message_queue_win32.cpp
        Sources/TinyClan/Display/Platform/Win32/dwm_functions.cpp
        Sources/TinyClan/Display/Platform/Win32/hid.cpp
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32hid.cpp
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32keyboard.cpp
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32mouse.cpp
        Sources/TinyClan/Display/Platform/Win32/screen_info_provider_win32.cpp
        Sources/TinyClan/Display/Platform/Win32/win32_window.cpp
        Sources/TinyClan/Sound/Platform/Win32/init_guids.cpp
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_directsound.cpp
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_win32.cpp
        # Vulkan Win32 surface
        Sources/TinyClan/VK/Platform/Win32/vulkan_window_provider.cpp
    )

elseif(METHANE_LINUX)
    set(SOURCES_PLATFORM
        # App entry point (main)
        Sources/TinyClan/App/Unix/clanapp.cpp
        # Core
        Sources/TinyClan/Core/IOData/Unix/directory_scanner_unix.cpp
        Sources/TinyClan/Core/System/Unix/system_unix.cpp
        # Display (X11)
        Sources/TinyClan/Display/Platform/X11/display_message_queue_x11.cpp
        Sources/TinyClan/Display/Platform/X11/input_device_provider_linuxjoystick.cpp
        Sources/TinyClan/Display/Platform/X11/input_device_provider_x11keyboard.cpp
        Sources/TinyClan/Display/Platform/X11/input_device_provider_x11mouse.cpp
        Sources/TinyClan/Display/Platform/X11/x11_atoms.cpp
        Sources/TinyClan/Display/Platform/X11/x11_window.cpp
        # Sound — ALSA is the required Linux sound backend.
        Sources/TinyClan/Sound/Platform/Linux/soundoutput_alsa.cpp
        # Vulkan Xlib surface
        Sources/TinyClan/VK/Platform/X11/vulkan_window_provider_x11.cpp
    )
endif()

# ============================================================
# Executable
# ============================================================
add_executable(methane
    Sources/precomp.cpp          # PCH creation TU (MSVC); no-op on GCC/Clang
    ${SOURCES_GAME}
    ${SOURCES_TC_CORE}
    ${SOURCES_TC_DISPLAY}
    ${SOURCES_TC_SOUND}
    ${SOURCES_TC_VK}
    ${SOURCES_PLATFORM}
)

# ============================================================
# Include directories
# ============================================================
target_include_directories(methane PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TinyClan
    ${Vulkan_INCLUDE_DIRS}
)

# ============================================================
# Compile definitions
# ============================================================
target_compile_definitions(methane PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    _WINDOWS
)

if(WIN32)
    # MSVC targeting x64 does NOT auto-define WIN32 (only _WIN32/_WIN64).
    # The source uses bare WIN32 throughout for platform detection.
    target_compile_definitions(methane PRIVATE WIN32 NOMINMAX)
endif()

if(METHANE_LINUX)
    if(HAVE_X11_EXTENSIONS_XRENDER_H)
        target_compile_definitions(methane PRIVATE HAVE_X11_EXTENSIONS_XRENDER_H)
    endif()
    if(HAVE_LINUX_JOYSTICK_H)
        target_compile_definitions(methane PRIVATE HAVE_LINUX_JOYSTICK_H)
    endif()
endif()

# ============================================================
# Precompiled header  (MSVC only — manual /Yc + /Yu approach)
# ============================================================
if(MSVC)
    # precomp.cpp — creates the PCH.
    set_source_files_properties(Sources/precomp.cpp PROPERTIES
        COMPILE_FLAGS "/Yc\"precomp.h\" /Fpprecomp.pch /I\"${CMAKE_CURRENT_SOURCE_DIR}/Sources\" /I\"${CMAKE_CURRENT_SOURCE_DIR}/Sources/TinyClan\""
    )

    # All other C++ TUs consume the PCH from the same implicit location.
    target_compile_options(methane PRIVATE
        /Yu"precomp.h"
        /Fpprecomp.pch
    )

    set_source_files_properties(
        Sources/TinyClan/Sound/Platform/Win32/init_guids.cpp
        PROPERTIES COMPILE_FLAGS "/Y-"
    )
endif()

set_source_files_properties(Sources/TinyClan/VK/volk.c PROPERTIES
    LANGUAGE C
)
if(MSVC)
    # /Y- overrides the /Yu"precomp.h" that flows in from target_compile_options.
    set_source_files_properties(Sources/TinyClan/VK/volk.c PROPERTIES
        COMPILE_FLAGS "/Y-"
    )
endif()

# ============================================================
# Third-party headers — suppress all warnings
# ============================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_include_directories(methane SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TinyClan/Sound/SoundProviders
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/TinyClan/API/VK
    )
endif()

# ============================================================
# Compiler options
# ============================================================
if(MSVC)
    target_compile_options(methane PRIVATE
        /W3             # Warning level — matches vcxproj WarningLevel=Level3
        /WX-            # Warnings are not errors
        /MP             # Multi-processor compilation
        /EHsc           # Standard C++ exception handling
        /permissive-    # Strict conformance — matches ConformanceMode=true
        /std:c++20      # C++20 — explicit MSVC flag (reinforces CMAKE_CXX_STANDARD)

        # Runtime library:
        #   Debug   -> /MDd  Multi-threaded Debug DLL  (msvcrtd.lib)
        #   Release -> /MD   Multi-threaded DLL         (msvcrt.lib)
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>

        $<$<CONFIG:Release>:/O2 /Oi /Gy>
        $<$<CONFIG:Debug>:/Od /RTC1>
    )

    target_compile_definitions(methane PRIVATE UNICODE _UNICODE)

    target_link_libraries(methane PRIVATE
        kernel32.lib
        user32.lib
        gdi32.lib
        winspool.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        uuid.lib
        odbc32.lib
        odbccp32.lib
    )

    # SubSystem:Windows -> WinMain entry point (matches x64 vcxproj configs)
    set_target_properties(methane PROPERTIES
        WIN32_EXECUTABLE TRUE
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(methane PRIVATE
        -Wall
        -Wextra
        -msse2              # Required by sound_sse.cpp and pixel SSE paths
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-unused-variable
        -Wno-implicit-fallthrough
        -Wno-missing-field-initializers

        $<$<CXX_COMPILER_ID:GNU>:-Wno-cast-function-type>

        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-g -O0>
    )
endif()

# ============================================================
# Link libraries
# ============================================================
if(METHANE_LINUX)
    target_link_libraries(methane PRIVATE
        ${ALSA_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xinerama_LIB}
        pthread
        dl
    )
    if(X11_Xrender_FOUND AND HAVE_X11_EXTENSIONS_XRENDER_H)
        target_link_libraries(methane PRIVATE ${X11_Xrender_LIB})
    endif()
endif()

# ============================================================
# Visual Studio: source groups mirror vcxproj.filters layout
# ============================================================
if(MSVC)
    macro(vs_group GROUP)
        source_group("${GROUP}" FILES ${ARGN})
    endmacro()

    vs_group("Sources"
        Sources/precomp.cpp
    )
    vs_group("Sources/Game"
        ${SOURCES_GAME}
    )
    vs_group("Sources/TinyClan/App/Win32"
        Sources/TinyClan/App/Win32/clanapp.cpp
    )
    vs_group("Sources/TinyClan/Core"
        Sources/TinyClan/Core/core_iostream.cpp
        Sources/TinyClan/Core/ErrorReporting/exception_dialog.cpp
    )
    vs_group("Sources/TinyClan/Core/IOData"
        Sources/TinyClan/Core/IOData/directory.cpp
        Sources/TinyClan/Core/IOData/directory_listing.cpp
        Sources/TinyClan/Core/IOData/directory_listing_entry.cpp
        Sources/TinyClan/Core/IOData/directory_scanner.cpp
        Sources/TinyClan/Core/IOData/endianess.cpp
        Sources/TinyClan/Core/IOData/file.cpp
        Sources/TinyClan/Core/IOData/file_help.cpp
        Sources/TinyClan/Core/IOData/file_system.cpp
        Sources/TinyClan/Core/IOData/file_system_provider_file.cpp
        Sources/TinyClan/Core/IOData/iodevice.cpp
        Sources/TinyClan/Core/IOData/iodevice_provider_file.cpp
        Sources/TinyClan/Core/IOData/iodevice_provider_memory.cpp
        Sources/TinyClan/Core/IOData/memory_device.cpp
        Sources/TinyClan/Core/IOData/path_help.cpp
        Sources/TinyClan/Core/IOData/Win32/directory_scanner_win32.cpp
    )
    vs_group("Sources/TinyClan/Core/Math"
        Sources/TinyClan/Core/Math/angle.cpp
        Sources/TinyClan/Core/Math/half_float.cpp
        Sources/TinyClan/Core/Math/mat2.cpp
        Sources/TinyClan/Core/Math/mat3.cpp
        Sources/TinyClan/Core/Math/mat4.cpp
        Sources/TinyClan/Core/Math/origin.cpp
        Sources/TinyClan/Core/Math/rect.cpp
        Sources/TinyClan/Core/Math/vec2.cpp
        Sources/TinyClan/Core/Math/vec3.cpp
        Sources/TinyClan/Core/Math/vec4.cpp
    )
    vs_group("Sources/TinyClan/Core/System"
        Sources/TinyClan/Core/System/block_allocator.cpp
        Sources/TinyClan/Core/System/console_window.cpp
        Sources/TinyClan/Core/System/console_window_generic.cpp
        Sources/TinyClan/Core/System/databuffer.cpp
        Sources/TinyClan/Core/System/datetime.cpp
        Sources/TinyClan/Core/System/detect_cpu_ext.cpp
        Sources/TinyClan/Core/System/disposable_object.cpp
        Sources/TinyClan/Core/System/exception.cpp
        Sources/TinyClan/Core/System/game_time.cpp
        Sources/TinyClan/Core/System/setup_core.cpp
        Sources/TinyClan/Core/System/system.cpp
        Sources/TinyClan/Core/System/thread_local_storage.cpp
        Sources/TinyClan/Core/System/thread_local_storage_impl.cpp
        Sources/TinyClan/Core/System/tls_instance.cpp
        Sources/TinyClan/Core/System/Win32/system_win32.cpp
    )
    vs_group("Sources/TinyClan/Core/Text"
        Sources/TinyClan/Core/Text/console.cpp
        Sources/TinyClan/Core/Text/console_logger.cpp
        Sources/TinyClan/Core/Text/logger.cpp
        Sources/TinyClan/Core/Text/string_format.cpp
        Sources/TinyClan/Core/Text/string_help.cpp
    )
    vs_group("Sources/TinyClan/Core/Zip"
        Sources/TinyClan/Core/Zip/miniz.cpp
        Sources/TinyClan/Core/Zip/zlib_compression.cpp
    )
    vs_group("Sources/TinyClan/Display"
        Sources/TinyClan/Display/display_target.cpp
        Sources/TinyClan/Display/screen_info.cpp
        Sources/TinyClan/Display/setup_display.cpp
    )
    vs_group("Sources/TinyClan/Display/2D"
        Sources/TinyClan/Display/2D/canvas.cpp
        Sources/TinyClan/Display/2D/canvas_batcher.cpp
        Sources/TinyClan/Display/2D/canvas_impl.cpp
        Sources/TinyClan/Display/2D/color.cpp
    )
    vs_group("Sources/TinyClan/Display/Image"
        Sources/TinyClan/Display/Image/cpu_pixel_buffer_provider.cpp
        Sources/TinyClan/Display/Image/icon_set.cpp
        Sources/TinyClan/Display/Image/image_import_description.cpp
        Sources/TinyClan/Display/Image/pixel_buffer.cpp
        Sources/TinyClan/Display/Image/pixel_buffer_help.cpp
        Sources/TinyClan/Display/Image/pixel_buffer_impl.cpp
        Sources/TinyClan/Display/Image/pixel_buffer_set.cpp
        Sources/TinyClan/Display/Image/pixel_converter.cpp
    )
    vs_group("Sources/TinyClan/Display/ImageProviders"
        Sources/TinyClan/Display/ImageProviders/PNGLoader/png_loader.cpp
        Sources/TinyClan/Display/ImageProviders/png_provider.cpp
        Sources/TinyClan/Display/ImageProviders/provider_factory.cpp
        Sources/TinyClan/Display/ImageProviders/provider_type.cpp
    )
    vs_group("Sources/TinyClan/Display/Platform/Win32"
        Sources/TinyClan/Display/Platform/Win32/display_message_queue_win32.cpp
        Sources/TinyClan/Display/Platform/Win32/dwm_functions.cpp
        Sources/TinyClan/Display/Platform/Win32/hid.cpp
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32hid.cpp
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32keyboard.cpp
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32mouse.cpp
        Sources/TinyClan/Display/Platform/Win32/screen_info_provider_win32.cpp
        Sources/TinyClan/Display/Platform/Win32/win32_window.cpp
    )
    vs_group("Sources/TinyClan/Display/Render"
        Sources/TinyClan/Display/Render/graphic_context.cpp
        Sources/TinyClan/Display/Render/graphic_context_impl.cpp
        Sources/TinyClan/Display/Render/graphic_context_state.cpp
        Sources/TinyClan/Display/Render/graphic_screen.cpp
        Sources/TinyClan/Display/Render/primitives_array.cpp
        Sources/TinyClan/Display/Render/program_object.cpp
        Sources/TinyClan/Display/Render/render_buffer.cpp
        Sources/TinyClan/Display/Render/shader_object.cpp
        Sources/TinyClan/Display/Render/shared_gc_data.cpp
        Sources/TinyClan/Display/Render/shared_gc_data_impl.cpp
        Sources/TinyClan/Display/Render/storage_buffer.cpp
        Sources/TinyClan/Display/Render/texture.cpp
        Sources/TinyClan/Display/Render/texture_2d.cpp
        Sources/TinyClan/Display/Render/texture_impl.cpp
        Sources/TinyClan/Display/Render/transfer_buffer.cpp
        Sources/TinyClan/Display/Render/uniform_buffer.cpp
        Sources/TinyClan/Display/Render/vertex_array_buffer.cpp
    )
    vs_group("Sources/TinyClan/Display/System"
        Sources/TinyClan/Display/System/run_loop.cpp
        Sources/TinyClan/Display/System/timer.cpp
    )
    vs_group("Sources/TinyClan/Display/Window"
        Sources/TinyClan/Display/Window/display_window.cpp
        Sources/TinyClan/Display/Window/display_window_description.cpp
        Sources/TinyClan/Display/Window/input_device.cpp
        Sources/TinyClan/Display/Window/input_event.cpp
        Sources/TinyClan/Display/Window/keys.cpp
    )
    vs_group("Sources/TinyClan/Sound"
        Sources/TinyClan/Sound/clan_sound.cpp
        Sources/TinyClan/Sound/setupsound.cpp
        Sources/TinyClan/Sound/sound_sse.cpp
        Sources/TinyClan/Sound/soundbuffer.cpp
        Sources/TinyClan/Sound/soundbuffer_impl.cpp
        Sources/TinyClan/Sound/soundbuffer_session.cpp
        Sources/TinyClan/Sound/soundbuffer_session_impl.cpp
        Sources/TinyClan/Sound/soundfilter.cpp
        Sources/TinyClan/Sound/soundoutput.cpp
        Sources/TinyClan/Sound/soundoutput_description.cpp
        Sources/TinyClan/Sound/soundoutput_impl.cpp
    )
    vs_group("Sources/TinyClan/Sound/Mixer"
        Sources/TinyClan/Sound/Mixer/sound_format_conversion.cpp
    )
    vs_group("Sources/TinyClan/Sound/Platform/Win32"
        Sources/TinyClan/Sound/Platform/Win32/init_guids.cpp
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_directsound.cpp
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_win32.cpp
    )
    vs_group("Sources/TinyClan/Sound/SoundProviders"
        Sources/TinyClan/Sound/SoundProviders/soundprovider.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_factory.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_raw.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_raw_session.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_session.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_type.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis_session.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_wave.cpp
        Sources/TinyClan/Sound/SoundProviders/soundprovider_wave_session.cpp
    )
    vs_group("Sources/TinyClan/VK"
        Sources/TinyClan/VK/setup_vulkan.cpp
        Sources/TinyClan/VK/volk.c
        Sources/TinyClan/VK/vulkan_context_description.cpp
        Sources/TinyClan/VK/vulkan_device.cpp
        Sources/TinyClan/VK/vulkan_vma.cpp
        Sources/TinyClan/VK/vulkan_window_provider_base.cpp
    )
    vs_group("Sources/TinyClan/VK/Platform/Win32"
        Sources/TinyClan/VK/Platform/Win32/vulkan_window_provider.cpp
    )
    vs_group("Sources/TinyClan/VK/VK1"
        Sources/TinyClan/VK/VK1/vulkan_buffer_object_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_graphic_context_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_primitives_array_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_program_object_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_render_buffer_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_shader_object_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_storage_buffer_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_texture_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_transfer_buffer_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_uniform_buffer_provider.cpp
        Sources/TinyClan/VK/VK1/vulkan_vertex_array_buffer_provider.cpp
    )
endif()

# ============================================================
# Configure summary
# ============================================================
message(STATUS "")
message(STATUS "=== Super Methane Brothers ===")
message(STATUS "  Platform        : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  C++ standard    : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type      : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Vulkan headers  : ${Vulkan_INCLUDE_DIRS}")
if(METHANE_LINUX)
    message(STATUS "  ALSA            : ${ALSA_LIBRARIES}")
    message(STATUS "  Xinerama        : ${X11_Xinerama_FOUND}")
    message(STATUS "  Xrender         : ${HAVE_X11_EXTENSIONS_XRENDER_H}")
    message(STATUS "  Linux joystick  : ${HAVE_LINUX_JOYSTICK_H}")
endif()
message(STATUS "==============================")
message(STATUS "")

