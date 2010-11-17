#ifndef SOUNDBUFFER_H
#define SOUNDBUFFER_H

#include "dojo_config.h"

#include <OpenAL/al.h>

#include <string>

namespace Dojo 
{
	class SoundManager;

	///Classe che gestisce la memoria caricata da un wav.
	/**
	Non dovrebbe essere utilizzato all'esterno di SoundManager.
	*/
	class SoundBuffer
	{
	public:

		///Costruttore
		SoundBuffer( SoundManager* creator, const std::string& soundName );
		///Distruttore
		~SoundBuffer();

		bool load();
		void unload();

		inline const std::string& getName()		{	return name;	}

		///Ottieni il numero delle volte che questo buffer e' stato collegato ad un source
		inline unsigned int getUses()			{	return uses;	}
		///Ottieni la dimensione in memoria di questo buffer.
		inline int getSize()					{	return size;	}
		///Ottieni la frequenza di questo buffer.
		inline int getFrequency()				{	return freq;	}

		inline bool isLoaded()					{	return buffer != AL_NONE;	}

		///-uso interno-
		inline void _resetUses()	{	uses = 0;	}
		///-uso interno-
		inline void _notifyUsed()			{	uses++;	}
		inline void _notifyReleased()		{	uses--;	}

		///-uso interno-
		inline ALuint _getOpenALBuffer()	
		{
			return buffer;
		}

	protected:

		SoundManager* mgr;

		std::string name;
		
		ALsizei		size, freq;

		ALuint buffer;

		unsigned int uses;
		
		void _loadCAFBuffer();
	};
}

#endif
