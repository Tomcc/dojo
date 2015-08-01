#pragma once

#include "dojo_common_header.h"

#include "Renderable.h"

namespace Dojo {
	class Font;
	class Tessellation;

	class PolyTextArea : public Renderable {
	public:

		enum RenderingType {
			RT_OUTLINE,
			RT_SURFACE,
			RT_EXTRUDED
		};

		///creates a new PolyTextArea object at position, using "font", centered or not around the center, using rendering options RT
		PolyTextArea(Object& parent, RenderLayer::ID layer, Font& font, bool centered, RenderingType rt);

		virtual ~PolyTextArea();

		///sets the extrusion parameters, changes taking effect once per frame
		/**to disable beveled extrusion, pass 0 to both bevelDepth and inflateRadius
		\param depth the depth of the extrusion as a fraction of the font height
		\param bevelDepth the depth of the bevel as a fraction of the font height
		\param inflateRadius the radius of the bevel's inflation as a fraction of the font height
		*/
		void setExtrusionParameters(float depth, float bevelDepth = 0, float inflateRadius = 0);

		///sets additional interline height
		void setInterline(float interline);

		///adds some text to this poly area
		void addText(const std::string& str);

		///replaces the poly area content with this text
		void setText(const std::string& str);

		void clear();

		///gets the current interline
		float getInterline();

		virtual void update(float dt) override;

	protected:

		std::string mContent;

		bool mCentered;
		Unique<Mesh> mMesh;
		Font* pFont;

		RenderingType mRendering;
		float mDepth, mBevelDepth, mInflateRadius;
		float mSpaceWidth, mInterline;

		bool mDirty;

		int mPrevLayerIdx;

		void _centerLine(int rowStartIdx, float rowWidth);

		void _prepare();

		void _tesselateExtrusionStrip(Tessellation* t, int layerAbaseIdx, int layerBbaseIdx);
		void _addExtrusionLayer(Tessellation* t, const Vector& origin, float inflate, const Vector* forcedNormal = nullptr);

	private:
	};
}
