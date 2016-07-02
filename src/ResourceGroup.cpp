#include "ResourceGroup.h"

#include "Platform.h"
#include "Timer.h"
#include "FrameSet.h"
#include "Mesh.h"
#include "Font.h"
#include "Table.h"
#include "SoundSet.h"
#include "SoundBuffer.h"

#include <Poco/File.h>
#include "Texture.h"
#include "Path.h"

using namespace Dojo;

ResourceGroup::ResourceGroup() :
	finalized(false),
	disableBilinear(false),
	disableMipmaps(false),
	disableTiling(false) {
	//link map array
	mapArray[enum_cast(ResourceType::FrameSet)] = &frameSets;
	mapArray[enum_cast(ResourceType::Font)] = &fonts;
	mapArray[enum_cast(ResourceType::Mesh)] = &meshes;
	mapArray[enum_cast(ResourceType::SoundSet)] = &sounds;
	mapArray[enum_cast(ResourceType::Table)] = &tables;
	mapArray[enum_cast(ResourceType::Material)] = &shaders;
	mapArray[enum_cast(ResourceType::ShaderProgram)] = &programs;

	emptyFrameSet = make_unique<FrameSet>(self);
}

ResourceGroup::~ResourceGroup() {
	unloadResources(false);
}

void ResourceGroup::addLocalizedFolder(utf::string_view basefolder, int version) {
	utf::string lid = Path::makeCanonical(basefolder);

	utf::string localeDirPath = lid + locale;

	//check if the folder exists or fallback to the default one
	Poco::File localeDir(localeDirPath.bytes());

	if (localeDir.exists()) {
		addFolderSimple(localeDirPath, version);
	}
	else {
		addFolderSimple(lid + fallbackLocale, version);
	}
}

Table& ResourceGroup::addTable(utf::string_view name, Unique<Table> t) {
	DEBUG_ASSERT(not name.empty(), "addTable: a table with this name was already added" );
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified" );

	if (logchanges) {
		DEBUG_MESSAGE( "+" + name + "\t\t table" );
	}

	return *(tables[name.copy()] = std::move(t));
}

void ResourceGroup::addSets(utf::string_view subdirectory, int version) {
	DEBUG_ASSERT( subdirectory.not_empty(), "addSets: folder path is empty" );
	DEBUG_ASSERT( version >= 0, "addSets: negative versions are invalid" );

	std::vector<utf::string> paths;
	utf::string_view name, lastName;

	FrameSet* currentSet = nullptr;

	//find pngs and jpgs
	Platform::singleton().getFilePathsForType("png", subdirectory, paths);
	Platform::singleton().getFilePathsForType("jpg", subdirectory, paths);

	for(auto&& path : paths) {
		name = Path::getFileName(path);

		//skip wrong versions
		if (Path::getVersion(name) != version) {
			continue;
		}

		if (lastName.empty() or not Path::arePathsInSequence(lastName, name)) {
			auto setPrefix = Path::removeTags(name);

			//create a new set
			currentSet = &addFrameSet(make_unique<FrameSet>(self), setPrefix);
		}

		//create a new buffer
		currentSet->addTexture(make_unique<Texture>(self, path));

		lastName = name;
	}

	paths.clear();
	Platform::singleton().getFilePathsForType("atlasinfo", subdirectory, paths);

	//now add atlases!
	Table def;

	for(auto&& path : paths) {
		name = Path::getFileName(path);

		//skip wrong versions
		if (Path::getVersion(name) != version) {
			continue;
		}

		name = Path::removeVersion(name);

		def = Platform::singleton().load(path);

		//standard flat atlasinfo
		if (def.getArrayLength() == 0) {
			auto set = make_unique<FrameSet>(self);
			set->setAtlas(def, self);

			currentSet = &addFrameSet(std::move(set), name);
		}
		else
			for (int j = 0; j < def.getArrayLength(); ++j) {
				auto& sub = def.getTable(j);

				auto set = make_unique<FrameSet>(self);
				set->setAtlas(sub, self);

				currentSet = &addFrameSet(std::move(set), sub.getString("name"));
			}

		def.clear();
	}
}

void ResourceGroup::addFonts(utf::string_view subdirectory, int version) {
	//add all the sets in the given folder
	DEBUG_ASSERT( subdirectory.not_empty(), "addFonts: folder path is empty" );
	DEBUG_ASSERT( version >= 0, "addFonts: negative versions are invalid" );

	utf::string_view name;
	std::vector<utf::string> paths;

	Platform::singleton().getFilePathsForType("font", subdirectory, paths);

	///just add a Font for any .ttf file found
	for (auto&& path : paths) {
		name = Path::getFileName(path);

		//skip wrong versions
		if (Path::getVersion(name) != version) {
			continue;
		}

		name = Path::removeTags(name);

		addFont(make_unique<Font>(self, path), name);
	}
}

void ResourceGroup::addMeshes(utf::string_view subdirectory) {
	std::vector<utf::string> paths;

	Platform::singleton().getFilePathsForType("mesh", subdirectory, paths);

	for (auto&& path : paths) {
		auto name = Path::getFileName(path);

		addMesh(make_unique<Mesh>(self, path), name);
	}
}

void ResourceGroup::addSounds(utf::string_view subdirectory) {
	//ask all the sound files to the main bundle
	std::vector<utf::string> paths;
	utf::string_view name, lastName;

	SoundSet* currentSet = nullptr;

	Platform::singleton().getFilePathsForType("ogg", subdirectory, paths);

	for (auto&& path : paths) {
		name = Path::getFileName(path);

		if (lastName.empty() or not Path::arePathsInSequence(lastName, name)) {
			//create a new set
			auto setPrefix = Path::removeTags(name);
			currentSet = &addSoundSet(make_unique<SoundSet>(self, setPrefix), setPrefix);
		}

		//create a new buffer
		currentSet->addBuffer(make_unique<SoundBuffer>(self, path));

		lastName = name;
	}
}

void ResourceGroup::addTables(utf::string_view folder) {
	std::vector<utf::string> paths;

	Platform::singleton().getFilePathsForType("ds", folder, paths);
		
	for (size_t i = 0u; i < paths.size(); ++i)
		addTable(
			Path::getFileName(paths[i]),
			make_unique<Table>(self, paths[i])
			);
}

void ResourceGroup::addPrograms(utf::string_view folder) {
	std::vector<utf::string> paths;

	auto d = ShaderProgram::vertexExtension.byte_size();
	UNUSED(d);

	Platform::singleton().getFilePathsForType(ShaderProgram::vertexExtension, folder, paths);
	Platform::singleton().getFilePathsForType(ShaderProgram::fragmentExtension, folder, paths);

	for (auto&& path : paths) {
		auto name = Path::getFileName(path);
		addProgram(make_unique<ShaderProgram>(self, path), name);
	}
}

void ResourceGroup::addShaders(utf::string_view folder) {
	std::vector<utf::string> paths;

	Platform::singleton().getFilePathsForType("material", folder, paths);

	for (auto&& path : paths) {
		auto name = Path::getFileName(path);
		addShader(make_unique<Shader>(self, path), name);
	}
}

void ResourceGroup::loadResources(bool recursive) {
	_load<FrameSet>(frameSets);
	_load<Font>(fonts);
	_load<Mesh>(meshes);
	_load<SoundSet>(sounds);
	_load<Table>(tables);
	_load<ShaderProgram>(programs);
	_load<Shader>(shaders);

	//load sets again to load missing atlases!
	_load<FrameSet>(frameSets);

	if (recursive)
		for (auto&& sub : subs) {
			sub->loadResources(recursive);
		}
}

void ResourceGroup::unloadResources(bool recursive) {
	//FONTS DEPEND ON SETS, DO NOT FREE BEFORE
	_unload<Font>(fonts, false);
	_unload<FrameSet>(frameSets, false);
	_unload<Mesh>(meshes, false);
	_unload<SoundSet>(sounds, false);
	_unload<Table>(tables, false);
	_unload<Shader>(shaders, false);
	_unload<ShaderProgram>(programs, false);

	if (recursive) {
		for (auto&& sub : subs) {
			sub->unloadResources(recursive);
		}
	}
}

void ResourceGroup::softUnloadResources(bool recursive) {
	_unload<Font>(fonts, true);
	_unload<FrameSet>(frameSets, true);
	_unload<Mesh>(meshes, true);
	_unload<SoundSet>(sounds, true);
	_unload<Table>(tables, true);
	_unload<ShaderProgram>(programs, true);
	_unload<Shader>(shaders, true);

	if (recursive)
		for (auto&& sub : subs) {
			sub->softUnloadResources(recursive);
		}
}

void ResourceGroup::addFolderSimple(utf::string_view folder, int version) {
	if (logchanges) {
		DEBUG_MESSAGE("[" + folder + "]");
	}

	addSets(folder, version);
	addFonts(folder, version);
	addMeshes(folder);
	addSounds(folder);
	addTables(folder);
	addPrograms(folder);
	addShaders(folder);
}

FrameSet& ResourceGroup::addFrameSet(Unique<FrameSet> resource, utf::string_view name) {
	DEBUG_ASSERT_INFO(getFrameSet(name).is_none(), "A FrameSet with this name already exists", "name = " + name);
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t set");
	}

	return *(frameSets[name.copy()] = std::move(resource));
}

Texture& ResourceGroup::addTexture(Unique<Texture> texture, utf::string_view name) {
	auto frameset = make_unique<FrameSet>(self);
	auto& ref = *texture;
	frameset->addTexture(std::move(texture));
	addFrameSet(std::move(frameset), name);

	return ref;
}


Font& ResourceGroup::addFont(Unique<Font> resource, utf::string_view name) {
	DEBUG_ASSERT_INFO(getFont(name).is_none(), "A Sound with this name already exists", "name = " + name);
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t font");
	}

	return *(fonts[name.copy()] = std::move(resource));
}

Mesh& ResourceGroup::addMesh(Unique<Mesh> resource, utf::string_view name) {
	DEBUG_ASSERT_INFO(getMesh(name).is_none(), "A Mesh with this name already exists", "name = " + name);
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t mesh");
	}

	return *(meshes[name.copy()] = std::move(resource));
}

SoundSet& ResourceGroup::addSoundSet(Unique<SoundSet> resource, utf::string_view name) {
	DEBUG_ASSERT_INFO(getSound(name).is_none(), "A Sound with this name already exists", "name = " + name);
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t sound");
	}

	return *(sounds[name.copy()] = std::move(resource));
}

Shader& ResourceGroup::addShader(Unique<Shader> resource, utf::string_view name) {
	DEBUG_ASSERT_INFO(getShader(name).is_none(), "A Shader with this name already exists", "name = " + name);
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t shader");
	}

	return *(shaders[name.copy()] = std::move(resource));
}

ShaderProgram& ResourceGroup::addProgram(Unique<ShaderProgram> resource, utf::string_view name) {
	DEBUG_ASSERT_INFO(getProgram(name).is_none(), "A ShaderProgram with this name already exists", "name = " + name);
	DEBUG_ASSERT(not finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t shader program");
	}

	return *(programs[name.copy()] = std::move(resource));
}

void ResourceGroup::addSubgroup(ResourceGroup& g) {
	subs.emplace(&g);
}

void ResourceGroup::removeSubgroup(ResourceGroup& g) {
	subs.erase(&g);
}

void ResourceGroup::removeAllSubgroups() {
	subs.clear();
}

void ResourceGroup::removeFrameSet(utf::string_view name) {
	frameSets.erase(frameSets.find(name));
}

void ResourceGroup::removeFont(utf::string_view name) {
	fonts.erase(fonts.find(name));
}

void ResourceGroup::removeMesh(utf::string_view name) {
	meshes.erase(meshes.find(name));
}

void ResourceGroup::removeSound(utf::string_view name) {
	sounds.erase(sounds.find(name));
}

void ResourceGroup::removeTable(utf::string_view name) {
	tables.erase(tables.find(name));
}

FrameSet& ResourceGroup::getEmptyFrameSet() const {
	return *emptyFrameSet;
}

optional_ref<FrameSet> ResourceGroup::getFrameSet(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "getFrameSet: empty name provided");

	return find<FrameSet>(name, ResourceType::FrameSet);
}

optional_ref<Texture> ResourceGroup::getTexture(utf::string_view name) const {
	if (auto s = getFrameSet(name).to_ref()) {
		return s.get().getFrame(0);
	} 
	return{};
}

optional_ref<Font> ResourceGroup::getFont(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "empty name provided");
	return find<Font>(name, ResourceType::Font);
}

optional_ref<Mesh> ResourceGroup::getMesh(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "empty name provided");
	return find<Mesh>(name, ResourceType::Mesh);
}

optional_ref<SoundSet> ResourceGroup::getSound(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "empty name provided");
	return find<SoundSet>(name, ResourceType::SoundSet);
}

optional_ref<Table> ResourceGroup::getTable(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "empty name provided");
	return find<Table>(name, ResourceType::Table);
}

optional_ref<Shader> ResourceGroup::getShader(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "empty name provided");
	return find<Shader>(name, ResourceType::Material);
}

optional_ref<ShaderProgram> ResourceGroup::getProgram(utf::string_view name) const {
	DEBUG_ASSERT(name.not_empty(), "empty name provided");
	return find<ShaderProgram>(name, ResourceType::ShaderProgram);
}

utf::string_view ResourceGroup::getLocale() const {
	return locale;
}

void ResourceGroup::addFolder(utf::string_view folder, int version /*= 0*/) {
	addFolderSimple(folder, version);

	//localized loading
	if (isLocalizationRequired()) {
		addLocalizedFolder(folder, version);
	}
}

void ResourceGroup::addPrefabMeshes() {
	//create an empty texturedQuad
	{
		auto m = make_unique<Mesh>(self);
		m->setTriangleMode(PrimitiveMode::TriangleStrip);
		m->setVertexFields({ VertexField::Position2D, VertexField::UV0, VertexField::Normal, VertexField::Color });

		m->begin(4);

		m->vertex({-0.5, -0.5});
		m->uv(0, 1);
		m->normal(Vector::UnitZ);
		m->color(Color::White);

		m->vertex({0.5, -0.5});
		m->uv(1, 1);
		m->normal(Vector::UnitZ);
		m->color(Color::White);

		m->vertex({-0.5, 0.5});
		m->uv(0, 0);
		m->normal(Vector::UnitZ);
		m->color(Color::White);

		m->vertex({0.5, 0.5});
		m->uv(1, 0);
		m->normal(Vector::UnitZ);
		m->color(Color::White);

		m->end();

		addMesh(std::move(m), "texturedQuad");
	}

	//textured quad xz
	{
		auto m = make_unique<Mesh>(self);
		m->setTriangleMode(PrimitiveMode::TriangleStrip);
		m->setVertexFields({ VertexField::Position3D, VertexField::UV0 });

		m->begin(4);

		m->vertex({-0.5, 0, -0.5});
		m->uv(0, 0);

		m->vertex({-0.5, 0, 0.5});
		m->uv(0, 1);

		m->vertex({0.5, 0, -0.5});
		m->uv(1, 0);

		m->vertex({0.5, 0, 0.5});
		m->uv(1, 1);

		m->end();

		addMesh(std::move(m), "texturedQuadXZ");
	}

	//create a texturedCube
#define l 0.501f
	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(24);

		m->vertex({l, l, l});
		m->normal(Vector::UnitZ);
		m->uv(1, 1);
		m->vertex({l, -l, l});
		m->normal(Vector::UnitZ);
		m->uv(0, 1);
		m->vertex({-l, l, l});
		m->normal(Vector::UnitZ);
		m->uv(1, 0);
		m->vertex({-l, -l, l});
		m->normal(Vector::UnitZ);
		m->uv(0, 0);

		m->quad(0, 1, 2, 3);

		m->vertex({l, l, -l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(1, 1);
		m->vertex({-l, l, -l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(0, 1);
		m->vertex({l, -l, -l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(1, 0);
		m->vertex({-l, -l, -l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(0, 0);

		m->quad(4, 5, 6, 7);

		m->vertex({l, l, l});
		m->normal(Vector::UnitX);
		m->uv(1, 1);
		m->vertex({l, l, -l});
		m->normal(Vector::UnitX);
		m->uv(0, 1);
		m->vertex({l, -l, l});
		m->normal(Vector::UnitX);
		m->uv(1, 0);
		m->vertex({l, -l, -l});
		m->normal(Vector::UnitX);
		m->uv(0, 0);

		m->quad(8, 9, 10, 11);

		m->vertex({-l, l, l});
		m->normal(Vector::NegativeUnitX);
		m->uv(1, 1);
		m->vertex({-l, -l, l});
		m->normal(Vector::NegativeUnitX);
		m->uv(0, 1);
		m->vertex({-l, l, -l});
		m->normal(Vector::NegativeUnitX);
		m->uv(1, 0);
		m->vertex({-l, -l, -l});
		m->normal(Vector::NegativeUnitX);
		m->uv(0, 0);

		m->quad(12, 13, 14, 15);

		m->vertex({l, l, l});
		m->normal(Vector::UnitY);
		m->uv(1, 1);
		m->vertex({-l, l, l});
		m->normal(Vector::UnitY);
		m->uv(0, 1);
		m->vertex({l, l, -l});
		m->normal(Vector::UnitY);
		m->uv(1, 0);
		m->vertex({-l, l, -l});
		m->normal(Vector::UnitY);
		m->uv(0, 0);

		m->quad(16, 17, 18, 19);

		m->vertex({l, -l, l});
		m->normal(Vector::NegativeUnitY);
		m->uv(1, 1);
		m->vertex({l, -l, -l});
		m->normal(Vector::NegativeUnitY);
		m->uv(0, 1);
		m->vertex({-l, -l, l});
		m->normal(Vector::NegativeUnitY);
		m->uv(1, 0);
		m->vertex({-l, -l, -l});
		m->normal(Vector::NegativeUnitY);
		m->uv(0, 0);

		m->quad(20, 21, 22, 23);

		m->end();

		addMesh(std::move(m), "texturedCube");
	}

	//create a cube
	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//-Z
		m->vertex({l, l, -l});
		m->normal(Vector::UnitZ);
		m->uv(1, 0);
		m->vertex({l, -l, -l});
		m->normal(Vector::UnitZ);
		m->uv(1, 1);
		m->vertex({-l, l, -l});
		m->normal(Vector::UnitZ);
		m->uv(0, 0);
		m->vertex({-l, -l, -l});
		m->normal(Vector::UnitZ);
		m->uv(0, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox-Z");
	}

	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//+X
		m->vertex({l, l, l});
		m->normal(Vector::NegativeUnitX);
		m->uv(1, 0);
		m->vertex({l, -l, l});
		m->normal(Vector::NegativeUnitX);
		m->uv(1, 1);
		m->vertex({l, l, -l});
		m->normal(Vector::NegativeUnitX);
		m->uv(0, 0);
		m->vertex({l, -l, -l});
		m->normal(Vector::NegativeUnitX);
		m->uv(0, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox-X");
	}

	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//+Z
		m->vertex({l, l, l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(0, 0);
		m->vertex({-l, l, l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(1, 0);
		m->vertex({l, -l, l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(0, 1);
		m->vertex({-l, -l, l});
		m->normal(Vector::NegativeUnitZ);
		m->uv(1, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox+Z");
	}
	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//-X
		m->vertex({-l, l, l});
		m->normal(Vector::UnitX);
		m->uv(0, 0);
		m->vertex({-l, l, -l});
		m->normal(Vector::UnitX);
		m->uv(1, 0);
		m->vertex({-l, -l, l});
		m->normal(Vector::UnitX);
		m->uv(0, 1);
		m->vertex({-l, -l, -l});
		m->normal(Vector::UnitX);
		m->uv(1, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox+X");
	}
	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);

		m->vertex({l, l, l});
		m->normal(Vector::NegativeUnitY);
		m->uv(1, 1);
		m->vertex({l, l, -l});
		m->normal(Vector::NegativeUnitY);
		m->uv(0, 1);
		m->vertex({-l, l, l});
		m->normal(Vector::NegativeUnitY);
		m->uv(1, 0);
		m->vertex({-l, l, -l});
		m->normal(Vector::NegativeUnitY);
		m->uv(0, 0);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox+Y");
	}
	{
		auto m = make_unique<Mesh>(self);

		m->setIndexByteSize(1); //uint8_t indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);

		m->vertex({l, -l, l});
		m->normal(Vector::UnitY);
		m->uv(1, 0);
		m->vertex({-l, -l, l});
		m->normal(Vector::UnitY);
		m->uv(1, 1);
		m->vertex({l, -l, -l});
		m->normal(Vector::UnitY);
		m->uv(0, 0);
		m->vertex({-l, -l, -l});
		m->normal(Vector::UnitY);
		m->uv(0, 1);

		m->quad(0, 1, 2, 3);

		m->end();

		addMesh(std::move(m), "prefabSkybox-Y");
	}

	//add cube for wireframe use
	{
		auto m = make_unique<Mesh>(self);
		m->setTriangleMode(PrimitiveMode::LineStrip);
		m->setVertexFieldEnabled(VertexField::Position2D);

		m->begin(4);

		m->vertex({ 0.5, 0.5 });
		m->vertex({ -0.5, 0.5 });
		m->vertex({ 0.5, -0.5 });
		m->vertex({ -0.5, -0.5 });

		m->index(0);
		m->index(1);
		m->index(3);
		m->index(2);
		m->index(0);
		m->index(3);

		m->end();

		addMesh(std::move(m), "wireframeQuad");
	}
}
