#include "stdafx.h"

#include "ResourceGroup.h"

#include "Platform.h"

using namespace Dojo;

ResourceGroup::ResourceGroup() :
finalized( false )
{
	//link map array
	mapArray[ RT_FRAMESET ] = &frameSets;
	mapArray[ RT_FONT ] = &fonts;
	mapArray[ RT_MESH ] = &meshes;
	mapArray[ RT_SOUND ] = &sounds;
	mapArray[ RT_TABLE ] = &tables;
	
	empty = new FrameSet( this, "empty" );
}
 
ResourceGroup::~ResourceGroup()
{
	delete empty;
	
	unloadAll();
}

void ResourceGroup::loadSets( const String& subdirectory )
{
	//load all the sets in the given folder
	
	DEBUG_ASSERT( subdirectory.size() );
	
	std::vector< String > paths;
	String name, lastName;
	
	FrameSet* currentSet = NULL;
	
	Platform::getSingleton()->getFilePathsForType( "png", subdirectory, paths );
		
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] ); 
		
		if( !Utils::areStringsNearInSequence( lastName, name ) )
		{
			//load last set
			if( currentSet )
				currentSet->load();
			
			String setPrefix = Utils::removeTag( name );
			
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
	Platform::getSingleton()->getFilePathsForType( "atlasinfo", subdirectory, paths );
	
	//now load atlases!		
	Table def;
	for(uint  i = 0; i < paths.size(); ++i)
	{
		name = Utils::getFileName( paths[i] ); 

		Platform::getSingleton()->load( &def, paths[i] );
			
		currentSet = new FrameSet( this, name );

		if( currentSet->loadAtlas( &def, this ) )
			addFrameSet( currentSet, name );

		def.clear();
	}
}

void ResourceGroup::loadFonts( const String& subdirectory )
{
	//load all the sets in the given folder
	DEBUG_ASSERT( subdirectory.size() );
	
	String name;
	std::vector<String> paths;
	
	Platform::getSingleton()->getFilePathsForType( "font", subdirectory, paths );
	
	///just load a Font for any .ttf file found
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] ); 
						
		addFont( new Font( paths[i] ), name );
	}
}

void ResourceGroup::loadMeshes( const String& subdirectory )
{
	std::vector<String> paths;
	String name;
	
	//look for different binary files on different platforms
	String format = Math::isLittleEndian() ? "lem" : "bem"; 
	
	Platform::getSingleton()->getFilePathsForType( format, subdirectory, paths );
	
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] );
		
		Mesh* mesh = new Mesh( this, paths[i] );
		if( mesh->load() )
			addMesh( mesh, name );
	}
}

void ResourceGroup::loadSounds( const String& subdirectory )
{
	//ask all the sound files to the main bundle
	std::vector< String > paths;
	String name, lastName;

	SoundSet* currentSet = NULL;

	Platform::getSingleton()->getFilePathsForType( "ogg", subdirectory, paths );
	
	for( uint i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] );
		
		if( !Utils::areStringsNearInSequence( lastName, name ) )
		{
			String setPrefix = Utils::removeTag( name );
			
			//create a new set
			currentSet = new SoundSet( setPrefix );
			
			addSound( currentSet, setPrefix );
		}
			
		//create and load a new buffer
		SoundBuffer* b = new SoundBuffer( this, paths[i] );
		b->load();
		
		currentSet->addBuffer( b );
		
		lastName = name;
	}
}

void ResourceGroup::loadTables( const String& folder )
{
	std::vector< String > paths;
	
	Platform::getSingleton()->getFilePathsForType("ds", folder, paths );
	
	for( uint i = 0; i < paths.size(); ++i )
	{
		Table* t = new Table( Utils::getFileName( paths[i] ) );
		
		Platform::getSingleton()->load( t, paths[i] );
		
		addTable( t );
	}
}

void ResourceGroup::loadPrefabMeshes()
{
	//create an empty texturedQuad
	Mesh* m = new Mesh( this );
	m->setTriangleMode( Mesh::TM_STRIP );
	m->setVertexFieldEnabled( Mesh::VF_POSITION2D );
	m->setVertexFieldEnabled( Mesh::VF_UV );
	
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
	
	//create a texturedCube
#define l 0.501f
	
	Mesh* cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 24 );
	
	cube->vertex( l, l, l );	cube->normal( 0,0,1 );	cube->uv(1,1);
	cube->vertex( l, -l, l );	cube->normal( 0,0,1 );  cube->uv(0,1);
	cube->vertex( -l, l, l );	cube->normal( 0,0,1 );	cube->uv(1,0);
	cube->vertex( -l, -l, l );	cube->normal( 0,0,1 );	cube->uv(0,0);
	
	cube->quad(0,1,2,3);
	
	cube->vertex( l, l, -l );	cube->normal( 0,0,-1 );	cube->uv(1,1);
	cube->vertex( -l, l, -l );	cube->normal( 0,0,-1 );	cube->uv(0,1);
	cube->vertex( l, -l, -l );	cube->normal( 0,0,-1 );	cube->uv(1,0);
	cube->vertex( -l, -l, -l );	cube->normal( 0,0,-1 );	cube->uv(0,0);
	
	cube->quad(4,5,6,7);
	
	cube->vertex( l, l, l );	cube->normal( 1,0, 0 );	cube->uv(1,1);
	cube->vertex( l, l, -l );	cube->normal( 1,0, 0 );	cube->uv(0,1);
	cube->vertex( l, -l, l );	cube->normal( 1,0, 0 );	cube->uv(1,0);
	cube->vertex( l, -l, -l );	cube->normal( 1,0, 0 );	cube->uv(0,0);
	
	cube->quad(8,9,10,11);
	
	cube->vertex( -l, l, l );	cube->normal( -1,0, 0 );	cube->uv(1,1);
	cube->vertex( -l, -l, l );	cube->normal( -1,0, 0 );	cube->uv(0,1);
	cube->vertex( -l, l, -l );	cube->normal( -1,0, 0 );	cube->uv(1,0);
	cube->vertex( -l, -l, -l );	cube->normal( -1,0, 0 );	cube->uv(0,0);
	
	cube->quad(12,13,14,15);
	
	cube->vertex( l, l, l );	cube->normal( 0,1,0 );	cube->uv(1,1);
	cube->vertex( -l, l, l );	cube->normal( 0,1,0 );  cube->uv(0,1);
	cube->vertex( l, l, -l );	cube->normal( 0,1,0 );	cube->uv(1,0);
	cube->vertex( -l, l, -l );	cube->normal( 0,1,0 );	cube->uv(0,0);
	
	cube->quad(16,17,18,19);
	
	cube->vertex( l, -l, l );	cube->normal( 0,-1,0 );	cube->uv(1,1);
	cube->vertex( l, -l, -l );	cube->normal( 0,-1,0 );	cube->uv(0,1);
	cube->vertex( -l, -l, l );	cube->normal( 0,-1,0 );	cube->uv(1,0);
	cube->vertex( -l, -l, -l );	cube->normal( 0,-1,0 );	cube->uv(0,0);
	
	cube->quad(20,21,22,23);
	
	cube->end();
	
	addMesh( cube, "texturedCube" );
	
	//MEGA HACK -------------------------------------------
	
	cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 4 );
	//-Z
	cube->vertex( l, l, -l );	cube->normal( 0,0,1 );	cube->uv(1,0);
	cube->vertex( l, -l, -l );	cube->normal( 0,0,1 );	cube->uv(1,1);
	cube->vertex( -l, l, -l );	cube->normal( 0,0,1 );	cube->uv(0,0);
	cube->vertex( -l, -l, -l );	cube->normal( 0,0,1 );	cube->uv(0,1);
	
	cube->quad(0,1,2,3);
	
	cube->end();
	addMesh( cube, "prefabSkybox_1" );
	
	cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 4 );
	//+X
	cube->vertex( l, l, l );	cube->normal( -1,0, 0 );	cube->uv(1,0);
	cube->vertex( l, -l, l );	cube->normal( -1,0, 0 );	cube->uv(1,1);
	cube->vertex( l, l, -l );	cube->normal( -1,0, 0 );	cube->uv(0,0);
	cube->vertex( l, -l, -l );	cube->normal( -1,0, 0 );	cube->uv(0,1);
	
	cube->quad(0,1,2,3);
	
	cube->end();
	addMesh( cube, "prefabSkybox_2" );
	
	cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 4 );
	//+Z
	cube->vertex( l, l, l );	cube->normal( 0,0,-1 );	cube->uv(0,0);
	cube->vertex( -l, l, l );	cube->normal( 0,0,-1 );	cube->uv(1,0);
	cube->vertex( l, -l, l );	cube->normal( 0,0,-1 ); cube->uv(0,1);
	cube->vertex( -l, -l, l );	cube->normal( 0,0,-1 );	cube->uv(1,1);
	
	cube->quad(0,1,2,3);
	
	cube->end();
	addMesh( cube, "prefabSkybox_3" );
	
	cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 4 );
	//-X
	cube->vertex( -l, l, l );	cube->normal( 1,0, 0 );	cube->uv(0,0);
	cube->vertex( -l, l, -l );	cube->normal( 1,0, 0 );	cube->uv(1,0);
	cube->vertex( -l, -l, l );	cube->normal( 1,0, 0 );	cube->uv(0,1);
	cube->vertex( -l, -l, -l );	cube->normal( 1,0, 0 );	cube->uv(1,1);
	
	cube->quad(0,1,2,3);
	
	cube->end();
	addMesh( cube, "prefabSkybox_4" );
	
	cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 4 );
	
	cube->vertex( l, l, l );	cube->normal( 0,-1,0 );	cube->uv(1,1);
	cube->vertex( l, l, -l );	cube->normal( 0,-1,0 );  cube->uv(0,1);
	cube->vertex( -l, l, l );	cube->normal( 0,-1,0 );	cube->uv(1,0);
	cube->vertex( -l, l, -l );	cube->normal( 0,-1,0 );	cube->uv(0,0);
	
	cube->quad(0,1,2,3);
	
	cube->end();
	addMesh( cube, "prefabSkybox_5" );
	
	cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 4 );
	
	cube->vertex( l, -l, l );	cube->normal( 0,1,0 );	cube->uv(1,0);
	cube->vertex( -l, -l, l );	cube->normal( 0,1,0 );	cube->uv(1,1);
	cube->vertex( l, -l, -l );	cube->normal( 0,1,0 );	cube->uv(0,0);
	cube->vertex( -l, -l, -l );	cube->normal( 0,1,0 );	cube->uv(0,1);
	
	cube->quad(0,1,2,3);
	
	cube->end();
	
	addMesh( cube, "prefabSkybox_6");
}
