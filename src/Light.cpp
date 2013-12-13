#include "stdafx.h"

#include "Light.h"
#include "GameState.h"
#include "Platform.h"
#include "Render.h"

using namespace Dojo;

Light::Light( GameState* s, 
	  const Vector& pos, 
	  const Color& diffuse, 
	  const Color& specular, 
	  const Color& ambient ) :
Object( s, pos, Vector::ZERO ),
type( LT_NONE ),
specularExponent(10)
{
	setColors( diffuse, specular, ambient );
}

void Light::setColors(	const Color& d, const Color& s, const Color& a )
{
	diffuse = d;
	specular = s;
	ambient = a;
}

///setups this light to be a point light
void Light::initPoint( float a )
{
	type = LT_POINT;
	
	attenuation = a;
}

///setups this light to be a directional light
void Light::initDirectional( const Vector& dir )
{
	type = LT_DIRECTIONAL;
	
	position = dir;
}

///setups this light to be a spotlight
void Light::initSpotlight( const Vector& dir, float fov, float exponent, float a )
{
	type = LT_SPOT;
	
	spotFOV = fov;
	spotExponent = exponent;
	attenuation = a;
	
    DEBUG_TODO;
    //angle = dir.angleFromDirection();
}

inline void toFV( float* fv, const Vector& v, float w )
{
	fv[0] = v.x;
	fv[1] = v.y;
	fv[2] = v.z;
	fv[3] = w;
}

void Light::bind( int i, const Matrix& view )
{
	GLenum light = GL_LIGHT0 + i;
	
	if( type == LT_NONE )
	{
		glDisable( light );
	}
	else
	{
		//setup all the lighting parameters
		glEnable( light );
		
		float* ambientp = hasAmbient() ? (float*) &ambient : (float*)&Platform::getSingleton()->getRender()->getDefaultAmbient();

		glLightfv( light, GL_DIFFUSE, (float*) &diffuse );
		glLightfv( light, GL_SPECULAR, (float*) &specular );
		glLightfv( light, GL_AMBIENT, ambientp );
		
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, ambientp );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)&diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, (float*)&specular );
		
		glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, specularExponent );

		Matrix world = glm::translate( Matrix(1.f), position );

		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixf( glm::value_ptr( view * world ) );

		glm::vec4 p( position.x, position.y, position.z, (type == LT_POINT) ? 1.f : 0.f );
		glLightfv( light, GL_POSITION, glm::value_ptr( p ) );

		if( type == LT_POINT ) //attenuation only affects POINT and SPOT lights
		{
			glLightf( light, GL_LINEAR_ATTENUATION, attenuation );
			
			if( type == LT_SPOT )
			{
                DEBUG_TODO;
			}
		}
	}	
}
