/*
 *  ResourceGroup.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/27/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ResourceGroup_h__
#define ResourceGroup_h__

#include "dojo_config.h"

#include <map>
#include <string>

#include "Font.h"
#include "FrameSet.h"
#include "Mesh.h"

namespace Dojo {
	
	class ResourceGroup
	{			
	public:		
		
		typedef std::map<std::string, FrameSet*> FrameSetMap;
		typedef std::map<std::string, Font*> FontMap;
		typedef std::map<std::string, Mesh*> MeshMap;
			
		ResourceGroup();
		
		virtual ~ResourceGroup();
		
		inline bool isFrameSetLoaded( const std::string& name )
		{
			return frameSets.find( name ) != frameSets.end();
		}
		
		inline bool isFontLoaded( const std::string& name )
		{
			return fonts.find( name ) != fonts.end();
		}
		
		inline bool isMeshLoaded( const std::string& name )
		{
			return meshes.find( name ) != meshes.end();
		}
		
		inline FrameSet* getFrameSet( const std::string& name )
		{
			if( isFrameSetLoaded( name ) )
				return frameSets[name];
			
			return NULL;
		}
		
		inline void addFrameSet( FrameSet* set, const std::string& name )
		{
			//HACK
			//DOJO_ASSERT( !isFrameSetLoaded( name ) );
			
			frameSets[name] = set;
		}
		
		inline void addFont( Font* f, const std::string& name )
		{
			DOJO_ASSERT( !isFontLoaded( name ) );
			
			fonts[name] = f;
		}
		
		inline void addMesh( Mesh* m, const std::string& name )
		{
			DOJO_ASSERT( !isMeshLoaded( name ) );
			
			meshes[ name ] = m;
		}		
		
		inline FrameSet* getEmptyFrameSet()			{	return empty;	}
		
		inline Font* getFont( const std::string& name )
		{
			if( isFontLoaded( name ) )
				return fonts[name];
			
			return NULL;
		}
		
		inline Mesh* getMesh( const std::string& name )
		{
			if( isMeshLoaded( name ) )
				return meshes[name];
			
			return NULL;
		}
						
		void loadSets( const std::string& folder = "" );		
		void loadFonts( const std::string& subdirectory = "" );
		void loadMeshes( const std::string& folder = "" );
				
		void unloadSets();
		void unloadFonts();
		void unloadMeshes();
		
		void unloadAll()
		{
			unloadSets();
			unloadFonts();
			unloadMeshes();
		}
		
	protected:
		
		FrameSet* empty;
		
		FrameSetMap frameSets;
		FontMap fonts;
		MeshMap meshes;
	};
}

#endif 