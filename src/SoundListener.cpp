#include "SoundListener.h"

#include "Platform.h"
#include "SoundManager.h"
#include "Object.h"

using namespace Dojo;

SoundListener::SoundListener(Object& parent, float zOffset) 
	: Component(parent)
	, mZOffset(zOffset) {

}

void SoundListener::onAttach() {
	Platform::singleton().getSoundManager()._setSoundListener(self);
}

void SoundListener::onDetach() {
	Platform::singleton().getSoundManager()._notifySoundListenerDetached(self);
}

Matrix SoundListener::getTransform() const {
	return glm::translate(getObject().getWorldTransform(), { 0, 0, mZOffset });
}
