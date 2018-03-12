#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H

#include "dojo_common_header.h"

#include "dojostring.h"
#include <string>
#include <vector>
#include <zzip/zzip.h>

namespace Dojo {

	class ZipArchive;
	class ZipFile;

	class ZipFile {
	public:

		friend class ZipArchive;

		explicit ZipFile(ZZIP_FILE* file);
		~ZipFile();
		//close file
		void close();
		//read from file
		size_t read(void* ptr, size_t size, size_t count);
		//seek from file
		int seek(int64_t offset, int origin);
		//tell from file
		int64_t tell();
		//get file size
		int64_t size();
		//return a uchar cast in int
		int getc();
		//rewind from file
		void rewind();
		//write to file
		size_t write(const void* ptr, size_t size, size_t count);

	private:

		ZZIP_FILE* file;
	};

	class ZipArchive {
	public:

		ZipArchive();
		explicit ZipArchive(utf::string_view path);
		~ZipArchive();
		//open zip file
		bool open(utf::string_view path);
		//close zip file
		void close();
		//open file
		std::unique_ptr<ZipFile> openFile(utf::string_view path, utf::string_view mode);
		//paths and files in zip
		void getList(utf::string_view path, std::vector<utf::string>& out);
		void getListFiles(utf::string_view path, std::vector<utf::string>& out);
		void getListSubDirectories(utf::string_view path, std::vector<utf::string>& out);

		void getListAll(utf::string_view path, std::vector<utf::string>& out);
		void getListAllFiles(utf::string_view path, std::vector<utf::string>& out);
		void getListAllSubDirectories(utf::string_view path, std::vector<utf::string>& out);

	private:

		utf::string makeValidPath(utf::string_view path);
		ZZIP_DIR* zip_file;

	};


};


#endif
