#pragma once

#include "dojo_common_header.h"

#include "FileStream.h"

namespace Dojo {
	///File is a real filesystem file
	class File : public FileStream {
	public:

		File(const std::string& path);
		virtual ~File();

		virtual bool open(Access accessType) override;

		virtual int64_t getSize();

		virtual Access getAccess();

		virtual int64_t getCurrentPosition();

		virtual int seek(int64_t offset, int64_t fromWhere = SEEK_SET);

		virtual int64_t read(byte* buf, int64_t number);

		virtual void write(byte* buf, int size);

		virtual void close();

		///creates a new File* to read the underlying file. Does not copy reading status
		virtual Unique<Stream> copy() const override;

	protected:

		FILE* mFile;
		int64_t mSize;

		Access mAccess;

		void _updateSize();
	private:
	};
}
