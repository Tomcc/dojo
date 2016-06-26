#pragma once

#include "dojo_common_header.h"

#include "Platform.h"

namespace Dojo {
	class FontSystem {
	public:

		typedef std::unordered_map<utf::string, FT_Face> FaceMap;

		FontSystem();

		virtual ~FontSystem();

		FT_Face getFace(const utf::string& fileName);

		FT_Stroker getStroker(float width);

	private:

		FaceMap faceMap;
		std::vector<std::vector<uint8_t>> ownedBuffers;

		FT_Library freeType;

		FT_Face _createFaceForFile(const utf::string& fileName);

	private:
	};
}
