#ifndef ARCHIVER_FILE_WORK_HPP_INCLUDED
#define ARCHIVER_FILE_WORK_HPP_INCLUDED

// Includes:

#include <cstdio>
#include <cstring>

#include <cctype>
#include <utility>
#include <limits>

#include <vector>

// Code:

namespace BinFileWork
{
	using namespace VaExc;

	// Constants:
		const size_t CHUNK_SIZE = 1024;

	// Reading from file:
	// (Binary file)

		class ReadBinaryFile
		{
		public:
			// Ctors && dtor:
				explicit ReadBinaryFile(const char* filename);

				~ReadBinaryFile();

			// Functions:
				std::vector<unsigned char> getBytes(size_t count);

				void setPos(size_t pos);

				bool finished() const;
				size_t fileSize();

		private:
			// Variables:
				const char* filename_;
				std::FILE* file_;

				size_t size_;
				unsigned char buf_[CHUNK_SIZE];
				size_t index_;

				size_t fileSize_;
		};

		// Ctor && dtor:
			ReadBinaryFile::ReadBinaryFile(const char* filename) :
				filename_ (filename),
				file_     (std::fopen(filename, "rb")),
				size_     (0),
				buf_      (),
				index_    (0),
				fileSize_ (0)
			{
				if (file_ == nullptr)
				{
					throw Exception(ArgMsg("Unable to open file (file: %s)", filename_));
				}

				// Calculating size:
				std::fseek(file_, 0, SEEK_END);
				fileSize_ = std::ftell(file_);
				std::fseek(file_, 0, SEEK_SET);

				std::memset(buf_, 0, sizeof(buf_));

				size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);
			}

			ReadBinaryFile::~ReadBinaryFile()
			{
				std::fclose(file_);
			}

		// Other func:
			std::vector<unsigned char> ReadBinaryFile::getBytes(size_t count)
			{	
				std::vector<unsigned char> toReturn = std::vector<unsigned char>(count);

				for (size_t bytesRead = 0; ; ++index_, ++bytesRead)
				{
					if (index_ == size_)
					{
						std::memset(buf_, 0, sizeof(buf_));

						size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);

						index_ = 0;

						if (size_ == 0) // File finished
						{
							for (; bytesRead < count; ++bytesRead)
							{
								toReturn[bytesRead] = '\0';
							}

							break;
						}
					}

					if (bytesRead == count) break;

					toReturn[bytesRead] = buf_[index_];
				}

				return toReturn;
			}

			void ReadBinaryFile::setPos(size_t pos) {
				fseek(file_, pos, SEEK_SET);

				size_ = CHUNK_SIZE;
				index_ = CHUNK_SIZE;
			}

			bool ReadBinaryFile::finished() const
			{
				if (size_ == 0) return true;

				return false;
			}

			size_t ReadBinaryFile::fileSize()
			{
				return fileSize_;
			}

	// Writing to file:
	// (Binary file)
		class WriteBinaryFile
		{
		public:
			// Ctor && dtor:
				explicit WriteBinaryFile(const char* filename);

				~WriteBinaryFile();

			// Other func:
				void writeBytes(const std::vector<unsigned char>& toWrite);

		private:
			// Variables:
				const char* filename_;
				std::FILE* file_;
				unsigned char buf_[CHUNK_SIZE];
				size_t size_;
		};

		// Ctor && dtor:
			WriteBinaryFile::WriteBinaryFile(const char* filename) :
				filename_ (filename),
				file_ (std::fopen(filename, "wb")),
				buf_  (),
				size_ (0)
			{
				std::memset(buf_, 0, sizeof(buf_));

				if (file_ == nullptr)
				{
					throw Exception(ArgMsg("Unable to create file (file: %s)", filename_));
				}
			}

			WriteBinaryFile::~WriteBinaryFile()
			{
				std::fwrite(buf_, sizeof(*buf_), size_, file_);

				std::fclose(file_);
			}

		// Other func:
			void WriteBinaryFile::writeBytes(const std::vector<unsigned char>& toWrite)
			{
				for (size_t vectIndex = 0; vectIndex < toWrite.size(); ++vectIndex, ++size_)
				{
					if (size_ == CHUNK_SIZE)
					{
						size_t written = std::fwrite(buf_, sizeof(*buf_), sizeof(buf_), file_);

						std::memset(buf_, 0, sizeof(buf_));

						size_ = 0;

						if (written != CHUNK_SIZE)
						{
							throw Exception(ArgMsg("Unable to write to file (file: %s)", filename_));
						}
					}

					buf_[size_] = toWrite.at(vectIndex);
				}
			}

} // namespace BinFileWork 

#endif /*ARCHIVER_FILE_WORK_HPP_INCLUDED*/