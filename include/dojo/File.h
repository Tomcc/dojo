#pragma once

#include "dojo_common_header.h"

#include "FileStream.h"

namespace Dojo {
	///File is a real filesystem file
	class File : public FileStream {
	public:

		File(utf::string_view path);
		virtual ~File();

		virtual bool open(Access accessType) override;

        virtual int64_t getSize() override;

		virtual Access getAccess() override;

		virtual int64_t getCurrentPosition() override;

		virtual int seek(int64_t offset, int64_t fromWhere = SEEK_SET) override;

		virtual int64_t read(uint8_t* buf, int64_t number) override;

		virtual void write(uint8_t* buf, int size) override;

		virtual void close() override;

		///creates a new File* to read the underlying file. Does not copy reading status
		virtual Unique<Stream> copy() const override;

	private:

		FILE* mFile;
		int64_t mSize;

		Access mAccess;

		void _updateSize();
	private:
	};
}
