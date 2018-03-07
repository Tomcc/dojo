#pragma once

#include "dojo_common_header.h"

#include "Resource.h"
#include "dojomath.h"

namespace Dojo {
	class SoundBuffer;

	///A SoundSet is a collection that contains and abstracts one or more SoundBuffers
	/**the abstraction is useful when it's needed to play a set of sounds randomly, eg, collision sounds.

	To define a SoundSet with more than 1 Buffer in it, it's just needed to name sounds with a numeric tag, ie:
	step_1.ogg
	step_2.ogg
	step_3.ogg */
	class SoundSet : public Resource {
	public:

		///Creates a new set named setName
		SoundSet(optional_ref<ResourceGroup> creator, utf::string_view setName);

		virtual bool onLoad() override;
		virtual void onUnload(bool soft = true) override;

		///returns a random buffer (-1) or the one at index i
		SoundBuffer& getBuffer(int i = -1);

		size_t getResourceNb() {
			return buffers.size();
		}

		utf::string_view getName() {
			return name;
		}

		void addBuffer(std::unique_ptr<SoundBuffer> b);

	private:

		utf::string name;

		std::vector<std::unique_ptr<SoundBuffer>> buffers;
	};
}
