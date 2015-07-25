#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	///Stream represents a binary stream that can be either read-only, write-only or read+write
	/**
	it is an interface to be implemented by actual data sources.
	*/
	class Stream {
	public:

		///Stream::Access describes the kind of access to the underlying data a Stream offers
		enum class Access {
			///this Stream was not initialized or cannot find its source
			BadFile,
			///this Stream can only be used to read
			Read,
			///this Stream can only be used to write
			WriteOnly,
			///this Stream can be used both write and to read
			ReadWrite
		};

		virtual ~Stream() {

		}

		///reads up to "number" bytes from the stream into buf, returns the number of bytes read
		virtual int64_t read(byte* buf, int64_t number);

		void readToFill(std::string& buf);

		///writes the given buffer to the stream
		virtual void write(byte* buf, int size);

		void write(const std::string& buf) {
			write((byte*)buf.data(), (int)buf.size());
		}

		///returns the total bytes in the stream, -1 if this stream has no end
		virtual int64_t getSize() = 0;

		///returns the kind of access this stream provides, ie. read-only
		virtual Access getAccess() = 0;

		///returns the current reading/writing position
		virtual int64_t getCurrentPosition() = 0;

		///creates a new unopened Stream that points to the same underlying data, useful for concurrent reads in multithreading.
		virtual Unique<Stream> copy() const;

		///goes to the given position
		virtual int seek(int64_t offset, int64_t fromWhere = SEEK_SET) = 0;

		///utility method that tells if the stream is open
		bool isOpen() {
			return getAccess() != Access::BadFile;
		}

		///tells if the stream can be read
		bool isReadable() {
			return getAccess() == Access::Read || getAccess() == Access::ReadWrite;
		}

		///tells if the stream can be written to
		bool isWriteable() {
			return getAccess() == Access::WriteOnly || getAccess() == Access::ReadWrite;
		}

		///ensures that the stream is in sync with the underlying data, ie. buffered data will be written to file
		virtual void flush() {

		}

		///tells the Stream to prepare to be read from
		virtual bool open(Access accessType) {
			return true;
		}

		///closes the Stream and releases its resources
		virtual void close() {
			flush();
		}
	};
}
