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

		Resource(optional_ref<ResourceGroup> group = {}) :
			creator(group),
			loaded(false),
			size(0) {

		}

		Resource(optional_ref<ResourceGroup> group, const utf::string& path) :
			creator(group),
			loaded(false),
			size(0),
			filePath(path) {
			DEBUG_ASSERT( path.not_empty(), "The file path is empty" );
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

		bool isReloadable() {
			return filePath.not_empty();
		}

	protected:
		bool loaded;

		optional_ref<ResourceGroup> creator;

		int size;

		utf::string filePath;
	};
}
