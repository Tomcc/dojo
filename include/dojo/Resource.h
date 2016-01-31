/*
 *  Resource.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/10/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class ResourceGroup;

	class Resource {
	public:

		///Resource::DataProvider is a virtual interface for resource loading
		/**
		It allows non-file-baseed resources to be unloaded and reloaded on the fly
		*/
		class DataProvider {
		public:

			///onLoad is called when a registered resource needs to be loaded
			virtual void onLoad(Resource*) = 0;
		};

		Resource(optional_ref<ResourceGroup> group = {}) :
			creator(group),
			loaded(false),
			size(0),
			pDataProvider(nullptr) {

		}

		Resource(optional_ref<ResourceGroup> group, const utf::string& path) :
			creator(group),
			loaded(false),
			size(0),
			filePath(path),
			pDataProvider(nullptr) {
			DEBUG_ASSERT( path.not_empty(), "The file path is empty" );
		}

		Resource(optional_ref<ResourceGroup> group, DataProvider& source) :
			creator(group),
			loaded(false),
			size(0),
			pDataProvider(source) {

		}

		virtual ~Resource() {
			//must be unloaded at this point
			DEBUG_ASSERT( loaded == false, "A Resource was destroyed without being unloaded before (resource leak!)" );
		}

		virtual bool onLoad() = 0;
		virtual void onUnload(bool soft = false) = 0;

		bool isLoaded() const {
			return loaded;
		}

		int getByteSize() {
			return size;
		}

		optional_ref<ResourceGroup> getCreator() {
			return creator;
		}

		const utf::string& getFilePath() {
			return filePath;
		}

		optional_ref<DataProvider> getDataProvider() {
			return pDataProvider;
		}

		bool isFiledBased() {
			return filePath.not_empty();
		}

		bool isReloadable() {
			return isFiledBased() || getDataProvider().is_some();
		}

	protected:

		optional_ref<ResourceGroup> creator;

		bool loaded;
		int size;

		utf::string filePath;
		optional_ref<DataProvider> pDataProvider;
	};
}
