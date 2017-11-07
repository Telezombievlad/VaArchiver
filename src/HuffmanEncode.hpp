#ifndef ARCHIVER_HUFFMAN_ENCODE_HPP_INCLUDED
#define ARCHIVER_HUFFMAN_ENCODE_HPP_INCLUDED

#include <cstdio>

#include <limits>

#include <array>
#include <vector>

namespace HuffmanEncode
{
	using namespace VaExc;

	using Frequencies_t = std::array<long, 512>;
	using Index_t = short;
	using Indexes_t = std::array<Index_t, 512>;

	Frequencies_t calculateLetterFrequence(BinFileWork::ReadBinaryFile& file)
	{
		Frequencies_t counts{};
		std::vector<unsigned char> letters{};

		counts['\0'] = 1;

		while (!file.finished())
		{
			letters = file.getBytes(BinFileWork::CHUNK_SIZE);

			for (const unsigned char& ch : letters)
			{
				if (ch == '\0') break;				

				counts[ch] += 1;
			}
		}

		file.setPos(0);
		
		return counts;
	}

	struct HuffmanTreeRead
	{
	public: 
		HuffmanTreeRead(Frequencies_t counts);

		Index_t currentRoot() const;

		std::vector<unsigned char> treeToBytes() const;

		std::vector<bool> getCode(unsigned char chr) const;

	private: 
		// Variables: 
			Frequencies_t counts_;
			Indexes_t lefts_;
			Indexes_t rghts_;
			Indexes_t prevs_;
			Index_t curNode_;

		// Additional functions:
			std::pair<Index_t, Index_t> getTwoMinNodes();
			void addNode(std::pair<Index_t, Index_t> minNodes);
	};

	HuffmanTreeRead::HuffmanTreeRead(Frequencies_t counts) :
		counts_  (counts),
		lefts_   ({}),
		rghts_   ({}),
		prevs_   ({}),
		curNode_ (256)
	{
		lefts_.fill(-1);
		rghts_.fill(-1);
		prevs_.fill(-1);

		for (std::pair<Index_t, Index_t> minNodes = getTwoMinNodes();
	    	 std::get<0>(minNodes) != std::numeric_limits<Index_t>::max();
	     	 minNodes = getTwoMinNodes())
		{
			addNode(minNodes);
		}
	}

	Index_t HuffmanTreeRead::currentRoot() const
	{
		return curNode_;
	}

	std::pair<Index_t, Index_t> HuffmanTreeRead::getTwoMinNodes()
	{
		Index_t min0 = 0;
		Index_t min1 = 0;

		long minCount0 = std::numeric_limits<long>::max();
		long minCount1 = std::numeric_limits<long>::max();

		for (Index_t i = 0; i < curNode_; ++i)
		{
			if (counts_[i] > 0)
			{
				if (counts_[i] < minCount0)
				{
					 min1 = min0;
					 min0 = i;

					 minCount1 = minCount0;
					 minCount0 = counts_[i];
				}
				else if (counts_[i] < minCount1)
				{
					min1 = i;

					minCount1 = counts_[i];
				}
			}
		}

		if (minCount0 == std::numeric_limits<long>::max() ||
			minCount1 == std::numeric_limits<long>::max())
		{
			return std::make_pair(std::numeric_limits<Index_t>::max(), 0);
		}

		return std::make_pair(min0, min1);
	}

	void HuffmanTreeRead::addNode(std::pair<Index_t, Index_t> minNodes)
	{
		if (curNode_ >= 512)
		{
			throw Exception("HuffmanTreeRead::addNode(): curNode_ >= 512"_msg, VAEXC_POS);
		}

		Index_t l = std::get<0>(minNodes);
		Index_t r = std::get<1>(minNodes);

		lefts_[curNode_] = l;
		rghts_[curNode_] = r;

		counts_[curNode_] = counts_[l] + counts_[r];

		prevs_[l] = curNode_;
		prevs_[r] = curNode_;

		counts_[l] *= -1;
		counts_[r] *= -1;

		++curNode_;
	}

	std::vector<unsigned char> HuffmanTreeRead::treeToBytes() const
	{
		unsigned char indexVarSize = sizeof(Index_t);

		std::vector<unsigned char> curWrite = std::vector<unsigned char>((256 * 3 + 1) * indexVarSize);

		for (Index_t i = 0, k = 256; i < 256; ++i, ++k)
		{
			*reinterpret_cast<Index_t*>(&curWrite[(i * 3 + 0) * indexVarSize]) = lefts_[k];
			*reinterpret_cast<Index_t*>(&curWrite[(i * 3 + 1) * indexVarSize]) = rghts_[k];
			*reinterpret_cast<Index_t*>(&curWrite[(i * 3 + 2) * indexVarSize]) = prevs_[k];
		}

		*reinterpret_cast<Index_t*>(&curWrite[256 * 3 * indexVarSize]) = curNode_;

		return curWrite;
	}

	std::vector<bool> HuffmanTreeRead::getCode(unsigned char chr) const
	{
		// std::printf("<%c> - %02d\n", chr, chr);

		std::vector<bool> toReturn{};

		for (size_t i = chr; prevs_[i] != -1; i = prevs_[i])
		{
			if (i == lefts_[prevs_[i]]) toReturn.insert(toReturn.begin(), 0);
			else toReturn.insert(toReturn.begin(), 1);
		}

		return toReturn;
	}

} // namespace HuffmanEncode

#endif // ARCHIVER_HUFFMAN_ENCODE_HPP_INCLUDED