#ifndef ARCHIVER_HUFFMAN_DECODE_HPP_INCLUDED
#define ARCHIVER_HUFFMAN_DECODE_HPP_INCLUDED

#include <cstdio>

#include <limits>

#include <array>
#include <vector>
#include <queue>
	
namespace HuffmanDecode
{
	using namespace VaExc;

	using Frequencies_t = std::array<long, 512>;
	using Index_t = HuffmanEncode::Index_t;
	using Indexes_t = std::array<Index_t, 512>;

	struct HuffmanTreeWrite
	{
		public: 
			HuffmanTreeWrite(Indexes_t lefts, Indexes_t rghts, Indexes_t prevs, size_t curNode);
			HuffmanTreeWrite(BinFileWork::ReadBinaryFile& src);

			Index_t currentRoot() const;

			unsigned char parseChar(std::queue<bool>& huffmanCode) const;

		private: 
			Indexes_t lefts_;
			Indexes_t rghts_;
			Indexes_t prevs_;
			Index_t curNode_;
	};

	Index_t HuffmanTreeWrite::currentRoot() const
	{
		return curNode_;
	}

	HuffmanTreeWrite::HuffmanTreeWrite(BinFileWork::ReadBinaryFile& fileSrc) : 
		lefts_ ({}),
		rghts_ ({}),
		prevs_ ({}),
		curNode_ (0)
	{
		for (Index_t i = 0; i < 256; ++i)
		{
			lefts_[i] = -1;
			rghts_[i] = -1;
			prevs_[i] = -1;
		}

		for (Index_t i = 256; i < 512; ++i)
		{
			if (fileSrc.finished())
			{
				throw Exception("HuffmanTreeWrite::ctor(): Not enough data in source file"_msg, VAEXC_POS);
			}

			std::vector<unsigned char> bytes = fileSrc.getBytes(3 * sizeof(Index_t));

			lefts_[i] = *(reinterpret_cast<Index_t*>(&bytes[0 * sizeof(Index_t)]));
			rghts_[i] = *(reinterpret_cast<Index_t*>(&bytes[1 * sizeof(Index_t)]));
			prevs_[i] = *(reinterpret_cast<Index_t*>(&bytes[2 * sizeof(Index_t)]));
		}

		// -1, because we ++ curNode_ after every added node
		curNode_ = *(reinterpret_cast<Index_t*>(&(fileSrc.getBytes(sizeof(Index_t))[0]))) - 1;
	}

	unsigned char HuffmanTreeWrite::parseChar(std::queue<bool>& huffmanCode) const
	{
		for (Index_t curIndex = curNode_; !huffmanCode.empty();)
		{
			if (lefts_[curIndex] == -1 && rghts_[curIndex] == -1)
			{
				return static_cast<unsigned char>(curIndex);
			}
			else
			{
				if (huffmanCode.front() == 0)
				{
					if (lefts_[curIndex] == -1)
					{
						throw Exception("HuffmanTreeWrite::parseChar(): Left subtree is empty"_msg, VAEXC_POS);
					}

					curIndex = lefts_[curIndex];
				}
				else
				{
					if (rghts_[curIndex] == -1)
					{
						throw Exception("HuffmanTreeWrite::parseChar(): Right subtree is empty"_msg, VAEXC_POS);
					}

					curIndex = rghts_[curIndex];
				}

				huffmanCode.pop();
			}
		}

		throw Exception("HuffmanTreeWrite::parseChar(): Not enough data"_msg);
	}

} // namespace HuffmanDecode

#endif // ARCHIVER_HUFFMAN_DECODE_HPP_INCLUDED