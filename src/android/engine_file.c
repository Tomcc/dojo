#include <stdio.h>
#include "engine_file.h"

#ifdef __ANDROID__
#include <string.h>
////////////////////
//gestione Zip
ZZIP_DIR* _apk_android;
char _apk_filename[1024];
//
void _apk_Open(const char * filename){
		_apk_android=zzip_opendir_ext_io(filename,ZZIP_CASELESS|ZZIP_ONLYZIP, 0, 0);	
		strcpy (_apk_filename,filename);
}
//
void _apk_Close(){		
		zzip_closedir (_apk_android);
		_apk_filename[0]='\0';
}
////////////////////
const char *_apk_GetNameDir(){
	return (char*)_apk_filename;
}

ZZIP_DIR* _apk_GetApk(){
	return _apk_android;
}
//open a file
THFILE* THfopen( const char * filename, const char * mode ){

	int mode_flags=ZZIP_CASELESS ;
	if(mode){
		if(mode[0]){
		//first char
			if(mode[0]=='r') 
				mode_flags|=O_RDONLY;
		#if defined( WIN32 ) || defined(__WIN32) || defined(_WIN32) || defined(__WIN32__)
			else if(mode[0]=='b') 
				mode_flags|=O_BINARY;
		#endif
		//second char	
			if(mode[1]){
				if(mode[1]=='r') 
					mode_flags|=O_RDONLY;
		#if defined( WIN32 ) || defined(__WIN32) || defined(_WIN32) || defined(__WIN32__)
				else if(mode[0]=='b') 
					mode_flags|=O_BINARY;
		#endif
			}
			
		}
	}		
	
	//global dir
	int len_file_dir=strlen(filename);	
	//strlen ("assets/") = 7
	char *realdir=malloc(sizeof(char)*(len_file_dir+7+1));
	//write "assets/"
	realdir[0]='a';	realdir[1]='s';	realdir[2]='s';
	realdir[3]='e';	realdir[4]='t';	realdir[5]='s';
	realdir[6]='/';
	//copy filename to realdir
	strcpy(&realdir[7],filename);
	//sicurezza
	realdir[len_file_dir+7]='\0'; 
	
	//open file
	THFILE *file_out=zzip_file_open (_apk_android,realdir, mode_flags);
	
	//free memory
	free(realdir);
	
	return file_out;
}
//close a file
void THfclose(THFILE * file){
		zzip_close(file);
}
//read from file
size_t THfread ( void * ptr, size_t size, size_t count, THFILE * stream ){
    return zzip_read( stream, ptr, size*count ) / size;
}
//seek from file
int THfseek ( THFILE * stream, long int offset, int origin ){
    return (zzip_seek(stream, offset, origin) !=-1 ? 0 : -1);
}
//tell from file
long int THftell ( THFILE * stream ){
	return zzip_tell(stream);
}
//return a uchar cast in int 
int THfgetc ( THFILE * stream ){ 
	char c;
	return (THfread(&c, sizeof(char), 1, stream) == 0) ? EOF : (int)c;
}
//rewind from file
void THrewind ( THFILE * stream ){
	zzip_rewind(stream);
}
//write to file
size_t THfwrite ( const void * ptr, size_t size, size_t count, THFILE * stream ) { 
	return zzip_write( stream, ptr, size*count ) / size;
}
#endif

