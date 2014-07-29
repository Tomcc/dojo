#include "stdafx.h"

#include "InputSystemListener.h"
#include "InputSystem.h"

using namespace Dojo;


InputSystemListener::InputSystemListener() :
source(NULL) {

}

InputSystemListener::~InputSystemListener() {
	if (source)
		source->removeListener(this);
}
