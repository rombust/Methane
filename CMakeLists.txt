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

# --- Game layer headers ---
set(HEADERS_GAME
    Sources/Game/amiga_anim.h
    Sources/Game/baddie.h
    Sources/Game/bitdraw.h
    Sources/Game/bitgroup.h
    Sources/Game/bititem.h
    Sources/Game/boss.h
    Sources/Game/game.h
    Sources/Game/game_frag_debug_11.spv.h
    Sources/Game/game_frag_release_11.spv.h
    Sources/Game/game_render_batch_triangle.h
    Sources/Game/game_vert_debug_11.spv.h
    Sources/Game/game_vert_release_11.spv.h
    Sources/Game/gasobj.h
    Sources/Game/gfxdef.h
    Sources/Game/global.h
    Sources/Game/goodie.h
    Sources/Game/mapdef.h
    Sources/Game/maps.h
    Sources/Game/misc.h
    Sources/Game/objlist.h
    Sources/Game/objtypes.h
    Sources/Game/player.h
    Sources/Game/power.h
    Sources/Game/snddef.h
    Sources/Game/sound.h
    Sources/Game/suck.h
    Sources/Game/target.h
    Sources/Game/weapon.h
    Sources/precomp.h
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

# --- TinyClan Core headers ---
set(HEADERS_TC_CORE
    # API
    Sources/TinyClan/API/Core/core_iostream.h
    Sources/TinyClan/API/Core/ErrorReporting/exception_dialog.h
    Sources/TinyClan/API/Core/IOData/cl_endian.h
    Sources/TinyClan/API/Core/IOData/directory.h
    Sources/TinyClan/API/Core/IOData/directory_listing.h
    Sources/TinyClan/API/Core/IOData/directory_listing_entry.h
    Sources/TinyClan/API/Core/IOData/directory_scanner.h
    Sources/TinyClan/API/Core/IOData/file.h
    Sources/TinyClan/API/Core/IOData/file_help.h
    Sources/TinyClan/API/Core/IOData/file_system.h
    Sources/TinyClan/API/Core/IOData/file_system_provider.h
    Sources/TinyClan/API/Core/IOData/iodevice.h
    Sources/TinyClan/API/Core/IOData/iodevice_provider.h
    Sources/TinyClan/API/Core/IOData/memory_device.h
    Sources/TinyClan/API/Core/IOData/path_help.h
    Sources/TinyClan/API/Core/Math/angle.h
    Sources/TinyClan/API/Core/Math/cl_math.h
    Sources/TinyClan/API/Core/Math/half_float.h
    Sources/TinyClan/API/Core/Math/half_float_vector.h
    Sources/TinyClan/API/Core/Math/mat2.h
    Sources/TinyClan/API/Core/Math/mat3.h
    Sources/TinyClan/API/Core/Math/mat4.h
    Sources/TinyClan/API/Core/Math/origin.h
    Sources/TinyClan/API/Core/Math/point.h
    Sources/TinyClan/API/Core/Math/rect.h
    Sources/TinyClan/API/Core/Math/size.h
    Sources/TinyClan/API/Core/Math/vec2.h
    Sources/TinyClan/API/Core/Math/vec3.h
    Sources/TinyClan/API/Core/Math/vec4.h
    Sources/TinyClan/API/Core/Signals/bind_member.h
    Sources/TinyClan/API/Core/Signals/signal.h
    Sources/TinyClan/API/Core/System/block_allocator.h
    Sources/TinyClan/API/Core/System/cl_platform.h
    Sources/TinyClan/API/Core/System/comptr.h
    Sources/TinyClan/API/Core/System/console_window.h
    Sources/TinyClan/API/Core/System/databuffer.h
    Sources/TinyClan/API/Core/System/datetime.h
    Sources/TinyClan/API/Core/System/disposable_object.h
    Sources/TinyClan/API/Core/System/exception.h
    Sources/TinyClan/API/Core/System/game_time.h
    Sources/TinyClan/API/Core/System/system.h
    Sources/TinyClan/API/Core/System/thread_local_storage.h
    Sources/TinyClan/API/Core/Text/console.h
    Sources/TinyClan/API/Core/Text/console_logger.h
    Sources/TinyClan/API/Core/Text/logger.h
    Sources/TinyClan/API/Core/Text/string_format.h
    Sources/TinyClan/API/Core/Text/string_help.h
    Sources/TinyClan/API/Core/Zip/zlib_compression.h
    # Implementation
    Sources/TinyClan/Core/ErrorReporting/exception_dialog_impl.h
    Sources/TinyClan/Core/IOData/directory_scanner_impl.h
    Sources/TinyClan/Core/IOData/file_system_provider_file.h
    Sources/TinyClan/Core/IOData/iodevice_impl.h
    Sources/TinyClan/Core/IOData/iodevice_provider_file.h
    Sources/TinyClan/Core/IOData/iodevice_provider_memory.h
    Sources/TinyClan/Core/System/console_window_generic.h
    Sources/TinyClan/Core/System/game_time_impl.h
    Sources/TinyClan/Core/System/setup_core.h
    Sources/TinyClan/Core/System/thread_local_storage_impl.h
    Sources/TinyClan/Core/System/tls_instance.h
    Sources/TinyClan/Core/Zip/miniz.h
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

# --- TinyClan Display headers ---
set(HEADERS_TC_DISPLAY
    # API
    Sources/TinyClan/API/App/clanapp.h
    Sources/TinyClan/API/Display/display_target.h
    Sources/TinyClan/API/Display/screen_info.h
    Sources/TinyClan/API/Display/2D/canvas.h
    Sources/TinyClan/API/Display/2D/color.h
    Sources/TinyClan/API/Display/2D/gradient.h
    Sources/TinyClan/API/Display/Image/buffer_usage.h
    Sources/TinyClan/API/Display/Image/icon_set.h
    Sources/TinyClan/API/Display/Image/image_import_description.h
    Sources/TinyClan/API/Display/Image/pixel_buffer.h
    Sources/TinyClan/API/Display/Image/pixel_buffer_help.h
    Sources/TinyClan/API/Display/Image/pixel_buffer_lock.h
    Sources/TinyClan/API/Display/Image/pixel_buffer_set.h
    Sources/TinyClan/API/Display/Image/pixel_converter.h
    Sources/TinyClan/API/Display/Image/texture_format.h
    Sources/TinyClan/API/Display/ImageProviders/png_provider.h
    Sources/TinyClan/API/Display/ImageProviders/provider_factory.h
    Sources/TinyClan/API/Display/ImageProviders/provider_type.h
    Sources/TinyClan/API/Display/ImageProviders/provider_type_register.h
    Sources/TinyClan/API/Display/Render/graphic_context.h
    Sources/TinyClan/API/Display/Render/primitives_array.h
    Sources/TinyClan/API/Display/Render/program_object.h
    Sources/TinyClan/API/Display/Render/render_batcher.h
    Sources/TinyClan/API/Display/Render/render_buffer.h
    Sources/TinyClan/API/Display/Render/shader_object.h
    Sources/TinyClan/API/Display/Render/shared_gc_data.h
    Sources/TinyClan/API/Display/Render/storage_buffer.h
    Sources/TinyClan/API/Display/Render/storage_vector.h
    Sources/TinyClan/API/Display/Render/texture.h
    Sources/TinyClan/API/Display/Render/texture_2d.h
    Sources/TinyClan/API/Display/Render/transfer_buffer.h
    Sources/TinyClan/API/Display/Render/transfer_vector.h
    Sources/TinyClan/API/Display/Render/uniform_buffer.h
    Sources/TinyClan/API/Display/Render/uniform_vector.h
    Sources/TinyClan/API/Display/Render/vertex_array_buffer.h
    Sources/TinyClan/API/Display/Render/vertex_array_vector.h
    Sources/TinyClan/API/Display/System/run_loop.h
    Sources/TinyClan/API/Display/System/timer.h
    Sources/TinyClan/API/Display/TargetProviders/display_target_provider.h
    Sources/TinyClan/API/Display/TargetProviders/display_window_provider.h
    Sources/TinyClan/API/Display/TargetProviders/graphic_context_provider.h
    Sources/TinyClan/API/Display/TargetProviders/input_device_provider.h
    Sources/TinyClan/API/Display/TargetProviders/pixel_buffer_provider.h
    Sources/TinyClan/API/Display/TargetProviders/primitives_array_provider.h
    Sources/TinyClan/API/Display/TargetProviders/program_object_provider.h
    Sources/TinyClan/API/Display/TargetProviders/render_buffer_provider.h
    Sources/TinyClan/API/Display/TargetProviders/shader_object_provider.h
    Sources/TinyClan/API/Display/TargetProviders/storage_buffer_provider.h
    Sources/TinyClan/API/Display/TargetProviders/texture_provider.h
    Sources/TinyClan/API/Display/TargetProviders/transfer_buffer_provider.h
    Sources/TinyClan/API/Display/TargetProviders/uniform_buffer_provider.h
    Sources/TinyClan/API/Display/TargetProviders/vertex_array_buffer_provider.h
    Sources/TinyClan/API/Display/Window/display_window.h
    Sources/TinyClan/API/Display/Window/display_window_description.h
    Sources/TinyClan/API/Display/Window/input_code.h
    Sources/TinyClan/API/Display/Window/input_device.h
    Sources/TinyClan/API/Display/Window/input_event.h
    Sources/TinyClan/API/Display/Window/keys.h
    # Implementation
    Sources/TinyClan/Display/2D/canvas_batcher.h
    Sources/TinyClan/Display/2D/canvas_impl.h
    Sources/TinyClan/Display/Image/cpu_pixel_buffer_provider.h
    Sources/TinyClan/Display/Image/image_import_description_impl.h
    Sources/TinyClan/Display/Image/pixel_buffer_impl.h
    Sources/TinyClan/Display/Image/pixel_converter_impl.h
    Sources/TinyClan/Display/Image/pixel_filter_gamma.h
    Sources/TinyClan/Display/Image/pixel_filter_premultiply_alpha.h
    Sources/TinyClan/Display/Image/pixel_filter_rgb_to_ycrcb.h
    Sources/TinyClan/Display/Image/pixel_filter_swizzle.h
    Sources/TinyClan/Display/Image/pixel_reader_cast.h
    Sources/TinyClan/Display/Image/pixel_reader_half_float.h
    Sources/TinyClan/Display/Image/pixel_reader_norm.h
    Sources/TinyClan/Display/Image/pixel_reader_special.h
    Sources/TinyClan/Display/Image/pixel_reader_sse.h
    Sources/TinyClan/Display/Image/pixel_writer_cast.h
    Sources/TinyClan/Display/Image/pixel_writer_half_float.h
    Sources/TinyClan/Display/Image/pixel_writer_norm.h
    Sources/TinyClan/Display/Image/pixel_writer_special.h
    Sources/TinyClan/Display/Image/pixel_writer_sse.h
    Sources/TinyClan/Display/ImageProviders/PNGLoader/png_loader.h
    Sources/TinyClan/Display/Render/graphic_context_impl.h
    Sources/TinyClan/Display/Render/graphic_context_state.h
    Sources/TinyClan/Display/Render/graphic_screen.h
    Sources/TinyClan/Display/Render/primitives_array_impl.h
    Sources/TinyClan/Display/Render/shared_gc_data_impl.h
    Sources/TinyClan/Display/Render/texture_impl.h
    Sources/TinyClan/Display/screen_info_provider.h
    Sources/TinyClan/Display/setup_display.h
    Sources/TinyClan/Display/System/run_loop_impl.h
    Sources/TinyClan/Display/Window/display_window_description_impl.h
    Sources/TinyClan/Display/Window/display_window_impl.h
    Sources/TinyClan/Display/Window/input_device_impl.h
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

# --- TinyClan Sound headers ---
set(HEADERS_TC_SOUND
    # API
    Sources/TinyClan/API/Sound/clan_sound.h
    Sources/TinyClan/API/Sound/sound_sse.h
    Sources/TinyClan/API/Sound/soundbuffer.h
    Sources/TinyClan/API/Sound/soundbuffer_session.h
    Sources/TinyClan/API/Sound/soundfilter.h
    Sources/TinyClan/API/Sound/soundformat.h
    Sources/TinyClan/API/Sound/soundoutput.h
    Sources/TinyClan/API/Sound/SoundProviders/soundfilter_provider.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_factory.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_raw.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_session.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_type.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_type_register.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_vorbis.h
    Sources/TinyClan/API/Sound/SoundProviders/soundprovider_wave.h
    # Implementation
    Sources/TinyClan/Sound/Mixer/sound_format_conversion.h
    Sources/TinyClan/Sound/Mixer/sound_mixer.h
    Sources/TinyClan/Sound/Mixer/sound_mixer_program.h
    Sources/TinyClan/Sound/Mixer/sound_mixing_buffers_container.h
    Sources/TinyClan/Sound/Mixer/sound_mixing_buffers_data.h
    Sources/TinyClan/Sound/Mixer/sound_sample_source.h
    Sources/TinyClan/Sound/Mixer/speaker_position.h
    Sources/TinyClan/Sound/setupsound.h
    Sources/TinyClan/Sound/soundbuffer_impl.h
    Sources/TinyClan/Sound/soundbuffer_session_impl.h
    Sources/TinyClan/Sound/soundoutput_impl.h
    Sources/TinyClan/Sound/SoundProviders/soundprovider_raw_impl.h
    Sources/TinyClan/Sound/SoundProviders/soundprovider_raw_session.h
    Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis_impl.h
    Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis_session.h
    Sources/TinyClan/Sound/SoundProviders/soundprovider_wave_impl.h
    Sources/TinyClan/Sound/SoundProviders/soundprovider_wave_session.h
    Sources/TinyClan/Sound/SoundProviders/stb_vorbis.h
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

# --- TinyClan Vulkan headers ---
set(HEADERS_TC_VK
    # API
    Sources/TinyClan/API/VK/setup_vulkan.h
    Sources/TinyClan/API/VK/vk_mem_alloc.h
    Sources/TinyClan/API/VK/vk_mem_alloc_config.h
    Sources/TinyClan/API/VK/volk.h
    Sources/TinyClan/API/VK/vulkan_context_description.h
    Sources/TinyClan/API/VK/vulkan_target.h
    # Implementation
    Sources/TinyClan/VK/vulkan_context_description_impl.h
    Sources/TinyClan/VK/vulkan_device.h
    Sources/TinyClan/VK/vulkan_window_provider_base.h
    Sources/TinyClan/VK/VK1/vulkan_buffer_object_provider.h
    Sources/TinyClan/VK/VK1/vulkan_graphic_context_provider.h
    Sources/TinyClan/VK/VK1/vulkan_primitives_array_provider.h
    Sources/TinyClan/VK/VK1/vulkan_program_object_provider.h
    Sources/TinyClan/VK/VK1/vulkan_render_buffer_provider.h
    Sources/TinyClan/VK/VK1/vulkan_shader_object_provider.h
    Sources/TinyClan/VK/VK1/vulkan_storage_buffer_provider.h
    Sources/TinyClan/VK/VK1/vulkan_texture_provider.h
    Sources/TinyClan/VK/VK1/vulkan_transfer_buffer_provider.h
    Sources/TinyClan/VK/VK1/vulkan_uniform_buffer_provider.h
    Sources/TinyClan/VK/VK1/vulkan_vertex_array_buffer_provider.h
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
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_win32.cpp
        # Vulkan Win32 surface
        Sources/TinyClan/VK/Platform/Win32/vulkan_window_provider.cpp
    )
    set(HEADERS_PLATFORM
        # Core
        Sources/TinyClan/Core/IOData/Win32/directory_scanner_win32.h
        Sources/TinyClan/Core/System/Win32/system_win32.h
        # Display
        Sources/TinyClan/Display/Platform/Win32/display_message_queue_win32.h
        Sources/TinyClan/Display/Platform/Win32/dwm_functions.h
        Sources/TinyClan/Display/Platform/Win32/hid.h
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32hid.h
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32keyboard.h
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32mouse.h
        Sources/TinyClan/Display/Platform/Win32/screen_info_provider_win32.h
        Sources/TinyClan/Display/Platform/Win32/win32_window.h
        # Sound
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_win32.h
        # Vulkan Win32 surface
        Sources/TinyClan/VK/Platform/Win32/vulkan_window_provider.h
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
    set(HEADERS_PLATFORM
        # Core
        Sources/TinyClan/Core/IOData/Unix/directory_scanner_unix.h
        Sources/TinyClan/Core/System/Unix/system_unix.h
        # Display (X11)
        Sources/TinyClan/Display/Platform/X11/display_message_queue_x11.h
        Sources/TinyClan/Display/Platform/X11/input_device_provider_linuxjoystick.h
        Sources/TinyClan/Display/Platform/X11/input_device_provider_x11keyboard.h
        Sources/TinyClan/Display/Platform/X11/input_device_provider_x11mouse.h
        Sources/TinyClan/Display/Platform/X11/x11_atoms.h
        Sources/TinyClan/Display/Platform/X11/x11_window.h
        # Sound
        Sources/TinyClan/Sound/Platform/Linux/soundoutput_alsa.h
        # Vulkan Xlib surface
        Sources/TinyClan/VK/Platform/X11/vulkan_window_provider_x11.h
    )
endif()

# ============================================================
# Executable
# ============================================================
add_executable(methane
    Sources/precomp.cpp          # PCH creation TU (MSVC); no-op on GCC/Clang
    ${SOURCES_GAME}
    ${HEADERS_GAME}
    ${SOURCES_TC_CORE}
    ${HEADERS_TC_CORE}
    ${SOURCES_TC_DISPLAY}
    ${HEADERS_TC_DISPLAY}
    ${SOURCES_TC_SOUND}
    ${HEADERS_TC_SOUND}
    ${SOURCES_TC_VK}
    ${HEADERS_TC_VK}
    ${SOURCES_PLATFORM}
    ${HEADERS_PLATFORM}
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
    # -msse2 is only valid on x86/x64 — detect architecture at configure time
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|x86|i[3-6]86|AMD64)$")
        set(METHANE_SSE2_FLAG -msse2)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
        target_compile_definitions(methane PRIVATE CL_DISABLE_SSE2 ARM_PLATFORM)
    else()
        target_compile_definitions(methane PRIVATE CL_DISABLE_SSE2)
    endif()

    target_compile_options(methane PRIVATE
        -Wall
        -Wextra
        ${METHANE_SSE2_FLAG}    # Required by sound_sse.cpp; empty on non-x86
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
        Sources/precomp.h
    )
    vs_group("Sources/Game"
        ${SOURCES_GAME}
        ${HEADERS_GAME}
    )
    vs_group("Sources/TinyClan/App/Win32"
        Sources/TinyClan/App/Win32/clanapp.cpp
    )
    vs_group("Sources/TinyClan/API/App"
        Sources/TinyClan/API/App/clanapp.h
    )
    vs_group("Sources/TinyClan/Core"
        Sources/TinyClan/Core/core_iostream.cpp
        Sources/TinyClan/Core/ErrorReporting/exception_dialog.cpp
    )
    vs_group("Sources/TinyClan/API/Core"
        Sources/TinyClan/API/Core/core_iostream.h
    )
    vs_group("Sources/TinyClan/API/Core/ErrorReporting"
        Sources/TinyClan/API/Core/ErrorReporting/exception_dialog.h
    )
    vs_group("Sources/TinyClan/API/Core/IOData"
        Sources/TinyClan/API/Core/IOData/cl_endian.h
        Sources/TinyClan/API/Core/IOData/directory.h
        Sources/TinyClan/API/Core/IOData/directory_listing.h
        Sources/TinyClan/API/Core/IOData/directory_listing_entry.h
        Sources/TinyClan/API/Core/IOData/directory_scanner.h
        Sources/TinyClan/API/Core/IOData/file.h
        Sources/TinyClan/API/Core/IOData/file_help.h
        Sources/TinyClan/API/Core/IOData/file_system.h
        Sources/TinyClan/API/Core/IOData/file_system_provider.h
        Sources/TinyClan/API/Core/IOData/iodevice.h
        Sources/TinyClan/API/Core/IOData/iodevice_provider.h
        Sources/TinyClan/API/Core/IOData/memory_device.h
        Sources/TinyClan/API/Core/IOData/path_help.h
    )
    vs_group("Sources/TinyClan/API/Core/Math"
        Sources/TinyClan/API/Core/Math/angle.h
        Sources/TinyClan/API/Core/Math/cl_math.h
        Sources/TinyClan/API/Core/Math/half_float.h
        Sources/TinyClan/API/Core/Math/half_float_vector.h
        Sources/TinyClan/API/Core/Math/mat2.h
        Sources/TinyClan/API/Core/Math/mat3.h
        Sources/TinyClan/API/Core/Math/mat4.h
        Sources/TinyClan/API/Core/Math/origin.h
        Sources/TinyClan/API/Core/Math/point.h
        Sources/TinyClan/API/Core/Math/rect.h
        Sources/TinyClan/API/Core/Math/size.h
        Sources/TinyClan/API/Core/Math/vec2.h
        Sources/TinyClan/API/Core/Math/vec3.h
        Sources/TinyClan/API/Core/Math/vec4.h
    )
    vs_group("Sources/TinyClan/API/Core/Signals"
        Sources/TinyClan/API/Core/Signals/bind_member.h
        Sources/TinyClan/API/Core/Signals/signal.h
    )
    vs_group("Sources/TinyClan/API/Core/System"
        Sources/TinyClan/API/Core/System/block_allocator.h
        Sources/TinyClan/API/Core/System/cl_platform.h
        Sources/TinyClan/API/Core/System/comptr.h
        Sources/TinyClan/API/Core/System/console_window.h
        Sources/TinyClan/API/Core/System/databuffer.h
        Sources/TinyClan/API/Core/System/datetime.h
        Sources/TinyClan/API/Core/System/disposable_object.h
        Sources/TinyClan/API/Core/System/exception.h
        Sources/TinyClan/API/Core/System/game_time.h
        Sources/TinyClan/API/Core/System/system.h
        Sources/TinyClan/API/Core/System/thread_local_storage.h
    )
    vs_group("Sources/TinyClan/API/Core/Text"
        Sources/TinyClan/API/Core/Text/console.h
        Sources/TinyClan/API/Core/Text/console_logger.h
        Sources/TinyClan/API/Core/Text/logger.h
        Sources/TinyClan/API/Core/Text/string_format.h
        Sources/TinyClan/API/Core/Text/string_help.h
    )
    vs_group("Sources/TinyClan/API/Core/Zip"
        Sources/TinyClan/API/Core/Zip/zlib_compression.h
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
        Sources/TinyClan/Core/IOData/directory_scanner_impl.h
        Sources/TinyClan/Core/IOData/file_system_provider_file.h
        Sources/TinyClan/Core/IOData/iodevice_impl.h
        Sources/TinyClan/Core/IOData/iodevice_provider_file.h
        Sources/TinyClan/Core/IOData/iodevice_provider_memory.h
        Sources/TinyClan/Core/IOData/Win32/directory_scanner_win32.h
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
        Sources/TinyClan/Core/System/console_window_generic.h
        Sources/TinyClan/Core/System/game_time_impl.h
        Sources/TinyClan/Core/System/setup_core.h
        Sources/TinyClan/Core/System/thread_local_storage_impl.h
        Sources/TinyClan/Core/System/tls_instance.h
        Sources/TinyClan/Core/System/Win32/system_win32.h
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
        Sources/TinyClan/Core/Zip/miniz.h
    )
    vs_group("Sources/TinyClan/Display"
        Sources/TinyClan/Display/display_target.cpp
        Sources/TinyClan/Display/screen_info.cpp
        Sources/TinyClan/Display/setup_display.cpp
        Sources/TinyClan/Display/screen_info_provider.h
        Sources/TinyClan/Display/setup_display.h
    )
    vs_group("Sources/TinyClan/API/Display"
        Sources/TinyClan/API/Display/display_target.h
        Sources/TinyClan/API/Display/screen_info.h
    )
    vs_group("Sources/TinyClan/API/Display/2D"
        Sources/TinyClan/API/Display/2D/canvas.h
        Sources/TinyClan/API/Display/2D/color.h
        Sources/TinyClan/API/Display/2D/gradient.h
    )
    vs_group("Sources/TinyClan/API/Display/Image"
        Sources/TinyClan/API/Display/Image/buffer_usage.h
        Sources/TinyClan/API/Display/Image/icon_set.h
        Sources/TinyClan/API/Display/Image/image_import_description.h
        Sources/TinyClan/API/Display/Image/pixel_buffer.h
        Sources/TinyClan/API/Display/Image/pixel_buffer_help.h
        Sources/TinyClan/API/Display/Image/pixel_buffer_lock.h
        Sources/TinyClan/API/Display/Image/pixel_buffer_set.h
        Sources/TinyClan/API/Display/Image/pixel_converter.h
        Sources/TinyClan/API/Display/Image/texture_format.h
    )
    vs_group("Sources/TinyClan/API/Display/ImageProviders"
        Sources/TinyClan/API/Display/ImageProviders/png_provider.h
        Sources/TinyClan/API/Display/ImageProviders/provider_factory.h
        Sources/TinyClan/API/Display/ImageProviders/provider_type.h
        Sources/TinyClan/API/Display/ImageProviders/provider_type_register.h
    )
    vs_group("Sources/TinyClan/API/Display/Render"
        Sources/TinyClan/API/Display/Render/graphic_context.h
        Sources/TinyClan/API/Display/Render/primitives_array.h
        Sources/TinyClan/API/Display/Render/program_object.h
        Sources/TinyClan/API/Display/Render/render_batcher.h
        Sources/TinyClan/API/Display/Render/render_buffer.h
        Sources/TinyClan/API/Display/Render/shader_object.h
        Sources/TinyClan/API/Display/Render/shared_gc_data.h
        Sources/TinyClan/API/Display/Render/storage_buffer.h
        Sources/TinyClan/API/Display/Render/storage_vector.h
        Sources/TinyClan/API/Display/Render/texture.h
        Sources/TinyClan/API/Display/Render/texture_2d.h
        Sources/TinyClan/API/Display/Render/transfer_buffer.h
        Sources/TinyClan/API/Display/Render/transfer_vector.h
        Sources/TinyClan/API/Display/Render/uniform_buffer.h
        Sources/TinyClan/API/Display/Render/uniform_vector.h
        Sources/TinyClan/API/Display/Render/vertex_array_buffer.h
        Sources/TinyClan/API/Display/Render/vertex_array_vector.h
    )
    vs_group("Sources/TinyClan/API/Display/System"
        Sources/TinyClan/API/Display/System/run_loop.h
        Sources/TinyClan/API/Display/System/timer.h
    )
    vs_group("Sources/TinyClan/API/Display/TargetProviders"
        Sources/TinyClan/API/Display/TargetProviders/display_target_provider.h
        Sources/TinyClan/API/Display/TargetProviders/display_window_provider.h
        Sources/TinyClan/API/Display/TargetProviders/graphic_context_provider.h
        Sources/TinyClan/API/Display/TargetProviders/input_device_provider.h
        Sources/TinyClan/API/Display/TargetProviders/pixel_buffer_provider.h
        Sources/TinyClan/API/Display/TargetProviders/primitives_array_provider.h
        Sources/TinyClan/API/Display/TargetProviders/program_object_provider.h
        Sources/TinyClan/API/Display/TargetProviders/render_buffer_provider.h
        Sources/TinyClan/API/Display/TargetProviders/shader_object_provider.h
        Sources/TinyClan/API/Display/TargetProviders/storage_buffer_provider.h
        Sources/TinyClan/API/Display/TargetProviders/texture_provider.h
        Sources/TinyClan/API/Display/TargetProviders/transfer_buffer_provider.h
        Sources/TinyClan/API/Display/TargetProviders/uniform_buffer_provider.h
        Sources/TinyClan/API/Display/TargetProviders/vertex_array_buffer_provider.h
    )
    vs_group("Sources/TinyClan/API/Display/Window"
        Sources/TinyClan/API/Display/Window/display_window.h
        Sources/TinyClan/API/Display/Window/display_window_description.h
        Sources/TinyClan/API/Display/Window/input_code.h
        Sources/TinyClan/API/Display/Window/input_device.h
        Sources/TinyClan/API/Display/Window/input_event.h
        Sources/TinyClan/API/Display/Window/keys.h
    )
    vs_group("Sources/TinyClan/Display/2D"
        Sources/TinyClan/Display/2D/canvas.cpp
        Sources/TinyClan/Display/2D/canvas_batcher.cpp
        Sources/TinyClan/Display/2D/canvas_impl.cpp
        Sources/TinyClan/Display/2D/color.cpp
        Sources/TinyClan/Display/2D/canvas_batcher.h
        Sources/TinyClan/Display/2D/canvas_impl.h
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
        Sources/TinyClan/Display/Image/cpu_pixel_buffer_provider.h
        Sources/TinyClan/Display/Image/image_import_description_impl.h
        Sources/TinyClan/Display/Image/pixel_buffer_impl.h
        Sources/TinyClan/Display/Image/pixel_converter_impl.h
        Sources/TinyClan/Display/Image/pixel_filter_gamma.h
        Sources/TinyClan/Display/Image/pixel_filter_premultiply_alpha.h
        Sources/TinyClan/Display/Image/pixel_filter_rgb_to_ycrcb.h
        Sources/TinyClan/Display/Image/pixel_filter_swizzle.h
        Sources/TinyClan/Display/Image/pixel_reader_cast.h
        Sources/TinyClan/Display/Image/pixel_reader_half_float.h
        Sources/TinyClan/Display/Image/pixel_reader_norm.h
        Sources/TinyClan/Display/Image/pixel_reader_special.h
        Sources/TinyClan/Display/Image/pixel_reader_sse.h
        Sources/TinyClan/Display/Image/pixel_writer_cast.h
        Sources/TinyClan/Display/Image/pixel_writer_half_float.h
        Sources/TinyClan/Display/Image/pixel_writer_norm.h
        Sources/TinyClan/Display/Image/pixel_writer_special.h
        Sources/TinyClan/Display/Image/pixel_writer_sse.h
    )
    vs_group("Sources/TinyClan/Display/ImageProviders"
        Sources/TinyClan/Display/ImageProviders/PNGLoader/png_loader.cpp
        Sources/TinyClan/Display/ImageProviders/png_provider.cpp
        Sources/TinyClan/Display/ImageProviders/provider_factory.cpp
        Sources/TinyClan/Display/ImageProviders/provider_type.cpp
        Sources/TinyClan/Display/ImageProviders/PNGLoader/png_loader.h
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
        Sources/TinyClan/Display/Platform/Win32/display_message_queue_win32.h
        Sources/TinyClan/Display/Platform/Win32/dwm_functions.h
        Sources/TinyClan/Display/Platform/Win32/hid.h
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32hid.h
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32keyboard.h
        Sources/TinyClan/Display/Platform/Win32/input_device_provider_win32mouse.h
        Sources/TinyClan/Display/Platform/Win32/screen_info_provider_win32.h
        Sources/TinyClan/Display/Platform/Win32/win32_window.h
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
        Sources/TinyClan/Display/Render/graphic_context_impl.h
        Sources/TinyClan/Display/Render/graphic_context_state.h
        Sources/TinyClan/Display/Render/graphic_screen.h
        Sources/TinyClan/Display/Render/primitives_array_impl.h
        Sources/TinyClan/Display/Render/shared_gc_data_impl.h
        Sources/TinyClan/Display/Render/texture_impl.h
    )
    vs_group("Sources/TinyClan/Display/System"
        Sources/TinyClan/Display/System/run_loop.cpp
        Sources/TinyClan/Display/System/timer.cpp
        Sources/TinyClan/Display/System/run_loop_impl.h
    )
    vs_group("Sources/TinyClan/Display/Window"
        Sources/TinyClan/Display/Window/display_window.cpp
        Sources/TinyClan/Display/Window/display_window_description.cpp
        Sources/TinyClan/Display/Window/input_device.cpp
        Sources/TinyClan/Display/Window/input_event.cpp
        Sources/TinyClan/Display/Window/keys.cpp
        Sources/TinyClan/Display/Window/display_window_description_impl.h
        Sources/TinyClan/Display/Window/display_window_impl.h
        Sources/TinyClan/Display/Window/input_device_impl.h
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
        Sources/TinyClan/Sound/soundoutput_impl.cpp
        Sources/TinyClan/Sound/setupsound.h
        Sources/TinyClan/Sound/soundbuffer_impl.h
        Sources/TinyClan/Sound/soundbuffer_session_impl.h
        Sources/TinyClan/Sound/soundoutput_impl.h
    )
    vs_group("Sources/TinyClan/API/Sound"
        Sources/TinyClan/API/Sound/clan_sound.h
        Sources/TinyClan/API/Sound/sound_sse.h
        Sources/TinyClan/API/Sound/soundbuffer.h
        Sources/TinyClan/API/Sound/soundbuffer_session.h
        Sources/TinyClan/API/Sound/soundfilter.h
        Sources/TinyClan/API/Sound/soundformat.h
        Sources/TinyClan/API/Sound/soundoutput.h
    )
    vs_group("Sources/TinyClan/API/Sound/SoundProviders"
        Sources/TinyClan/API/Sound/SoundProviders/soundfilter_provider.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_factory.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_raw.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_session.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_type.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_type_register.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_vorbis.h
        Sources/TinyClan/API/Sound/SoundProviders/soundprovider_wave.h
    )
    vs_group("Sources/TinyClan/Sound/Mixer"
        Sources/TinyClan/Sound/Mixer/sound_format_conversion.cpp
        Sources/TinyClan/Sound/Mixer/sound_format_conversion.h
        Sources/TinyClan/Sound/Mixer/sound_mixer.h
        Sources/TinyClan/Sound/Mixer/sound_mixer_program.h
        Sources/TinyClan/Sound/Mixer/sound_mixing_buffers_container.h
        Sources/TinyClan/Sound/Mixer/sound_mixing_buffers_data.h
        Sources/TinyClan/Sound/Mixer/sound_sample_source.h
        Sources/TinyClan/Sound/Mixer/speaker_position.h
    )
    vs_group("Sources/TinyClan/Sound/Platform/Win32"
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_win32.cpp
        Sources/TinyClan/Sound/Platform/Win32/soundoutput_win32.h
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
        Sources/TinyClan/Sound/SoundProviders/soundprovider_raw_impl.h
        Sources/TinyClan/Sound/SoundProviders/soundprovider_raw_session.h
        Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis_impl.h
        Sources/TinyClan/Sound/SoundProviders/soundprovider_vorbis_session.h
        Sources/TinyClan/Sound/SoundProviders/soundprovider_wave_impl.h
        Sources/TinyClan/Sound/SoundProviders/soundprovider_wave_session.h
        Sources/TinyClan/Sound/SoundProviders/stb_vorbis.h
    )
    vs_group("Sources/TinyClan/VK"
        Sources/TinyClan/VK/setup_vulkan.cpp
        Sources/TinyClan/VK/volk.c
        Sources/TinyClan/VK/vulkan_context_description.cpp
        Sources/TinyClan/VK/vulkan_device.cpp
        Sources/TinyClan/VK/vulkan_vma.cpp
        Sources/TinyClan/VK/vulkan_window_provider_base.cpp
        Sources/TinyClan/VK/vulkan_context_description_impl.h
        Sources/TinyClan/VK/vulkan_device.h
        Sources/TinyClan/VK/vulkan_window_provider_base.h
    )
    vs_group("Sources/TinyClan/API/VK"
        Sources/TinyClan/API/VK/setup_vulkan.h
        Sources/TinyClan/API/VK/vk_mem_alloc.h
        Sources/TinyClan/API/VK/vk_mem_alloc_config.h
        Sources/TinyClan/API/VK/volk.h
        Sources/TinyClan/API/VK/vulkan_context_description.h
        Sources/TinyClan/API/VK/vulkan_target.h
    )
    vs_group("Sources/TinyClan/VK/Platform/Win32"
        Sources/TinyClan/VK/Platform/Win32/vulkan_window_provider.cpp
        Sources/TinyClan/VK/Platform/Win32/vulkan_window_provider.h
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
        Sources/TinyClan/VK/VK1/vulkan_buffer_object_provider.h
        Sources/TinyClan/VK/VK1/vulkan_graphic_context_provider.h
        Sources/TinyClan/VK/VK1/vulkan_primitives_array_provider.h
        Sources/TinyClan/VK/VK1/vulkan_program_object_provider.h
        Sources/TinyClan/VK/VK1/vulkan_render_buffer_provider.h
        Sources/TinyClan/VK/VK1/vulkan_shader_object_provider.h
        Sources/TinyClan/VK/VK1/vulkan_storage_buffer_provider.h
        Sources/TinyClan/VK/VK1/vulkan_texture_provider.h
        Sources/TinyClan/VK/VK1/vulkan_transfer_buffer_provider.h
        Sources/TinyClan/VK/VK1/vulkan_uniform_buffer_provider.h
        Sources/TinyClan/VK/VK1/vulkan_vertex_array_buffer_provider.h
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

