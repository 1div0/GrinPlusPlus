#pragma once

#include <Config/Config.h>
#include <Crypto/SecretKey.h>
#include <Net/Tor/TorAddress.h>
#include <Net/Tor/TorConnection.h>

// Forward Declarations
class TorControl;

class TorProcess
{
public:
	using Ptr = std::shared_ptr<TorProcess>;

	static TorProcess::Ptr Initialize(const uint16_t socksPort, const uint16_t controlPort) noexcept;

	std::shared_ptr<TorAddress> AddListener(const SecretKey64& secretKey, const uint16_t portNumber);
	std::shared_ptr<TorAddress> AddListener(const std::string& serializedKey, const uint16_t portNumber);
	bool RemoveListener(const TorAddress& torAddress);

	std::shared_ptr<TorConnection> Connect(const TorAddress& address);

	// Returns true if TorControl connection is established.
	bool RetryInit();

private:
	TorProcess(const uint16_t socksPort, const uint16_t controlPort, const std::shared_ptr<TorControl>& pControl)
		: m_socksPort(socksPort), m_controlPort(controlPort), m_pControl(pControl) { }

	uint16_t m_socksPort;
	uint16_t m_controlPort;
	std::shared_ptr<TorControl> m_pControl;
};