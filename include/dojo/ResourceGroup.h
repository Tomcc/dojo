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

#include "Font.h"
#include "FrameSet.h"
#include "SoundSet.h"
#include "Mesh.h"
#include "Table.h"

#undef RT_FONT

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
			RT_TABLE
		};
				
		//various resource properties TODO: refactor
		bool disableBilinear, disableMipmaps, disableTiling;
		
		typedef std::map<String, FrameSet*> FrameSetMap;
		typedef std::map<String, Font*> FontMap;
		typedef std::map<String, Mesh*> MeshMap;
		typedef std::map<String, SoundSet*> SoundMap;
		typedef std::map<String, Table*> TableMap;
		typedef Array< ResourceGroup* > SubgroupList;
		
		ResourceGroup();
		
		virtual ~ResourceGroup();
		
		///sets the default locale to look for in this resource group
		/**
		A locale is a subfolder with the given name, selectively loaded when a locale is provided
		*/
		void setLocale( const Dojo::String& locID, const Dojo::String& fallbackLocaleID )
		{
			locale = locID;
			fallbackLocale = fallbackLocaleID;
		}
		
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
			for( int i = 0; i < subs.size(); ++i )
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
			DEBUG_ASSERT( !finalized );
			
			frameSets[name] = set;
			
			DEBUG_MESSAGE( "+" << name.ASCII() << "\t\t" << "set" );
		}
		
		inline void addFont( Font* f, const String& name )
		{
			DEBUG_ASSERT( !getFont( name ) );
			DEBUG_ASSERT( !finalized );
			
			fonts[name] = f;
			
			DEBUG_MESSAGE( "+" << name.ASCII() << "\t\t" << "font" );
		}
		
		inline void addMesh( Mesh* m, const String& name )
		{
			DEBUG_ASSERT( !getMesh( name ) );
			DEBUG_ASSERT( !finalized );
			
			meshes[ name ] = m;
			
			DEBUG_MESSAGE( "+" << name.ASCII() << "\t\t" << "mesh" );
		}
		
		inline void addSound( SoundSet* sb, const String& name )
		{
			DEBUG_ASSERT( !getSound( name ) );
			DEBUG_ASSERT( !finalized );
			
			sounds[ name ] = sb;
			
			DEBUG_MESSAGE( "+" << name.ASCII() << "\t\t" << "sound" );
		}
		
		void addTable( Table* t );
		
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
		
		inline const String& getLocale()
		{
			return locale;
		}
		
		inline bool isFinalized()
		{
			return finalized;
		}
		
		inline bool isLocalizationRequired()
		{
			return this->locale.size() > 0;
		}
		
		void loadSets( const String& folder );		
		void loadFonts( const String& folder );
		void loadMeshes( const String& folder );
		void loadSounds( const String& folder );
		void loadTables( const String& folder );
		
		void loadPrefabMeshes();
		
		void loadFolder( const String& folder )
		{
			DEBUG_MESSAGE( "[" << folder.ASCII() << "]" );
			
			loadSets( folder );
			loadFonts( folder );
			loadMeshes( folder );
			loadSounds( folder );
			loadTables( folder );
		}
		
		void loadLocalizedFolder( const String& basefolder )
		{
			String lid = basefolder;
				
			if( lid[ lid.size() - 1 ] != '/' )
				lid += '/';
			
			lid += locale;
			
			loadFolder( lid );
		}

		void loadResources( const String& folder )
		{
			loadFolder( folder );
			
			//localized loading
			if( isLocalizationRequired() )		
				loadLocalizedFolder( folder );
		}
		
		///asserts that this group will not load more resources in the future, useful for task-based loading
		void finalize()
		{
			finalized = true;
		}
				
		inline void unloadSets()
		{	
			unload< FrameSet >( frameSets );
		}
		
		inline void unloadFonts()
		{
			unload< Font >( fonts );
		}
		
		inline void unloadMeshes()
		{
			unload< Mesh >( meshes );
		}
		
		inline void unloadSounds()
		{
			unload< SoundSet >( sounds );
		}
		
		inline void unloadTables()
		{
			unload< Table >( tables );
		}
		
		inline void unloadAll()
		{
			//FONTS DEPEND ON SETS, DO NOT FREE BEFORE
			unloadFonts();
			unloadSets();
			unloadMeshes();
			unloadSounds();
			unloadTables();
		}
		
		inline FrameSetMap::const_iterator getFrameSets() const
		{
			return frameSets.begin();
		}
		inline FrameSetMap::const_iterator getFrameSetsEnd() const 
		{
			return frameSets.end();
		}
		
	protected:
		
		String locale, fallbackLocale;		
		bool finalized;
				
		FrameSet* empty;
		
		FrameSetMap frameSets;
		FontMap fonts;
		MeshMap meshes;
		SoundMap sounds;
		TableMap tables;
		
		void* mapArray[5];
		
		SubgroupList subs;
				
		template <class T> 
		void unload( std::map< String, T* >& map )
		{
			while( !map.empty() )
			{
				DEBUG_MESSAGE( "~" << map.begin()->first.ASCII() );
				
				SAFE_DELETE( map.begin()->second );
				
				map.erase( map.begin() );
			}
		}
	};
}

#endif 