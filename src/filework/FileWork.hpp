// Copyright 2017 Vladislav Aleinik
#ifndef MY_LISP_SRC_INCLUDE_FILE_WORK_HPP_
#define MY_LISP_SRC_INCLUDE_FILE_WORK_HPP_

#include <cstdio>
#include <cstring>
#include <cctype>

#include "../exception/VaException.hpp"

namespace FileWork
{
	using namespace VaExc;
	
	const size_t MAX_WORD_SIZE = 63;
	const size_t CHUNK_SIZE = 1024;

	struct Word
	{
	public:
		// Variables:
			const char* file;
			size_t line;
			size_t col;
			char word[MAX_WORD_SIZE + 1]; // Must have \0

		// Ctor:

			explicit Word(const char* newWord) :
				file (""),
				line (0),
				col  (0),
				word ()
			{
				std::strcpy(word, newWord);
			}

			explicit Word(const char* filename, size_t row, size_t column) :
				file (filename),
				line (row),
				col (column),
				word ()
			{}

		// Operator!=:
			bool operator!=(Word that) const
			{
				if (file != that.file) return true;
				if (line != that.line) return true;
				if (col  != that.col ) return true;
				if (std::strcmp(word, that.word) != 0) return true;

				return false;
			}
	};

	Word NULL_WORD{Word("")};

	// Reading text file
	class ReadTextFile
	{
	public:
		// Ctors && dtor:
			explicit ReadTextFile(const char* filename);

			~ReadTextFile();

		// Functions:
			Word getWord();

	private:
		// Variables:
			const char* filename_;
			size_t line_;
			size_t col_;
			std::FILE* file_;
			size_t size_;
			char buf_[CHUNK_SIZE]; // Without a \0
			size_t index_;
	};

	// Ctor && dtor:
	ReadTextFile::ReadTextFile(const char* filename) :
		filename_ (filename),
		line_     (1),
		col_      (0),
		file_     (std::fopen(filename, "r")),
		size_     (0),
		buf_      (),
		index_    (0)
	{
		if (file_ == nullptr)
		{
			throw Exception(ArgMsg("Unable to open file %s", filename_), VAEXC_POS);
		}

		std::memset(buf_, 0, sizeof(buf_));

		size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);
	}

	ReadTextFile::~ReadTextFile()
	{
		std::fclose(file_);
	}

	// Other func:
	Word ReadTextFile::getWord()
	{
		Word toReturn{NULL_WORD};

		bool inWord = false;
		for (size_t wordIndex = 0; wordIndex < MAX_WORD_SIZE; ++index_, ++col_)
		{
			if (index_ == size_)
			{
				std::memset(buf_, 0, sizeof(buf_));

				size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);

				index_ = 0;

				if (size_ == 0) // File finished
				{
					break;
				}
			}

			if (buf_[index_] == '\n')
			{
				++index_;
				++line_;
				col_ = 0;
				if (inWord) break;
			}

			if (std::isspace(buf_[index_]) != 0)
			{
				if (inWord) break;
			}
			else if (!inWord)
			{
				inWord = true;

				toReturn.file = filename_;
				toReturn.line = line_;
				toReturn.col = col_;
			}

			if (inWord)
			{
				toReturn.word[wordIndex] = buf_[index_];
				++wordIndex;
			}
		}

		return toReturn;
	}

	// Writing to file
	class WriteTextFile
	{
	public:
		// Ctor && dtor:
			explicit WriteTextFile(const char* filename);

			~WriteTextFile();

		// Other func:
			void writeWord(      Word&& toWrite);
			void writeWord(const Word&  toWrite);

	private:
		// Variables:
			const char* filename_;
			std::FILE* file_;
			char buf_[CHUNK_SIZE];
			size_t size_;
	};

	// Ctor && dtor:
		WriteTextFile::WriteTextFile(const char* filename) :
			filename_ (filename),
			file_ (std::fopen(filename, "w")),
			buf_  (),
			size_ (0)
		{
			std::memset(buf_, 0, sizeof(buf_));

			if (file_ == nullptr)
			{
				throw Exception(ArgMsg("Unable to create file (%s)", filename_), VAEXC_POS);;
			}
		}

		WriteTextFile::~WriteTextFile()
		{
			std::fwrite(buf_, sizeof(*buf_), size_, file_);

			std::fclose(file_);
		}

		// Other func:
		void WriteTextFile::writeWord(Word&& toWrite)
		{
			for
			(
				size_t wordIndex = 0;
				 wordIndex < MAX_WORD_SIZE && toWrite.word[wordIndex] != '\0';
				 ++wordIndex, ++size_
			)
			{
				if (size_ == CHUNK_SIZE)
				{
					size_t written = std::fwrite(buf_, sizeof(*buf_), sizeof(buf_), file_);

					std::memset(buf_, 0, sizeof(buf_));

					size_ = 0;

					if (written != CHUNK_SIZE)
					{
						throw Exception(ArgMsg("Unable to write to file (%s)", filename_), VAEXC_POS);
					}
				}

				buf_[size_] = toWrite.word[wordIndex];
			}
		}

		void WriteTextFile::writeWord(const Word& toWrite)
		{
			for
			(
				size_t wordIndex = 0;
				 wordIndex < MAX_WORD_SIZE && toWrite.word[wordIndex] != '\0';
				 ++wordIndex, ++size_
			)
			{
				if (size_ == CHUNK_SIZE)
				{
					size_t written = std::fwrite(buf_, sizeof(*buf_), sizeof(buf_), file_);

					std::memset(buf_, 0, sizeof(buf_));

					size_ = 0;

					if (written != CHUNK_SIZE)
					{
						throw Exception(ArgMsg("Unable to write to file (%s)", filename_), VAEXC_POS);
					}
				}

				buf_[size_] = toWrite.word[wordIndex];
			}
		}
}  // namespace FileWork

#endif  // MY_LISP_SRC_INCLUDE_FILE_WORK_HPP_