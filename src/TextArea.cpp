#include "TextArea.h"
#include "Game.h"
#include "GameState.h"
#include "Mesh.h"
#include "Viewport.h"
#include "Platform.h"
#include "Renderer.h"
#include "range.h"

using namespace Dojo;

Dojo::TextArea::TextArea(Object& l,
	RenderLayer::ID layer,
	const std::string& fontSetName,
	bool center /*= false*/,
	const Vector& bounds /*= Vector::One*/) :
	Renderable(l, layer),
	fontName(fontSetName),
	interline(0.2f),
	maxLineLenght(0xfffffff),
	centered(center),
	pixelScale(1, 1),
	currentLineLength(0),
	lastSpace(0),
	visibleCharsNumber(0xfffffff) {

	l.setSize(bounds); //TODO HMM

	font = getGameState().getFont(fontName);

	DEBUG_ASSERT_INFO( font, "Cannot find the required font for a TextArea", "fontName = " + fontName );

	charSpacing = font->getSpacing();
	spaceWidth = font->getCharacter(' ')->advance;

	//not visible until prepared!
	scale = Vector::Zero;

	//init
	clearText();
}

TextArea::~TextArea() {
	clearText();

	DEBUG_ASSERT(!mesh, "This isn't renderable by itself");

	_destroyLayers();
}

void Dojo::TextArea::setVisibleCharacters(uint32_t n) {
	if (n != visibleCharsNumber) {
		visibleCharsNumber = n;

		changed = true;
	}
}

void TextArea::clearText() {
	characters.clear();

	content.clear();

	cursorPosition = Vector::Zero;

	self.setSize(0, 0); //TODO hmm

	changed = true;

	visibleCharsNumber = INT_MAX;
	currentLineLength = 0;
	lastSpace = 0;
}

void TextArea::setMaxLineLength(int l) {
	//HACK PAZZESCOH
	maxLineLenght = (int)(l * ((float)getGameState().getGame().getNativeWidth() / (float)640));
}

void TextArea::addText(const std::string& text) {
	content += text;

	Font::Character* currentChar;
	uint32_t c;

	//parse and setup characters
	for (auto && i : range(text.size())) {
		c = text[i];

		currentChar = font->getCharacter(c);
		characters.emplace(currentChar);

		currentLineLength += currentChar->pixelWidth;

		if (c == ' ' || c == '\t') {
			lastSpace = characters.size() - 1;
		}

		else if (c == '\n') {
			lastSpace = 0;
			currentLineLength = 0;
		}

		//lenght eccess? find last whitespace and replace with \n.
		if (currentLineLength > maxLineLenght && lastSpace) {
			characters[lastSpace] = font->getCharacter('\n');
			lastSpace = 0;
			currentLineLength = 0;
		}
	}

	changed = true;
}

void TextArea::addText(int n, char paddingChar, int digits) {
	//TODO std::string already does this! remove

	auto number = std::to_string(n);

	//stay in the digit budget?
	if (paddingChar != 0) {
		//forget most significative digits
		if ((int)number.size() > digits) {
			number = number.substr(number.size() - digits);
		}

		//pad to fill
		else if ((int) number.size() < digits) {
			std::string padding;

			for (size_t i = 0; i < digits - number.size(); ++i) {
				padding += paddingChar;
			}

			number = padding + number;
		}
	}

	addText(number);
}

Renderable& Dojo::TextArea::_enableLayer(Texture& tex) {
	if (freeLayers.empty()) {
		_pushLayer();
	}

	auto& layer = **freeLayers.begin();

	layer.setVisible(true);
	layer.setTexture(&tex);

	layer.getMesh()->begin(getLenght() * 2);

	//move it to the busy layer
	busyLayers.emplace(std::move(*freeLayers.begin()));
	freeLayers.erase(freeLayers.begin());

	return layer;
}

void TextArea::_endLayers() {
	for (size_t i = 0; i < busyLayers.size(); ++i) {
		busyLayers[i]->getMesh()->end();
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

	busyLayers.erase(LayerList::find(busyLayers, r));
	freeLayers.erase(LayerList::find(freeLayers, r));
}


void TextArea::_prepare() {
	//not changed
	if (!changed) {
		return;
	}

	//no characters to show
	if (!visibleCharsNumber || getLenght() == 0) {
		return;
	}

	//setup the aspect ratio
	getGameState().getViewport()->makeScreenSize(screenSize, font->getFontWidth(), font->getFontHeight());

	pixelScale.z = 1;
	screenSize = Vector::mul(screenSize, pixelScale);
	scale = screenSize;

	//render the font
	Font::Character* rep, *lastRep = nullptr;
	Vector newSize(0, 0);
	bool doKerning = font->isKerningEnabled();
	int lastLineVertexID = 0;
	int idx = 0;

	cursorPosition.x = 0;
	cursorPosition.y = 0;

	//clear layers
	_hideLayers();

	//either reach the last valid character or the last existing character
	for (size_t i = 0; i < visibleCharsNumber && i < characters.size(); ++i) {
		rep = characters[i];

		//avoid to rendering spaces
		if (rep->character == '\n') {
			//if centered move every character of this line along x of 1/2 size
			if (centered) {
				DEBUG_TODO; //it kind of never worked with unicode
				// 				_centerLastLine( lastLineVertexID, cursorPosition.x );
				// 				lastLineVertexID = mesh->getVertexCount();
			}

			cursorPosition.y -= 1.f + interline;
			cursorPosition.x = 0;
			lastRep = nullptr;
		}
		else if (rep->character == '\t') {
			cursorPosition.x += spaceWidth * 4; //TODO align to nearest tab
			lastRep = nullptr;
		}
		else if (rep->character == ' ') {
			cursorPosition.x += spaceWidth;
			lastRep = nullptr;
		}
		else { //real character
			auto& layer = *_getLayer(rep->getTexture()).getMesh();

			float x = cursorPosition.x + rep->bearingU;
			float y = cursorPosition.y - rep->bearingV;

			if (doKerning && lastRep) {
				x += font->getKerning(rep, lastRep);
			}

			idx = layer.getVertexCount();

			//assign vertex positions and uv coordinates
			layer.vertex(x, y);
			layer.uv(rep->uvPos.x, rep->uvPos.y + rep->uvHeight);

			layer.vertex(x + rep->widthRatio, y);
			layer.uv(rep->uvPos.x + rep->uvWidth, rep->uvPos.y + rep->uvHeight);

			layer.vertex(x, y + rep->heightRatio);
			layer.uv(rep->uvPos.x, rep->uvPos.y);

			layer.vertex(x + rep->widthRatio, y + rep->heightRatio);
			layer.uv(rep->uvPos.x + rep->uvWidth, rep->uvPos.y);

			layer.triangle(idx, idx + 1, idx + 2);
			layer.triangle(idx + 1, idx + 3, idx + 2);

			//now move to the next character
			cursorPosition.x += rep->advance + charSpacing;

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

	for (auto && layer : busyLayers) {
		mLayersBound = mLayersBound.expandToFit(layer->getMesh()->getBounds());
	}

	self.setSize(mLayersBound.max - mLayersBound.min); //TODO hmm

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
Unique<Mesh> Dojo::TextArea::_createMesh() {
	auto mesh = make_unique<Mesh>();
	mesh->setDynamic(true);
	mesh->setVertexFields({VertexField::Position2D, VertexField::UV0});
	mesh->setTriangleMode(PrimitiveMode::TriangleList);

	return mesh;
}

void TextArea::_pushLayer() {
	meshPool.emplace_back(_createMesh());

	auto r = make_unique<Renderable>(getGameState(), getLayer(), *meshPool.back());
	r->scale = scale;
	r->setVisible(false);

	Platform::singleton().getRenderer().addRenderable(*r);

	freeLayers.emplace(std::move(r));
}

Renderable& Dojo::TextArea::_getLayer(Texture& tex) {
	//find this layer in the already assigned, or get new
	for (size_t i = 0; i < busyLayers.size(); ++i) {
		if (busyLayers[i]->getTexture() == &tex) {
			return *busyLayers[i];
		}
	}

	//does not exist, hit a free one
	return _enableLayer(tex);
}

void TextArea::update(float dt) {
	_prepare();

	//WARNING remember to keep this in sync with Renderable::update!

	worldBB = self.transformAABB(mLayersBound);

	advanceFade(dt);
}
