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
	
	///A render state is responsibile of managing the state of the underlying OGL render minimising the changes to be done when it is activated
	class RenderState 
	{
	public:

		class TextureUnit
		{
		public:

			Texture* texture;

			TextureUnit() :
			scale( 1,1 ),
			rotation( 0 ),
			offset( 0,0 ),
			texture( NULL ),
			optTransform( NULL )
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
		mesh( NULL ),
		srcBlend( GL_SRC_ALPHA ),
		destBlend( GL_ONE_MINUS_SRC_ALPHA ),
		blendEquation( GL_FUNC_ADD )
		{
			memset( textures, 0, sizeof( textures ) ); //zero all the textures
		}
		
		virtual ~RenderState()
		{
			for( int i = 0; i < DOJO_MAX_TEXTURE_UNITS; ++i )
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
			DEBUG_ASSERT( ID < DOJO_MAX_TEXTURE_UNITS, "An ID passed to setTexture must be smaller than DOJO_MAX_TEXTURE_UNITS" );

			if( textures[ID] == NULL ) //adding a new one
			{                
				++mTextureNumber;
				textures[ID] = new TextureUnit();
			}
			else if( tex == NULL )
				SAFE_DELETE( textures[ID] );

			textures[ID]->texture = tex;
		}

		inline void setBlendingEnabled( bool enabled )	{	blendingEnabled = enabled;	}
				
		inline Texture* getTexture( int ID = 0 )
		{
			DEBUG_ASSERT( ID >= 0, "Can't retrieve a negative texture ID" );
			DEBUG_ASSERT( ID < DOJO_MAX_TEXTURE_UNITS, "An ID passed to getTexture must be smaller than DOJO_MAX_TEXTURE_UNITS" );
	
			if( textures[ID] )
				return textures[ID]->texture;
			else
				return NULL;
		}

		inline TextureUnit* getTextureUnit( int ID )
		{
			DEBUG_ASSERT( ID >= 0, "Can't retrieve a negative textureUnit" );
			DEBUG_ASSERT( ID < DOJO_MAX_TEXTURE_UNITS, "An ID passed to getTextureUnit must be smaller than DOJO_MAX_TEXTURE_UNITS" );

			return textures[ID];
		}

		inline Mesh* getMesh()								{	return mesh;			}
		
		inline int getTextureNumber()
		{
			return mTextureNumber;
		}

		bool isAlphaRequired();
		
		///returns the "weight" of the changes needed to pass from "this" to "s"
		inline uint getDistance( RenderState* s )
		{
			DEBUG_ASSERT( s, "getDistance: The input RenderState is null" );
			
			uint dist = 0;
			
			if( s->mesh != mesh )
				dist += 3;
			
			//TODO - this needs to be fixed for texture units
			for( int i = 0; i < DOJO_MAX_TEXTURE_UNITS; ++i )
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
				
	protected:
			
		bool blendingEnabled;
		
		TextureUnit* textures[ DOJO_MAX_TEXTURE_UNITS ];
		int mTextureNumber;

		Mesh* mesh;

		void _bindTextureSlot( int i );
	};
}

#endif