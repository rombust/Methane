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
#include "asset_extractor_android.h"

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include <sys/stat.h>
#include <cstdio>
#include <vector>

namespace clan
{
	namespace
	{
		std::string g_extracted_resource_path;
	}

	std::string extract_android_assets(android_app *app)
	{
		if (!app || !app->activity || !app->activity->internalDataPath)
		{
			__android_log_print(ANDROID_LOG_ERROR, "TinyClan",
				"extract_android_assets: no internalDataPath available");
			return std::string();
		}

		std::string internal_path = app->activity->internalDataPath;
		std::string dest_path = internal_path + "/resources/";

		mkdir(internal_path.c_str(), 0700); // likely already exists; ignore failure
		if (mkdir(dest_path.c_str(), 0700) != 0)
		{
			// Already existing is fine (re-extracting on every launch);
			// anything else means writes below will fail too and get logged.
		}

		AAssetManager *mgr = app->activity->assetManager;
		if (!mgr)
		{
			__android_log_print(ANDROID_LOG_ERROR, "TinyClan", "extract_android_assets: no AAssetManager");
			return std::string();
		}

		// "" = the assets root, which is exactly the top-level resources/
		// folder's contents — see android/app/build.gradle.kts's
		// assets.srcDirs("../../resources").
		AAssetDir *asset_dir = AAssetManager_openDir(mgr, "");
		if (!asset_dir)
		{
			__android_log_print(ANDROID_LOG_ERROR, "TinyClan", "extract_android_assets: AAssetManager_openDir failed");
			return std::string();
		}

		int extracted_count = 0;
		const char *filename;
		std::vector<char> buffer(64 * 1024);

		while ((filename = AAssetDir_getNextFileName(asset_dir)) != nullptr)
		{
			AAsset *asset = AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING);
			if (!asset)
			{
				__android_log_print(ANDROID_LOG_WARN, "TinyClan", "extract_android_assets: failed to open asset '%s'", filename);
				continue;
			}

			std::string out_path = dest_path + filename;
			FILE *out = fopen(out_path.c_str(), "wb");
			if (out)
			{
				int bytes_read;
				while ((bytes_read = AAsset_read(asset, buffer.data(), buffer.size())) > 0)
					fwrite(buffer.data(), 1, static_cast<size_t>(bytes_read), out);
				fclose(out);
				extracted_count++;
			}
			else
			{
				__android_log_print(ANDROID_LOG_WARN, "TinyClan", "extract_android_assets: failed to open '%s' for writing", out_path.c_str());
			}

			AAsset_close(asset);
		}

		AAssetDir_close(asset_dir);

		__android_log_print(ANDROID_LOG_INFO, "TinyClan", "extract_android_assets: extracted %d file(s) to %s",
			extracted_count, dest_path.c_str());

		g_extracted_resource_path = dest_path;
		return dest_path;
	}

	const std::string &get_extracted_android_resource_path()
	{
		return g_extracted_resource_path;
	}
}
