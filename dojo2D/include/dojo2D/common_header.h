#pragma once

#include <dojo.h>

#include <Box2D/Box2D.h>
#include <unordered_set>
#include <mutex>

namespace Phys {

	struct Group : public Dojo::PseudoEnumClass<uint16_t> {
		static const Group None;

		template<typename T>
		Group(T raw) : PseudoEnumClass(raw) {}

		bool operator==(const Group& rhs) const {
			return value == rhs.value;
		}
	};

	//some reexports
	using Vector = Dojo::Vector;
	using Radians = Dojo::Radians;
	using Degrees = Dojo::Degrees;
	
	template<typename T>
	using optional_ref = Dojo::optional_ref<T>;

	typedef std::lock_guard<std::mutex> ScopedLock;

	namespace ComponentID {
		enum Enum {
			Body = Dojo::ComponentID::_count,
			ParticleSystemRenderer,
			_count
		};
	}
}
