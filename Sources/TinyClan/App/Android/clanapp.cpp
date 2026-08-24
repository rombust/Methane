/*
**  ClanLib SDK
**  Copyright (c) 1997-2020 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Mark Page
*/


#include "precomp.h"

#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "API/App/clanapp.h"
#include "API/Core/System/exception.h"
#include "API/Display/System/run_loop.h"
#include "Display/setup_display.h"
#include "Display/Platform/Android/display_message_queue_android.h"
#include "Core/IOData/Platform/Android/asset_extractor_android.h"

#define LOG_TAG "TinyClan"

namespace clan
{
	static ApplicationInstancePrivate *app_instance = nullptr;
	static bool enable_catch_exceptions = false;
	static int timing_timeout = 0;

	static std::vector<std::string> command_line_args;

	ApplicationInstancePrivate::ApplicationInstancePrivate(bool catch_exceptions)
	{
		app_instance = this;
		enable_catch_exceptions = catch_exceptions;
	}

	const std::vector<std::string> &Application::main_args()
	{
		return command_line_args;
	}

	void Application::use_timeout_timing(int timeout)
	{
		timing_timeout = timeout;
	}
}

void android_main(struct android_app *app)
{
	using namespace clan;

	command_line_args = { "SuperMethaneBrothers" };

	if (app_instance == nullptr)
	{
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "TinyClan: No global Application instance!");
		return;
	}

	DisplayMessageQueue_Android::set_pending_app(app);
	extract_android_assets(app);

	while (app->window == nullptr && !app->destroyRequested)
	{
		int events;
		android_poll_source *source;
		if (ALooper_pollOnce(-1, nullptr, &events, reinterpret_cast<void **>(&source)) >= 0)
		{
			if (source != nullptr)
				source->process(app, source);
		}
	}

	if (app->destroyRequested)
		return;

	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Native window ready, starting application");

	int retval = 0;

	auto run_app = [&]()
	{
		std::unique_ptr<Application> app_obj = app_instance->create();
		while (true)
		{
			if (!app_obj->update())
				break;

			if (!RunLoop::process(timing_timeout))
				break;
		}
	};

	if (enable_catch_exceptions)
	{
		try
		{
			run_app();
		}
		catch (Exception &exception)
		{
			__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Exception caught: %s",
			                     exception.get_message_and_stack_trace().c_str());
			retval = -1;
		}
	}
	else
	{
		run_app();
	}

	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "android_main exiting (retval=%d)", retval);
}
