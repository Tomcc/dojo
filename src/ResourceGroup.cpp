#include "stdafx.h"

#include "ResourceGroup.h"

#include "Platform.h"
#include "Timer.h"
#include "FrameSet.h"
#include "Mesh.h"
#include "Font.h"
#include "Table.h"
#include "SoundSet.h"
#include "SoundBuffer.h"

#include <Poco/File.h>

using namespace Dojo;

ResourceGroup::ResourceGroup() :
finalized( false ),
disableBilinear( false ),
disableMipmaps( false ),
disableTiling( false )
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
	SAFE_DELETE( empty );
	
	unloadResources( false );
}

void ResourceGroup::addLocalizedFolder( const String& basefolder, int version )
{
	String lid = basefolder;

	if( lid[ lid.size() - 1 ] != '/' )
		lid += '/';

	String localeDirPath = lid + locale;

	//check if the folder exists or fallback to the default one
	Poco::File localeDir( localeDirPath.UTF8() );
	
	if( localeDir.exists() )
		addFolderSimple( localeDirPath, version );
	else
		addFolderSimple( lid + fallbackLocale, version );
}

void ResourceGroup::addTable( Table* t )
{
	DEBUG_ASSERT( t != nullptr, "addTable: Table is null" );
	DEBUG_ASSERT( !getTable( t->getName() ), "addTable: a table with this name was already added" );
	DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );
	
	tables[ t->getName() ] = t;
	
	DEBUG_MESSAGE( "+" << t->getName().ASCII() << "\t\t" << "table" );
}

void ResourceGroup::addSets( const String& subdirectory, int version )
{
	DEBUG_ASSERT( subdirectory.size(), "addSets: folder path is empty" );
	DEBUG_ASSERT( version >= 0, "addSets: negative versions are invalid" );
	
	std::vector< String > paths;
	String name, lastName;
	
	FrameSet* currentSet = NULL;
	
	//find pngs and jpgs
	Platform::getSingleton()->getFilePathsForType( "png", subdirectory, paths );
	Platform::getSingleton()->getFilePathsForType( "jpg", subdirectory, paths );
		
	for( int i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] );
		
		//skip wrong versions
		if( Utils::getVersion( name ) != version ) continue;
		
		if( !Utils::areStringsNearInSequence( lastName, name ) )
		{	
			String setPrefix = Utils::removeTags( name );
			
			//create a new set
			currentSet = new FrameSet( this, setPrefix );
			
			addFrameSet( currentSet, setPrefix );
		}
		
		//create a new buffer
		Texture* t = new Texture( this, paths[i] );
		currentSet->addTexture( t, true );
		
		lastName = name;
	}

	paths.clear();
	Platform::getSingleton()->getFilePathsForType( "atlasinfo", subdirectory, paths );
	
	//now add atlases!		
	Table def;
	for( int  i = 0; i < paths.size(); ++i)
	{
		name = Utils::getFileName( paths[i] ); 
		
		//skip wrong versions
		if( Utils::getVersion( name ) != version ) 
			continue;

		name = Utils::removeVersion( name );

		Platform::getSingleton()->load( &def, paths[i] );

		//standard flat atlasinfo
		if( def.getAutoMembers() == 0 )
		{			
			currentSet = new FrameSet( this, name );
			currentSet->setAtlas( &def, this );
		
			addFrameSet( currentSet, name );
		}
		else for( int i = 0; i < def.getAutoMembers(); ++i )
		{
			Table* sub = def.getTable(i);
			const String& name = sub->getString("name");

			currentSet = new FrameSet( this, name );
			currentSet->setAtlas( sub, this );

			addFrameSet( currentSet, name );
		}

		def.clear();
	}
}

void ResourceGroup::addFonts( const String& subdirectory, int version )
{
	//add all the sets in the given folder
	DEBUG_ASSERT( subdirectory.size(), "addFonts: folder path is empty" );
	DEBUG_ASSERT( version >= 0, "addFonts: negative versions are invalid" );
	
	String name;
	std::vector<String> paths;
	
	Platform::getSingleton()->getFilePathsForType( "font", subdirectory, paths );
	
	///just add a Font for any .ttf file found
	for( int i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] ); 
		
		//skip wrong versions
		if( Utils::getVersion( name ) != version ) 
			continue;

		name = Utils::removeTags( name );
						
		addFont( new Font( this, paths[i] ), name );
	}
}

void ResourceGroup::addMeshes( const String& subdirectory )
{
	std::vector<String> paths;
	String name; 
	
	Platform::getSingleton()->getFilePathsForType( "mesh", subdirectory, paths );
	
	for( int i = 0; i < paths.size(); ++i )
	{
		name = Utils::getFileName( paths[i] );
		
		addMesh( new Mesh( this, paths[i] ), name );
	}
}

void ResourceGroup::addSounds( const String& subdirectory )
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
			//create a new set		
			String setPrefix = Utils::removeTags( name );
			currentSet = new SoundSet( this, setPrefix );
			addSound( currentSet, setPrefix );
		}
			
		//create a new buffer		
		currentSet->addBuffer( new SoundBuffer( this, paths[i] ) );
		
		lastName = name; 
	}
}

void ResourceGroup::addTables( const String& folder )
{
	std::vector< String > paths;
	
	Platform::getSingleton()->getFilePathsForType("ds", folder, paths );
	
	for( uint i = 0; i < paths.size(); ++i )
		addTable( new Table( this, paths[i] ) );
}

void ResourceGroup::addPrefabMeshes()
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
	
//textured quad xz
	m = new Mesh( this );
	m->setTriangleMode( Mesh::TM_STRIP );
	m->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	m->setVertexFieldEnabled( Mesh::VF_UV );

	m->begin(4);	

	m->vertex( -0.5, 0, -0.5 );		
	m->uv( 0,0 );

	m->vertex( -0.5, 0, 0.5 );		
	m->uv( 0,1 );

	m->vertex( 0.5, 0, -0.5 );		
	m->uv( 1,0 );

	m->vertex( 0.5, 0, 0.5 );
	m->uv( 1,1 );

	m->end();

	addMesh( m, "texturedQuadXZ" );

	//create a texturedCube
#define l 0.501f
	
	Mesh* cube = new Mesh( this );
	
	cube->setIndexByteSize( 1 ); //byte indices
	cube->setTriangleMode( Mesh::TM_LIST );
	cube->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	cube->setVertexFieldEnabled( Mesh::VF_NORMAL );
	cube->setVertexFieldEnabled( Mesh::VF_UV );
	
	cube->begin( 24 );
	
	cube->vertex( l, l, l );		cube->normal( 0,0,1 );	cube->uv(1,1);
	cube->vertex( l, -l, l );	cube->normal( 0,0,1 );  cube->uv(0,1);
	cube->vertex( -l, l, l );	cube->normal( 0,0,1 );	cube->uv(1,0);
	cube->vertex( -l, -l, l );	cube->normal( 0,0,1 );	cube->uv(0,0);
	
	cube->quad(0,1,2,3);
	
	cube->vertex( l, l, -l );	cube->normal( 0,0,-1 );	cube->uv(1,1);
	cube->vertex( -l, l, -l );	cube->normal( 0,0,-1 );	cube->uv(0,1);
	cube->vertex( l, -l, -l );	cube->normal( 0,0,-1 );	cube->uv(1,0);
	cube->vertex( -l, -l, -l );	cube->normal( 0,0,-1 );	cube->uv(0,0);
	
	cube->quad(4,5,6,7);
	
	cube->vertex( l, l, l );		cube->normal( 1,0, 0 );	cube->uv(1,1);
	cube->vertex( l, l, -l );	cube->normal( 1,0, 0 );	cube->uv(0,1);
	cube->vertex( l, -l, l );	cube->normal( 1,0, 0 );	cube->uv(1,0);
	cube->vertex( l, -l, -l );	cube->normal( 1,0, 0 );	cube->uv(0,0);
	
	cube->quad(8,9,10,11);
	
	cube->vertex( -l, l, l );	cube->normal( -1,0, 0 );	cube->uv(1,1);
	cube->vertex( -l, -l, l );	cube->normal( -1,0, 0 );	cube->uv(0,1);
	cube->vertex( -l, l, -l );	cube->normal( -1,0, 0 );	cube->uv(1,0);
	cube->vertex( -l, -l, -l );	cube->normal( -1,0, 0 );	cube->uv(0,0);
	
	cube->quad(12,13,14,15);
	
	cube->vertex( l, l, l );		cube->normal( 0,1,0 );	cube->uv(1,1);
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
	
	//create a cube
	
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
	addMesh( cube, "prefabSkybox-Z" );
	
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
	addMesh( cube, "prefabSkybox-X" );
	
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
	addMesh( cube, "prefabSkybox+Z" );
	
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
	addMesh( cube, "prefabSkybox+X" );
	
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
	addMesh( cube, "prefabSkybox+Y" );
	
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
	
	addMesh( cube, "prefabSkybox-Y");
	
	
	//add cube for wireframe use
	m = new Mesh( this );
	m->setTriangleMode( Mesh::TM_LINE_STRIP );
	m->setVertexFieldEnabled( Mesh::VF_POSITION2D );
	
	m->begin(4);	
	
	m->vertex( 0.5, 0.5 );	
	m->vertex( -0.5, 0.5 );	
	m->vertex( 0.5, -0.5 );	
	m->vertex( -0.5, -0.5 );
	
	m->index( 0 );
	m->index( 1 );
	m->index( 3 );
	m->index( 2 );
	m->index( 0 );
	m->index( 3 );
	
	m->end();
	
	addMesh( m, "wireframeQuad" );
}
