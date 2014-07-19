#ifndef Light_h__
#define Light_h__

#include "dojo_common_header.h"

#include "Object.h"
#include "Color.h"

namespace Dojo
{
	///A Light affects a single Render::Layer which has its lighting enabled, and can be set to different color and types (point, spot and specular)
	class Light : public Object
	{
	public:
		
		enum Type
		{
			LT_NONE,
			LT_DIRECTIONAL,
			LT_POINT,
			LT_SPOT
		};
		
		///creates a light
		/**
		 passing Color::NIL to ambient means that the default ambient color will be used
		 */
		Light( GameState* s, 
			  const Vector& pos, 
			  const Color& diffuse = Color::WHITE, 
			  const Color& specular = Color::WHITE, 
			  const Color& ambient = Color::NIL );
		
		///sets the light diffuse, specular and ambient colors
		void setColors(	const Color& diffuse, const Color& specular, const Color& ambient );
		
		void setSpecularExponent( float exponent )
		{
			DEBUG_ASSERT( exponent >= 0 && exponent <= 128, "The specular exponent must be between 0 and 128" );
			
			specularExponent = exponent;
		}
		
		///setups this light to be a point light
		void initPoint( float attenuation = 0 );
		
		///setups this light to be a directional light
		void initDirectional( const Vector& dir );
		
		///setups this light to be a spotlight
		void initSpotlight( const Vector& dir, float fov, float exponent, float attenuation );
		
		Type getType()
		{
			return type;
		}
		
		const Color& getDiffuse()
		{
			return diffuse;
		}
		
		const Color& getSpecular()
		{
			return specular;
		}
		
		const Color& getAmbient()
		{
			return ambient;
		}
		
		bool hasAmbient()
		{
			return ambient.a + ambient.r + ambient.g + ambient.b > 0;
		}

		///binds this light to the given slot for OpenGL rendering
		void bind( int slot, const Matrix& viewProj );
		
	protected:
		
		bool changed;
		
		Type type;
		Color ambient, diffuse, specular;
		float specularExponent;
		
		float attenuation;
		float spotFOV, spotExponent;
	};
}

#endif
