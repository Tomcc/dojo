//
//  ResourceManager.mm
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/27/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#include "ResourceGroup.h"

#include "Platform.h"

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
	
	DEBUG_ASSERT( subdirectory.size() );
	
	std::vector< std::string > paths;
	std::string name, lastName;
	
	FrameSet* currentSet = NULL;
	
	Platform::getFilePathsForType( "png", subdirectory, paths );
	Platform::getFilePathsForType( "pvrtc", subdirectory, paths );
		
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] ); 
		
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
		Texture* t = new Texture( this, paths[i] );
		currentSet->addTexture( t, true );
		
		lastName = name;
	}	
	
	//load last set
	if( currentSet )
		currentSet->load();
	
	paths.clear();

	Platform::getFilePathsForType( "atlasinfo", subdirectory, paths );
	
	//now load atlases!		
	for(uint  i = 0; i < paths.size(); ++i)
	{
		name = Utils::getFileName( paths[i] ); 
			
		currentSet = new FrameSet( this, name );
		if( currentSet->loadAtlas( paths[i] ) )
			addFrameSet( currentSet, name );
	}
}

void ResourceGroup::loadFonts( const std::string& subdirectory )
{
	//load all the sets in the given folder
	DEBUG_ASSERT( subdirectory.size() );
	
	std::string name;
	Font* font = NULL;
	
	std::vector<std::string> paths;
	
	Platform::getFilePathsForType( "font", subdirectory, paths );
	
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] ); 
		
		///use the frameset with the same name
		if( isFrameSetLoaded( name ) )
		{
			font = new Font( paths[i], getFrameSet( name ) );
			
			if( font->load() )			
				addFont( font, name );
		}	
	}
}

void ResourceGroup::loadMeshes( const std::string& subdirectory )
{
	std::vector<std::string> paths;
	std::string name;
	
	Platform::getFilePathsForType( "dms", subdirectory, paths );
	
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] );
		
		Mesh* mesh = new Mesh( this, paths[i] );
		if( mesh->load() )
			addMesh( mesh, name );
	}
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
