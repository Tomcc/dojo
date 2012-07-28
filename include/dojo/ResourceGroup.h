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
		
		inline Texture* getTexture( const String& name )
		{
			FrameSet* s = getFrameSet( name );
			
			return s ? s->getFrame(0) : NULL;
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
		
		void loadSets( const String& folder, int version = 0 );		
		void loadFonts( const String& folder, int version = 0 );
		void loadMeshes( const String& folder );
		void loadSounds( const String& folder );
		void loadTables( const String& folder );
		
		void loadPrefabMeshes();
		
		void loadFolder( const String& folder, int version = 0 )
		{
			DEBUG_MESSAGE( "[" << folder.ASCII() << "]" );
			
			loadSets( folder, version );
			loadFonts( folder, version );
			loadMeshes( folder );
			loadSounds( folder );
			loadTables( folder );
		}
		
		void loadLocalizedFolder( const String& basefolder, int version = 0 )
		{
			String lid = basefolder;
				
			if( lid[ lid.size() - 1 ] != '/' )
				lid += '/';
			
			lid += locale;
			
			loadFolder( lid );
		}

		void loadResources( const String& folder, int version = 0 )
		{
			loadFolder( folder, version );
			
			//localized loading
			if( isLocalizationRequired() )		
				loadLocalizedFolder( folder, version );
		}
		
		///asserts that this group will not load more resources in the future, useful for task-based loading
		void finalize()
		{
			finalized = true;
		}
				
		inline void unloadSets( bool softUnload = false )
		{	
			unload< FrameSet >( frameSets, softUnload );
		}
		
		inline void unloadFonts( bool softUnload = false )
		{
			unload< Font >( fonts, softUnload );
		}
		
		inline void unloadMeshes( bool softUnload = false )
		{
			unload< Mesh >( meshes, softUnload );
		}
		
		inline void unloadSounds( bool softUnload = false )
		{
			unload< SoundSet >( sounds, softUnload );
		}
		
		inline void unloadTables( bool softUnload = false )
		{
			unload< Table >( tables, softUnload );
		}
		
		
		///empties the group
		/**
		It can be used to completely purge the resources from memory, 
		or it can also try to soft-unload them (without deleting the actual Dojo resource object),
		It can be used to purge file-based resources from memory when the application is suspended
		*/
		inline void unload( bool softUnload = false )
		{
			//FONTS DEPEND ON SETS, DO NOT FREE BEFORE
			unloadFonts( softUnload );
			unloadSets( softUnload );
			unloadMeshes( softUnload );
			unloadSounds( softUnload );
			unloadTables( softUnload );
		}

		//loads all the resources that are in the group but aren't loaded
		void loadExistingResources()
		{
			DEBUG_TODO;
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
		void unload( std::map< String, T* >& map, bool softUnload )
		{
			//unload all the resources
			typedef std::map< String, T* > ResourceMap;
			ResourceMap::iterator itr = map.begin();
			ResourceMap::iterator end = map.end();
			for( ; itr != end; ++itr )
			{
				//unload either if reloadable or if we're purging memory
				if( itr->second->isReloadable() || !softUnload )
					itr->second->unload();

				//delete too?
				if( !softUnload )
				{
					DEBUG_MESSAGE( "~" << map.begin()->first.ASCII() );
					SAFE_DELETE( itr->second );
				}
			}

			if( !softUnload )
				map.clear();
		}
	};
}

#endif 