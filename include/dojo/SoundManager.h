#pragma once

#include "dojo_common_header.h"

#include "Vector.h"

#include "SoundSet.h"
#include "SoundSource.h"

#define NUM_SOURCES_MIN 16
#define NUM_SOURCES_MAX 256

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

namespace Dojo {

	class SoundListener;
	class SoundSource;

	///Dojo's audio system, based on OpenAL
	class SoundManager {
	public:
		typedef std::vector<std::unique_ptr<SoundSource>> SoundList;

		typedef std::function<float(float)> Easing;

		static const Easing LinearEasing;
		static const float m;

		SoundManager();

		~SoundManager();

		///clear() destroys the sound pool - use wisely!
		void clear();

		///Returns a sound source ready to play a new sound
		SoundSource& getSoundSource(SoundSet& set, int i = -1);

		///Returns a sound source ready to play a new sound, with the position already set
		SoundSource& getSoundSource(const Vector& pos, SoundSet& set);

		///Plays the given set without spatial positioning
		SoundSource& playSound(SoundSet& set, float volume = 1.0f);

		///Plays the given set at pos
		SoundSource& playSound(const Vector& pos, SoundSet& set, float volume = 1.0f);

		///Starts a new sound using it as background music
		/**
			\param trackFadeTime the duration of the intro fade-in
			*/
		void playMusic(SoundSet& music, float trackFadeTime = 0, const Easing& fadeEasing = LinearEasing);

		void pauseMusic();

		void resumeMusic();

		///stops the music, with an optional fade-out
		void stopMusic(float stopFadeTime = 0, const Easing& fadeEasing = LinearEasing);

		void setMusicVolume(float volume);

		void setMasterVolume(float volume);

		float getMasterVolume() {
			return masterVolume;
		}

		float getMusicVolume() {
			return musicVolume;
		}

		optional_ref<SoundSource> getMusicTrack() {
			return musicTrack;
		}

		///pauses all the active SoundSources (excluding the background music!)
		void pauseAll();
		///resumes all the active SoundSources (excluding the background music!)
		void resumeAll();

		///stops all the active SoundSources (excluding the background music!)
		void stopAllSounds();

		const SoundList& getActiveSounds() const {
			return busySoundPool;
		}

		///true if the music is fading
		bool isMusicFading() {
			return fadeState != FS_NONE;
		}

		///is the music already playing?
		bool isMusicPlaying() {
			return musicTrack.is_some();
		}

		void update(float dt);

		void _setSoundListener(SoundListener& listener);
		void _notifySoundListenerDetached(SoundListener& listener);

	private:

		enum FadeState {
			FS_NONE,
			FS_FADE_IN,
			FS_FADE_OUT
		};

		ALCcontext* context;
		ALCdevice* device;

		optional_ref<SoundListener> mListener;
		glm::vec4 lastListenerPos;

		//pool di suoni
		SoundList idleSoundPool;
		SoundList busySoundPool;

		std::unique_ptr<SoundSource> fakeSource;

		optional_ref<SoundSource> musicTrack, nextMusicTrack;
		float halfFadeTime;
		float currentFadeTime;
		FadeState fadeState;

		float musicVolume;
		float masterVolume;

		///sets the openAL Listener's world transform
		void _setListenerTransform(const Matrix& worldTransform);
	};
}
