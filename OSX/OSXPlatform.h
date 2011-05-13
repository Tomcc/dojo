//
//  OSXPlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef __OSXPlatform_h
#define __OSXPlatform_h

#include "Platform.h"

#ifdef __OBJC__
    #import <AppKit/NSOpenGLView.h>
    #import <AppKit/NSWindow.h>
#endif

namespace Dojo
{
    class OSXPlatform : public Platform
    {
    public:
    
        OSXPlatform()
        {
            
        }
        
        virtual void initialise();
		virtual void shutdown();
        
		virtual void acquireContext();
		virtual void present();
        
		virtual void step( float dt );
		virtual void loop( float frameTime );
        
		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height );
		
		///loads the given file in a buffer - WARNING not every format is supported on every platform
		virtual uint loadAudioFileContent( ALuint& buffer, const std::string& path );
        
		virtual void load( Table* dest, const std::string& relPath = "" );
		virtual void save( Table* table, const std::string& relPath = "" );
		        
		virtual void openWebPage( const std::string& site );
        
    protected:
        
#ifdef __OBJC__
        NSOpenGLView* view;
        NSWindow* window;
#endif
    };
}

#endif