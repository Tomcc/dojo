#ifndef IOSPlatform_h__
#define IOSPlatform_h__

#include "dojo_common_header.h"

#include "ApplePlatform.h"
#include "Utils.h"

#ifdef __OBJC__
	#import <UIKit/UIKit.h>

	#import <OpenGLES/EAGL.h>
	#import <OpenGLES/EAGLDrawable.h>

	#import <AVFoundation/AVAudioPlayer.h>

	#import "Application.h"
#endif

namespace Dojo
{
	class Texture;
	
	class IOSPlatform : public ApplePlatform
	{
	public:
		
		class GameCenterListener
		{
		public:
			GameCenterListener()
			{
				
			}
			
			virtual ~GameCenterListener()
			{
				
			}
			
			virtual void onLogin( bool result, const Dojo::String& playerName )=0;			
			virtual void onPostCompletion( bool error )=0;			
			virtual void onHighScoreGet( const Dojo::String& leaderboard, int score, bool error )=0;			
			virtual void onAchievementsGet( const std::vector< Dojo::String >& codes, bool error )=0;
			
		protected:
		};
		
		IOSPlatform( const Table& config );
		
		virtual ~IOSPlatform();
				
		virtual void initialise( Game* game );
		virtual void loop();
		virtual void shutdown();
		
		virtual void prepareThreadContext();
		
		virtual void acquireContext();
		virtual void present();
        
        ///sets the app fullscreen or windowed, has no effect on iOS
        virtual void setFullscreen( bool )
        {
            DEBUG_ASSERT_MSG( false, "Error: iOS apps can only run fullscreen" );
        }
				
        ///iOS specific - tells if the system sound is currently hold by another application
		virtual bool isSystemSoundInUse();
		
		virtual const String& getRootPath();
        virtual const String& getResourcesPath();
		virtual const String& getAppDataPath();
				
		virtual void openWebPage( const String& site );
		
		virtual bool isNPOTEnabled()
		{
			return mNPOTEnabled; //TODO: check if the model supports the extension	
		}
		
		virtual bool isSmallScreen();
		
#ifdef GAME_CENTER_ENABLED
		///IOS SPECIFIC - checks game center availability and logins into it.
		/** 
		Returns true if no error occurred */
		void loginToGameCenter( GameCenterListener* listener );
		
		void postScore( unsigned int score, const Dojo::String& leaderboard, GameCenterListener* listener );
		void requestScore( const Dojo::String& leaderboard, GameCenterListener* listener );
		
		void postAchievement( const Dojo::String& code, GameCenterListener* listener );
		void requestAchievements( GameCenterListener* listener);
		
		void showDefaultLeaderboard();
#endif
		
		void enableScreenSaver( bool s );
		
		///IOS SPECIFIC - copies the file from which the passed texture was created to the camera roll
		/**TODO - actually read the texture from VRAM, useful for auto screenshots*/
		void copyImageIntoCameraRoll( const Dojo::String& path );
				
		///IOS SPECIFIC - plays and loops an audio file using hardware system decompression
		void playMp3File( const Dojo::String& relPath, bool loop );		
		
		///IOS SPECIFIC - stops the currently playing mp3
		void stopMp3File();		
		
		///IOS SPECIFIC - sets mp3 volume (0.0-1.0)
		void setMp3FileVolume( float gain );		
		
		/*
		virtual void sendEmail( const Email& e );
		void onEmailSent( void* senderController, bool success );
		*/
		 
#ifdef __OBJC__
		void _initialiseImpl( Application* application );
		
		inline Application* getNativeApplication()
		{			
			return app;
		}
#endif
		
	protected:
        
        String mRootPath, mResourcesPath, mAppDataPath;
        
		/*
		typedef std::map< void*, Email::Listener* > SenderEmailListenerMap;
		SenderEmailListenerMap senderEmailListenerMap;
		*/
		
        bool mNPOTEnabled;
        
		bool _checkGameCenterAvailability();

		GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;
		
#ifdef __OBJC__		
		EAGLContext* context;
		
		Application* app;		
	
		AVAudioPlayer* player;
#else
        
        void* a, *b, *c; //dummy pointers to keep size consistent
        
#endif
		
	};
}

#endif