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
	mapArray[RT_FRAMESET] = &frameSets;
	mapArray[RT_FONT] = &fonts;
	mapArray[RT_MESH] = &meshes;
	mapArray[RT_SOUND] = &sounds;
	mapArray[RT_TABLE] = &tables;
	mapArray[RT_SHADER] = &shaders;
	mapArray[RT_PROGRAM] = &programs;

	emptyFrameSet = make_unique<FrameSet>(this);
}

ResourceGroup::~ResourceGroup() {
	unloadResources(false);
}

void ResourceGroup::addLocalizedFolder(const std::string& basefolder, int version) {
	std::string lid = basefolder;

	if (lid[lid.size() - 1] != '/') {
		lid += '/';
	}

	std::string localeDirPath = lid + locale;

	//check if the folder exists or fallback to the default one
	Poco::File localeDir(localeDirPath);

	if (localeDir.exists()) {
		addFolderSimple(localeDirPath, version);
	}
	else {
		addFolderSimple(lid + fallbackLocale, version);
	}
}

void ResourceGroup::addTable(const std::string& name, Unique<Table> t) {
	DEBUG_ASSERT( !name.empty(), "addTable: a table with this name was already added" );
	DEBUG_ASSERT( !finalized, "This ResourceGroup can't be modified" );

	tables[name] = std::move(t);

	if (logchanges) {
		DEBUG_MESSAGE( "+" + name + "\t\t table" );
	}
}

void ResourceGroup::addSets(const std::string& subdirectory, int version) {
	DEBUG_ASSERT( subdirectory.size(), "addSets: folder path is empty" );
	DEBUG_ASSERT( version >= 0, "addSets: negative versions are invalid" );

	std::vector<std::string> paths;
	std::string name, lastName;

	FrameSet* currentSet = nullptr;

	//find pngs and jpgs
	Platform::singleton().getFilePathsForType("png", subdirectory, paths);
	Platform::singleton().getFilePathsForType("jpg", subdirectory, paths);

	for (int i = 0; i < paths.size(); ++i) {
		name = Path::getFileName(paths[i]);

		//skip wrong versions
		if (Path::getVersion(name) != version) {
			continue;
		}

		if (!Path::arePathsInSequence(lastName, name)) {
			std::string setPrefix = Path::removeTags(name);

			//create a new set
			currentSet = &addFrameSet(make_unique<FrameSet>(this), setPrefix);
		}

		//create a new buffer
		currentSet->addTexture(make_unique<Texture>(this, paths[i]));

		lastName = name;
	}

	paths.clear();
	Platform::singleton().getFilePathsForType("atlasinfo", subdirectory, paths);

	//now add atlases!
	Table def;

	for (int i = 0; i < paths.size(); ++i) {
		name = Path::getFileName(paths[i]);

		//skip wrong versions
		if (Path::getVersion(name) != version) {
			continue;
		}

		name = Path::removeVersion(name);

		def = Platform::singleton().load(paths[i]);

		//standard flat atlasinfo
		if (def.getArrayLength() == 0) {
			auto set = make_unique<FrameSet>(this);
			set->setAtlas(def, *this);

			currentSet = &addFrameSet(std::move(set), name);
		}
		else
			for (int j = 0; j < def.getArrayLength(); ++j) {
				auto& sub = def.getTable(j);

				auto set = make_unique<FrameSet>(this);
				set->setAtlas(sub, *this);

				currentSet = &addFrameSet(std::move(set), sub.getString("name"));
			}

		def.clear();
	}
}

void ResourceGroup::addFonts(const std::string& subdirectory, int version) {
	//add all the sets in the given folder
	DEBUG_ASSERT( subdirectory.size(), "addFonts: folder path is empty" );
	DEBUG_ASSERT( version >= 0, "addFonts: negative versions are invalid" );

	std::string name;
	std::vector<std::string> paths;

	Platform::singleton().getFilePathsForType("font", subdirectory, paths);

	///just add a Font for any .ttf file found
	for (int i = 0; i < paths.size(); ++i) {
		name = Path::getFileName(paths[i]);

		//skip wrong versions
		if (Path::getVersion(name) != version) {
			continue;
		}

		name = Path::removeTags(name);

		addFont(make_unique<Font>(this, paths[i]), name);
	}
}

void ResourceGroup::addMeshes(const std::string& subdirectory) {
	std::vector<std::string> paths;
	std::string name;

	Platform::singleton().getFilePathsForType("mesh", subdirectory, paths);

	for (int i = 0; i < paths.size(); ++i) {
		name = Path::getFileName(paths[i]);

		addMesh(make_unique<Mesh>(this, paths[i]), name);
	}
}

void ResourceGroup::addSounds(const std::string& subdirectory) {
	//ask all the sound files to the main bundle
	std::vector<std::string> paths;
	std::string name, lastName;

	SoundSet* currentSet = nullptr;

	Platform::singleton().getFilePathsForType("ogg", subdirectory, paths);

	for (int i = 0; i < paths.size(); ++i) {
		name = Path::getFileName(paths[i]);

		if (!Path::arePathsInSequence(lastName, name)) {
			//create a new set
			std::string setPrefix = Path::removeTags(name);
			currentSet = &addSoundSet(make_unique<SoundSet>(this, setPrefix), setPrefix);
		}

		//create a new buffer
		currentSet->addBuffer(make_unique<SoundBuffer>(this, paths[i]));

		lastName = name;
	}
}

void ResourceGroup::addTables(const std::string& folder) {
	std::vector<std::string> paths;

	Platform::singleton().getFilePathsForType("ds", folder, paths);
		
	for (int i = 0; i < paths.size(); ++i)
		addTable(
			Path::getFileName(paths[i]),
			make_unique<Table>(this, paths[i])
			);
}

void ResourceGroup::addPrograms(const std::string& folder) {
	std::vector<std::string> paths;

	Platform::singleton().getFilePathsForType(ShaderProgram::vertexExtension, folder, paths);
	Platform::singleton().getFilePathsForType(ShaderProgram::fragmentExtension, folder, paths);

	for (auto&& path : paths) {
		std::string name = Path::getFileName(path);
		addProgram(make_unique<ShaderProgram>(this, path), name);
	}
}

void ResourceGroup::addShaders(const std::string& folder) {
	std::vector<std::string> paths;

	Platform::singleton().getFilePathsForType("shader", folder, paths);

	for (auto&& path : paths) {
		std::string name = Path::getFileName(path);
		addShader(make_unique<Shader>(this, path), name);
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

void ResourceGroup::addFolderSimple(const std::string& folder, int version) {
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

FrameSet& ResourceGroup::addFrameSet(Unique<FrameSet> resource, const std::string& name) {
	DEBUG_ASSERT_INFO(!getFrameSet(name), "A FrameSet with this name already exists", "name = " + name);
	DEBUG_ASSERT(!finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t set");
	}

	return *(frameSets[name] = std::move(resource));
}

void ResourceGroup::addFont(Unique<Font> resource, const std::string& name) {
	DEBUG_ASSERT_INFO(!getFont(name), "A Sound with this name already exists", "name = " + name);
	DEBUG_ASSERT(!finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	fonts[name] = std::move(resource);

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t font");
	}
}

void ResourceGroup::addMesh(Unique<Mesh> resource, const std::string& name) {
	DEBUG_ASSERT_INFO(!getMesh(name), "A Mesh with this name already exists", "name = " + name);
	DEBUG_ASSERT(!finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	meshes[name] = std::move(resource);

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t mesh");
	}
}

SoundSet& ResourceGroup::addSoundSet(Unique<SoundSet> resource, const std::string& name) {
	DEBUG_ASSERT_INFO(!getSound(name), "A Sound with this name already exists", "name = " + name);
	DEBUG_ASSERT(!finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t sound");
	}

	return *(sounds[name] = std::move(resource));
}

void ResourceGroup::addShader(Unique<Shader> resource, const std::string& name) {
	DEBUG_ASSERT_INFO(!getShader(name), "A Shader with this name already exists", "name = " + name);
	DEBUG_ASSERT(!finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	shaders[name] = std::move(resource);

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t shader");
	}
}

void ResourceGroup::addProgram(Unique<ShaderProgram> resource, const std::string& name) {
	DEBUG_ASSERT_INFO(!getProgram(name), "A ShaderProgram with this name already exists", "name = " + name);
	DEBUG_ASSERT(!finalized, "This ResourceGroup can't be modified");
	DEBUG_ASSERT(resource, "Invalid resource passed!");

	programs[name] = std::move(resource);

	if (logchanges) {
		DEBUG_MESSAGE("+" + name + "\t\t shader program");
	}
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

void ResourceGroup::removeFrameSet(const std::string& name) {
	frameSets.erase(name);
}

void ResourceGroup::removeFont(const std::string& name) {
	fonts.erase(name);
}

void ResourceGroup::removeMesh(const std::string& name) {
	meshes.erase(name);
}

void ResourceGroup::removeSound(const std::string& name) {
	sounds.erase(name);
}

void ResourceGroup::removeTable(const std::string& name) {
	tables.erase(name);
}

FrameSet& ResourceGroup::getEmptyFrameSet() const {
	return *emptyFrameSet;
}

FrameSet* ResourceGroup::getFrameSet(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "getFrameSet: empty name provided");

	return find<FrameSet>(name, RT_FRAMESET);
}

Texture* ResourceGroup::getTexture(const std::string& name) const {
	FrameSet* s = getFrameSet(name);

	return s ? s->getFrame(0) : nullptr;
}

Font* ResourceGroup::getFont(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "empty name provided");
	return find<Font>(name, RT_FONT);
}

Mesh* ResourceGroup::getMesh(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "empty name provided");
	return find<Mesh>(name, RT_MESH);
}

SoundSet* ResourceGroup::getSound(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "empty name provided");
	return find<SoundSet>(name, RT_SOUND);
}

Table* ResourceGroup::getTable(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "empty name provided");
	return find<Table>(name, RT_TABLE);
}

Shader* ResourceGroup::getShader(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "empty name provided");
	return find<Shader>(name, RT_SHADER);
}

ShaderProgram* ResourceGroup::getProgram(const std::string& name) const {
	DEBUG_ASSERT(name.size(), "empty name provided");
	return find<ShaderProgram>(name, RT_PROGRAM);
}

const std::string& ResourceGroup::getLocale() const {
	return locale;
}

void ResourceGroup::addFolder(const std::string& folder, int version /*= 0*/) {
	addFolderSimple(folder, version);

	//localized loading
	if (isLocalizationRequired()) {
		addLocalizedFolder(folder, version);
	}
}

void ResourceGroup::addPrefabMeshes() {
	//create an empty texturedQuad
	{
		auto m = make_unique<Mesh>(this);
		m->setTriangleMode(PrimitiveMode::TriangleStrip);
		m->setVertexFields({ VertexField::Position2D, VertexField::UV0 });

		m->begin(4);

		m->vertex(-0.5, -0.5);
		m->uv(0, 1);

		m->vertex(0.5, -0.5);
		m->uv(1, 1);

		m->vertex(-0.5, 0.5);
		m->uv(0, 0);

		m->vertex(0.5, 0.5);
		m->uv(1, 0);

		m->end();

		addMesh(std::move(m), "texturedQuad");
	}

	//textured quad xz
	{
		auto m = make_unique<Mesh>(this);
		m->setTriangleMode(PrimitiveMode::TriangleStrip);
		m->setVertexFields({ VertexField::Position3D, VertexField::UV0 });

		m->begin(4);

		m->vertex(-0.5, 0, -0.5);
		m->uv(0, 0);

		m->vertex(-0.5, 0, 0.5);
		m->uv(0, 1);

		m->vertex(0.5, 0, -0.5);
		m->uv(1, 0);

		m->vertex(0.5, 0, 0.5);
		m->uv(1, 1);

		m->end();

		addMesh(std::move(m), "texturedQuadXZ");
	}

	//create a texturedCube
#define l 0.501f
	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(24);

		m->vertex(l, l, l);
		m->normal(0, 0, 1);
		m->uv(1, 1);
		m->vertex(l, -l, l);
		m->normal(0, 0, 1);
		m->uv(0, 1);
		m->vertex(-l, l, l);
		m->normal(0, 0, 1);
		m->uv(1, 0);
		m->vertex(-l, -l, l);
		m->normal(0, 0, 1);
		m->uv(0, 0);

		m->quad(0, 1, 2, 3);

		m->vertex(l, l, -l);
		m->normal(0, 0, -1);
		m->uv(1, 1);
		m->vertex(-l, l, -l);
		m->normal(0, 0, -1);
		m->uv(0, 1);
		m->vertex(l, -l, -l);
		m->normal(0, 0, -1);
		m->uv(1, 0);
		m->vertex(-l, -l, -l);
		m->normal(0, 0, -1);
		m->uv(0, 0);

		m->quad(4, 5, 6, 7);

		m->vertex(l, l, l);
		m->normal(1, 0, 0);
		m->uv(1, 1);
		m->vertex(l, l, -l);
		m->normal(1, 0, 0);
		m->uv(0, 1);
		m->vertex(l, -l, l);
		m->normal(1, 0, 0);
		m->uv(1, 0);
		m->vertex(l, -l, -l);
		m->normal(1, 0, 0);
		m->uv(0, 0);

		m->quad(8, 9, 10, 11);

		m->vertex(-l, l, l);
		m->normal(-1, 0, 0);
		m->uv(1, 1);
		m->vertex(-l, -l, l);
		m->normal(-1, 0, 0);
		m->uv(0, 1);
		m->vertex(-l, l, -l);
		m->normal(-1, 0, 0);
		m->uv(1, 0);
		m->vertex(-l, -l, -l);
		m->normal(-1, 0, 0);
		m->uv(0, 0);

		m->quad(12, 13, 14, 15);

		m->vertex(l, l, l);
		m->normal(0, 1, 0);
		m->uv(1, 1);
		m->vertex(-l, l, l);
		m->normal(0, 1, 0);
		m->uv(0, 1);
		m->vertex(l, l, -l);
		m->normal(0, 1, 0);
		m->uv(1, 0);
		m->vertex(-l, l, -l);
		m->normal(0, 1, 0);
		m->uv(0, 0);

		m->quad(16, 17, 18, 19);

		m->vertex(l, -l, l);
		m->normal(0, -1, 0);
		m->uv(1, 1);
		m->vertex(l, -l, -l);
		m->normal(0, -1, 0);
		m->uv(0, 1);
		m->vertex(-l, -l, l);
		m->normal(0, -1, 0);
		m->uv(1, 0);
		m->vertex(-l, -l, -l);
		m->normal(0, -1, 0);
		m->uv(0, 0);

		m->quad(20, 21, 22, 23);

		m->end();

		addMesh(std::move(m), "texturedCube");
	}

	//create a cube
	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//-Z
		m->vertex(l, l, -l);
		m->normal(0, 0, 1);
		m->uv(1, 0);
		m->vertex(l, -l, -l);
		m->normal(0, 0, 1);
		m->uv(1, 1);
		m->vertex(-l, l, -l);
		m->normal(0, 0, 1);
		m->uv(0, 0);
		m->vertex(-l, -l, -l);
		m->normal(0, 0, 1);
		m->uv(0, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox-Z");
	}

	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//+X
		m->vertex(l, l, l);
		m->normal(-1, 0, 0);
		m->uv(1, 0);
		m->vertex(l, -l, l);
		m->normal(-1, 0, 0);
		m->uv(1, 1);
		m->vertex(l, l, -l);
		m->normal(-1, 0, 0);
		m->uv(0, 0);
		m->vertex(l, -l, -l);
		m->normal(-1, 0, 0);
		m->uv(0, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox-X");
	}

	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//+Z
		m->vertex(l, l, l);
		m->normal(0, 0, -1);
		m->uv(0, 0);
		m->vertex(-l, l, l);
		m->normal(0, 0, -1);
		m->uv(1, 0);
		m->vertex(l, -l, l);
		m->normal(0, 0, -1);
		m->uv(0, 1);
		m->vertex(-l, -l, l);
		m->normal(0, 0, -1);
		m->uv(1, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox+Z");
	}
	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);
		//-X
		m->vertex(-l, l, l);
		m->normal(1, 0, 0);
		m->uv(0, 0);
		m->vertex(-l, l, -l);
		m->normal(1, 0, 0);
		m->uv(1, 0);
		m->vertex(-l, -l, l);
		m->normal(1, 0, 0);
		m->uv(0, 1);
		m->vertex(-l, -l, -l);
		m->normal(1, 0, 0);
		m->uv(1, 1);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox+X");
	}
	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);

		m->vertex(l, l, l);
		m->normal(0, -1, 0);
		m->uv(1, 1);
		m->vertex(l, l, -l);
		m->normal(0, -1, 0);
		m->uv(0, 1);
		m->vertex(-l, l, l);
		m->normal(0, -1, 0);
		m->uv(1, 0);
		m->vertex(-l, l, -l);
		m->normal(0, -1, 0);
		m->uv(0, 0);

		m->quad(0, 1, 2, 3);

		m->end();
		addMesh(std::move(m), "prefabSkybox+Y");
	}
	{
		auto m = make_unique<Mesh>(this);

		m->setIndexByteSize(1); //byte indices
		m->setTriangleMode(PrimitiveMode::TriangleList);
		m->setVertexFields({ VertexField::Position3D, VertexField::Normal, VertexField::UV0 });

		m->begin(4);

		m->vertex(l, -l, l);
		m->normal(0, 1, 0);
		m->uv(1, 0);
		m->vertex(-l, -l, l);
		m->normal(0, 1, 0);
		m->uv(1, 1);
		m->vertex(l, -l, -l);
		m->normal(0, 1, 0);
		m->uv(0, 0);
		m->vertex(-l, -l, -l);
		m->normal(0, 1, 0);
		m->uv(0, 1);

		m->quad(0, 1, 2, 3);

		m->end();

		addMesh(std::move(m), "prefabSkybox-Y");
	}

	//add cube for wireframe use
	{
		auto m = make_unique<Mesh>(this);
		m->setTriangleMode(PrimitiveMode::LineStrip);
		m->setVertexFieldEnabled(VertexField::Position2D);

		m->begin(4);

		m->vertex(0.5, 0.5);
		m->vertex(-0.5, 0.5);
		m->vertex(0.5, -0.5);
		m->vertex(-0.5, -0.5);

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
