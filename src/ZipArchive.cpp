#include "ZipArchive.h"

extern "C" {
#include <zzip/write.h>
}

using namespace Dojo;

//ZipFile imp
ZipFile::ZipFile(ZZIP_FILE* zf) {
	file = zf;
}

ZipFile::~ZipFile() {
	close();
}

//close file
void ZipFile::close() {
	if (file != nullptr) {
		zzip_close(file);
	}

	file = nullptr;
}

//read from file
size_t ZipFile::read(void* ptr, size_t size, size_t count) {
	return zzip_read(file, ptr, size * count) / size;
}

//seek from file
int ZipFile::seek(int64_t offset, int origin) {
	return (zzip_seek(file, (zzip_off_t)offset, origin) != -1 ? 0 : -1);
}

//tell from file
int64_t ZipFile::tell() {
	return zzip_tell(file);
}

//get file size
int64_t ZipFile::size() {
	seek(0L, SEEK_END);
	int64_t size = tell();
	seek(0L, SEEK_SET);
	return size;
}

//return a uchar cast in int
int ZipFile::getc() {
	char c;
	return (read(&c, sizeof(char), 1) == 0) ? EOF : (int)c;
}

//rewind from file
void ZipFile::rewind() {
	zzip_rewind(file);
}

//write to file
size_t ZipFile::write(const void* ptr, size_t size, size_t count) {
	return zzip_write(file, ptr, size * count) / size;
}


//ZipArchive imp
ZipArchive::ZipArchive(): zip_file(nullptr) {
}

//
ZipArchive::ZipArchive(const utf::string& path) {
	open(path);
}

//
ZipArchive::~ZipArchive() {
	close();
}

//open zip file
bool ZipArchive::open(const utf::string& path) {
	zip_file = zzip_opendir_ext_io(path.bytes().data(), ZZIP_CASELESS | ZZIP_ONLYZIP, 0, 0);
	return zip_file != nullptr;
}

//close zip file
void ZipArchive::close() {
	if (zip_file != nullptr) {
		zzip_closedir(zip_file);
	}

	zip_file = nullptr;
}

//open file
Unique<ZipFile> Dojo::ZipArchive::openFile(const utf::string& path, const utf::string& mode) {
	if (zip_file != nullptr) {
		zzip_rewinddir(zip_file);
		int mode_flags = ZZIP_CASELESS;

		//fake fopen MODE like C
		if (mode.size()) {
#ifndef WIN32
#define  O_BINARY 0x0000
#endif
			mode_flags |= mode.front() == 'b' ? O_BINARY : 0;
			mode_flags |= mode.front() == 'r' ? O_RDONLY : 0;

			if (mode.size() >= 2) {
				mode_flags |= *(mode.begin()+1) == 'b' ? O_BINARY : 0;
				mode_flags |= *(mode.begin()+1) == 'r' ? O_RDONLY : 0;
			}
		}

		ZZIP_FILE* file_out = zzip_file_open(zip_file, path.bytes().data(), mode_flags);

		if (file_out != nullptr) {
			return make_unique<ZipFile>(file_out);
		}
	}

	return nullptr;
}

//open paths info
void ZipArchive::getList(const utf::string& inPath, std::vector<utf::string>& out) {
	if (zip_file != nullptr) {
		auto path = makeValidPath(inPath);
		ZZIP_DIRENT* dirp;
		int size = 0;
		int size_path = path.size();

		while ((dirp = zzip_readdir(zip_file)) != nullptr) {
			size = strlen(dirp->d_name);

			if (size_path < size && //not this dir and prev path
					strncmp(dirp->d_name, path.bytes().data(), size_path) == 0) { //path is a "sub path"

				//is not a sub sub directory?
				bool isnotasubsubdir = true;

				for (int c = size - 2; size_path <= c; --c) {
					if (dirp->d_name[c] == '/') {
						isnotasubsubdir = false;
						break;
					}
				}

				if (isnotasubsubdir) {
					out.emplace_back(&dirp->d_name[size_path]);
				}

			}
		}

		zzip_rewinddir(zip_file);
	}
}

void ZipArchive::getListAll(const utf::string& inPath, std::vector<utf::string>& out) {
	if (zip_file != nullptr) {
		auto path = makeValidPath(inPath);
		ZZIP_DIRENT* dirp;
		int size = 0;
		int size_path = path.size();

		while ((dirp = zzip_readdir(zip_file)) != nullptr) {
			size = strlen(dirp->d_name);

			if (size_path < size && //not this dir and prev path
					strncmp(dirp->d_name, path.bytes().data(), size_path) == 0) { //path is a "sub path"
				out.emplace_back(&dirp->d_name[size_path]);
			}
		}

		zzip_rewinddir(zip_file);
	}
}

void ZipArchive::getListFiles(const utf::string& inPath, std::vector<utf::string>& out) {
	if (zip_file != nullptr) {
		auto path = makeValidPath(inPath);
		ZZIP_DIRENT* dirp;
		int size = 0;
		int size_path = path.size();

		while ((dirp = zzip_readdir(zip_file)) != nullptr) {
			size = strlen(dirp->d_name);

			if (dirp->d_name[size - 1] != '/') { //is not a dir
				if (size_path < size && //not this dir and prev path
						strncmp(dirp->d_name, path.bytes().data(), size_path) == 0) { //file is in this directory

					//is not in sub sub directory?
					bool isnotasubsubdir = true;

					for (int c = size - 2; size_path <= c; --c) {
						if (dirp->d_name[c] == '/') {
							isnotasubsubdir = false;
							break;
						}
					}

					if (isnotasubsubdir) {
						out.emplace_back(&dirp->d_name[size_path]);
					}

				}
			}
		}

		zzip_rewinddir(zip_file);
	}
}

void ZipArchive::getListAllFiles(const utf::string& inPath, std::vector<utf::string>& out) {
	if (zip_file != nullptr) {
		auto path = makeValidPath(inPath);
		ZZIP_DIRENT* dirp;
		int size = 0;
		int size_path = path.size();

		while ((dirp = zzip_readdir(zip_file)) != nullptr) {
			size = strlen(dirp->d_name);

			if (dirp->d_name[size - 1] != '/') { //is not a dir
				if (size_path < size && //not this dir and prev path
						strncmp(dirp->d_name, path.bytes().data(), size_path) == 0) { //file is in this directory
					out.emplace_back(&dirp->d_name[size_path]);
				}
			}
		}

		zzip_rewinddir(zip_file);
	}
}

void ZipArchive::getListSubDirectories(const utf::string& inPath, std::vector<utf::string>& out) {
	if (zip_file != nullptr) {
		auto path = makeValidPath(inPath);
		ZZIP_DIRENT* dirp;
		int size = 0;
		int size_path = path.size();

		while ((dirp = zzip_readdir(zip_file)) != nullptr) {
			size = strlen(dirp->d_name);

			if (dirp->d_name[size - 1] == '/') { //is a dir
				if (size_path < size && //not this dir and prev path
						strncmp(dirp->d_name, path.bytes().data(), size_path) == 0) { //path is a "sub path"

					//is not a sub sub directory?
					bool isnotasubsubdir = true;

					for (int c = size - 2; size_path <= c; --c) {
						if (dirp->d_name[c] == '/') {
							isnotasubsubdir = false;
							break;
						}
					}

					if (isnotasubsubdir) {
						out.emplace_back(&dirp->d_name[size_path]);
					}

				}
			}
		}

		zzip_rewinddir(zip_file);
	}
}

void ZipArchive::getListAllSubDirectories(const utf::string& inPath, std::vector<utf::string>& out) {
	if (zip_file != nullptr) {
		auto path = makeValidPath(inPath);
		ZZIP_DIRENT* dirp;
		int size = 0;
		int size_path = path.size();

		while ((dirp = zzip_readdir(zip_file)) != nullptr) {
			size = strlen(dirp->d_name);

			if (dirp->d_name[size - 1] == '/') { //is a dir
				if (size_path < size && //not this dir and prev path
						strncmp(dirp->d_name, path.bytes().data(), size_path) == 0) { //path is a "sub path"
					out.emplace_back(&dirp->d_name[size_path]);
				}
			}
		}

		zzip_rewinddir(zip_file);
	}
}

utf::string ZipArchive::makeValidPath(const utf::string& path) {
	DEBUG_DEPRECATED; //this is pretty bad
	return{};

	// 	/* 1 delete \  */
	// 	for (auto&& c : path) {
	// 		if (c == '\\')
	// 			c = '/';
	// 	}
	// 	/* 2 delete void string:  "path/    " */
	// 	for (int i = path.size() - 1; -1 < i; --i) {
	// 		if (path[i] == '/')
	// 			break;
	// 		if (path[i] != ' ' && path[i] != '\0') {
	// 			path = path.substr(0, i + 1) + '/'; //delete void path
	// 			break;
	// 		}
	// 	}
	// 	/* 3 delete //  */
	// 	for (int i = path.size() - 1; 0 < i; --i) {
	// 		if (path[i] == '/' && path[i - 1] == '/') {
	// 			path = path.substr(0, i - 2) + path.substr(i, path.size() - i);
	// 			break;
	// 		}
	// 	}
	// 	/* delete . */
	// 	if (path.size() && path[0] == '.')
	// 		path = path.substr(1, path.size() - 1);
	// 	/* delete /path */
	// 	if (path.size() && path[0] == '/')
	// 		path = path.substr(1, path.size() - 1);
}
