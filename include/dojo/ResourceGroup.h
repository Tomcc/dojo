/*
 *  ResourceGroup.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/27/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ResourceGroup_h__
#define ResourceGroup_h__

#include "dojo_common_header.h"

#include <string>

#include "Font.h"
#include "FrameSet.h"
#include "Mesh.h"
#include "SoundSet.h"
#include "Table.h"

namespace Dojo {
	
	class ResourceGroup 
	{			
	public:		
		
		typedef std::map<String, FrameSet*> FrameSetMap;
		typedef std::map<String, Font*> FontMap;
		typedef std::map<String, Mesh*> MeshMap;
		typedef std::map<String, SoundSet*> SoundMap;
		typedef std::map<String, Table*> TableMap;
			
		ResourceGroup();
		
		virtual ~ResourceGroup();
		
		inline bool isFrameSetLoaded( const String& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return frameSets.find( name ) != frameSets.end();
		}
		
		inline bool isFontLoaded( const String& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return fonts.find( name ) != fonts.end();
		}
		
		inline bool isMeshLoaded( const String& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return meshes.find( name ) != meshes.end();
		}

		inline bool isSoundLoaded( const String& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return sounds.find( name ) != sounds.end();
		}
		
		inline bool isTableLoaded( const String& name )
		{
			DEBUG_ASSERT( name.size() );
			
			return tables.find( name ) != tables.end();
		}
				
		inline void addFrameSet( FrameSet* set, const String& name )
		{
			DEBUG_ASSERT( !isFrameSetLoaded( name ) );
			
			frameSets[name] = set;
		}
		
		inline void addFont( Font* f, const String& name )
		{
			DEBUG_ASSERT( !isFontLoaded( name ) );
			
			fonts[name] = f;
		}
		
		inline void addMesh( Mesh* m, const String& name )
		{
			DEBUG_ASSERT( !isMeshLoaded( name ) );
			
			meshes[ name ] = m;
		}
		
		inline void addSound( SoundSet* sb, const String& name )
		{
			DEBUG_ASSERT( !isSoundLoaded( name ) );

			sounds[ name ] = sb;
		}
		
		inline void addTable( Table* t )
		{
			DEBUG_ASSERT( t );
			DEBUG_ASSERT( !isTableLoaded( t->getName() ) );
			
			tables[ t->getName() ] = t;
		}

		inline FrameSet* getEmptyFrameSet()			{	return empty;	}

		inline FrameSet* getFrameSet( const String& name )
		{
			if( isFrameSetLoaded( name ) )
				return frameSets[name];

			return NULL;
		}
		
		inline Font* getFont( const String& name )
		{
			if( isFontLoaded( name ) )
				return fonts[name];
			
			return NULL;
		}
		
		inline Mesh* getMesh( const String& name )
		{
			if( isMeshLoaded( name ) )
				return meshes[name];
			
			return NULL;
		}

		inline SoundSet* getSound( const String& name )
		{
			if( isSoundLoaded( name ) )
				return sounds[ name ];

			return NULL;
		}
		
		inline Table* getTable( const String& name )
		{
			TableMap::iterator where = tables.find( name );
			
			if( where != tables.end() )
				return where->second;
			
			return NULL;
		}
						
		void loadSets( const String& folder );		
		void loadFonts( const String& folder );
		void loadMeshes( const String& folder );
		void loadSounds( const String& folder );
		void loadTables( const String& folder );

		void loadResources( const String& folder )
		{
			loadSets( folder );
			loadFonts( folder );
			loadMeshes( folder );
			loadSounds( folder );
			loadTables( folder );
		}
				
		void unloadSets();
		void unloadFonts();
		void unloadMeshes();
		void unloadSounds();
		void unloadTables();
		
		void unloadAll()
		{
			//FONTS DEPEND ON SETS, DO NOT FREE BEFORE
			unloadFonts();
			unloadSets();
			unloadMeshes();
			unloadSounds();
			unloadTables();
		}
		
	protected:
		
		FrameSet* empty;
		
		FrameSetMap frameSets;
		FontMap fonts;
		MeshMap meshes;
		SoundMap sounds;
		TableMap tables;
	};
}

#endif 