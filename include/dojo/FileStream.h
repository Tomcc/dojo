#pragma once

#include "dojo_common_header.h"

#include "Stream.h"

namespace Dojo {
	///FileStream is an interface to read and write from files
	/**
	FileStream will try to open the file as read+write, falling back to readonly if the file cannot be read- remember to check the access type!
	*/
	class FileStream : public Stream {
	public:

		///create a new FileStream associated to this filesystem path
		/**
		\param path the path
		\param write open the file for writing, create if necessary
		this WILL NOT open the file for i/o, remember to use open()
		*/
		FileStream(const std::string& path);

		virtual ~FileStream();

		///opens the file
		/**
		\returns the effective Access level granted. It might be read-only even if write was specified (ie. file is in signed iOS packages)
		*/
		virtual bool open(Access accessType) = 0;

		///closes the file and flushed any buffered content
		/**
		\remark the FileStream must have been opened
		*/
		virtual void close() = 0;

	protected:

		std::string mPath;
	private:
	};
}
