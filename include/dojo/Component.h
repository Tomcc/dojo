#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class Object;

	class Component
	{
	public:
		Object& self;

		Component(Object& self) :
			self(self) {

		}

		Object& getObject() {
			return self;
		}

		const Object& getObject() const {
			return self;
		}

		virtual void onDispose() {}

		///"authorization" method useful to ask the engine to keep the objects alive for longer after disposed of
		virtual bool canDestroy() const {
			return true;
		}
	};
}

