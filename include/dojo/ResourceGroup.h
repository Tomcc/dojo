/*
 *  ResourceGroup.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/27/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Font.h"
#include "FrameSet.h"
#include "SoundSet.h"
#include "Mesh.h"
#include "Table.h"
#include "Shader.h"
#include "ShaderProgram.h"
#include "Log.h"

#undef RT_FONT

namespace Dojo 
{
	///A ResourceGroup manages all of the Resources in Dojo
	/** 
	Resources and folders are first added to a ResourceGroup via add* methods, but they are NOT loaded;
	actual loading happens when loadResources() is called.
	This allows to unload and reload the resources without breaking the game's state, by keeping the "empty" Resource objects as placeholders.

	A ResourceGroup will load all the Tables, FrameSets, Sounds, Fonts and Meshes found in the folder that are added to it,
	and individual Resources are referenced by their name, eg: "data/graphics/ninja.png" is retrieved with getFrameSet( "ninja" )
	
	A ResourceGroup can be attached to one or more "sub" ResourceGroups to share their resources. */
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
			RT_SHADER,
			RT_PROGRAM,

			_RT_COUNT
		};
				
		//various resource properties TODO: refactor
		bool disableBilinear, disableMipmaps, disableTiling, logchanges = true;
		
		typedef std::unordered_map<String, FrameSet*> FrameSetMap;
		typedef std::unordered_map<String, Font*> FontMap;
		typedef std::unordered_map<String, Mesh*> MeshMap;
		typedef std::unordered_map<String, SoundSet*> SoundMap;
		typedef std::unordered_map<String, Table*> TableMap;
		typedef std::unordered_map<String, Shader* > ShaderMap;
		typedef std::unordered_map<String, ShaderProgram* > ProgramMap;
		typedef Array< ResourceGroup* > SubgroupList;
		
		///Create a new empty ResourceGroup
		ResourceGroup();
		
		virtual ~ResourceGroup();
		
		///sets the default locale to look for in this resource group
		/**
		A locale is a subfolder with the given name, selectively loaded when a locale is provided
		*/
		void setLocale( const String& locID, const String& fallbackLocaleID )
		{
			DEBUG_ASSERT( locID.size(), "setLocale: the locale was an empty string" );
			DEBUG_ASSERT( fallbackLocaleID.size(), "setLocale: the fallback locale was an empty string" );

			locale = locID;
			fallbackLocale = fallbackLocaleID;
		}
		
		///returns the map containing the required resource type
		template < class R >
		std::unordered_map< String, R* >* getResourceMap( ResourceType r ) const
		{
			return (std::unordered_map< String, R* >*)mapArray[ (int)r ];
		}
		
		///finds a named resource of type R
		template < class R >
		R* find( const String& name, ResourceType r ) const
		{
			typedef std::unordered_map< String, R* > RMap;
			
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
		
		void addFrameSet( FrameSet* set, const String& name )
		{
			DEBUG_ASSERT_INFO( !getFrameSet( name ), "A FrameSet with this name already exists", "name = " + name );
			DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );
			
			frameSets[name] = set;
			
			if (logchanges)
				DEBUG_MESSAGE("+" + name + "\t\t set");
		}
		
		void addFont( Font* f, const String& name )
		{
			DEBUG_ASSERT_INFO( !getFont( name ), "A Sound with this name already exists", "name = " + name );
			DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );
			
			fonts[name] = f;
			
			if (logchanges)
				DEBUG_MESSAGE("+" + name + "\t\t font");
		}
		
		void addMesh( Mesh* m, const String& name )
		{
			DEBUG_ASSERT_INFO( !getMesh( name ), "A Mesh with this name already exists", "name = " + name );
			DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );
			
			meshes[ name ] = m;
			
			if (logchanges)
				DEBUG_MESSAGE("+" + name + "\t\t mesh");
		}
		
		void addSound( SoundSet* sb, const String& name )
		{
			DEBUG_ASSERT_INFO( !getSound( name ), "A Sound with this name already exists", "name = " + name );
			DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );
			
			sounds[ name ] = sb;
			
			if (logchanges)
				DEBUG_MESSAGE("+" + name + "\t\t sound");
		}
		
		void addTable( const String& name, Unique<Table> t );

		///adds an existing Shader to this group
		void addShader( Shader* s, const String& name )
		{
			DEBUG_ASSERT_INFO( !getShader( name ), "A Shader with this name already exists", "name = " + name );
			DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );

			shaders[ name ] = s;

			if (logchanges)
				DEBUG_MESSAGE("+" + name + "\t\t shader");
		}

		///adds an existing ShaderProgram to this group
		void addProgram( ShaderProgram* sp, const String& name )
		{
			DEBUG_ASSERT_INFO( !getProgram( name ), "A ShaderProgram with this name already exists", "name = " + name );
			DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );

			programs[ name ] = sp;

			if (logchanges)
				DEBUG_MESSAGE("+" + name + "\t\t shader program");
		}
		
		///adds a ResourceGroup as an additional subgroup where to look for Resources
		void addSubgroup( ResourceGroup* g )
		{
			DEBUG_ASSERT( g != nullptr, "Adding a null subgroup" );
			
			subs.add( g );
		}
		
		///removes a subgroup
		void removeSubgroup( ResourceGroup* g )
		{
			DEBUG_ASSERT( g != nullptr, "Removing a null subgroup" );
			
			subs.remove( g );
		}

		///removes all of the registered subgrops from this ResourceGroup
		void removeAllSubgroups()
		{
			subs.clear();
		}
		
		void removeFrameSet( const String& name )
		{
			frameSets.erase( name );
		}
		
		void removeFont( const String& name )
		{
			fonts.erase( name );
		}
		
		void removeMesh( const String& name )
		{
			meshes.erase( name );
		}
		
		void removeSound( const String& name )
		{
			sounds.erase( name );
		}
		
		void removeTable( const String& name )
		{
			tables.erase( name );
		}

		///returns a dummy empty FrameSet
		FrameSet* getEmptyFrameSet() const 
		{	
			return empty;	
		}

		FrameSet* getFrameSet( const String& name ) const
		{
			DEBUG_ASSERT( name.size(), "getFrameSet: empty name provided" );

			return find< FrameSet >( name, RT_FRAMESET );
		}
		
		Texture* getTexture(const String& name) const
		{
			FrameSet* s = getFrameSet( name );
			
			return s ? s->getFrame(0) : NULL;
		}
		
		Font* getFont(const String& name) const
		{
			DEBUG_ASSERT( name.size(), "empty name provided" );
			return find< Font >( name, RT_FONT );
		}
		
		Mesh* getMesh(const String& name) const
		{
			DEBUG_ASSERT( name.size(), "empty name provided" );
			return find< Mesh >( name, RT_MESH );
		}

		SoundSet* getSound( const String& name ) const
		{
			DEBUG_ASSERT( name.size(), "empty name provided" );
			return find< SoundSet >( name, RT_SOUND );
		}
		
		Table* getTable(const String& name) const
		{
			DEBUG_ASSERT( name.size(), "empty name provided" );
			return find< Table >( name, RT_TABLE );
		}

		Shader* getShader(const String& name) const
		{
			DEBUG_ASSERT( name.size(), "empty name provided" );
			return find< Shader >( name, RT_SHADER );
		}

		ShaderProgram* getProgram(const String& name) const
		{
			DEBUG_ASSERT( name.size(), "empty name provided" );
			return find< ShaderProgram >( name, RT_PROGRAM );
		}
		
		///return the locale of this ResourceGroup, eg: en, it, de, se
		const String& getLocale() const
		{
			return locale;
		}

		///returns if this group is finalized, meaning that its loading is finished
		/**\remark useful for loading subgroups in the background! */
		bool isFinalized() const
		{
			return finalized;
		}
		
		///true if localization-specific folders will be added too when adding a folder
		bool isLocalizationRequired() const
		{
			return locale.size() > 0;
		}
		
		///add all the Sets in a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addSets( const String& folder, int version = 0 );		
		///add all the Fonts in a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addFonts( const String& folder, int version = 0 );
		///add all the Meshes in a folder
		void addMeshes( const String& folder );
		///add all the Sounds in a folder
		void addSounds( const String& folder );
		///add all the Tables in a folder
		void addTables( const String& folder );
		///add all the Shaders (.dsh) in a folder
		void addShaders( const String& folder );
		///add all the ShaderPrograms (.vsh, .psh, ...) in a folder
		void addPrograms( const String& folder );
		
		///adds the prefab meshes, like quads, cubes, skyboxes...
		void addPrefabMeshes();
		
		///adds all the file inside a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addFolderSimple( const String& folder, int version = 0 )
		{
			if (logchanges)
				DEBUG_MESSAGE( "[" + folder + "]" );
			
			addSets( folder, version );
			addFonts( folder, version );
			addMeshes( folder );
			addSounds( folder );
			addTables( folder );
			addPrograms( folder );
			addShaders( folder );
		}
		
		///adds a localization folder located in baseFolder, choosing it using the current locale
		/** 
		for example, "base/en" if en; "base/it" if it, etc */
		void addLocalizedFolder( const String& basefolder, int version = 0 );

		///adds all the resources and the localized resources in a folder
		void addFolder( const String& folder, int version = 0 )
		{
			addFolderSimple( folder, version );
			
			//localized loading
			if( isLocalizationRequired() )		
				addLocalizedFolder( folder, version );
		}
		
		///asserts that this group will not load more resources in the future, useful for task-based loading
		void finalize()
		{
			finalized = true;
		}

		///loads all the resources that are in the group but aren't loaded
		void loadResources( bool recursive = false )
		{
			_load< FrameSet >( frameSets );
			_load< Font >( fonts );
			_load< Mesh >( meshes );
			_load< SoundSet >( sounds );
			_load< Table >( tables );
			_load< ShaderProgram >( programs );
			_load< Shader >( shaders );

			//load sets again to load missing atlases!
			_load< FrameSet >( frameSets );

			if( recursive)
				for( int i = 0; i < subs.size(); ++i )	subs[i]->loadResources( recursive );
		}

		///empties the group destroying all the resources
		void unloadResources( bool recursive = false )
		{
			//FONTS DEPEND ON SETS, DO NOT FREE BEFORE
			_unload< Font >( fonts, false );
			_unload< FrameSet >( frameSets, false );
			_unload< Mesh >( meshes, false );
			_unload< SoundSet >( sounds, false );
			_unload< Table >( tables, false );
			_unload< Shader >( shaders, false );
			_unload< ShaderProgram >( programs, false );

			if( recursive )
				for( int i = 0; i < subs.size(); ++i )	subs[i]->unloadResources( recursive );
		}

		///unloads re-loadable resources without actually destroying resource objects
		void softUnloadResources( bool recursive = false )
		{
			_unload< Font >( fonts, true );
			_unload< FrameSet >( frameSets, true );
			_unload< Mesh >( meshes, true );
			_unload< SoundSet >( sounds, true );
			_unload< Table >( tables, true );
			_unload< ShaderProgram >( programs, true );
			_unload< Shader >( shaders, true );

			if( recursive )
				for( int i = 0; i < subs.size(); ++i )	subs[i]->softUnloadResources( recursive );
		}
			
		FrameSetMap::const_iterator getFrameSets() const
		{
			return frameSets.begin();
		}
		FrameSetMap::const_iterator getFrameSetsEnd() const 
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
		ShaderMap shaders;
		ProgramMap programs;
		
		void* mapArray[ _RT_COUNT ];
		
		SubgroupList subs;

		///load all unloaded registered resources
		template< class T>
		void _load( std::unordered_map< String, T* >& map )
		{
			for( auto resourcePair : map )
			{
				//unload either if reloadable or if we're purging memory
				if( !resourcePair.second->isLoaded() )
					resourcePair.second->onLoad();
			}
		}
				
		template <class T> 
		void _unload( std::unordered_map< String, T* >& map, bool softUnload )
		{
			//unload all the resources
			for( auto resourcePair : map )
			{
				//unload either if reloadable or if we're purging memory
                if(resourcePair.second->isLoaded() )
                    resourcePair.second->onUnload( softUnload );

				//delete too?
				if( !softUnload )
				{	
					if (logchanges)
						DEBUG_MESSAGE("-" + resourcePair.first);
					SAFE_DELETE( resourcePair.second );
				}
				else if( !resourcePair.second->isLoaded() )
				{
					if (logchanges)
						DEBUG_MESSAGE("~" + resourcePair.first);
				}
			}

			if( !softUnload )
				map.clear();
		}
	};
}
