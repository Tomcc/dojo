//
//  OSXPlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef __OSXPlatform_h
#define __OSXPlatform_h

#include "ApplePlatform.h"

#ifdef __OBJC__
    #import <AppKit/NSOpenGLView.h>
    #import <AppKit/NSWindow.h>

	#import "CustomOpenGLView.h"
#endif

namespace Dojo
{
	
#ifndef __OBJC__
	class NSOpenGLView;
	class NSWindow;
	class CustomOpenGLView;
#endif

	
    class OSXPlatform : public ApplePlatform
    {
    public:
     
        OSXPlatform( const Table& config ) :
		ApplePlatform( config )
        {
            
        }
		
		virtual ~OSXPlatform();
        
        virtual void initialise();
		virtual void shutdown();
        
		virtual void acquireContext();
		virtual void present();
        
		virtual void loop( float frameTime );
	
	protected:
                
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
        CustomOpenGLView* view;
        NSWindow* window;
		NSAutoreleasePool* pool;
    };
}

#endif