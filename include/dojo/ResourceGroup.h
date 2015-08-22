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

namespace Dojo {
	///A ResourceGroup manages all of the Resources in Dojo
	/**
	Resources and folders are first added to a ResourceGroup via add* methods, but they are NOT loaded;
	actual loading happens when loadResources() is called.
	This allows to unload and reload the resources without breaking the game's state, by keeping the "empty" Resource objects as placeholders.

	A ResourceGroup will load all the Tables, FrameSets, Sounds, Fonts and Meshes found in the folder that are added to it,
	and individual Resources are referenced by their name, eg: "data/graphics/ninja.png" is retrieved with getFrameSet( "ninja" )

	A ResourceGroup can be attached to one or more "sub" ResourceGroups to share their resources. */
	class ResourceGroup {
	public:

		enum ResourceType {
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

		typedef std::unordered_map<utf::string, Unique<FrameSet>> FrameSetMap;
		typedef std::unordered_map<utf::string, Unique<Font>> FontMap;
		typedef std::unordered_map<utf::string, Unique<Mesh>> MeshMap;
		typedef std::unordered_map<utf::string, Unique<SoundSet>> SoundMap;
		typedef std::unordered_map<utf::string, Unique<Table>> TableMap;
		typedef std::unordered_map<utf::string, Unique<Shader>> ShaderMap;
		typedef std::unordered_map<utf::string, Unique<ShaderProgram>> ProgramMap;
		typedef SmallSet<ResourceGroup*> SubgroupList;

		///Create a new empty ResourceGroup
		ResourceGroup();

		virtual ~ResourceGroup();

		///sets the default locale to look for in this resource group
		/**
		A locale is a subfolder with the given name, selectively loaded when a locale is provided
		*/
		void setLocale(const utf::string& locID, const utf::string& fallbackLocaleID) {
			DEBUG_ASSERT( locID.size(), "setLocale: the locale was an empty string" );
			DEBUG_ASSERT( fallbackLocaleID.size(), "setLocale: the fallback locale was an empty string" );

			locale = locID;
			fallbackLocale = fallbackLocaleID;
		}

		///returns the map containing the required resource type
		template <class R>
		std::unordered_map<utf::string, R*>* getResourceMap(ResourceType r) const {
			return (std::unordered_map<utf::string, R*>*)mapArray[(int)r];
		}

		///finds a named resource of type R
		template <class R>
		R* find(const utf::string& name, ResourceType r) const {
			typedef std::unordered_map<utf::string, R*> RMap;

			RMap* map = getResourceMap<R>(r);

			typename RMap::iterator itr = map->find(name);

			if (itr != map->end()) {
				return itr->second;
			}

			//try in subgroups
			R* f;

			for (auto&& sub : subs) {
				f = sub->find<R>(name, r);

				if (f) {
					return f;
				}
			}

			return nullptr;
		}

		FrameSet& addFrameSet(Unique<FrameSet> resource, const utf::string& name);

		void addFont(Unique<Font> resource, const utf::string& name);

		void addMesh(Unique<Mesh> resource, const utf::string& name);

		SoundSet& addSoundSet(Unique<SoundSet> resource, const utf::string& name);

		void addTable(const utf::string& name, Unique<Table> t);

		///adds an existing Shader to this group
		void addShader(Unique<Shader> resource, const utf::string& name);

		///adds an existing ShaderProgram to this group
		void addProgram(Unique<ShaderProgram> resource, const utf::string& name);

		///adds a ResourceGroup as an additional subgroup where to look for Resources
		void addSubgroup(ResourceGroup& g);

		///removes a subgroup
		void removeSubgroup(ResourceGroup& g);

		///removes all of the registered subgrops from this ResourceGroup
		void removeAllSubgroups();

		void removeFrameSet(const utf::string& name);
		void removeFont(const utf::string& name);
		void removeMesh(const utf::string& name);
		void removeSound(const utf::string& name);
		void removeTable(const utf::string& name);

		///returns a dummy empty FrameSet
		FrameSet& getEmptyFrameSet() const;
		FrameSet* getFrameSet(const utf::string& name) const;
		Texture* getTexture(const utf::string& name) const;
		Font* getFont(const utf::string& name) const;
		Mesh* getMesh(const utf::string& name) const;
		SoundSet* getSound(const utf::string& name) const;
		Table* getTable(const utf::string& name) const;
		Shader* getShader(const utf::string& name) const;
		ShaderProgram* getProgram(const utf::string& name) const;

		///return the locale of this ResourceGroup, eg: en, it, de, se
		const utf::string& getLocale() const;

		///returns if this group is finalized, meaning that its loading is finished
		/**\remark useful for loading subgroups in the background! */
		bool isFinalized() const {
			return finalized;
		}

		///true if localization-specific folders will be added too when adding a folder
		bool isLocalizationRequired() const {
			return locale.size() > 0;
		}

		///add all the Sets in a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addSets(const utf::string& folder, int version = 0);
		///add all the Fonts in a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addFonts(const utf::string& folder, int version = 0);
		///add all the Meshes in a folder
		void addMeshes(const utf::string& folder);
		///add all the Sounds in a folder
		void addSounds(const utf::string& folder);
		///add all the Tables in a folder
		void addTables(const utf::string& folder);
		///add all the Shaders (.dsh) in a folder
		void addShaders(const utf::string& folder);
		///add all the ShaderPrograms (.vsh, .psh, ...) in a folder
		void addPrograms(const utf::string& folder);

		///adds the prefab meshes, like quads, cubes, skyboxes...
		void addPrefabMeshes();

		///adds all the file inside a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addFolderSimple(const utf::string& folder, int version = 0);
		///adds a localization folder located in baseFolder, choosing it using the current locale
		/**
		for example, "base/en" if en; "base/it" if it, etc */
		void addLocalizedFolder(const utf::string& basefolder, int version = 0);

		///adds all the resources and the localized resources in a folder
		void addFolder(const utf::string& folder, int version = 0);

		///asserts that this group will not load more resources in the future, useful for task-based loading
		void finalize() {
			finalized = true;
		}

		///loads all the resources that are in the group but aren't loaded
		void loadResources(bool recursive = false);

		///empties the group destroying all the resources
		void unloadResources(bool recursive = false);

		///unloads re-loadable resources without actually destroying resource objects
		void softUnloadResources(bool recursive = false);

		FrameSetMap::const_iterator getFrameSets() const {
			return frameSets.begin();
		}

		FrameSetMap::const_iterator getFrameSetsEnd() const {
			return frameSets.end();
		}

	protected:

		utf::string locale, fallbackLocale;
		bool finalized;

		Unique<FrameSet> emptyFrameSet;

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
		template <class T>
		void _load(std::unordered_map<utf::string, Unique<T>>& map) {
			for (auto&& resourcePair : map) {
				//unload either if reloadable or if we're purging memory
				if (!resourcePair.second->isLoaded()) {
					resourcePair.second->onLoad();
				}
			}
		}

		template <class T>
		void _unload(std::unordered_map<utf::string, Unique<T>>& map, bool softUnload) {
			//unload all the resources
			for (auto&& resourcePair : map) {
				//unload either if reloadable or if we're purging memory
				if (resourcePair.second->isLoaded()) {
					resourcePair.second->onUnload(softUnload);
				}

				//delete too?
				if (!softUnload) {
					if (logchanges) {
						DEBUG_MESSAGE("-" + resourcePair.first);
					}
				}
				else if (softUnload && !resourcePair.second->isLoaded()) {
					if (logchanges) {
						DEBUG_MESSAGE("~" + resourcePair.first);
					}
				}
			}

			if (!softUnload) {
				map.clear();
			}
		}
	};
}
