#include "SoundManager.h"

#include "SoundSource.h"
#include "Platform.h"

using namespace Dojo;

const float SoundManager::m = 100;

const SoundManager::Easing SoundManager::LinearEasing = []( float t ) {
	return t;
};

void SoundManager::vectorToALfloat(const Vector& vector, ALfloat* ALpos) {
	DEBUG_ASSERT(ALpos, "nullptr AL position vector");

	ALpos[0] = vector.x / m;
	ALpos[1] = vector.y / m;
	ALpos[2] = vector.z / m;
}

///////////////////////////////////////

SoundManager::SoundManager() :
	fadeState(FS_NONE),
	musicVolume(1),
	masterVolume(1),
	currentFadeTime(0) {
	alGetError();

	// Initialization
	device = alcOpenDevice(nullptr); // select the "preferred device"

	if (!device) {
		return;    //running without audio :(
	}

	context = alcCreateContext(device, nullptr);

	DEBUG_ASSERT( context, "Cannot create an OpenAL context" );

	alcMakeContextCurrent(context);

	CHECK_AL_ERROR;

	//preload sounds
	static_assert( NUM_SOURCES_MAX >= NUM_SOURCES_MIN, "Min source number cannot be > Max source number" );

	//create at least MIN sources, the rest will be lazy-loaded
	for (int i = 0; i < NUM_SOURCES_MIN; ++i) {
		ALuint src;
		alGenSources(1, &src);

		if (alGetError() == AL_NO_ERROR) {
			idleSoundPool.emplace_back(make_unique<SoundSource>(src));
		}
		else {
			break;
		}
	}

	//ensure at least MIN sources have been built
	DEBUG_ASSERT_INFO(
		idleSoundPool.size() >= NUM_SOURCES_MIN,
		"OpenAL could not preload the minimum sources number", "NUM_SOURCES_MIN = " + utf::to_string(NUM_SOURCES_MIN) );

	//dummy source to manage source shortage
	fakeSource = make_unique<SoundSource>(0);

	setListenerTransform(Matrix(1));

	CHECK_AL_ERROR;
}


void SoundManager::clear() {
	for (auto&& busy : busySoundPool) {
		busy->stop();
		idleSoundPool.emplace_back(std::move(busy));
	}

	busySoundPool.clear();

	musicTrack = {};
	fadeState = FS_NONE;
}


SoundManager::~SoundManager() {
	if (device) {
		alcCloseDevice(device);
	}
}

SoundSource& SoundManager::getSoundSource(SoundSet& set, int i) {

	//try to lazy-create a new source, if allowed
	if (idleSoundPool.empty() && busySoundPool.size() < NUM_SOURCES_MAX) {
		ALuint src;
		alGenSources(1, & src);

		if (alGetError() == AL_NO_ERROR) {
			idleSoundPool.emplace_back(make_unique<SoundSource>(src));
		}
	}

	//is there a source now?
	if (!idleSoundPool.empty()) {
		busySoundPool.emplace_back(std::move(idleSoundPool.back()));
		idleSoundPool.pop_back();

		busySoundPool.back()->_setup(set.getBuffer(i));
		return *busySoundPool.back();
	}

	//failed, return mute source
	return *fakeSource;
}

void SoundManager::playMusic(SoundSet& next, float trackFadeTime /* = 0 */, const Easing& easing) {
	//TODO use easing

	//override music activation if the system sound is in use
	if (Platform::singleton().isSystemSoundInUse()) {
		return;
	}

	nextMusicTrack = getSoundSource(next);

	halfFadeTime = trackFadeTime * 0.5f;
	currentFadeTime = 0;

	fadeState = FS_FADE_OUT;
}

void SoundManager::setMasterVolume(float volume) {
	DEBUG_ASSERT( volume >= 0, "Volumes cannot be negative" );

	if (std::abs(masterVolume - volume) > 0.01f || volume == 0) { //avoid doing this too often
		masterVolume = volume;

		//update volumes (masterVolume is used inside setVolume!)
		if (nextMusicTrack.is_none() && musicTrack.is_some()) {
			musicTrack.unwrap().setVolume(musicVolume);
		}

		//update all the existing sounds
		for (auto&& s : busySoundPool) {
			s->setVolume(s->getVolume());
		}
	}
}

void SoundManager::update(float dt) {
	for (size_t i = 0; i < busySoundPool.size(); ++i) {
		auto& current = busySoundPool[i];
		current->_update(dt);

		if (current->_isWaitingForDelete()) {
			current->_reset();

			idleSoundPool.emplace_back(std::move(current));
			busySoundPool.erase(busySoundPool.begin() + i);

			--i;
		}
	}

	//fai il fade
	if (fadeState == FS_FADE_OUT) { //abbassa il volume della track corrente
		if (musicTrack.is_some() && currentFadeTime < halfFadeTime) {
			musicTrack.unwrap().setVolume(musicVolume * (1.f - currentFadeTime / halfFadeTime));
		}
		else { //scambia le tracks e fai partire la prossima
			if (auto track = musicTrack.cast()) {
				track.get().stop();
			}

			musicTrack = nextMusicTrack;

			//parte il fade in
			if (auto track = musicTrack.cast()) {
				nextMusicTrack = {};

				track.get().play(0);
				track.get().setAutoRemove(false);

				track.get().setLooping(true);

				fadeState = FS_FADE_IN;
				currentFadeTime = 0;
			}
			else {
				fadeState = FS_NONE;
			}
		}

		currentFadeTime += dt;
	}
	else if (fadeState == FS_FADE_IN) { //alza il volume della track successiva
		if (currentFadeTime < halfFadeTime) {
			musicTrack.unwrap().setVolume(musicVolume * (currentFadeTime / halfFadeTime));
		}

		else { //finisci
			currentFadeTime = 0;

			//force volume at max
			musicTrack.unwrap().setVolume(musicVolume);

			fadeState = FS_NONE;
		}

		currentFadeTime += dt;
	}
}

void SoundManager::resumeMusic() {
	//resume music, but only if the user didn't enable itunes meanwhile!
	if (musicTrack.is_some() && !Platform::singleton().isSystemSoundInUse()) {
		musicTrack.unwrap().play();
	}
}

void SoundManager::setListenerTransform(const Matrix& worldTransform) {
	glm::vec4 pos(0.f, 0.f, 0.f, 1.f), up(0.f, 1.f, 0.f, 0.f), forward(0.f, 0.f, -1.f, 0.f);

	pos = worldTransform * pos;
	forward = worldTransform * forward;
	up = worldTransform * up;

	ALfloat orientation[6] = {
		forward.x, forward.y, forward.z,
		up.x, up.y, up.z
	};

	alListenerfv(AL_POSITION, glm::value_ptr(pos));
	alListenerfv(AL_VELOCITY, glm::value_ptr(pos - lastListenerPos));
	alListenerfv(AL_ORIENTATION, orientation);

	lastListenerPos = pos;
}

SoundSource& SoundManager::getSoundSource(const Vector& pos, SoundSet& set) {
	auto& s = getSoundSource(set);
	s.setPosition(pos);

	return s;
}

SoundSource& SoundManager::playSound(SoundSet& set, float volume /*= 1.0f*/) {
	auto& s = getSoundSource(set);
	s.play(volume);
	return s;
}

SoundSource& SoundManager::playSound(const Vector& pos, SoundSet& set, float volume /*= 1.0f */) {
	auto& s = getSoundSource(pos, set);
	s.play(volume);
	return s;
}

void SoundManager::pauseMusic() {
	DEBUG_ASSERT(isMusicPlaying(), "pauseMusic: music is not playing");

	musicTrack.unwrap().pause();
}

void SoundManager::stopMusic(float stopFadeTime /*= 0*/, const Easing& fadeEasing /*= LinearEasing */) {
	//TODO use easing
	DEBUG_MESSAGE("Music fading out in " + utf::to_string(stopFadeTime) + " s");

	fadeState = FS_FADE_OUT;
	nextMusicTrack = {};

	halfFadeTime = stopFadeTime;
	currentFadeTime = 0;
}

void SoundManager::setMusicVolume(float volume) {
	DEBUG_ASSERT(volume >= 0, "setMusicVolume: volume is negative");

	musicVolume = volume;

	if (nextMusicTrack.is_none() && musicTrack.is_some()) {
		musicTrack.unwrap().setVolume(musicVolume);
	}
}

void SoundManager::pauseAll() {
	for (auto&& s : busySoundPool) {
		s->pause();
	}
}

void SoundManager::resumeAll() {
	for (auto&& s : busySoundPool) {
		s->play();
	}
}

void SoundManager::stopAllSounds() {
	for (auto&& s : busySoundPool) {
		s->stop();
	}
}
