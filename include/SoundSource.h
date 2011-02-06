#ifndef SoundSource_h__
#define SoundSource_h__

#include "dojo_common_header.h"

#include "Vector.h"

#include "SoundBuffer.h"
#include "BaseObject.h"

namespace Dojo
{
		class Buffer;
		class SoundManager;

		///Mattone base dell'audio di eVolve. SoundSource e' un suono vero e proprio.
		/**
		Quando si ha bisogno di far partire un suono, basta solamente creare un nuovo SoundSource, configurarlo
		e usare play().
		SoundManager implementa un sistema automatico di gestione dei buffer e delle sources, e anche 
		un Garbage Collector che fa pulizia dei suoni non piu' utilizzati.
		*/
		class SoundSource : public BaseObject
		{				
		public:

			enum SoundState 			
			{
				SS_INITIALISING, ///< il suono e' stato appena creato ma non e' mai stato usato play().
				SS_PLAYING,   ///< il suono e' in esecuzione.
				SS_PAUSED,   ///< play() e' stato usato ma il suono e' stato messo in pausa.
				SS_FINISHED  ///< il suono e' terminato e sta aspettando di essere eliminato.
			};
		
			///Costruttore privato - usare SoundManager::getSound!
			SoundSource( SoundManager* manager );

			///distruttore
			virtual ~SoundSource();

			///-uso interno- carica le resources del suono
			void _loadResources();

			///imposta la posizione del suono; non ha effetto se si usa attach.		
			void setPosition(const Vector& newPos)			
			{	
				pos = newPos;			
				positionChanged = true;
			}

			///imposta il volume del suono, da 0 a infinito
			void setVolume( float v );
			
			///setta se il suono si ripete all'infinito o no.;			
			/**
			\remark il suono non viene eliminato se e' settato a looping. E' consigliabile
			tenere un pointer all'istanza.
			*/
			inline void setLooping(bool l)
			{
				looping = l;
				if( source ) alSourcei (source, AL_LOOPING, looping);
			}
			///Imposta l'altezza del suono.
			inline void setPitch( float p)			
			{		
				pitch = p;
				if( source ) alSourcef (source, AL_PITCH,  pitch);			
			}
			///-uso avanzato-
			/**
			Imposta se il buffer che utilizza questo suono deve essere eliminato
			insieme al SoundSource stesso.
			\remark
			Utile per suoni che si sentono una volta sola, come musiche looped o pezzi di dialoghi.
			*/
			inline void setFlushMemoryWhenRemoved(bool f)	{		flush = f;		}
			///Imposta se quando il suono non e' usato il garbage collector lo elimina automaticamente.
			inline void setAutoRemove(bool a)				{	autoRemove = a;		}	

			///Fa sentire il suono nel gioco.
			void play( float volume = 1.0f );
			///Mette in pausa il suono.
			void pause();
			///Stoppa il suono e lo prepara per il cancellamento.
			inline void stop()
			{
				state = SS_FINISHED;
				alSourceStop(source);
			}
			//resetta il suono all'inizio e lo mette di nuovo in pausa.
			void rewind();

			///Restituisce lo stato della riproduzione del suono.
			inline SoundState getState()		{	return state;	}
			inline bool isPlaying()				{	return state == SS_PLAYING;	}

			inline ALuint getSource()			{	return source;	}

			///Restituisce il volume a cui il suono sta suonando
			float getVolume();	
			
			///Metodo che dice se il suono e' valido, cioe' play() avra' un effetto
			virtual bool isValid()
			{
				return true;
			}

			///-uso interno-
			/**
			Dice se il suono sta aspettando di essere eliminato.
			\remark Non usare un suono in questo stato!
			*/
			inline bool _isWaitingForDelete()
			{
				return (state == SS_FINISHED);
			}

			inline bool _isPaused()
			{
				return (state == SS_PAUSED);
			}

			///metodo tipico di update per frame.
			void _update();

			///metodo per assegnare un buffer
			void _setup( SoundBuffer* b )
			{
				DEBUG_ASSERT( b );
				
				buffer = b;
			}

			///metodo interno per riportare il SoundSource allo stato iniziale
			void _reset();

		protected:
			
			SoundManager* mgr;
			
			Vector pos;
			bool positionChanged;

			//members			
			SoundBuffer* buffer;
			ALuint source;
			ALfloat position[3];
			ALint playState;
			
			SoundState state;

			//params
			bool flush, looping, autoRemove;	
			float volume, pitch;
		};
}

#endif
