#include "IOSPlatform.h"

#include "TargetConditionals.h"

#import <Foundation/NSString.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>	
#import <UIKit/UIKit.h>
#import <MessageUI/MFMailComposeViewController.h>
#import <UIKit/UIScreen.h>

#include "Utils.h"
#include "dojomath.h"

#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "FontSystem.h"
#include "Game.h"
#include "Table.h"
#include "Texture.h"
#include "BackgroundQueue.h"

#include "AppDelegate.h"
#include "Application.h"

using namespace Dojo;

IOSPlatform::IOSPlatform( const Table& config ) :
ApplePlatform( config ),
app( NULL ),
player( NULL )
{
    //call the cocoa wrapper classes to keep them in the library
    [AppDelegate _keepMeInLibraryFile];
    [Application _keepMeInLibraryFile];
    
    //store the system directories
    mRootPath = [[NSBundle mainBundle] executablePath];
    
    NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	mAppDataPath = [nspaths objectAtIndex:0];
    
    mResourcesPath = [[NSBundle mainBundle] resourcePath ];
    
    //screen and window size are the same, we don't support running in subviews (todo?)
	windowWidth = screenWidth = [[UIScreen mainScreen] bounds].size.width * [[UIScreen mainScreen] scale];
    windowHeight = screenHeight = [[UIScreen mainScreen] bounds].size.height * [[UIScreen mainScreen] scale];
    
    screenOrientation = DO_PORTRAIT; //screen is in portrait mode by default
}

IOSPlatform::~IOSPlatform()
{
	if( player )
		[player release];
}

void IOSPlatform::initialize( Game* newGame )
{
    //do not initialize twice
    game = newGame;
	DEBUG_ASSERT( game != nullptr, "A non-null Game implementation must be provided to initialize()" );
        
    //swap reported screen and window dimensions if the game requires a different ratio
    if( game->getNativeOrientation() == DO_LANDSCAPE_LEFT || game->getNativeOrientation() == DO_LANDSCAPE_RIGHT )
    {
        std::swap( screenWidth, screenHeight );
        std::swap( windowWidth, windowHeight );
    }
}

void IOSPlatform::_initializeImpl(Application *application)
{
    app = application;

    DEBUG_ASSERT( app, "null Application" );
    
    uint width, height;
		
//RENDER
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	
    if (!context || ![EAGLContext setCurrentContext:context] )
		return;
	
//on IOS the default target is always a separate renderbuffer
	
	CAEAGLLayer* layer = (CAEAGLLayer*)app.layer;	
	GLint w, h;
	
	glGenFramebuffers(1, &defaultFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
	
//color render buffer
	glGenRenderbuffers(1, &colorRenderbuffer);	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);		
	
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];	
	
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
	
	width = w;
	height = h;
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
	
//depth render buffer
	glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, 
                             GL_DEPTH_COMPONENT16_OES, 
                             w, h );
	
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER, depthRenderbuffer );
    
    
	//some local configs
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    const char* extensions = (char*) glGetString(GL_EXTENSIONS);
    mNPOTEnabled = strstr(extensions, "GL_APPLE_texture_2D_limited_npot") != 0;
    
    //override the config or load it from file
    Table userConfig;
    load( &userConfig, getAppDataPath() + "/config.ds" );
    
    config.inherit( &userConfig ); //merge the table loaded from file and override with hardcoded directives
    
    //create the background task queue
    int userThreadOverride = config.getNumber( "threads", -1 );
    mBackgroundQueue = new BackgroundQueue( userThreadOverride );
	
	render = new Render( width, height, screenOrientation );
	
//SOUND MANAGER

	AudioSessionInitialize ( NULL, NULL, 
							NULL, // may want interruption callback here
							NULL );
	
	//if using mp3 playback, it is needed to exclude other applications' sounds
#ifdef HARDWARE_SOUND
	UInt32 sessionCategory = kAudioSessionCategory_SoloAmbientSound;    // 1
#else
	uint sessionCategory = kAudioSessionCategory_AmbientSound;
#endif
	AudioSessionSetProperty (kAudioSessionProperty_AudioCategory, sizeof (sessionCategory), &sessionCategory);
	AudioSessionSetActive (true);
	
	sound = new SoundManager();
	input = new InputSystem();
	fonts = new FontSystem();	
	
	_createApplicationDirectory();
		
	game->begin();
	
	running = true;
}
	
void IOSPlatform::shutdown()
{
	game->end();
	
	if( render )
	{
		delete render;
		
		// Tear down GL
		if (defaultFramebuffer)
		{
			glDeleteFramebuffers(1, &defaultFramebuffer);
			defaultFramebuffer = 0;
		}
		
		if (colorRenderbuffer)
		{
			glDeleteRenderbuffers(1, &colorRenderbuffer);
			colorRenderbuffer = 0;
		}
		
		if( depthRenderbuffer )
		{
			glDeleteRenderbuffersOES(1, &depthRenderbuffer);
			depthRenderbuffer = 0;
		}
		
		// Tear down context
		if ([EAGLContext currentContext] == context)
			[EAGLContext setCurrentContext:nil];
		
		[context release];	
	}
}

void IOSPlatform::acquireContext()
{
	[EAGLContext setCurrentContext:context];
	
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
}

void IOSPlatform::prepareThreadContext()
{	
	DEBUG_ASSERT( context, "There is no context defined" );
		
	EAGLContext* clone = [[EAGLContext alloc] initWithAPI:[context API] sharegroup:[context sharegroup]];
		
	bool err = [EAGLContext setCurrentContext:clone];
	DEBUG_ASSERT( err, "Error: cannot share the OpenGL Context" );
	
	[[NSAutoreleasePool alloc] init];
}

void IOSPlatform::present()
{
	realFrameTime = frameTimer.getElapsedTime();
    
	[EAGLContext setCurrentContext:context];
	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

void IOSPlatform::step( float dt )
{
    //do nothing if the app isn't still initialized
    if( running )
       ApplePlatform::step( dt );
}

void IOSPlatform::loop()
{
    //launch the iphone app
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    //TODO figure out how to bring argc and argv here
    UIApplicationMain( 0, nullptr, nil, nil);
    [pool release];
}

bool IOSPlatform::isSystemSoundInUse()
{
	UInt32 otherAudioIsPlaying;
	UInt32 size = sizeof(otherAudioIsPlaying);
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &size, &otherAudioIsPlaying);
	
	return otherAudioIsPlaying;
}

const String& IOSPlatform::getRootPath()
{
    return mRootPath;
	
}

const String& IOSPlatform::getResourcesPath()
{
    return mResourcesPath;
}

const String& IOSPlatform::getAppDataPath()
{
    return mAppDataPath;    
}

void IOSPlatform::openWebPage( const String& site )
{
	NSURL* url = [NSURL URLWithString: site.toNSString() ];
	
	[[UIApplication sharedApplication] openURL:url];
}


       void IOSPlatform::enableScreenSaver( bool s )
{
	[[UIApplication sharedApplication] setIdleTimerDisabled: !s ];
}

void IOSPlatform::copyImageIntoCameraRoll( const Dojo::String& path )
{
	DEBUG_ASSERT( path.size(), "The camera roll image path cannot be empty" );
	
	//create an UIImage
	UIImage* img = [UIImage imageWithContentsOfFile: path.toNSString() ];
	
	UIImageWriteToSavedPhotosAlbum(img, nil, nil, nil );
}

void IOSPlatform::playMp3File( const Dojo::String& relPath, bool loop )
{
	NSString* path = [[[NSBundle mainBundle] bundlePath] stringByAppendingString: ( "/" + relPath ).toNSString() ];
	
	NSURL* url = [NSURL fileURLWithPath: path ];
	
	if( player )
		[player stop];
	else if( !player )
		player = [AVAudioPlayer alloc];
	
	[player initWithContentsOfURL: url error:NULL ];
	
	//loop and play
	if( loop )
		player.numberOfLoops = -1;
	
	[player play];
}

void IOSPlatform::stopMp3File()
{
	DEBUG_ASSERT( player, "No iOS hardware player created" );
	DEBUG_ASSERT( player.playing, "The iOS hardware player is already in use" );
	
	[player stop];
}

void IOSPlatform::setMp3FileVolume( float volume )
{
	DEBUG_ASSERT( volume >= 0.0, "The iOS HW mp3 volume cannot be negative" );
	DEBUG_ASSERT( volume <= 1.0, "The iOS HW mp3 volume cannot be " );
	
	if( !player )
		player = [AVAudioPlayer alloc];
	
	player.volume = volume;
}

//------------------------------------------------------------------------------------
// GAME CENTER SUPPORT
//------------------------------------------------------------------------------------

#ifdef GAME_CENTER_ENABLED

#import <GameKit/GKLocalPlayer.h>
#import <GameKit/GKScore.h>
#import <GameKit/GKLeaderboard.h>
#import <GameKit/GKAchievement.h>
#import <GameKit/GKLeaderboardViewController.h>

bool IOSPlatform::_checkGameCenterAvailability()
{
	// Check for presence of GKLocalPlayer class.
    BOOL localPlayerClassAvailable = (NSClassFromString(@"GKLocalPlayer")) != nil;
	
    // The device must be running iOS 4.1 or later.
    NSString *reqSysVer = @"4.1";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    BOOL osVersionSupported = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
	
    return (localPlayerClassAvailable && osVersionSupported);
}

void IOSPlatform::loginToGameCenter( GameCenterListener* listener )
{
	DEBUG_ASSERT( listener, "The game center listener must not be null" );
	
	if( !_checkGameCenterAvailability() )
	{
		listener->onLogin( false, "" );
	}
	else
	{		
		GKLocalPlayer *localPlayer = [GKLocalPlayer localPlayer];
	    [localPlayer authenticateWithCompletionHandler:^(NSError *error) 
		{
			Dojo::String pname;
			
			if( error == nil && [[GKLocalPlayer localPlayer] alias] != nil )
				pname = [[GKLocalPlayer localPlayer] alias];
			
			else if( error )
				NSLog( @"Error %@ %@", error, [error userInfo]  );
			
			listener->onLogin( error != nil || !(localPlayer.isAuthenticated), pname );
		}];
	}
}

void IOSPlatform::postScore( unsigned int score, const Dojo::String& leaderboard, GameCenterListener* listener )
{
	DEBUG_ASSERT( listener, "The Game Center Listener must not be null" );
	
	GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:leaderboard.toNSString()] autorelease];
    scoreReporter.value = score;
	
    [scoreReporter reportScoreWithCompletionHandler:^(NSError *error) 
	{
		listener->onPostCompletion( error != nil );
    }];
}


void IOSPlatform::requestScore( const Dojo::String& leaderboard, GameCenterListener* listener )
{
	DEBUG_ASSERT( listener, "The Game Center Listener must not be null" );
	
	NSArray* players = [NSArray arrayWithObject: [[GKLocalPlayer localPlayer] playerID] ];
	
	GKLeaderboard *query = [[GKLeaderboard alloc] initWithPlayerIDs: players ];
	
    if (query != nil)
    {
		query.category = leaderboard.toNSString();
				
        [query loadScoresWithCompletionHandler: ^(NSArray *scores, NSError *error) {
				
			if( error == nil )
			{	
				int64_t value = [query localPlayerScore].value;
				
				//no listed score
				if( scores == nil )
					value = 0;
				
				listener->onHighScoreGet( query.category, (int)value, false );
			}
			else
			{
				NSLog(@"Error: %@ %@", error, [error userInfo]);
				listener->onHighScoreGet( query.category, -1, true );
			}
        }];
    }
}

void IOSPlatform::postAchievement( const Dojo::String& code, GameCenterListener* listener )
{
	DEBUG_ASSERT( listener, "The Game Center Listener must not be null" );
	
	GKAchievement *achievement = [[[GKAchievement alloc] initWithIdentifier: code.toNSString() ] autorelease];
    if (achievement)
    {
		achievement.percentComplete = 100;
		[achievement reportAchievementWithCompletionHandler:^(NSError *error)
		 {
			 listener->onPostCompletion( error != nil );
		 }];
    }
}

void IOSPlatform::requestAchievements( GameCenterListener* listener)
{
	DEBUG_ASSERT( listener, "The Game Center Listener must not be null" );
	
	[GKAchievement loadAchievementsWithCompletionHandler:^(NSArray *achievements, NSError *error) 
	{
		std::vector< String > codes;
		
		if (achievements != nil && error == nil )
        {
			//convert all the achievements
			for( int i = 0; i < [achievements count]; ++i )
			{
				GKAchievement* achievement = [achievements objectAtIndex:i];
				
				if( achievement.completed )				
					codes.push_back( String( achievement.identifier ) );
			}
        }
		
		listener->onAchievementsGet( codes, error != nil );
	}];
}

void IOSPlatform::showDefaultLeaderboard()
{
	[[[UIApplication sharedApplication] delegate] showGameCenterLeaderboard];
}

bool IOSPlatform::isSmallScreen()
{
	return [[UIDevice currentDevice] userInterfaceIdiom] != UIUserInterfaceIdiomPad;
}

#endif
