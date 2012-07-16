#include "stdafx.h"

#include "Light.h"

using namespace Dojo;

Light::Light( GameState* s, 
	  const Vector& pos, 
	  const Color& diffuse, 
	  const Color& specular, 
	  const Color& ambient ) :
Object( s, pos, Vector::ZERO ),
type( LT_NONE )
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
	
	//set the direction
    DEBUG_TODO;
	//angle = dir.angleFromDirection();
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

void Light::bind( uint i, const Matrix& viewProj )
{
	GLenum light = GL_LIGHT0 + i;
	
	if( type == LT_NONE )
	{
		glDisable( light );
	}
	else
	{
		//setup all the lighting parameters
		float fv[4];
		
		glEnable( light );
		
		glLightfv( light, GL_DIFFUSE, (float*) &diffuse );
		glLightfv( light, GL_SPECULAR, (float*) &specular );
		
		glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, (float*)&ambient );
		glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, (float*)&diffuse );
		glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, (float*)&specular );
		
		glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, specularExponent );
		
		if( hasAmbient() )
			glLightfv( light, GL_AMBIENT, (float*) &ambient );
		
		if( type == LT_DIRECTIONAL )
		{
            DEBUG_TODO;
            /*
			Vector worldDirection = getWorldDirection();			
			toFV( fv, worldDirection, 1 );
			
			glLightfv( light, GL_POSITION, fv );*/
		}
		else
		{            
            Matrix m = viewProj * getWorldTransform();			
            glm::vec4 v = m * glm::vec4(0,0,0,1);
                       
			glLightfv( light, GL_POSITION, glm::value_ptr( v ) );
			
			glLightf( light, GL_LINEAR_ATTENUATION, attenuation );
			
			if( type == LT_SPOT )
			{
                DEBUG_TODO;
                /*
				glLightf( light, GL_SPOT_CUTOFF, spotFOV );
				glLightf( light, GL_SPOT_EXPONENT, spotExponent );
								
				Vector worldDirection = getWorldDirection();			
				toFV( fv, worldDirection, 1 );
				
				glLightfv( light, GL_SPOT_DIRECTION, fv );*/
			}
		}		
	}	
}
