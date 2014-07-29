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
#include "Texture.h"

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
	mapArray[ RT_SHADER ] = &shaders;
	mapArray[ RT_PROGRAM ] = &programs;
	
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
	
	if (logchanges)
		DEBUG_MESSAGE( "+" + t->getName() + "\t\t table" );
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
	
	for( int i = 0; i < paths.size(); ++i )
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
	
	for( int i = 0; i < paths.size(); ++i )
		addTable( new Table( this, paths[i] ) );
}

void ResourceGroup::addPrograms( const String& folder )
{
	std::vector< String > paths;

	Platform::getSingleton()->getFilePathsForType("vs", folder, paths );
	Platform::getSingleton()->getFilePathsForType("ps", folder, paths );

	for( auto& path : paths )
	{
		String name = Utils::getFileName( path );
		addProgram( new ShaderProgram( this, path ), name );
	}
}

void ResourceGroup::addShaders( const String& folder )
{
	std::vector< String > paths;

	Platform::getSingleton()->getFilePathsForType("shader", folder, paths );

	for( auto& path : paths )
	{
		String name = Utils::getFileName( path );
		addShader( new Shader( this, path ), name );
	}
}

void ResourceGroup::addPrefabMeshes()
{
	//create an empty texturedQuad
	Mesh* m = new Mesh(this);
	m->setTriangleMode(TriangleMode::TriangleStrip);
	m->setVertexFields({ VertexField::Position2D, VertexField::UV0 });

	m->begin(4);

	m->vertex(-0.5, -0.5);
	m->uv(0, 1);

	m->vertex(0.5, -0.5);
	m->uv(1, 1);

	m->vertex(-0.5, 0.5);
	m->uv(0, 0);

	m->vertex(0.5, 0.5);
	m->uv(1, 0);

	m->end();

	addMesh(m, "texturedQuad");

	//textured quad xz
	m = new Mesh(this);
	m->setTriangleMode(TriangleMode::TriangleStrip);
	m->setVertexFields({ VertexField::Position3D, VertexField::UV0 });

	m->begin(4);

	m->vertex(-0.5, 0, -0.5);
	m->uv(0, 0);

	m->vertex(-0.5, 0, 0.5);
	m->uv(0, 1);

	m->vertex(0.5, 0, -0.5);
	m->uv(1, 0);

	m->vertex(0.5, 0, 0.5);
	m->uv(1, 1);

	m->end();

	addMesh(m, "texturedQuadXZ");

	//create a texturedCube
#define l 0.501f

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

	m->begin(24);

	m->vertex(l, l, l);		m->normal(0, 0, 1);	m->uv(1, 1);
	m->vertex(l, -l, l);	m->normal(0, 0, 1);  m->uv(0, 1);
	m->vertex(-l, l, l);	m->normal(0, 0, 1);	m->uv(1, 0);
	m->vertex(-l, -l, l);	m->normal(0, 0, 1);	m->uv(0, 0);

	m->quad(0, 1, 2, 3);

	m->vertex(l, l, -l);	m->normal(0, 0, -1);	m->uv(1, 1);
	m->vertex(-l, l, -l);	m->normal(0, 0, -1);	m->uv(0, 1);
	m->vertex(l, -l, -l);	m->normal(0, 0, -1);	m->uv(1, 0);
	m->vertex(-l, -l, -l);	m->normal(0, 0, -1);	m->uv(0, 0);

	m->quad(4, 5, 6, 7);

	m->vertex(l, l, l);		m->normal(1, 0, 0);	m->uv(1, 1);
	m->vertex(l, l, -l);	m->normal(1, 0, 0);	m->uv(0, 1);
	m->vertex(l, -l, l);	m->normal(1, 0, 0);	m->uv(1, 0);
	m->vertex(l, -l, -l);	m->normal(1, 0, 0);	m->uv(0, 0);

	m->quad(8, 9, 10, 11);

	m->vertex(-l, l, l);	m->normal(-1, 0, 0);	m->uv(1, 1);
	m->vertex(-l, -l, l);	m->normal(-1, 0, 0);	m->uv(0, 1);
	m->vertex(-l, l, -l);	m->normal(-1, 0, 0);	m->uv(1, 0);
	m->vertex(-l, -l, -l);	m->normal(-1, 0, 0);	m->uv(0, 0);

	m->quad(12, 13, 14, 15);

	m->vertex(l, l, l);		m->normal(0, 1, 0);	m->uv(1, 1);
	m->vertex(-l, l, l);	m->normal(0, 1, 0);  m->uv(0, 1);
	m->vertex(l, l, -l);	m->normal(0, 1, 0);	m->uv(1, 0);
	m->vertex(-l, l, -l);	m->normal(0, 1, 0);	m->uv(0, 0);

	m->quad(16, 17, 18, 19);

	m->vertex(l, -l, l);	m->normal(0, -1, 0);	m->uv(1, 1);
	m->vertex(l, -l, -l);	m->normal(0, -1, 0);	m->uv(0, 1);
	m->vertex(-l, -l, l);	m->normal(0, -1, 0);	m->uv(1, 0);
	m->vertex(-l, -l, -l);	m->normal(0, -1, 0);	m->uv(0, 0);

	m->quad(20, 21, 22, 23);

	m->end();

	addMesh(m, "texturedCube");

	//create a cube

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

	m->begin(4);
	//-Z
	m->vertex(l, l, -l);	m->normal(0, 0, 1);	m->uv(1, 0);
	m->vertex(l, -l, -l);	m->normal(0, 0, 1);	m->uv(1, 1);
	m->vertex(-l, l, -l);	m->normal(0, 0, 1);	m->uv(0, 0);
	m->vertex(-l, -l, -l);	m->normal(0, 0, 1);	m->uv(0, 1);

	m->quad(0, 1, 2, 3);

	m->end();
	addMesh(m, "prefabSkybox-Z");

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

	m->begin(4);
	//+X
	m->vertex(l, l, l);	m->normal(-1, 0, 0);	m->uv(1, 0);
	m->vertex(l, -l, l);	m->normal(-1, 0, 0);	m->uv(1, 1);
	m->vertex(l, l, -l);	m->normal(-1, 0, 0);	m->uv(0, 0);
	m->vertex(l, -l, -l);	m->normal(-1, 0, 0);	m->uv(0, 1);

	m->quad(0, 1, 2, 3);

	m->end();
	addMesh(m, "prefabSkybox-X");

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

	m->begin(4);
	//+Z
	m->vertex(l, l, l);	m->normal(0, 0, -1);	m->uv(0, 0);
	m->vertex(-l, l, l);	m->normal(0, 0, -1);	m->uv(1, 0);
	m->vertex(l, -l, l);	m->normal(0, 0, -1); m->uv(0, 1);
	m->vertex(-l, -l, l);	m->normal(0, 0, -1);	m->uv(1, 1);

	m->quad(0, 1, 2, 3);

	m->end();
	addMesh(m, "prefabSkybox+Z");

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

	m->begin(4);
	//-X
	m->vertex(-l, l, l);	m->normal(1, 0, 0);	m->uv(0, 0);
	m->vertex(-l, l, -l);	m->normal(1, 0, 0);	m->uv(1, 0);
	m->vertex(-l, -l, l);	m->normal(1, 0, 0);	m->uv(0, 1);
	m->vertex(-l, -l, -l);	m->normal(1, 0, 0);	m->uv(1, 1);

	m->quad(0, 1, 2, 3);

	m->end();
	addMesh(m, "prefabSkybox+X");

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

	m->begin(4);

	m->vertex(l, l, l);	m->normal(0, -1, 0);	m->uv(1, 1);
	m->vertex(l, l, -l);	m->normal(0, -1, 0);  m->uv(0, 1);
	m->vertex(-l, l, l);	m->normal(0, -1, 0);	m->uv(1, 0);
	m->vertex(-l, l, -l);	m->normal(0, -1, 0);	m->uv(0, 0);

	m->quad(0, 1, 2, 3);

	m->end();
	addMesh(m, "prefabSkybox+Y");

	m = new Mesh(this);

	m->setIndexByteSize(1); //byte indices
	m->setTriangleMode(TriangleMode::TriangleList);
	m->setVertexFields({VertexField::Position3D, VertexField::Normal, VertexField::UV0});
	
	m->begin( 4 );
	
	m->vertex( l, -l, l );	m->normal( 0,1,0 );	m->uv(1,0);
	m->vertex( -l, -l, l );	m->normal( 0,1,0 );	m->uv(1,1);
	m->vertex( l, -l, -l );	m->normal( 0,1,0 );	m->uv(0,0);
	m->vertex( -l, -l, -l );	m->normal( 0,1,0 );	m->uv(0,1);
	
	m->quad(0,1,2,3);
	
	m->end();
	
	addMesh( m, "prefabSkybox-Y");
	
	
	//add cube for wireframe use
	m = new Mesh( this );
	m->setTriangleMode( TriangleMode::LineStrip );
	m->setVertexFieldEnabled( VertexField::Position2D );
	
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
