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
			virtual void onLoad(Resource*) =0;
		};

		Resource(ResourceGroup* group = NULL) :
			creator(group),
			loaded(false),
			size(0),
			pDataProvider(NULL) {

		}

		Resource(ResourceGroup* creatorGroup, const std::string& path) :
			creator(creatorGroup),
			loaded(false),
			size(0),
			filePath(path),
			pDataProvider(NULL) {
			DEBUG_ASSERT( path.size(), "The file path is empty" );
		}

		Resource(ResourceGroup* group, DataProvider* source) :
			creator(group),
			loaded(false),
			size(0),
			pDataProvider(source) {
			DEBUG_ASSERT( source, "The DataProvider is NULL" );
		}

		virtual ~Resource() {
			//must be unloaded at this point
			DEBUG_ASSERT( loaded == false, "A Resource was destroyed without being unloaded before (resource leak!)" );
		}

		virtual bool onLoad() =0;
		virtual void onUnload(bool soft = false) =0;

		bool isLoaded() {
			return loaded;
		}

		int getByteSize() {
			return size;
		}

		ResourceGroup* getCreator() {
			return creator;
		}

		const std::string& getFilePath() {
			return filePath;
		}

		DataProvider* getDataProvider() {
			return pDataProvider;
		}

		bool isFiledBased() {
			return filePath.size() > 0;
		}

		bool isReloadable() {
			return isFiledBased() || getDataProvider();
		}

	protected:

		ResourceGroup* creator;

		bool loaded;
		int size;

		std::string filePath;
		DataProvider* pDataProvider;
	};
}
