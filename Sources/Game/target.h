/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 * Program WebSite: http://methane.sourceforge.net/index.html              *
 *                                                                         *
 ***************************************************************************/

//------------------------------------------------------------------------------
// GameTarget class Header File
//------------------------------------------------------------------------------

#ifndef _target_h
#define _target_h 1

#include "TinyClan/API/App/clanapp.h"
#include "TinyClan/API/Core/System/cl_platform.h"
#include "TinyClan/API/Core/System/comptr.h"
#include "TinyClan/API/Core/System/databuffer.h"
#include "TinyClan/API/Core/System/block_allocator.h"
#include "TinyClan/API/Core/System/datetime.h"
#include "TinyClan/API/Core/System/disposable_object.h"
#include "TinyClan/API/Core/System/exception.h"
#include "TinyClan/API/Core/System/game_time.h"
#include "TinyClan/API/Core/System/system.h"
#include "TinyClan/API/Core/Signals/signal.h"
#include "TinyClan/API/Core/IOData/file.h"
#include "TinyClan/API/Core/IOData/file_help.h"
#include "TinyClan/API/Core/IOData/path_help.h"
#include "TinyClan/API/Core/IOData/iodevice.h"
#include "TinyClan/API/Core/Text/string_format.h"
#include "TinyClan/API/Core/Text/string_help.h"
#include "TinyClan/API/Core/Text/console_logger.h"
#include "TinyClan/API/Core/Math/cl_math.h"
#include "TinyClan/API/Core/Math/vec2.h"
#include "TinyClan/API/Core/Math/vec3.h"
#include "TinyClan/API/Core/Math/vec4.h"
#include "TinyClan/API/Core/Math/mat4.h"
#include "TinyClan/API/Core/Math/rect.h"
#include "TinyClan/API/Core/Math/size.h"
#include "TinyClan/API/Core/IOData/directory.h"
#include "TinyClan/API/Core/core_iostream.h"
#include "TinyClan/API/Core/Zip/zlib_compression.h"
#include "TinyClan/API/Display/display_target.h"
#include "TinyClan/API/Display/screen_info.h"
#include "TinyClan/API/Display/2D/canvas.h"
#include "TinyClan/API/Display/2D/color.h"
#include "TinyClan/API/Display/System/run_loop.h"
#include "TinyClan/API/Display/System/timer.h"
#include "TinyClan/API/Display/Image/pixel_buffer.h"
#include "TinyClan/API/Display/Image/pixel_buffer_lock.h"
#include "TinyClan/API/Display/Image/buffer_usage.h"
#include "TinyClan/API/Display/Image/texture_format.h"
#include "TinyClan/API/Display/ImageProviders/png_provider.h"
#include "TinyClan/API/Display/ImageProviders/provider_factory.h"
#include "TinyClan/API/Display/ImageProviders/provider_type.h"
#include "TinyClan/API/Display/Render/transfer_buffer.h"
#include "TinyClan/API/Display/Render/graphic_context.h"
#include "TinyClan/API/Display/Render/primitives_array.h"
#include "TinyClan/API/Display/Render/program_object.h"
#include "TinyClan/API/Display/Render/render_batcher.h"
#include "TinyClan/API/Display/Render/render_buffer.h"
#include "TinyClan/API/Display/Render/shader_object.h"
#include "TinyClan/API/Display/Render/shared_gc_data.h"
#include "TinyClan/API/Display/Render/texture.h"
#include "TinyClan/API/Display/Render/texture_2d.h"
#include "TinyClan/API/Display/Render/vertex_array_buffer.h"
#include "TinyClan/API/Display/TargetProviders/display_target_provider.h"
#include "TinyClan/API/Display/TargetProviders/display_window_provider.h"
#include "TinyClan/API/Display/TargetProviders/graphic_context_provider.h"
#include "TinyClan/API/Display/TargetProviders/input_device_provider.h"
#include "TinyClan/API/Display/TargetProviders/program_object_provider.h"
#include "TinyClan/API/Display/TargetProviders/render_buffer_provider.h"
#include "TinyClan/API/Display/TargetProviders/shader_object_provider.h"
#include "TinyClan/API/Display/TargetProviders/texture_provider.h"
#include "TinyClan/API/Display/TargetProviders/vertex_array_buffer_provider.h"
#include "TinyClan/API/Display/TargetProviders/primitives_array_provider.h"
#include "TinyClan/API/Display/Window/display_window.h"
#include "TinyClan/API/Display/Window/display_window_description.h"
#include "TinyClan/API/Display/Window/input_code.h"
#include "TinyClan/API/Display/Window/input_device.h"
#include "TinyClan/API/Display/Window/input_event.h"
#include "TinyClan/API/Display/Window/keys.h"
#include "TinyClan/API/VK/volk.h"
#include "TinyClan/API/VK/vk_mem_alloc_config.h"
#include "TinyClan/API/VK/vulkan_target.h"
#include "TinyClan/API/VK/setup_vulkan.h"
#include "TinyClan/API/VK/vulkan_context_description.h"
#include "TinyClan/API/Sound/clan_sound.h"
#include "TinyClan/API/Sound/soundoutput.h"
#include "TinyClan/API/Sound/soundoutput_description.h"
#include "TinyClan/API/Sound/soundformat.h"
#include "TinyClan/API/Sound/SoundProviders/soundprovider.h"
#include "TinyClan/API/Sound/SoundProviders/soundprovider_session.h"
#include "TinyClan/API/Sound/SoundProviders/soundprovider_type_register.h"
#include "TinyClan/API/Sound/soundbuffer.h"
#include "TinyClan/API/Sound/soundbuffer_session.h"
#include "TinyClan/API/Sound/SoundProviders/soundprovider_wave.h"
#include "TinyClan/API/Sound/SoundProviders/soundprovider_raw.h"
#include "TinyClan/API/Sound/SoundProviders/soundfilter_provider.h"

#include "game.h"

class RenderBatchTriangle;
class CMethDoc;
class CGameTarget
{
private:
	CMethDoc *m_pDoc;	// The portal to the outside world

public:
	CGameTarget();
	void Init(CMethDoc *pdoc, clan::Canvas &canvas);
	void InitGame();
	void RedrawScreen();
	void StartGame();
	void MainLoop();
	void PrepareSoundDriver();
	void PlayModule(int id);
	void StopModule();
	void PlaySample(int id, int pos, int rate);
	void UpdateModule(int id);
	void Draw(int dest_xpos, int dest_ypos, int width, int height, int texture_number, int texture_xpos, int texture_ypos, bool draw_white);
	void Draw(const std::string &text, float dest_xpos, float dest_ypos, const clan::Colorf &colour);


	CGame m_Game;		// The Main Game
	JOYSTICK m_Joy1;	// To be written to by the OS
	JOYSTICK m_Joy2;	// To be written to by the OS
	int m_FadeChangeFlag;	// 0 = Palette has not changed

	float m_Lighting;	// -1 = Black. 0 = Normal. 1 = White
	std::string m_ResourceDir;

	std::shared_ptr<RenderBatchTriangle> m_Batcher;

	clan::Texture2D m_OptionsBackdrop;

private:
	struct GameFont
	{
		int glyph = 0;
		clan::Rect texture_rect;
		clan::Sizef size;
		clan::Pointf offset;
		float advance = 0;
	};

	clan::Canvas m_Canvas;	//!< The canvas

	static const int m_NumTextures = 5;
	clan::Texture2D m_Texture[m_NumTextures];

	clan::Texture2D m_Font;

	clan::SoundBuffer m_WAV_blow;
	clan::SoundBuffer m_WAV_bowling;
	clan::SoundBuffer m_WAV_candle;
	clan::SoundBuffer m_WAV_card;
	clan::SoundBuffer m_WAV_car;
	clan::SoundBuffer m_WAV_chicken;
	clan::SoundBuffer m_WAV_cookie;
	clan::SoundBuffer m_WAV_crying;
	clan::SoundBuffer m_WAV_day;
	clan::SoundBuffer m_WAV_die2;
	clan::SoundBuffer m_WAV_duck;
	clan::SoundBuffer m_WAV_feather;
	clan::SoundBuffer m_WAV_finlev1;
	clan::SoundBuffer m_WAV_hurry;
	clan::SoundBuffer m_WAV_marble;
	clan::SoundBuffer m_WAV_mask;
	clan::SoundBuffer m_WAV_moon;
	clan::SoundBuffer m_WAV_oil;
	clan::SoundBuffer m_WAV_pickup1;
	clan::SoundBuffer m_WAV_pstar;
	clan::SoundBuffer m_WAV_redstar;
	clan::SoundBuffer m_WAV_spiningtop;
	clan::SoundBuffer m_WAV_spit;
	clan::SoundBuffer m_WAV_splat;
	clan::SoundBuffer m_WAV_tap;
	clan::SoundBuffer m_WAV_train;
	clan::SoundBuffer m_WAV_tribble;
	clan::SoundBuffer m_WAV_turbo;
	clan::SoundBuffer m_WAV_twinkle;
	clan::SoundBuffer m_WAV_wings;
	clan::SoundBuffer m_WAV_wpotion;
	clan::SoundBuffer m_WAV_xylo;
	clan::SoundBuffer m_MOD_boss;
	clan::SoundBuffer m_MOD_complete;
	clan::SoundBuffer m_MOD_empty;
	clan::SoundBuffer m_MOD_title;
	clan::SoundBuffer m_MOD_tune1;
	clan::SoundBuffer m_MOD_tune2;

	clan::SoundBuffer_Session m_Session;
	bool m_bSessionActive;

	static std::vector<GameFont> m_GameFont;
};

extern CGameTarget *GLOBAL_GameTarget;

extern bool GLOBAL_SoundEnable;

#endif


