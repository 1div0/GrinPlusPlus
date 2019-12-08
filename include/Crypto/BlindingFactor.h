#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <Crypto/Hash.h>
#include <Crypto/SecretKey.h>
#include <Core/Serialization/ByteBuffer.h>
#include <Core/Serialization/Serializer.h>

class BlindingFactor
{
public:
	//
	// Constructors
	//
	BlindingFactor(Hash&& blindingFactorBytes)
		: m_blindingFactorBytes(std::move(blindingFactorBytes))
	{

	}
	BlindingFactor(const Hash& blindingFactorBytes)
		: m_blindingFactorBytes(blindingFactorBytes)
	{

	}
	BlindingFactor(const BlindingFactor& other) = default;
	BlindingFactor(BlindingFactor&& other) noexcept = default;

	//
	// Destructor
	//
	~BlindingFactor() = default;

	//
	// Operators
	//
	BlindingFactor& operator=(const BlindingFactor& other) = default;
	BlindingFactor& operator=(BlindingFactor&& other) noexcept = default;
	inline bool operator<(const BlindingFactor& rhs) const { return m_blindingFactorBytes < rhs.GetBytes(); }
	inline bool operator!=(const BlindingFactor& rhs) const { return m_blindingFactorBytes != rhs.GetBytes(); }
	inline bool operator==(const BlindingFactor& rhs) const { return m_blindingFactorBytes == rhs.GetBytes(); }

	//
	// Getters
	//
	inline const CBigInteger<32>& GetBytes() const { return m_blindingFactorBytes; }
	inline const std::vector<unsigned char>& GetVec() const { return m_blindingFactorBytes.GetData(); }
	inline const unsigned char* data() const { return m_blindingFactorBytes.data(); }

	//
	// Serialization/Deserialization
	//
	void Serialize(Serializer& serializer) const
	{
		serializer.AppendBigInteger<32>(m_blindingFactorBytes);
	}

	static BlindingFactor Deserialize(ByteBuffer& byteBuffer)
	{
		return BlindingFactor(byteBuffer.ReadBigInteger<32>());
	}

	//
	// Converts BlindingFactor to SecretKey.
	// WARNING: BlindingFactor is unusable after calling this.
	//
	SecretKey ToSecretKey()
	{
		return SecretKey(std::move(m_blindingFactorBytes));
	}

private:
	// The 32 byte blinding factor.
	Hash m_blindingFactorBytes;
};