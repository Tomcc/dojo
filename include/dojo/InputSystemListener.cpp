#include "InputSystemListener.h"
#include "InputSystem.h"

using namespace Dojo;

InputSystemListener::~InputSystemListener() {
	if (auto s = source.cast()) {
		s.get().removeListener(self);
	}
}
