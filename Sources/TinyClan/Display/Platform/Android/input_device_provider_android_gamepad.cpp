/*
**  ClanLib SDK
**  Copyright (c) 1997-2026 The ClanLib Team
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
#include "input_device_provider_android_gamepad.h"

#ifdef __ANDROID__

#include "API/Display/Window/input_event.h"
#include "API/Display/Window/input_code.h"

#include <android/keycodes.h>
#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>
#include <jni.h>
#include <cmath>
#include <algorithm>

namespace clan
{
	namespace
	{
		class ScopedJniEnv
		{
		public:
			explicit ScopedJniEnv(JavaVM *java_vm) : vm(java_vm)
			{
				if (!vm)
					return;

				if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) == JNI_OK)
					return;

				if (vm->AttachCurrentThread(&env, nullptr) == JNI_OK)
					attached = true;
				else
					env = nullptr;
			}

			~ScopedJniEnv()
			{
				if (attached && vm)
					vm->DetachCurrentThread();
			}

			ScopedJniEnv(const ScopedJniEnv &) = delete;
			ScopedJniEnv &operator=(const ScopedJniEnv &) = delete;

			JNIEnv *get() const { return env; }

		private:
			JavaVM *vm = nullptr;
			JNIEnv *env = nullptr;
			bool attached = false;
		};

		bool clear_exception(JNIEnv *env)
		{
			if (!env->ExceptionCheck())
				return false;

			env->ExceptionClear();
			return true;
		}
	}

	std::vector<AndroidGamepadInfo> enumerate_android_gamepads(android_app *app)
	{
		std::vector<AndroidGamepadInfo> found;

		if (!app || !app->activity)
			return found;

		ScopedJniEnv jni(app->activity->vm);
		JNIEnv *env = jni.get();
		if (!env)
		{
			__android_log_print(ANDROID_LOG_WARN, "TinyClan",
				"Could not attach to the JVM; game controllers will only be found once one sends an event");
			return found;
		}

		jclass input_device_class = env->FindClass("android/view/InputDevice");
		if (!input_device_class || clear_exception(env))
		{
			__android_log_print(ANDROID_LOG_WARN, "TinyClan",
				"android.view.InputDevice not reachable over JNI");
			return found;
		}

		jmethodID get_device_ids = env->GetStaticMethodID(input_device_class, "getDeviceIds", "()[I");
		jmethodID get_device = env->GetStaticMethodID(input_device_class, "getDevice",
			"(I)Landroid/view/InputDevice;");
		jmethodID get_sources = env->GetMethodID(input_device_class, "getSources", "()I");
		jmethodID get_name = env->GetMethodID(input_device_class, "getName", "()Ljava/lang/String;");
		jmethodID get_controller_number = env->GetMethodID(input_device_class, "getControllerNumber", "()I");

		if (!get_device_ids || !get_device || !get_sources || !get_name || !get_controller_number ||
			clear_exception(env))
		{
			env->DeleteLocalRef(input_device_class);
			return found;
		}

		jintArray ids = static_cast<jintArray>(
			env->CallStaticObjectMethod(input_device_class, get_device_ids));

		if (!ids || clear_exception(env))
		{
			env->DeleteLocalRef(input_device_class);
			return found;
		}

		jsize count = env->GetArrayLength(ids);
		jint *elements = env->GetIntArrayElements(ids, nullptr);

		__android_log_print(ANDROID_LOG_INFO, "TinyClan",
			"Android reports %d input device(s); checking which are controllers", count);

		for (jsize i = 0; elements && i < count; i++)
		{
			jobject device = env->CallStaticObjectMethod(input_device_class, get_device, elements[i]);
			if (!device || clear_exception(env))
				continue;

			jint sources = env->CallIntMethod(device, get_sources);
			if (clear_exception(env))
			{
				env->DeleteLocalRef(device);
				continue;
			}

			jint controller_number = env->CallIntMethod(device, get_controller_number);
			if (clear_exception(env))
				controller_number = 0;

			if (android_source_is_gamepad(sources) && (controller_number > 0))
			{
				AndroidGamepadInfo info;
				info.device_id = elements[i];
				info.name = "Gamepad";

				jstring java_name = static_cast<jstring>(env->CallObjectMethod(device, get_name));
				if (java_name && !clear_exception(env))
				{
					const char *utf = env->GetStringUTFChars(java_name, nullptr);
					if (utf)
					{
						info.name = utf;
						env->ReleaseStringUTFChars(java_name, utf);
					}
					env->DeleteLocalRef(java_name);
				}

				found.push_back(info);
			}

			env->DeleteLocalRef(device);
		}

		if (elements)
			env->ReleaseIntArrayElements(ids, elements, JNI_ABORT);

		env->DeleteLocalRef(ids);
		env->DeleteLocalRef(input_device_class);

		__android_log_print(ANDROID_LOG_INFO, "TinyClan",
			"%zu game controller(s) found", found.size());

		return found;
	}

	std::string get_android_input_device_name(android_app *app, int32_t device_id)
	{
		for (const AndroidGamepadInfo &info : enumerate_android_gamepads(app))
		{
			if (info.device_id == device_id)
				return info.name;
		}

		return std::string();
	}

	void enable_android_gamepad_axes()
	{
		GameActivityPointerAxes_enableAxis(AMOTION_EVENT_AXIS_HAT_X);
		GameActivityPointerAxes_enableAxis(AMOTION_EVENT_AXIS_HAT_Y);
		GameActivityPointerAxes_enableAxis(AMOTION_EVENT_AXIS_Z);
		GameActivityPointerAxes_enableAxis(AMOTION_EVENT_AXIS_RZ);
	}

	std::string InputDeviceProvider_AndroidGamepad::get_key_name(int id) const
	{
		switch (id)
		{
		case 0: return "A";
		case 1: return "B";
		case 2: return "X";
		case 3: return "Y";
		default: return std::string();
		}
	}

	bool InputDeviceProvider_AndroidGamepad::get_keycode(int keycode) const
	{
		if (keycode < 0 || keycode >= button_count)
			return false;

		return buttons[keycode];
	}

	float InputDeviceProvider_AndroidGamepad::get_axis(int index) const
	{
		if (index == joystick_x) return axis_x;
		if (index == joystick_y) return axis_y;
		return 0.0f;
	}

	std::vector<int> InputDeviceProvider_AndroidGamepad::get_axis_ids() const
	{
		return { joystick_x, joystick_y };
	}

	bool InputDeviceProvider_AndroidGamepad::received_key_event(InputDevice &device,
	                                                             int32_t android_keycode, bool down)
	{
		int button = -1;

		switch (android_keycode)
		{
		case AKEYCODE_BUTTON_A:
		case AKEYCODE_BUTTON_L1:
		case AKEYCODE_BUTTON_Z:
			button = 0;
			break;

		case AKEYCODE_BUTTON_B:
		case AKEYCODE_BUTTON_R1:
		case AKEYCODE_BUTTON_C:
			button = 1;
			break;

		case AKEYCODE_BUTTON_X:
		case AKEYCODE_BUTTON_L2:
			button = 2;
			break;

		case AKEYCODE_BUTTON_Y:
		case AKEYCODE_BUTTON_R2:
			button = 3;
			break;

		case AKEYCODE_DPAD_LEFT:
			dpad_left = down;
			refresh_axes(device);
			return true;

		case AKEYCODE_DPAD_RIGHT:
			dpad_right = down;
			refresh_axes(device);
			return true;

		case AKEYCODE_DPAD_UP:
			dpad_up = down;
			refresh_axes(device);
			return true;

		case AKEYCODE_DPAD_DOWN:
			dpad_down = down;
			refresh_axes(device);
			return true;

		default:
			if (android_keycode >= AKEYCODE_BUTTON_1 && android_keycode <= AKEYCODE_BUTTON_16)
			{
				button = (android_keycode - AKEYCODE_BUTTON_1) % button_count;
				break;
			}

			return false;
		}

		if (buttons[button] == down)
			return true;

		buttons[button] = down;
		emit_button_event(device, button, down);
		return true;
	}

	void InputDeviceProvider_AndroidGamepad::received_motion_event(InputDevice &device,
	                                                                GameActivityMotionEvent &event)
	{
		if (event.pointerCount == 0)
			return;

		GameActivityPointerAxes &pointer = event.pointers[0];

		stick_x = GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_X);
		stick_y = GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_Y);

		hat_x = GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_HAT_X);
		hat_y = GameActivityPointerAxes_getAxisValue(&pointer, AMOTION_EVENT_AXIS_HAT_Y);

		refresh_axes(device);
	}

	void InputDeviceProvider_AndroidGamepad::refresh_axes(InputDevice &device)
	{
		const float previous_x = axis_x;
		const float previous_y = axis_y;

		float new_x = 0.0f;
		float new_y = 0.0f;

		const float magnitude = std::sqrt(stick_x * stick_x + stick_y * stick_y);
		stick_engaged = stick_engaged
			? (magnitude > stick_release_threshold)
			: (magnitude > stick_engage_threshold);

		if (stick_engaged)
		{
			constexpr float pi = 3.14159265358979323846f;
			float degrees = std::atan2(stick_y, stick_x) * 180.0f / pi;
			if (degrees < 0.0f)
				degrees += 360.0f;

			auto within = [degrees](float centre)
			{
				float difference = std::fabs(degrees - centre);
				if (difference > 180.0f)
					difference = 360.0f - difference;
				return difference <= cardinal_half_angle;
			};

			if (within(0.0f))         { new_x =  1.0f; new_y =  0.0f; }
			else if (within(90.0f))   { new_x =  0.0f; new_y =  1.0f; }
			else if (within(180.0f))  { new_x = -1.0f; new_y =  0.0f; }
			else if (within(270.0f))  { new_x =  0.0f; new_y = -1.0f; }
			else
			{
				// Outside every cardinal sector, so it is one of the four
				// diagonals; which one follows from the signs.
				new_x = (stick_x > 0.0f) ? 1.0f : -1.0f;
				new_y = (stick_y > 0.0f) ? 1.0f : -1.0f;
			}
		}

		if (std::fabs(hat_x) > 0.5f) new_x = (hat_x > 0.0f) ? 1.0f : -1.0f;
		if (std::fabs(hat_y) > 0.5f) new_y = (hat_y > 0.0f) ? 1.0f : -1.0f;

		if (dpad_left) new_x = -1.0f;
		if (dpad_right) new_x = 1.0f;
		if (dpad_up) new_y = -1.0f;
		if (dpad_down) new_y = 1.0f;

		axis_x = std::clamp(new_x, -1.0f, 1.0f);
		axis_y = std::clamp(new_y, -1.0f, 1.0f);

		if (axis_x != previous_x)
		{
			InputEvent event;
			event.type = InputEvent::axis_moved;
			event.id = static_cast<InputCode>(joystick_x);
			event.id_offset = joystick_x;
			event.axis_pos = axis_x;
			event.device = device;
			device.sig_axis_move()(event);
		}

		if (axis_y != previous_y)
		{
			InputEvent event;
			event.type = InputEvent::axis_moved;
			event.id = static_cast<InputCode>(joystick_y);
			event.id_offset = joystick_y;
			event.axis_pos = axis_y;
			event.device = device;
			device.sig_axis_move()(event);
		}
	}

	void InputDeviceProvider_AndroidGamepad::emit_button_event(InputDevice &device, int button, bool down)
	{
		InputEvent event;
		event.type = down ? InputEvent::pressed : InputEvent::released;
		event.id = static_cast<InputCode>(button);
		event.id_offset = button;
		event.device = device;

		if (down)
			device.sig_key_down()(event);
		else
			device.sig_key_up()(event);
	}
}

#endif
