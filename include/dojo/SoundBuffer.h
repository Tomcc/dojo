#pragma once

#include "dojo_common_header.h"

#include "Resource.h"

namespace Dojo {
	class Stream;
	class SoundManager;
	class FileStream;

	///Internal class to contain a SoundBuffer loaded from file, or from memory
	/**
	Sounds are referenced using SoundSets, which contain one or more SoundBuffers, just like FrameSets and Textures.
	*/
	class SoundBuffer : public Resource {
	public:

		///A Chunk represents an actual OpenAL buffer. Chunks are used to preallocate streaming resources
		class Chunk : public Resource {
		public:

			///the max size in PCM of a chunk
			static const int MAX_DURATION = 3;
			static const int MAX_SIZE = 41000 * sizeof( short) * MAX_DURATION;

			///Creates a new chunk that will use the given source span to load
			Chunk(SoundBuffer& parent, int64_t streamStartPosition, int64_t uncompressedSize);

			~Chunk();

			///acquires one reference to this Chunk, and loads in a background thread
			void getAsync();

			///acquires one reference to this Chunk, and loads it
			void get();

			///releases one reference to this Chunk, and unloads it if needed
			void release();

			///returns the underlying OpenAL buffer
			uint32_t getOpenALBuffer();

			///returns the size of this chunk
			int getSize() {
				return size;
			}

			///loads in the default background queue
			void loadAsync();

			///loads the chunk asynchronously
			virtual bool onLoad() override;

			virtual void onUnload(bool soft = false) override;

		private:

			SoundBuffer& pParent;
			int64_t mStartPosition, mUncompressedSize;

			uint32_t size;
			uint32_t alBuffer;
			std::atomic<int> references;
		};

		typedef std::vector<std::unique_ptr<Chunk>> ChunkList;

		///Creates a new file-loaded SoundBuffer in the given resourcegroup, for the given file path
		SoundBuffer(optional_ref<ResourceGroup> creator, utf::string_view path);

		~SoundBuffer();

		virtual bool onLoad() override;
		virtual void onUnload(bool soft = false) override;

		///tells the memory size of this SoundBuffer
		int getSize() {
			return size;
		}

		///returns the number of chunks that compose this sound
		int getChunkNumber() {
			return mChunks.size();
		}

		///tells the duration in seconds of this SoundBuffer
		float getDuration() {
			return mDuration;
		}

		bool isLoaded() const {
			return not mChunks.empty();
		}

		virtual bool isReloadable() const {
			return mSource.is_some();
		}

		///tells if this buffer has been loaded as a streaming (multi-part, lazy-loaded) buffer.
		bool isStreaming() {
			return mChunks.size() > 1;
		}

		///get the nth part of this SoundBuffer. only streaming buffers have more than one part.
		/**
		if n is greater than the number of chunks, it will loop, just like animation frames
		*/
		Chunk& getChunk(int n, bool loadAsync = false);

	private:

		uint32_t size;
		float mDuration;

		ChunkList mChunks;
		optional_ref<Stream> mSource;
		Unique<FileStream> mFile; //this unique ptr keeps ownership of the file accessor when the src is a file

		bool _loadOgg(Stream& source);
		bool _loadOggFromFile();
	};
}
