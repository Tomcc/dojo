#include "Platform.h"

#include "Utils.h"

using namespace Dojo;

void Platform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:NSType inDirectory:NSPath ];
	
	for( uint i = 0; i < [paths count]; ++i )
		out.push_back( Utils::toSTDString( (NSString*)[paths objectAtIndex:i] ) );
								  
}

uint Platform::loadFileContent( char*& bufptr, const std::string& path )
{
	bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
	
	NSString* NSPath = Utils::toNSString( path );
	
	NSData* data = [[NSData alloc] initWithContentsOfFile: NSPath ];
	
	if( !data )
		return false;
	
	uint size = [data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	[data release];
	
	return size;
}