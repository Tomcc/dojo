#ifndef PolyTextArea_h__
#define PolyTextArea_h__

#include "dojo_common_header.h"

#include "Renderable.h"
#include "Font.h"

namespace Dojo
{
	class PolyTextArea : public Renderable
	{
	public:

		enum RenderingType
		{
			RT_OUTLINE,
			RT_SURFACE,
			RT_EXTRUDED
		};

		///creates a new PolyTextArea object at position, using "font", centered or not around the center, using rendering options RT
		PolyTextArea( Object* parent, const Vector& position, Font* font, bool centered, RenderingType rt, float extrudeDepth = 0 );

		virtual ~PolyTextArea();

		///adds some text to this poly area
		void addText( const String& str )
		{
			mContent += str;
			mDirty = true;
		}

		///replaces the poly area content with this text
		void setText( const String& str )
		{
			mContent = str;
			mDirty = true;
		}

		void clear()
		{
			mContent = String::EMPTY;
			mDirty = true;
		}

		virtual void onAction( float dt )
		{
			if( mDirty )
				_prepare();

			Renderable::onAction(dt);
		}

	protected:

		String mContent;

		bool mCentered;
		Mesh* mMesh;
		Font* pFont;

		RenderingType mRendering;
		float mDepth;
		float mSpaceWidth;

		bool mDirty;

		void _centerLine( int rowStartIdx, float rowWidth );

		void _prepare();

	private:
	};
}


#endif // PolyTextArea_h__
