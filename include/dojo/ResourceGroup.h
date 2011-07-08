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
#include <map>

#include "Font.h"
#include "FrameSet.h"
#include "Mesh.h"
#include "SoundSet.h"
#include "Table.h"

namespace Dojo {
	
	class ResourceGroup 
	{			
	public:		
		
		enum ResourceType
		{
			RT_FRAMESET,
			RT_FONT,
			RT_MESH,
			RT_SOUND,
			RT_TABLE,
		};
		
		typedef std::map<String, FrameSet*> FrameSetMap;
		typedef std::map<String, Font*> FontMap;
		typedef std::map<String, Mesh*> MeshMap;
		typedef std::map<String, SoundSet*> SoundMap;
		typedef std::map<String, Table*> TableMap;
		typedef Array< ResourceGroup* > SubgroupList;
		
		ResourceGroup();
		
		virtual ~ResourceGroup();
		
		template < class R >
		inline std::map< String, R* >* getResourceMap( ResourceType r ) const
		{
			return (std::map< String, R* >*)mapArray[ (uint)r ];
		}
		
		
		template < class R >
		inline R* find( const String& name, ResourceType r )
		{
			typedef std::map< String, R* > RMap;
			
			RMap* map = getResourceMap< R >( r );
			
			typename RMap::iterator itr = map->find( name );
			
			if( itr != map->end() )
				return itr->second;
			
			//try in subgroups
			R* f;
			for( uint i = 0; i < subs.size(); ++i )
			{
				f = subs[i]->find< R >( name, r );
				
				if( f )
					return f;
			}
			
			return NULL;
		}
				
		inline void addFrameSet( FrameSet* set, const String& name )
		{
			DEBUG_ASSERT( !getFrameSet( name ) );
			
			frameSets[name] = set;
		}
		
		inline void addFont( Font* f, const String& name )
		{
			DEBUG_ASSERT( !getFont( name ) );
			
			fonts[name] = f;
		}
		
		inline void addMesh( Mesh* m, const String& name )
		{
			DEBUG_ASSERT( !getMesh( name ) );
			
			meshes[ name ] = m;
		}
		
		inline void addSound( SoundSet* sb, const String& name )
		{
			DEBUG_ASSERT( !getSound( name ) );

			sounds[ name ] = sb;
		}
		
		inline void addTable( Table* t )
		{
			DEBUG_ASSERT( t );
			DEBUG_ASSERT( !getTable( t->getName() ) );
			
			tables[ t->getName() ] = t;
		}
		
		inline void addSubgroup( ResourceGroup* g )
		{
			DEBUG_ASSERT( g );
			
			subs.add( g );
		}
		
		inline void removeSubgroup( ResourceGroup* g )
		{
			DEBUG_ASSERT( g );
			
			subs.remove( g );
		}
		
		inline void removeFrameSet( const String& name )
		{
			frameSets.erase( name );
		}
		
		inline void removeFont( const String& name )
		{
			fonts.erase( name );
		}
		
		inline void removeMesh( const String& name )
		{
			meshes.erase( name );
		}
		
		inline void removeSound( const String& name )
		{
			sounds.erase( name );
		}
		
		inline void removeTable( const String& name )
		{
			tables.erase( name );
		}

		inline FrameSet* getEmptyFrameSet()			{	return empty;	}

		inline FrameSet* getFrameSet( const String& name )
		{
			return find< FrameSet >( name, RT_FRAMESET );
		}
		
		inline Font* getFont( const String& name )
		{
			return find< Font >( name, RT_FONT );
		}
		
		inline Mesh* getMesh( const String& name )
		{
			return find< Mesh >( name, RT_MESH );
		}

		inline SoundSet* getSound( const String& name )
		{
			return find< SoundSet >( name, RT_SOUND );
		}
		
		inline Table* getTable( const String& name )
		{
			return find< Table >( name, RT_TABLE );
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
		
		void* mapArray[5];
		
		SubgroupList subs;
		
		
	};
}

#endif 