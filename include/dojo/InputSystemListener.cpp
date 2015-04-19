#include "stdafx.h"

#include "InputSystemListener.h"
#include "InputSystem.h"

using namespace Dojo;


InputSystemListener::InputSystemListener() :
source(nullptr) {

}

InputSystemListener::~InputSystemListener() {
	if (source)
		source->removeListener(*this);
}
