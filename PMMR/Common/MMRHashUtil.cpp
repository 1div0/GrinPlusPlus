#include "MMRHashUtil.h"
#include "MMRUtil.h"

#include <Crypto.h>
#include <Serialization/Serializer.h>

void MMRHashUtil::AddHashes(HashFile& hashFile, const std::vector<unsigned char>& serializedLeaf, const PruneList* pPruneList)
{
	// Calculate next position
	uint64_t position = hashFile.GetSize();
	if (pPruneList != nullptr)
	{
		position += pPruneList->GetTotalShift();
	}

	// Add in the new leaf hash
	const Hash leafHash = HashLeafWithIndex(serializedLeaf, position);
	hashFile.AddHash(leafHash);

	// Add parent hashes
	uint64_t peak = 1;
	while (MMRUtil::GetHeight(position + 1) > 0)
	{
		const uint64_t leftSiblingPosition = (position + 1) - (2 * peak);

		const Hash leftHash = GetHashAt(hashFile, leftSiblingPosition, pPruneList);
		const Hash rightHash = GetHashAt(hashFile, position, pPruneList);

		++position;
		peak *= 2;

		const Hash parentHash = HashParentWithIndex(leftHash, rightHash, position);
		hashFile.AddHash(parentHash);
	}
}

Hash MMRHashUtil::Root(const HashFile& hashFile, const uint64_t size, const PruneList* pPruneList)
{
	if (size == 0)
	{
		return ZERO_HASH;
	}

	Hash hash = ZERO_HASH;
	const std::vector<uint64_t> peakIndices = MMRUtil::GetPeakIndices(size);
	for (auto iter = peakIndices.crbegin(); iter != peakIndices.crend(); iter++)
	{
		Hash peakHash = GetHashAt(hashFile, *iter, pPruneList);
		if (peakHash != ZERO_HASH)
		{
			if (hash == ZERO_HASH)
			{
				hash = peakHash;
			}
			else
			{
				hash = HashParentWithIndex(peakHash, hash, size);
			}
		}
	}

	return hash;
}

Hash MMRHashUtil::GetHashAt(const HashFile& hashFile, const uint64_t mmrIndex, const PruneList* pPruneList)
{
	if (pPruneList != nullptr)
	{
		if (pPruneList->IsCompacted(mmrIndex))
		{
			return ZERO_HASH;
		}

		const uint64_t shift = pPruneList->GetShift(mmrIndex);
		const uint64_t shiftedIndex = (mmrIndex - shift);

		return hashFile.GetHashAt(shiftedIndex);
	}
	else
	{
		return hashFile.GetHashAt(mmrIndex);
	}
}

Hash MMRHashUtil::HashLeafWithIndex(const std::vector<unsigned char>& serializedLeaf, const uint64_t mmrIndex)
{
	Serializer hashSerializer;
	hashSerializer.Append<uint64_t>(mmrIndex);
	hashSerializer.AppendByteVector(serializedLeaf);
	return Crypto::Blake2b(hashSerializer.GetBytes());
}

Hash MMRHashUtil::HashParentWithIndex(const Hash& leftChild, const Hash& rightChild, const uint64_t parentIndex)
{
	Serializer serializer;
	serializer.Append<uint64_t>(parentIndex);
	serializer.AppendBigInteger<32>(leftChild);
	serializer.AppendBigInteger<32>(rightChild);
	return Crypto::Blake2b(serializer.GetBytes());
}