//
//  ResourceManager.mm
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/27/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#include "ResourceGroup.h"

using namespace Dojo;

ResourceGroup::ResourceGroup()
{
	empty = new FrameSet( this, "empty" );
	
	//create an empty texturedQuad
	Mesh* m = new Mesh( this );
	m->setVertexFieldEnabled( Mesh::VF_POSITION2D, true );
	m->setVertexFieldEnabled( Mesh::VF_UV, true );
	
	m->begin(4);	
	m->vertex( -0.5, -0.5 );		
	m->uv( 0,1 );
	
	m->vertex( 0.5, -0.5 );		
	m->uv( 1,1 );
	
	m->vertex( -0.5, 0.5 );		
	m->uv( 0,0 );
	
	m->vertex( 0.5, 0.5 );
	m->uv( 1,0 );
	
	m->end();
	
	addMesh( m, "texturedQuad" );
}
 
ResourceGroup::~ResourceGroup()
{
	delete empty;
	
	unloadAll();
}

void ResourceGroup::loadSets( const std::string& subdirectory )
{
	//load all the sets in the given folder
	NSString* dir = nil;
	
	if( subdirectory.size() > 0 )
		dir = Utils::toNSString( subdirectory );
	
	NSArray* pathsPNG = [[NSBundle mainBundle] pathsForResourcesOfType: @"png" inDirectory: dir ];
	NSArray* pathsPVR = [[NSBundle mainBundle] pathsForResourcesOfType: @"pvrtc" inDirectory: dir ];
	
	NSArray* paths = [pathsPNG arrayByAddingObjectsFromArray:pathsPVR];
	
	NSString* NSpath;
	std::string path, name, lastName;
	FrameSet* currentSet = NULL;
	
	for( uint i = 0; i < [paths count]; ++i )
	{
		NSpath = (NSString*)[paths objectAtIndex:i];
		
		path = Utils::toSTDString( NSpath );		
		name = Utils::getFileName( path ); 
		
		if( !Utils::areStringsNearInSequence( lastName, name ) )
		{
			//load last set
			if( currentSet )
				currentSet->load();
			
			std::string setPrefix = Utils::removeTag( name );
			
			//create a new set
			currentSet = new FrameSet( this, setPrefix );
			
			addFrameSet( currentSet, setPrefix );
		}
		
		//create and load a new buffer
		Texture* t = new Texture( this, path );
		currentSet->addTexture( t, true );
		
		lastName = name;
	}	
	
	//load last set
	if( currentSet )
		currentSet->load();
	
	NSArray* pathsAtlases = [[NSBundle mainBundle] pathsForResourcesOfType: @"atlasinfo" inDirectory: dir ];
	
	//now load atlases!		
	for(uint  i = 0; i < [pathsAtlases count]; ++i)
	{
		NSpath = (NSString*)[pathsAtlases objectAtIndex:i];
			
		path = Utils::toSTDString( NSpath );		
		name = Utils::getFileName( path ); 
			
		currentSet = new FrameSet( this, name );
		if( currentSet->loadAtlas( path ) )
			addFrameSet( currentSet, name );
	}
	
	//HACK
	/*[pathsPNG release];
	[pathsPVR release];
	[paths release];
	[dir release];*/
}

void ResourceGroup::loadFonts( const std::string& subdirectory )
{
	//load all the sets in the given folder
	NSString* dir = nil;
	
	if( subdirectory.size() > 0 )
		dir = Utils::toNSString( subdirectory );
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType: @"font" inDirectory: dir ];
		
	NSString* NSpath;
	std::string path, name;
	Font* font = NULL;
	
	for( uint i = 0; i < [paths count]; ++i )
	{
		NSpath = (NSString*)[paths objectAtIndex:i];
		
		path = Utils::toSTDString( NSpath );
		
		name = Utils::getFileName( path ); 
		
		///use the frameset with the same name
		if( isFrameSetLoaded( name ) )
		{
			font = new Font( path, getFrameSet( name ) );
			
			if( font->load() )			
				addFont( font, name );
		}	
	}		
	
	//HACK
	//[paths release];
	//[dir release];
}

void ResourceGroup::loadMeshes( const std::string& folder )
{
	NSString* dir = Utils::toNSString( folder );
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType: @"dms" inDirectory:dir ];
	
	NSString* path;
	std::string name, filePath;
	
	for( uint i = 0; i < [paths count]; ++i )
	{
		path = (NSString*)[paths objectAtIndex:i];
		
		filePath = Utils::toSTDString( path );
		name = Utils::getFileName( filePath );
		
		Mesh* mesh = new Mesh( this, filePath );
		if( mesh->load() )
			addMesh( mesh, name );
	}
	
	//HACK
	//[paths release];
	//[dir release];
}

void ResourceGroup::unloadSets()
{	
	FrameSetMap::iterator itr = frameSets.begin();
	
	for( ; itr != frameSets.end(); ++itr )
		delete itr->second;	
		
	frameSets.clear();
}

void ResourceGroup::unloadFonts()
{
	FontMap::iterator itr = fonts.begin();
	
	for( ; itr != fonts.end(); ++itr )
		delete itr->second;	
	
	fonts.clear();
}

void ResourceGroup::unloadMeshes()
{
	MeshMap::iterator itr = meshes.begin();
	
	for( ; itr != meshes.end(); ++itr )
		delete itr->second;	
	
	meshes.clear();
}
