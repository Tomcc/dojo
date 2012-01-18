#include "IOSPlatform.h"

#include "TargetConditionals.h"

#import <Foundation/NSString.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>	
#import <UIKit/UIKit.h>
#import <MessageUI/MFMailComposeViewController.h>

#include "Utils.h"
#include "dojomath.h"
#include "Email.h"

#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "FontSystem.h"
#include "Game.h"
#include "Table.h"
#include "Texture.h"

using namespace Dojo;

IOSPlatform::IOSPlatform( const Table& config ) :
ApplePlatform( config ),
app( NULL ),
player( NULL )
{
	
}

IOSPlatform::~IOSPlatform()
{
	if( player )
		[player release];
}

void IOSPlatform::initialise()
{
	DEBUG_ASSERT( app );
	
	CGRect bounds = [[UIScreen mainScreen] bounds];
		
	uint devicePixelScale;
	
	if( game->getNativeOrientation() == Render::RO_LANDSCAPE_LEFT || game->getNativeOrientation() == Render::RO_LANDSCAPE_RIGHT )
		devicePixelScale = game->getNativeWidth() / bounds.size.height;
	else
		devicePixelScale = game->getNativeWidth() / bounds.size.width;
	
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
	
	render = new Render( width, height, devicePixelScale, Render::RO_PORTRAIT );
	
	//some local configs
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
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
	DEBUG_ASSERT( context );
		
	EAGLContext* clone = [[EAGLContext alloc] initWithAPI:[context API] sharegroup:[context sharegroup]];
		
	bool err = [EAGLContext setCurrentContext:clone];
	DEBUG_ASSERT( err );
	
	[[NSAutoreleasePool alloc] init];
}

void IOSPlatform::present()
{	
	[EAGLContext setCurrentContext:context];
	
	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

void IOSPlatform::loop( float minstep )
{
	DEBUG_TODO;
}

bool IOSPlatform::isSystemSoundInUse()
{
	UInt32 otherAudioIsPlaying;
	UInt32 size = sizeof(otherAudioIsPlaying);
	AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &size, &otherAudioIsPlaying);
	
	return otherAudioIsPlaying;
}

String IOSPlatform::getRootPath()
{
	return String( [[NSBundle mainBundle] bundlePath] );
}

String IOSPlatform::getAppDataPath()
{
	NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* nspath = [nspaths objectAtIndex:0];
	
	return String( nspath );
}

void IOSPlatform::openWebPage( const String& site )
{
	NSURL* url = [NSURL URLWithString: site.toNSString() ];
	
	[[UIApplication sharedApplication] openURL:url];
}


/*
void IOSPlatform::sendEmail( const Email& e )
{	
	//check if this device can send emails
	if( ![MFMailComposeViewController canSendMail ] )
	{
		e.listener->onEmailFailed();
		return;
	}
	
	MFMailComposeViewController* controller = [[MFMailComposeViewController alloc] init];
	controller.mailComposeDelegate = app;
	
	NSArray* recipients = [NSArray arrayWithObject:e.dest.toNSString() ];
	
	[controller setToRecipients: recipients ];
	[controller setSubject: e.subject.toNSString() ];
	[controller setMessageBody: e.message.toNSString()  isHTML:NO];
	
	if( e.attachmentData )
	{		
		NSData* attachmentData = [NSData dataWithBytes:e.attachmentData length:e.attachmentSize ];
		
		[controller addAttachmentData: attachmentData 
					mimeType: e.attachmentMimeType.toNSString() 
					fileName:e.attachmentName.toNSString() ];
	}
	
	senderEmailListenerMap[ controller ] = e.listener;
	
	[app addSubview: [controller view] ];
	[[controller view] becomeFirstResponder];
	
	//[controller release];
}

void IOSPlatform::onEmailSent( void* senderController, bool success )
{
	SenderEmailListenerMap::iterator pos = senderEmailListenerMap.find( senderController );
	
	if( pos != senderEmailListenerMap.end() )
	{
		if( success )
			pos->second->onEmailSent();
		else
			pos->second->onEmailFailed();
		
		senderEmailListenerMap.erase( pos );
	}
}
*/
void IOSPlatform::enableScreenSaver( bool s )
{
	[[UIApplication sharedApplication] setIdleTimerDisabled: !s ];
}

void IOSPlatform::copyImageIntoCameraRoll( const Dojo::String& path )
{
	DEBUG_ASSERT( path.size() );
	
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
	DEBUG_ASSERT( player );
	DEBUG_ASSERT( player.playing );
	
	[player stop];
}

void IOSPlatform::setMp3FileVolume( float gain )
{
	DEBUG_ASSERT( gain >= 0.0 );
	DEBUG_ASSERT( gain <= 1.0 );
	
	if( !player )
		player = [AVAudioPlayer alloc];
	
	player.volume = gain;
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
	DEBUG_ASSERT( listener );
	
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
	DEBUG_ASSERT( listener );
	
	GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:leaderboard.toNSString()] autorelease];
    scoreReporter.value = score;
	
    [scoreReporter reportScoreWithCompletionHandler:^(NSError *error) 
	{
		listener->onPostCompletion( error != nil );
    }];
}


void IOSPlatform::requestScore( const Dojo::String& leaderboard, GameCenterListener* listener )
{
	DEBUG_ASSERT( listener );
	
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
				
				listener->onHighScoreGet( query.category, value, false );
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
	DEBUG_ASSERT( listener );
	
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
	DEBUG_ASSERT( listener );
	
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
