#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H

#include <string>
#include <vector>
#include <zzip/zzip.h>

namespace Dojo{

	class ZipArchive;
	class ZipFile;

	class ZipFile
	{       
	public:

        friend class ZipArchive;

                ~ZipFile();
                //close file
                void close();
                //read from file
                size_t read(void * ptr, size_t size, size_t count);
                //seek from file
                int seek (long int offset, int origin );
                //tell from file
                long int tell ();
                //get file size
                long int size();
                //return a uchar cast in int
                int getc();
                //rewind from file
                void rewind ();
                //write to file
                size_t write ( const void * ptr, size_t size, size_t count);
       
	private:
        
                ZipFile(ZZIP_FILE* file);
                ZZIP_FILE* file;
	};

	class ZipArchive
	{
	public:
	
                ZipArchive();
                ZipArchive(const std::string& path);
                ~ZipArchive();
                //open zip file
                bool open(const std::string& path);
                //close zip file
                void close();
                //open file
                ZipFile* openFile(const std::string& path,const std::string& mode);
                //paths and files in zip
                void getList(std::string path,std::vector<std::string>& out);
                void getListFiles(std::string path,std::vector<std::string>& out);
                void getListSubDirectories(std::string path,std::vector<std::string>& out);

                void getListAll(std::string path,std::vector<std::string>& out);
                void getListAllFiles(std::string path,std::vector<std::string>& out);
                void getListAllSubDirectories(std::string path,std::vector<std::string>& out);

	private:

                void madeValidPath(std::string& path);
                ZZIP_DIR* zip_file;

	};


};


#endif
