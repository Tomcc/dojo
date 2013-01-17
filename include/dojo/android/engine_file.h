#ifndef ENGINE_FILE_H
#define ENGINE_FILE_H

#ifdef __ANDROID__

#include <zzip/zzip.h>

	#ifdef __cplusplus
	extern "C"{
	#endif
	////////////////////
	//gestione Zip
	extern ZZIP_DIR* _apk_android;
	//
	extern void _apk_Open(const char * filename);
	//
	extern void _apk_Close();
	//
	const char *_apk_GetNameDir();
	ZZIP_DIR*   _apk_GetApk();
	////////////////////
	//#define THFILE ZZIP_FILE
	typedef ZZIP_FILE THFILE;
	//open a file
	extern THFILE* THfopen( const char * filename, const char * mode );
	//close a file
	extern void THfclose(THFILE * file);
	//read from file
	extern size_t THfread( void * ptr, size_t size, size_t count, THFILE * stream );
	//seek from file
	extern int THfseek ( THFILE * stream, long int offset, int origin );
	//tell from file
	extern long int THftell ( THFILE * stream );
	//return a uchar cast in int
    extern int THfgetc ( THFILE * stream );
	//rewind from file
    extern void THrewind ( THFILE * stream );
    //write to file
    extern size_t THfwrite ( const void * ptr, size_t size, size_t count, THFILE * stream );

	#ifdef __cplusplus
	}
	#endif


#endif

#endif
