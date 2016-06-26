#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class Touch;
	class InputSystem;
	class InputDevice;
	class Vector;

	class InputSystemListener {
	public:

		virtual ~InputSystemListener();

		optional_ref<InputSystem> getSource() {
			return source;
		}

		virtual void onTouchBegan(const Touch& touch) {
		}

		virtual void onTouchMove(const Touch& touch) {
		}

		virtual void onTouchEnd(const Touch& touch) {
		}

		virtual void onMouseMove(const Vector& curPos, const Vector& oldPos) {
		}

		virtual void onScrollWheel(float scroll) {
		}

		virtual void onShake() {
		}

		virtual void onAcceleration(const Vector& accel, float roll) {
		}

		virtual void onDeviceConnected(InputDevice& j) {
		}

		virtual void onDeviceDisconnected(InputDevice& j) {
		}

		void _notifySource(optional_ref<InputSystem> src) {
			source = src;
		}

	private:

		optional_ref<InputSystem> source;
	};
}
