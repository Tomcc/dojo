/*
 *  Resource.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef Email_h__
#define Email_h__

#include "dojo_common_header.h"

namespace Dojo 
{	
	///A simple email class used to send a text message
	class Email
	{
	public:
		
		class Listener
		{
		public:
			
			virtual void onEmailSent()=0;
			virtual void onEmailFailed()=0;
			
		}* listener;		
		
		String dest, message, subject, attachmentName;
		
		void* attachmentData;
		int attachmentSize;
		Dojo::String attachmentMimeType;
		
				
		Email( const Dojo::String& destAddress, const Dojo::String& messageSubject, const Dojo::String& messageText, Listener* successListener ) :
		dest( destAddress ),
		subject( messageSubject ),
		message( messageText ),
		listener( successListener ),
		attachmentData( NULL )
		{
			DEBUG_ASSERT( listener );
			DEBUG_ASSERT( subject.size() );
			DEBUG_ASSERT( destAddress.size() );
		}
		
		void addAttachment( const Dojo::String& name, void* dataptr, int size, const Dojo::String& type )
		{
			DEBUG_ASSERT( dataptr );
			DEBUG_ASSERT( size > 0 );
			DEBUG_ASSERT( type.size() );
			DEBUG_ASSERT( name.size() );
			
			attachmentName = name;
			attachmentData = dataptr;
			attachmentSize = size;
			attachmentMimeType = type;
		}
		
	protected:
		
	};
}

#endif