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

#include "dojo_common_header.h"

#include <string>

#include "Font.h"
#include "FrameSet.h"
#include "Mesh.h"
#include "SoundSet.h"

namespace Dojo {
	
	class ResourceGroup : public BaseObject
	{			
	public:		
		
		typedef std::map<std::string, FrameSet*> FrameSetMap;
		typedef std::map<std::string, Font*> FontMap;
		typedef std::map<std::string, Mesh*> MeshMap;
		typedef std::map<std::string, SoundSet*> SoundMap;
			
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

		inline bool isSoundLoaded( const std::string& name )
		{
			return sounds.find( name ) != sounds.end();
		}
				
		inline void addFrameSet( FrameSet* set, const std::string& name )
		{
			DEBUG_ASSERT( !isFrameSetLoaded( name ) );
			
			frameSets[name] = set;
		}
		
		inline void addFont( Font* f, const std::string& name )
		{
			DEBUG_ASSERT( !isFontLoaded( name ) );
			
			fonts[name] = f;
		}
		
		inline void addMesh( Mesh* m, const std::string& name )
		{
			DEBUG_ASSERT( !isMeshLoaded( name ) );
			
			meshes[ name ] = m;
		}		
		
		inline void addSound( SoundSet* sb, const std::string& name )
		{
			DEBUG_ASSERT( !isSoundLoaded( name ) );

			sounds[ name ] = sb;
		}

		inline FrameSet* getEmptyFrameSet()			{	return empty;	}

		inline FrameSet* getFrameSet( const std::string& name )
		{
			if( isFrameSetLoaded( name ) )
				return frameSets[name];

			return NULL;
		}
		
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

		inline SoundSet* getSound( const std::string& name )
		{
			if( isSoundLoaded( name ) )
				return sounds[ name ];

			return NULL;
		}
						
		void loadSets( const std::string& folder );		
		void loadFonts( const std::string& folder );
		void loadMeshes( const std::string& folder );
		void loadSounds( const std::string& folder );

		void loadResources( const std::string& folder )
		{
			loadSets( folder );
			loadFonts( folder );
			loadMeshes( folder );
			loadSounds( folder );
		}
				
		void unloadSets();
		void unloadFonts();
		void unloadMeshes();
		void unloadSounds();
		
		void unloadAll()
		{
			unloadSets();
			unloadFonts();
			unloadMeshes();
			unloadSounds();
		}
		
	protected:
		
		FrameSet* empty;
		
		FrameSetMap frameSets;
		FontMap fonts;
		MeshMap meshes;
		SoundMap sounds;
	};
}

#endif 