/*
 *  RenderState.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/12/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef RenderState_h__
#define RenderState_h__

#include "dojo_common_header.h"

#include "Color.h"
#include "Vector.h"
#include "Array.h"
#include "dojomath.h"

namespace Dojo
{
	class Texture;
	class Mesh;
	class Shader;
	
	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState 
	{
	public:

		///an enum describing high-level photoshop-like blending modes
		enum BlendingMode
		{
			BM_ALPHA,
			BM_MULTIPLY,
			BM_ADD
		};

		class TextureUnit
		{
		public:

			Texture* texture;

			TextureUnit() :
			scale( 1,1 ),
			rotation( 0 ),
			offset( 0,0 ),
			texture( nullptr ),
			optTransform( nullptr )
			{

			}

			~TextureUnit()
			{
				if( optTransform )
					SAFE_DELETE( optTransform );
			}

			void applyTransform()
			{
				DEBUG_ASSERT( optTransform, "Tried to apply a non-existing texture transform" );

				glMatrixMode( GL_TEXTURE );
				glLoadMatrixf( glm::value_ptr( *optTransform ) );
			}

			inline void setOffset( const Vector& v )
			{
				offset = v;
				_updateTransform();
			}

			inline void setScale( const Vector& v )
			{
				scale = v;
				_updateTransform();
			}

			inline void setRotation( const float r )
			{
				rotation = r;
				_updateTransform();
			}

			inline const Vector& getOffset()
			{
				return offset;
			}

			inline const Vector& getScale()
			{
				return scale;
			}

			inline float getRotation()
			{
				return rotation;
			}

			inline const Matrix& getTransform()
			{
				static const Matrix identityMatrix;
				return isTransformRequired() ? *optTransform : identityMatrix;
			}

			inline bool isTransformRequired()
			{
				return optTransform != nullptr;
			}

		protected:

			Vector offset, scale;
			float rotation;

			Matrix* optTransform;

			void _updateTransform()
			{
				if( !optTransform )
					optTransform = new Matrix;

				//build the transform
				*optTransform = glm::scale( Matrix(1), scale );
				*optTransform = glm::translate( *optTransform, offset );
				*optTransform = glm::rotate( *optTransform, Math::toEuler(rotation), Vector::UNIT_Z );
			}

		};

		enum CullMode
		{
			CM_FRONT,
			CM_BACK,
			CM_DISABLED
		};
				
		Color color;

		GLenum srcBlend, destBlend, blendEquation;

		CullMode cullMode;

		RenderState() :
		mTextureNumber( 0 ),
		cullMode( CM_BACK ),
		blendingEnabled( true ),
		srcBlend( GL_SRC_ALPHA ),
		destBlend( GL_ONE_MINUS_SRC_ALPHA ),
		blendEquation( GL_FUNC_ADD ),
		mesh( nullptr ),
		pShader( nullptr )
		{
			memset( textures, 0, sizeof( textures ) ); //zero all the textures
		}
		
		virtual ~RenderState()
		{
			for( int i = 0; i < DOJO_MAX_TEXTURES; ++i )
				delete textures[i];
		}
		
		inline void setMesh( Mesh* m )
		{			
			DEBUG_ASSERT( m, "setMesh requires a non-null mesh" );

			mesh = m;
		}

		///Sets a texture in the required slot.
		/**
		It can be NULL, which means that the slot is disabled.
		*/
		inline void setTexture( Texture* tex, int ID = 0 )
		{
			DEBUG_ASSERT( ID >= 0, "Passed a negative texture ID to setTexture()" );
			DEBUG_ASSERT( ID < DOJO_MAX_TEXTURES, "An ID passed to setTexture must be smaller than DOJO_MAX_TEXTURE_UNITS" );

			if( textures[ID] == NULL ) //adding a new one
			{                
				++mTextureNumber;
				textures[ID] = new TextureUnit();
			}
			else if( tex == NULL )
				SAFE_DELETE( textures[ID] );

			textures[ID]->texture = tex;
		}

		///enables or disables blending of this RS
		inline void setBlendingEnabled( bool enabled )	{	blendingEnabled = enabled;	}

		///sets the blending mode with a single call
		void setBlending( GLenum src, GLenum dest )
		{
			srcBlend = src;
			destBlend = dest;
		}

		///sets an abstract photoshop-like blending mode
		void setBlending( BlendingMode mode )
		{
			static const GLenum modeToGLTable[] = {
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, //alphablend
				GL_DST_COLOR, GL_ZERO, //multiply
				GL_ONE, GL_ONE, //add
			};

			setBlending( modeToGLTable[mode*2], modeToGLTable[mode*2+1] );
		}

		///sets the Shader material to be used for this RenderState
		/**
		\remark the shader may be null to remove shader use
		*/
		void setShader( Shader* shader )
		{
			pShader = shader;
		}
				
		inline Texture* getTexture( int ID = 0 )
		{
			DEBUG_ASSERT( ID >= 0, "Can't retrieve a negative texture ID" );
			DEBUG_ASSERT( ID < DOJO_MAX_TEXTURES, "An ID passed to getTexture must be smaller than DOJO_MAX_TEXTURE_UNITS" );
	
			if( textures[ID] )
				return textures[ID]->texture;
			else
				return NULL;
		}

		inline TextureUnit* getTextureUnit( int ID )
		{
			DEBUG_ASSERT( ID >= 0, "Can't retrieve a negative textureUnit" );
			DEBUG_ASSERT( ID < DOJO_MAX_TEXTURES, "An ID passed to getTextureUnit must be smaller than DOJO_MAX_TEXTURE_UNITS" );

			return textures[ID];
		}

		///returns the Mesh currently used by this state
		inline Mesh* getMesh()								{	return mesh;			}

		///returns the Shader currently bound to this state
		inline Shader* getShader()
		{
			return pShader;
		}
		
		///returns the number of used texture units
		inline int getTextureNumber()
		{
			return mTextureNumber;
		}

		bool isAlphaRequired();
		
		///returns the "weight" of the changes needed to pass from "this" to "s"
		inline int getDistance( RenderState* s )
		{
			DEBUG_ASSERT( s, "getDistance: The input RenderState is null" );
			
			int dist = 0;
			
			if( s->mesh != mesh )
				dist += 3;
			
			for( int i = 0; i < DOJO_MAX_TEXTURES; ++i )
			{
				if( textures[i] != s->textures[i] )
					dist += 2;
			}
			
			if( s->isAlphaRequired() != isAlphaRequired() )
				dist += 1;
			
			return dist;
		}
		
		void applyState();
		
		void commitChanges( RenderState* nextState );

		///sets up destBlend and srcBlend to use normal alpha blending
		void useAlphaBlend()
		{
			srcBlend = GL_SRC_ALPHA;
			destBlend = GL_ONE_MINUS_SRC_ALPHA;
			blendEquation = GL_FUNC_ADD;
		}

		///sets up destBlend and srcBlend to use alpha+additive blending
		void useAdditiveBlend()
		{
			destBlend = GL_ONE;
			srcBlend = GL_SRC_ALPHA;
			blendEquation = GL_FUNC_ADD;
		}
				
	protected:
			
		bool blendingEnabled;
		
		TextureUnit* textures[ DOJO_MAX_TEXTURES ];
		int mTextureNumber;

		Mesh* mesh;

		Shader* pShader;

		void _bindTextureSlot( int i );
	};
}

#endif