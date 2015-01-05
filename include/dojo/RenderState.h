/*
 *  RenderState.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/12/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Color.h"
#include "Vector.h"
#include "Array.h"
#include "dojomath.h"
#include "BlendingMode.h"

namespace Dojo
{
	class Texture;
	class Mesh;
	class Shader;

	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState 
	{
	public:
		class TextureUnit
		{
		public:

			Texture* texture;

			TextureUnit();

			~TextureUnit();

			void applyTransform();

			void setOffset( const Vector& v )
			{
				offset = v;
				_updateTransform();
			}

			void setScale( const Vector& v )
			{
				scale = v;
				_updateTransform();
			}

			void setRotation( const float r )
			{
				rotation = r;
				_updateTransform();
			}

			const Vector& getOffset()
			{
				return offset;
			}

			const Vector& getScale()
			{
				return scale;
			}

			float getRotation()
			{
				return rotation;
			}

			const Matrix& getTransform() const;

			bool isTransformRequired() const
			{
				return optTransform != nullptr;
			}

		protected:

			Vector offset, scale;
			float rotation;

			Matrix* optTransform;

			void _updateTransform();

		};

		enum CullMode
		{
			CM_FRONT,
			CM_BACK,
			CM_DISABLED
		};
				
		Color color;

		GLenum srcBlend, destBlend, blendFunction;

		CullMode cullMode;

		RenderState();
		
		virtual ~RenderState();
		
		void setMesh( Mesh* m )
		{			
			DEBUG_ASSERT( m, "setMesh requires a non-null mesh" );

			mesh = m;
		}

		///Sets a texture in the required slot.
		/**
		It can be NULL, which means that the slot is disabled.
		*/
		void setTexture( Texture* tex, int ID = 0 );

		///enables or disables blending of this RS
		void setBlendingEnabled( bool enabled )	{	blendingEnabled = enabled;	}

		///sets the blending mode with a single call
		void setBlending( GLenum src, GLenum dest )
		{
			srcBlend = src;
			destBlend = dest;
		}

		///sets an abstract photoshop-like blending mode
		void setBlending( BlendingMode mode );

		///sets the Shader material to be used for this RenderState
		/**
		\remark the shader may be null to remove shader use
		*/
		void setShader( Shader* shader );
				
		Texture* getTexture( int ID = 0 ) const;

		const TextureUnit& getTextureUnit( int ID ) const;

		///returns the Mesh currently used by this state
		Mesh* getMesh()	const							{	return mesh;			}

		///returns the Shader currently bound to this state
		Shader* getShader()
		{
			return pShader;
		}
		
		///returns the number of used texture units
		int getTextureNumber()
		{
			return mTextureNumber;
		}

		bool isAlphaRequired();
		
		///returns the "weight" of the changes needed to pass from "this" to "s"
		int getDistance( RenderState* s );
		
		void applyState();
		
		void commitChanges();

	protected:
			
		bool blendingEnabled;
		
		TextureUnit* textures[ DOJO_MAX_TEXTURES ];
		int mTextureNumber;

		Mesh* mesh;

		Shader* pShader;

		void _bindTextureSlot( int i );
	};
}

