#ifndef Light_h__
#define Light_h__

#include "dojo_common_header.h"

#include "Object.h"
#include "Color.h"

namespace Dojo
{
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
		
		void setColors(	const Color& diffuse, const Color& specular, const Color& ambient );
		
		void setSpecularExponent( float exponent )
		{
			DEBUG_ASSERT( exponent >= 0 );
			
			specularExponent = exponent;
		}
		
		///setups this light to be a point light
		void initPoint( float attenuation = 0 );
		
		///setups this light to be a directional light
		void initDirectional( const Vector& dir );
		
		///setups this light to be a spotlight
		void initSpotlight( const Vector& dir, float fov, float exponent, float attenuation );
		
		void disable()
		{
			type = LT_NONE;
		}
		
		void bind( uint slot, const Matrix& viewProj ); //updates light for rendering in the assigned slot
		
		inline Type getType()
		{
			return type;
		}
		
		inline const Color& getDiffuse()
		{
			return diffuse;
		}
		
		inline const Color& getSpecular()
		{
			return specular;
		}
		
		inline const Color& getAmbient()
		{
			return ambient;
		}
		
		inline bool hasAmbient()
		{
			return ambient.a + ambient.r + ambient.g + ambient.b > 0;
		}
		
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
