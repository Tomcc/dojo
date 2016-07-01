#pragma once

#include "dojo_common_header.h"

#include "Platform.h"

namespace Dojo {
	class FontSystem {
	public:

		typedef std::map<utf::string, FT_Face, utf::str_less> FaceMap;

		FontSystem();

		virtual ~FontSystem();

		FT_Face getFace(utf::string_view fileName);

		FT_Stroker getStroker(float width);

	private:

		FaceMap faceMap;
		std::vector<std::vector<uint8_t>> ownedBuffers;

		FT_Library freeType;

		FT_Face _createFaceForFile(utf::string_view fileName);

	private:
	};
}
