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

		enum class ResourceType {
			FrameSet,
			Font,
			Mesh,
			SoundSet,
			Table,
			Material,
			ShaderProgram,

			_count
		};

		//various resource properties TODO: refactor
		bool disableBilinear, disableMipmaps, disableTiling, logchanges = true;

		typedef std::map<utf::string, std::unique_ptr<FrameSet>, utf::str_less> FrameSetMap;
		typedef std::map<utf::string, std::unique_ptr<Font>, utf::str_less> FontMap;
		typedef std::map<utf::string, std::unique_ptr<Mesh>, utf::str_less> MeshMap;
		typedef std::map<utf::string, std::unique_ptr<SoundSet>, utf::str_less> SoundMap;
		typedef std::map<utf::string, std::unique_ptr<Table>, utf::str_less> TableMap;
		typedef std::map<utf::string, std::unique_ptr<Shader>, utf::str_less> ShaderMap;
		typedef std::map<utf::string, std::unique_ptr<ShaderProgram>, utf::str_less> ProgramMap;
		typedef SmallSet<ResourceGroup*> SubgroupList;

		///Create a new empty ResourceGroup
		ResourceGroup();

		virtual ~ResourceGroup();

		///sets the default locale to look for in this resource group
		/**
		A locale is a subfolder with the given name, selectively loaded when a locale is provided
		*/
		void setLocale(utf::string_view locID, utf::string_view fallbackLocaleID) {
			DEBUG_ASSERT( locID.not_empty(), "setLocale: the locale was an empty string" );
			DEBUG_ASSERT( fallbackLocaleID.not_empty(), "setLocale: the fallback locale was an empty string" );

			locale = locID.copy();
			fallbackLocale = fallbackLocaleID.copy();
		}

		///returns the map containing the required resource type
		template <class R>
		std::map<utf::string, R*, utf::str_less>& getResourceMap(ResourceType r) const {
			return *(std::map<utf::string, R*, utf::str_less>*)mapArray[enum_cast(r)];
		}

		///finds a named resource of type R
		template <class R>
		optional_ref<R> find(utf::string_view name, ResourceType r) const {
			auto& map = getResourceMap<R>(r);
			auto itr = map.find(name);

			if (itr != map.end()) {
				return *itr->second;
			}

			//try in subgroups too
			for (auto&& sub : subs) {
				if (auto f = sub->find<R>(name, r).to_ref()) {
					return f.get();
				}
			}

			return{};
		}

		FrameSet& addFrameSet(std::unique_ptr<FrameSet> resource, utf::string_view name);
		Texture& addTexture(std::unique_ptr<Texture> texture, utf::string_view name);

		Font& addFont(std::unique_ptr<Font> resource, utf::string_view name);

		Mesh& addMesh(std::unique_ptr<Mesh> resource, utf::string_view name);

		SoundSet& addSoundSet(std::unique_ptr<SoundSet> resource, utf::string_view name);

		Table& addTable(utf::string_view name, std::unique_ptr<Table> t);

		///adds an existing Shader to this group
		Shader& addShader(std::unique_ptr<Shader> resource, utf::string_view name);

		///adds an existing ShaderProgram to this group
		ShaderProgram& addProgram(std::unique_ptr<ShaderProgram> resource, utf::string_view name);

		///adds a ResourceGroup as an additional subgroup where to look for Resources
		void addSubgroup(ResourceGroup& g);

		///removes a subgroup
		void removeSubgroup(ResourceGroup& g);

		///removes all of the registered subgrops from this ResourceGroup
		void removeAllSubgroups();

		void removeFrameSet(utf::string_view name);
		void removeFont(utf::string_view name);
		void removeMesh(utf::string_view name);
		void removeSound(utf::string_view name);
		void removeTable(utf::string_view name);

		///returns a dummy empty FrameSet
		FrameSet& getEmptyFrameSet() const;
		optional_ref<FrameSet> getFrameSet(utf::string_view name) const;
		optional_ref<Texture> getTexture(utf::string_view name) const;
		optional_ref<Font> getFont(utf::string_view name) const;
		optional_ref<Mesh> getMesh(utf::string_view name) const;
		optional_ref<SoundSet> getSound(utf::string_view name) const;
		optional_ref<Table> getTable(utf::string_view name) const;
		optional_ref<Shader> getShader(utf::string_view name) const;
		optional_ref<ShaderProgram> getProgram(utf::string_view name) const;

		///return the locale of this ResourceGroup, eg: en, it, de, se
		utf::string_view getLocale() const;

		///returns if this group is finalized, meaning that its loading is finished
		/**\remark useful for loading subgroups in the background! */
		bool isFinalized() const {
			return finalized;
		}

		///true if localization-specific folders will be added too when adding a folder
		bool isLocalizationRequired() const {
			return locale.not_empty();
		}

		///add all the Sets in a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addSets(utf::string_view folder, int version = 0);
		///add all the Fonts in a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addFonts(utf::string_view folder, int version = 0);
		///add all the Meshes in a folder
		void addMeshes(utf::string_view folder);
		///add all the Sounds in a folder
		void addSounds(utf::string_view folder);
		///add all the Tables in a folder
		void addTables(utf::string_view folder);
		///add all the Shaders (.dsh) in a folder
		void addShaders(utf::string_view folder);
		///add all the ShaderPrograms (.vsh, .psh, ...) in a folder
		void addPrograms(utf::string_view folder);

		///adds the prefab meshes, like quads, cubes, skyboxes...
		void addPrefabMeshes();

		///adds all the file inside a folder
		/**\param version the version of the assets to be loaded, eg ninja@0.png or ninja@1.png
		\remark all the assets without a version are by default version 0*/
		void addFolderSimple(utf::string_view folder, int version = 0);
		///adds a localization folder located in baseFolder, choosing it using the current locale
		/**
		for example, "base/en" if en; "base/it" if it, etc */
		void addLocalizedFolder(utf::string_view basefolder, int version = 0);

		///adds all the resources and the localized resources in a folder
		void addFolder(utf::string_view folder, int version = 0);

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

	private:

		utf::string locale, fallbackLocale;
		bool finalized;

		std::unique_ptr<FrameSet> emptyFrameSet;

		FrameSetMap frameSets;
		FontMap fonts;
		MeshMap meshes;
		SoundMap sounds;
		TableMap tables;
		ShaderMap shaders;
		ProgramMap programs;

		void* mapArray[ enum_cast(ResourceType::_count) ];

		SubgroupList subs;

		///load all unloaded registered resources
		template <class T>
		void _load(std::map<utf::string, std::unique_ptr<T>, utf::str_less>& map) {
			for (auto&& resourcePair : map) {
				//unload either if reloadable or if we're purging memory
				if (not resourcePair.second->isLoaded()) {
					resourcePair.second->onLoad();
				}
			}
		}

		template <class T>
		void _unload(std::map<utf::string, std::unique_ptr<T>, utf::str_less>& map, bool softUnload) {
			//unload all the resources
			for (auto&& resourcePair : map) {
				//unload either if reloadable or if we're purging memory
				if (resourcePair.second->isLoaded()) {
					resourcePair.second->onUnload(softUnload);
				}

				//delete too?
				if (not softUnload) {
					if (logchanges) {
						DEBUG_MESSAGE("-" + resourcePair.first);
					}
				}
				else if (softUnload and not resourcePair.second->isLoaded()) {
					if (logchanges) {
						DEBUG_MESSAGE("~" + resourcePair.first);
					}
				}
			}

			if (not softUnload) {
				map.clear();
			}
		}
	};
}
