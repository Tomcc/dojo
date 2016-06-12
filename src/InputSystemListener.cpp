#include "InputSystemListener.h"
#include "InputSystem.h"

using namespace Dojo;

InputSystemListener::~InputSystemListener() {
	if (auto s = source.to_ref()) {
		s.get().removeListener(self);
	}
}
