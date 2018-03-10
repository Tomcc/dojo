#pragma once

#include "common_header.h"

namespace Phys {
	class Material {
	public:
		enum State {
			Gas,
			Fluid,
			Solid
		};

		const utf::string name;
		float density = 1.f;

		//fluid parameters
		float viscosity = 1.f;
		float pressure = 0.05f;

		//solid parameters
		float friction = 0.5f;
		float restitution = 0.f;

		State state = Solid;

		optional_ref<Dojo::SoundSet> impactHard, impactSoft;

		Material(utf::string_view name, const Dojo::Table& desc = Dojo::Table::Empty, optional_ref<const Dojo::ResourceGroup> group = {});
		Material(const Material&) = delete;
		Material& operator=(const Material&) = delete;
	};
}



