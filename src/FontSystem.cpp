#include "FontSystem.h"

using namespace Dojo;

FontSystem::FontSystem() {
	//launch FreeType
	int err = FT_Init_FreeType(&freeType);

	DEBUG_ASSERT(err == 0, "FreeType failed to initialize");
}

FontSystem::~FontSystem() {
	FT_Done_FreeType(freeType);
}

FT_Face FontSystem::getFace(utf::string_view fileName) {
	auto where = faceMap.find(fileName);
	return where != faceMap.end() ? where->second : _createFaceForFile(fileName);
}

FT_Stroker FontSystem::getStroker(float width) {
	FT_Stroker s;
	FT_Stroker_New(freeType, &s);

	FT_Stroker_Set(s,
		(FT_Fixed)(width * 64.f),
		FT_STROKER_LINECAP_ROUND,
		FT_STROKER_LINEJOIN_ROUND,
		0);

	return s;
}

FT_Face FontSystem::_createFaceForFile(utf::string_view fileName) {
	auto buf = Platform::singleton().loadFileContent(fileName);

	//create new face from memory - loading from memory is needed for zip loading
	FT_Face face;
	auto err = FT_New_Memory_Face(freeType, (FT_Byte*)buf.data(), static_cast<FT_Long>(buf.size()), 0, &face);
	faceMap.emplace(fileName.copy(), face);
	ownedBuffers.emplace_back(std::move(buf)); //keep the memory

	DEBUG_ASSERT_INFO(err == 0, "FreeType could not load a Font file", "path = " + fileName);

	return face;
}
