#ifndef ARCHIVER_HUFFMAN_HPP_INCLUDED
#define ARCHIVER_HUFFMAN_HPP_INCLUDED

namespace HuffmanAlgorithm
{
	// Format:
	// magic number                       [4 bytes]
	// flags                              [1 byte]
	// tree node array (prev, left, rght) [(k + k + k) * 256 bytes, where k = sizeof(Index_t)]
	// text                               [n bytes]

	// (*) If n < 3*k*256 the tree won't be used

	const std::vector<unsigned char> MAGIC_NUMBER = {
		0xFA,
		0xBA,
		0xFA,
		0xBA
	};

	const unsigned char FLAG_CALCULATE_HUFFMAN = 0b00000001;

	void writeBitsToFile(const std::vector<bool>& bits,
	                     unsigned char& curByte,
	                     unsigned char& bitIndex,
	                     BinFileWork::WriteBinaryFile& fileDst)
	{
		for (auto bit : bits)
		{
			curByte += bit;
			if (bitIndex < 7) curByte <<= 1;

			++bitIndex;

			if (bitIndex == 8)
			{
				bitIndex = 0;
				fileDst.writeBytes({curByte});
				curByte = 0;
			}
		}
	}

	void huffmanEncode(BinFileWork::ReadBinaryFile& fileSrc, BinFileWork::WriteBinaryFile& fileDst)
	{
		// Creating a tree:

		HuffmanEncode::HuffmanTreeRead tree{HuffmanEncode::calculateLetterFrequence(fileSrc)};

		// Writing tree to file:

		fileDst.writeBytes(tree.treeToBytes());

		// Building and writing huffman code:

		unsigned char currentByte = 0;
		unsigned char bitIndex = 0;

		while (!fileSrc.finished())
		{
			unsigned char readCharacter = fileSrc.getBytes(1)[0];
			
			std::vector<bool> curCode = tree.getCode(readCharacter);

			writeBitsToFile(curCode, currentByte, bitIndex, fileDst);
		}

		writeBitsToFile(tree.getCode('\0'), currentByte, bitIndex, fileDst);

		writeBitsToFile({0,0,0,0,0,0,0,0}, currentByte, bitIndex, fileDst);
	}

	void huffmanDecode(BinFileWork::ReadBinaryFile& fileSrc, BinFileWork::WriteBinaryFile& fileDst)
	{
		// Creating a tree:

		HuffmanDecode::HuffmanTreeWrite tree{fileSrc};

		// Getting huffman code:

		std::queue<bool> huffmanCode{};

		while(!fileSrc.finished())
		{
			unsigned char byteOfCode = fileSrc.getBytes(1)[0];

			// std::vector<bool> is not continuous
			for (int bit = 7; 0 <= bit; --bit)
			{
				huffmanCode.push((byteOfCode >> bit) % 2);
			}

			if (huffmanCode.size() >= 256)
			{
				unsigned char curChar = tree.parseChar(huffmanCode);

				if (curChar == '\0') return;

				fileDst.writeBytes({curChar});
			}
		}

		while (!huffmanCode.empty())
		{
			unsigned char curChar = tree.parseChar(huffmanCode);

			if (curChar == '\0') return;

			fileDst.writeBytes({curChar});
		}
	}

} // namespace HuffmanFormat


void archive(const char* src, const char* dst)
{
	BinFileWork::ReadBinaryFile  fileSrc{src};
	BinFileWork::WriteBinaryFile fileDst{dst};

	fileDst.writeBytes(HuffmanAlgorithm::MAGIC_NUMBER);

	if (fileSrc.fileSize() * 1.5 > 256 * 3 * sizeof(HuffmanEncode::Index_t))
	{
		fileDst.writeBytes({HuffmanAlgorithm::FLAG_CALCULATE_HUFFMAN});

		HuffmanAlgorithm::huffmanEncode(fileSrc, fileDst);
	}
	else
	{
		fileDst.writeBytes({0});

		while (!fileSrc.finished())
		{
			auto curChunk = fileSrc.getBytes(BinFileWork::CHUNK_SIZE);

			if (curChunk[BinFileWork::CHUNK_SIZE - 1] == '\0')
			{
				for (size_t i = 0; i < BinFileWork::CHUNK_SIZE && curChunk[i] != '\0'; ++i)
				{
					fileDst.writeBytes({curChunk[i]});
				}
			}
			else fileDst.writeBytes(curChunk);
		} 
	}
}

// anarchive
void unarchive(const char* src, const char* dst)
{
	using namespace VaExc;

	BinFileWork::ReadBinaryFile  fileSrc{src};
	BinFileWork::WriteBinaryFile fileDst{dst};

	std::vector<unsigned char> magicNumRead = fileSrc.getBytes(4);
	for (size_t i = 0; i < 4; ++i)
	{
		if (magicNumRead[i] != HuffmanAlgorithm::MAGIC_NUMBER[i])
		{
			throw Exception("Error: va_unarchive accepts only .vaarch files (created by va_archive)"_msg, VAEXC_POS);
		}
	}

	unsigned char flags = *reinterpret_cast<unsigned char*>(&fileSrc.getBytes(1)[0]);

	if (flags & HuffmanAlgorithm::FLAG_CALCULATE_HUFFMAN)
	{
		HuffmanAlgorithm::huffmanDecode(fileSrc, fileDst);
	}
	else
	{
		fileDst.writeBytes(fileSrc.getBytes(std::numeric_limits<size_t>::max()));
	}
}

#endif // ARCHIVER_HUFFMAN_HPP_INCLUDED