#include "Font.h"
#include "Platform.h"
#include "Table.h"
#include "FontSystem.h"
#include "Timer.h"
#include "ResourceGroup.h"
#include "Texture.h"
#include "FrameSet.h"
#include "Tessellation.h"
#include "Path.h"

using namespace Dojo;


Font::Character::Character() {

}

Texture& Font::Character::getTexture() {
	return parentPage.unwrap().getTexture();
}

Tessellation* Font::Character::getTesselation() {
	DEBUG_ASSERT(mTesselation.get(), "Tried to get a tesselation but this Font wasn't tesselated (forgot to specify 'tesselate' in the config?)");

	return mTesselation.get();
}

void Font::_blit(byte* dest, FT_Bitmap* bitmap, int x, int y, int destside) {
	DEBUG_ASSERT( dest, "null destination buffer" );
	DEBUG_ASSERT( bitmap, "Null freetype bitmap" );

	int rowy, idx;

	if (bitmap->pixel_mode != FT_PIXEL_MODE_MONO) {
		for (uint32_t i = 0; i < bitmap->rows; ++i) {
			rowy = (i + y) * destside;

			for (uint32_t j = 0; j < bitmap->width; ++j) {
				idx = 4 * ((j + x) + rowy);

				dest[idx + 3] = bitmap->buffer[j + i * bitmap->pitch]; //the font is really the alpha
			}
		}
	}
	else {
		byte b;

		for (uint32_t i = 0; i < bitmap->rows; ++i) {
			rowy = (i + y) * destside;

			for (uint32_t j = 0; j < bitmap->width; ++j) {
				idx = 4 * ((j + x) + rowy);

				b = bitmap->buffer[j / 8 + i * bitmap->pitch];
				b &= 1 << (7 - (j % 8));

				dest[idx + 3] = (b > 0) ? 0xff : 0; //the font is really the alpha
			}
		}
	}
}

void Font::_blitborder(byte* dest, FT_Bitmap* bitmap, int x, int y, int destside, const Color& col) {
	DEBUG_ASSERT( dest, "null destination buffer" );
	DEBUG_ASSERT( bitmap, "Null freetype bitmap" );

	int rowy;
	byte* ptr;

	for (uint32_t i = 0; i < bitmap->rows; ++i) {
		rowy = (i + y) * destside;

		for (uint32_t j = 0; j < bitmap->width; ++j) {
			ptr = dest + 4 * ((j + x) + rowy);

			float a = (float)bitmap->buffer[j + i * bitmap->pitch];
			a /= 255.f;
			float inva = 1.f - a;

			ptr[0] = (byte)(ptr[0] * inva + a * col.r * 255.f);
			ptr[1] = (byte)(ptr[1] * inva + a * col.g * 255.f);
			ptr[2] = (byte)(ptr[2] * inva + a * col.b * 255.f);
			ptr[3] = (byte)(ptr[3] * inva + a * a * 255.f); //the font is really the alpha
		}
	}
}

float gCurrentScale;

int _moveTo(const FT_Vector* to, void* ptr) {
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->startPath(Vector(to->x * gCurrentScale, to->y * gCurrentScale));

	return 0;
}

int _lineTo(const FT_Vector* to, void* ptr) {
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->addSegment(Vector(to->x * gCurrentScale, to->y * gCurrentScale));
	return 0;
}

int _conicTo(const FT_Vector* control, const FT_Vector* to, void* ptr) {
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->addQuadradratic(
		Vector(control->x * gCurrentScale, control->y * gCurrentScale),
		Vector(to->x * gCurrentScale, to->y * gCurrentScale),
		((Font::Character*)ptr)->parentPage.unwrap().getFont().getPolyOutlineQuality());

	return 0;
}

int _cubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* ptr) {
	auto t = ((Font::Character*)ptr)->getTesselation();

	t->addCubic(
		Vector(control1->x * gCurrentScale, control1->y * gCurrentScale),
		Vector(control2->x * gCurrentScale, control2->y * gCurrentScale),
		Vector(to->x * gCurrentScale, to->y * gCurrentScale),
		((Font::Character*)ptr)->parentPage.unwrap().getFont().getPolyOutlineQuality());

	return 0;
}

void Font::Character::init(Page& page, uint32_t c, int x, int y, int sx, int sy, FT_Glyph_Metrics& metrics, FT_Outline& outline) {

	float fsx = (float)sx;
	float fsy = (float)sy;
	float fw = (float)page.getFont().getFontWidth();
	float fh = (float)page.getFont().getFontHeight();
	
	character = c;
	gliphIdx = page.getFont().getCharIndex(self);

	pixelWidth = page.getFont().mCellWidth;
	float pixelHeight = (float)metrics.height * FONT_PPI + page.getFont().glowRadius * 2;

	uvWidth = (float)pixelWidth / fsx;
	uvHeight = pixelHeight / fsy;
	uvPos.x = (float)x / fsy;
	uvPos.y = (float)y / fsy;
	widthRatio = (float)pixelWidth / fw;
	heightRatio = pixelHeight / fh;

	bearingU = ((float)metrics.horiBearingX * FONT_PPI) / fsy;
	bearingV = ((float)(metrics.height - metrics.horiBearingY) * FONT_PPI) / fh;

	advance = ((float)metrics.horiAdvance * FONT_PPI) / fw;

	if (page.getFont().generateEdge || page.getFont().generateSurface) { //tesselate ALL the things!
		Timer timer;

		mTesselation = make_unique<Tessellation>();

		//find the normalizing scale and call the tesselation functions
		gCurrentScale = (float)FONT_PPI / (float)fw;
		FT_Outline_Funcs funcs = {_moveTo, _lineTo, _conicTo, _cubicTo, 0, 0};

		FT_Outline_Decompose(&outline, &funcs, &self);

		//now that everything is loaded & in order, tessellate the mesh
		if (mTesselation->segments.size() && page.getFont().generateSurface) { //HACK

			int options = Tessellation::PREPARE_EXTRUSION | Tessellation::GUESS_HOLES;

			if (!page.getFont().generateEdge) {
				options |= Tessellation::CLEAR_INPUTS;
			}

			mTesselation->tessellate(options); //keep edges if they are needed too
		}
	}

	parentPage = page;
}


Font::Page::Page(Font& font, int index) :
	Resource(),
	index(index),
	firstCharIdx(index * FONT_CHARS_PER_PAGE),
	font(font),
	texture(make_unique<Texture>()) {
}

Font::Page::~Page() {

}


bool Font::Page::onLoad() {
	//create the texture
	int sx = font.mCellWidth * FONT_PAGE_SIDE;
	int sy = font.mCellHeight * FONT_PAGE_SIDE;

	bool npot = Platform::singleton().isNPOTEnabled();
	int sxp2 = npot ? sx : glm::powerOfTwoAbove(sx);
	int syp2 = npot ? sy : glm::powerOfTwoAbove(sy);

	int pixelNumber = sxp2 * syp2;
	std::vector<byte> buf(pixelNumber * 4);

	auto ptr = (unsigned int*)buf.data();

	//set alpha to 0 and colours to white
	for (int i = sxp2 * syp2 - 1; i >= 0; --i) {
		*ptr++ = 0x00ffffff;
	}

	//render into buffer
	FT_Render_Mode renderMode = (font.isAntialiased()) ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO;
	FT_GlyphSlot slot = font.face->glyph;
	FT_Bitmap* bitmap;
	int gliphidx;

	font._prepareFace();

	auto code = (uint32_t)firstCharIdx;
	
	Font::Character* character = chars.data();
	int x, y;

	Timer timer;

	for (int i = 0; i < FONT_PAGE_SIDE; ++i) {
		for (int j = 0; j < FONT_PAGE_SIDE; ++j, ++code, ++character) {
			gliphidx = FT_Get_Char_Index(font.face, code);
			FT_Load_Glyph(font.face, gliphidx, FT_LOAD_DEFAULT);

			x = j * font.mCellWidth;
			y = i * font.mCellHeight;

			//load character data
			character->init(
				self,
				code,
				x, y,
				sxp2, syp2,
				slot->metrics,
				slot->outline
			);

			FT_Render_Glyph(slot, renderMode);

			bitmap = &(slot->bitmap);

			//blit the bitmap if it was rendered
			if (bitmap->buffer) {
				_blit(buf.data(), bitmap, x + font.glowRadius, y + font.glowRadius, sxp2);
			}
		}
	}

	//glow?
	if (font.glowRadius > 0) {
		unsigned int glowCol = font.glowColor.toRGBA();
		byte* glowColChannel = (byte*)&glowCol;

		//duplicate the buffer
		auto glowBuf = buf;

		for (int iteration = 0; iteration < font.glowRadius; ++iteration) {
			for (int i = 1; i < syp2 - 1; ++i) {
				for (int j = 1; j < sxp2 - 1; ++j) {
					byte* cur = glowBuf.data() + (j + i * sxp2) * 4;
					byte* up = glowBuf.data() + (j + (i + 1) * sxp2) * 4;
					byte* down = glowBuf.data() + (j + (i - 1) * sxp2) * 4;
					byte* left = glowBuf.data() + (j + 1 + i * sxp2) * 4;
					byte* right = glowBuf.data() + (j - 1 + i * sxp2) * 4;

					cur[0] = glowColChannel[0];
					cur[1] = glowColChannel[1];
					cur[2] = glowColChannel[2];
					cur[3] = (byte)((float)(up[3] + down[3] + left[3] + right[3]) * 0.25f);
				}
			}
		}

		//now alpha-blend the blur over the original buffer
		for (int i = 0; i < pixelNumber; ++i) {
			byte* orig = buf.data() + i * 4;
			byte* glow = glowBuf.data() + i * 4;

			float s = (float)orig[3] / 255.f; //blend using the alpha in the original buffer

			for (int channel = 0; channel < 4; ++channel) {
				orig[channel] = (byte)(orig[channel] * s + glow[channel] * (1.f - s));
			}
		}
	}

	//drop the buffer in the texture
	//TODO can probably easily use 565 or less
	loaded = texture->loadFromMemory(buf.data(), sxp2, syp2, PixelFormat::RGBA_8_8_8_8);
	texture->disableBilinearFiltering();
	texture->disableTiling();

	return loaded;
}


void Font::Page::onUnload(bool soft /*= false */) {
	//a font page can always unload
	texture->onUnload(); //force unload

	loaded = false;
}


/// --------------------------------------------------------------------------------

/// --------------------------------------------------------------------------------

Font::Font(optional_ref<ResourceGroup> creator, const utf::string& path) :
	Resource(creator, path) {

}

Font::~Font() {

}

bool Font::onLoad() {
	DEBUG_ASSERT( !isLoaded(), "onLoad: this font is already loaded" );

	Table t = Platform::singleton().load(filePath);

	fontFile = Path::getDirectory(filePath) + '/' + t.getString("truetype");
	fontWidth = fontHeight = t.getInt("size");

	antialias = t.getBool("antialias");
	kerning = t.getBool("kerning");
	generateEdge = t.getBool("polyOutline");
	generateSurface = t.getBool("polySurface");
	spacing = t.getNumber("spacing");

	mPolyOutlineQuality = t.getNumber("polyOutlineQuality", 10);

	glowRadius = t.getInt("glowRadius");
	glowColor = t.getColor("glowColor");

	mCellWidth = fontWidth + glowRadius * 2;
	mCellHeight = fontHeight + glowRadius * 2;

	face = Platform::singleton().getFontSystem().getFace(fontFile);

	auto& preload = t.getTable("preloadedPages");

	for (int i = 0; i < preload.getArrayLength(); ++i) {
		getPage(preload.getInt(i));
	}

	//load existing pages that were trimmed during a previous unload
	for (auto&& pair : pages) {
		if (!pair.second->isLoaded()) {
			pair.second->onLoad();
		}
	}

	return loaded = true;
}

void Font::onUnload(bool soft) {
	for (auto&& pair : pages) {
		if (pair.second->isLoaded()) {
			pair.second->onUnload();
		}
	}

	if (!soft) {
		pages.clear();
	}

	loaded = false;
}

float Font::getKerning(const Character& next, const Character& prev) {
	DEBUG_ASSERT( kerning, "getKerning: kerning is not enabled on this font" );

	FT_Vector vec;

	FT_Get_Kerning(
		face,
		prev.gliphIdx,
		next.gliphIdx,
		FT_KERNING_DEFAULT,
		&vec);

	return ((float)vec.x * FONT_PPI) / (float)fontWidth;
}

void Font::_prepareFace() {
	//set dimensions
	FT_Set_Pixel_Sizes(
		face,
		fontWidth,
		fontHeight);
}

int Font::getPixelLength(const utf::string& str) {
	int l = 0;

	auto lastChar = optional_ref<Character>();

	for(auto&& c : str) {
		auto& chr = getCharacter(c);
		l += (int)(chr.advance * chr.pixelWidth);

		if (lastChar.is_some() && isKerningEnabled()) {
			l += (int)(getKerning(chr, lastChar.unwrap()) * fontWidth);
		}

		lastChar = chr;
	}

	return l;
}

Font::Page& Font::getPage(int index) {
	DEBUG_ASSERT(index < FONT_MAX_PAGES, "getPage: requested page index is past the max page index");

	PageMap::iterator itr = pages.find(index);

	if (itr == pages.end()) {
		auto& p = *(pages[index] = make_unique<Page>(self, index));
		p.onLoad();
		return p;
	}
	else {
		return *itr->second;
	}
}

int Font::getCharIndex(Character& c) {
	return FT_Get_Char_Index(face, c.character);
}

Font::Character& Font::getCharacter(uint32_t c) {
	return getPageForChar(c).getChar(c);
}

Texture& Font::getTexture(uint32_t c) {
	return getPageForChar(c).getTexture();
}

void Font::preloadPages(const char pages[], int n) {
	for (int i = 0; i < n; ++i) {
		getPage(pages[i]);
	}
}
