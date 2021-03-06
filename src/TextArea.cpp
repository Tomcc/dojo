#include "TextArea.h"
#include "Game.h"
#include "GameState.h"
#include "Mesh.h"
#include "Viewport.h"
#include "Platform.h"
#include "Renderer.h"
#include "range.h"

using namespace Dojo;

TextArea::TextArea(Object& l,
	RenderLayer::ID layer,
	utf::string_view fontSetName,
	bool center /*= false*/,
	const Vector& bounds /*= Vector::One*/,
	optional_ref<Shader> customMaterial
	) :
	Renderable(l, layer),
	fontName(fontSetName.copy()),
	interline(0.2f),
	maxLineLength(0xfffffff),
	centered(center),
	pixelScale(1, 1),
	currentLineLength(0),
	lastSpace(0),
	visibleCharsNumber(0xfffffff),
	font(getGameState().getFont(fontName).unwrap()),
	mMaterial(customMaterial.unwrap_or(getGameState().getShader("textured").unwrap())) {

	l.setSize(bounds); //TODO HMM

	charSpacing = font.getSpacing();
	spaceWidth = font.getCharacter(' ').advance;

	//not visible until prepared!
	scale = Vector::Zero;

	//init
	clearText();
}

TextArea::~TextArea() {
	clearText();
	_destroyLayers();
}

void TextArea::setVisibleCharacters(size_t n) {
	if (n != visibleCharsNumber) {
		visibleCharsNumber = n;

		changed = true;
	}
}

void TextArea::clearText() {
	characters.clear();

	content.clear();

	cursorPosition = Vector::Zero;

	object.setSize(0, 0); //TODO hmm

	changed = true;

	visibleCharsNumber = INT_MAX;
	currentLineLength = 0;
	lastSpace = 0;
}

void TextArea::setMaxLineLength(int l) {
	//HACK PAZZESCOH
	maxLineLength = (int)(l * ((float)getGameState().getGame().getNativeWidth() / (float)640));
}

void TextArea::addText(utf::string_view text) {
	content += text;

	//parse and setup characters
	for(auto&& c : text) {
		auto& currentChar = font.getCharacter(c);
		characters.emplace(currentChar);

		currentLineLength += currentChar.pixelWidth;

		if (c == ' ' or c == '\t') {
			lastSpace = characters.size() - 1;
		}

		else if (c == '\n') {
			lastSpace = 0;
			currentLineLength = 0;
		}

		//lenght eccess? find last whitespace and replace with \n.
		if (currentLineLength > maxLineLength and lastSpace) {
			characters[lastSpace] = font.getCharacter('\n');
			lastSpace = 0;
			currentLineLength = 0;
		}
	}

	changed = true;
}

// void TextArea::addText(int n, char paddingChar, int digits) {
// 	//TODO utf::string already does this! remove
// 
// 	auto number = utf::to_string(n);
// 
// 	//stay in the digit budget?
// 	if (paddingChar != 0) {
// 		//forget most significative digits
// 		if ((int)number.size() > digits) {
// 			number = number.substr(number.size() - digits);
// 		}
// 
// 		//pad to fill
// 		else if ((int) number.size() < digits) {
// 			utf::string padding;
// 
// 			for (size_t i = 0; i < digits - number.size(); ++i) {
// 				padding += paddingChar;
// 			}
// 
// 			number = padding + number;
// 		}
// 	}
// 
// 	addText(number);
// }

Renderable& TextArea::_enableLayer(Texture& tex) {
	if (freeLayers.empty()) {
		_pushLayer();
	}

	auto& layer = **freeLayers.begin();

	layer.setVisible(true);
	layer.setTexture(tex);

	layer.getMesh().unwrap().begin(static_cast<Mesh::IndexType>(getLength() * 2));

	//move it to the busy layer
	busyLayers.emplace(std::move(*freeLayers.begin()));
	freeLayers.erase(freeLayers.begin());

	return layer;
}

void TextArea::_endLayers() {
	for (size_t i = 0; i < busyLayers.size(); ++i) {
		busyLayers[i]->getMesh().unwrap().end();
	}
}

///Free any created layer
void TextArea::_hideLayers() {
	for (size_t i = 0; i < busyLayers.size(); ++i) {
		auto& l = busyLayers[i];
		l->setVisible(false);

		//move it in the free elements
		freeLayers.emplace(std::move(l));
	}

	actualCharacters = 0;
	busyLayers.clear();
}

void TextArea::_destroyLayer(Renderable& r) {

	Platform::singleton().getRenderer().removeRenderable(r);

	{
		auto elem = LayerList::find(busyLayers, r);
		if (elem != busyLayers.end()) {
			busyLayers.erase(elem);
		}
	}
	{
		auto elem = LayerList::find(freeLayers, r);
		if (elem != freeLayers.end()) {
			freeLayers.erase(elem);
		}
	}
}


void TextArea::_prepare() {
	if (not changed) {
		return;
	}

	//no characters to show
	if (visibleCharsNumber == 0 or getLength() == 0) {
		return;
	}

	//setup the aspect ratio
	screenSize = getGameState().getViewport().unwrap().makeScreenSize(font.getFontWidth(), font.getFontHeight());

	pixelScale.z = 1;
	screenSize = Vector::mul(screenSize, pixelScale);
	scale = screenSize;

	//render the font
	Vector newSize(0, 0);
	bool doKerning = font.isKerningEnabled();
	int lastLineVertexID = 0;
	int idx = 0;

	cursorPosition.x = 0;
	cursorPosition.y = 0;

	//clear layers
	_hideLayers();

	optional_ref<Font::Character> lastRep;

	//either reach the last valid character or the last existing character
	for (size_t i = 0; i < visibleCharsNumber and i < characters.size(); ++i) {
		auto& rep = characters[i].unwrap();

		//avoid to rendering spaces
		if (rep.character == '\n') {
			//if centered move every character of this line along x of 1/2 size
			if (centered) {
				DEBUG_TODO; //it kind of never worked with unicode
				// 				_centerLastLine( lastLineVertexID, cursorPosition.x );
				// 				lastLineVertexID = mesh->getVertexCount();
			}

			cursorPosition.y -= 1.f + interline;
			cursorPosition.x = 0;
			lastRep = {};
		}
		else if (rep.character == '\t') {
			cursorPosition.x += spaceWidth * 4; //TODO align to nearest tab
			lastRep = {};
		}
		else if (rep.character == ' ') {
			cursorPosition.x += spaceWidth;
			lastRep = {};
		}
		else { //real character
			auto& layer = _getLayer(rep.getTexture()).getMesh().unwrap();

			float x = cursorPosition.x + rep.bearingU;
			float y = cursorPosition.y - rep.bearingV;

			if (doKerning and lastRep.is_some()) {
				x += font.getKerning(rep, lastRep.unwrap());
			}

			idx = layer.getVertexCount();

			//assign vertex positions and uv coordinates
			layer.vertex({x, y});
			layer.uv(rep.uvPos.x, rep.uvPos.y + rep.uvHeight);

			layer.vertex({x + rep.widthRatio, y});
			layer.uv(rep.uvPos.x + rep.uvWidth, rep.uvPos.y + rep.uvHeight);

			layer.vertex({x, y + rep.heightRatio});
			layer.uv(rep.uvPos.x, rep.uvPos.y);

			layer.vertex({x + rep.widthRatio, y + rep.heightRatio});
			layer.uv(rep.uvPos.x + rep.uvWidth, rep.uvPos.y);

			layer.triangle(idx, idx + 1, idx + 2);
			layer.triangle(idx + 1, idx + 3, idx + 2);

			//now move to the next character
			cursorPosition.x += rep.advance + charSpacing;

			lastRep = rep;
			++actualCharacters;
		}

		//update size to contain cursorPos to the longest line
		if (cursorPosition.x > newSize.x) {
			newSize.x = cursorPosition.x;
		}
	}

	//if centered move every character of this line along x of 1/2 size
	if (centered) {
		_centerLastLine(lastLineVertexID, cursorPosition.x);
	}

	//push any active layer on the GPU
	_endLayers();

	//find real mesh bounds
	mLayersBound = AABB::Invalid;

	for (auto&& layer : busyLayers) {
		mLayersBound = mLayersBound.expandToFit(layer->getMesh().unwrap().getBounds());
	}

	object.setSize(mLayersBound.max - mLayersBound.min); //TODO hmm

	changed = false;
}

void TextArea::_destroyLayers() {
	while (busyLayers.size() > 0) {
		_destroyLayer(**busyLayers.begin());
	}

	while (freeLayers.size() > 0) {
		_destroyLayer(**freeLayers.begin());
	}

	meshPool.clear();
}

void TextArea::_centerLastLine(int startingAt, float size) {
	DEBUG_TODO; //it kind of never worked with unicode

	// 	if (mesh->getVertexCount() == 0)
	// 		return;
	//
	// 	float halfWidth = size * 0.5f;
	//
	// 	for (Mesh::IndexType i = startingAt; i < mesh->getVertexCount(); ++i)
	// 		mesh->getVertex(i).x -= halfWidth;
}

///create a mesh to be used for text
std::unique_ptr<Mesh> TextArea::_createMesh() {
	auto mesh = make_unique<Mesh>();
	mesh->setDynamic(true);
	mesh->setVertexFields({VertexField::Position2D, VertexField::UV0});
	mesh->setTriangleMode(PrimitiveMode::TriangleList);

	return mesh;
}

void TextArea::_pushLayer() {
	meshPool.emplace_back(_createMesh());

	auto r = make_unique<Renderable>(getObject(), getLayerID(), *meshPool.back(), mMaterial);
	r->scale = scale;
	r->setVisible(false);

	Platform::singleton().getRenderer().addRenderable(*r);

	freeLayers.emplace(std::move(r));
}

Renderable& TextArea::_getLayer(Texture& tex) {
	//find this layer in the already assigned, or get new
	for (size_t i = 0; i < busyLayers.size(); ++i) {
		if (busyLayers[i]->getTexture() == tex) {
			return *busyLayers[i];
		}
	}

	//does not exist, hit a free one
	return _enableLayer(tex);
}

void TextArea::update(float dt) {
	_prepare();

	//WARNING remember to keep this in sync with Renderable::update!

	mWorldBB = object.transformAABB(mLayersBound);

	advanceFade(dt);
}
