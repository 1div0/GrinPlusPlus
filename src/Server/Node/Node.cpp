#include "Node.h"
#include "NodeRestServer.h"
#include "NodeClients/DefaultNodeClient.h"
#include "../io.h"

#include <P2P/P2PServer.h>
#include <Config/Config.h>
#include <BlockChain/BlockChainServer.h>
#include <Crypto/Crypto.h>
#include <Consensus/BlockDifficulty.h>
#include <Database/Database.h>
#include <Common/Logger.h>
#include <PMMR/TxHashSetManager.h>

#include <iostream>
#include <thread>

Node::Node(
	const Context::Ptr& pContext,
	std::unique_ptr<NodeRestServer>&& pNodeRestServer,
	std::shared_ptr<DefaultNodeClient> pNodeClient,
	std::unique_ptr<GrinJoinController>&& pGrinJoinController)
	: m_pContext(pContext),
	m_pNodeRestServer(std::move(pNodeRestServer)),
	m_pNodeClient(pNodeClient),
	m_pGrinJoinController(std::move(pGrinJoinController))
{

}

Node::~Node()
{
	LOG_INFO("Shutting down node daemon");
}

std::unique_ptr<Node> Node::Create(const Context::Ptr& pContext)
{
	auto pNodeClient = DefaultNodeClient::Create(pContext);
	auto pNodeRestServer = NodeRestServer::Create(
		pContext->GetConfig(),
		pNodeClient->GetNodeContext()
	);

	std::unique_ptr<GrinJoinController> pGrinJoinController = nullptr;
	if (!pContext->GetConfig().GetServerConfig().GetGrinJoinSecretKey().empty())
	{
		pGrinJoinController = GrinJoinController::Create(
			pContext->GetTorProcess(),
			pNodeClient->GetNodeContext(),
			pContext->GetConfig().GetServerConfig().GetGrinJoinSecretKey()
		);
	}

	return std::make_unique<Node>(
		pContext,
		std::move(pNodeRestServer),
		pNodeClient,
		std::move(pGrinJoinController)
	);
}

void Node::UpdateDisplay(const int secondsRunning)
{
	SyncStatusConstPtr pSyncStatus = m_pNodeClient->GetP2PServer()->GetSyncStatus();

	IO::Clear();

	std::cout << "Time Running: " << secondsRunning << "s";

	const ESyncStatus status = pSyncStatus->GetStatus();
	if (status == ESyncStatus::NOT_SYNCING)
	{
		std::cout << "\nStatus: Running";
	}
	else if (status == ESyncStatus::WAITING_FOR_PEERS)
	{
		std::cout << "\nStatus: Waiting for Peers";
	}
	else if (status == ESyncStatus::SYNCING_HEADERS)
	{
		const uint64_t networkHeight = pSyncStatus->GetNetworkHeight();
		const uint64_t percentage = networkHeight > 0 ? (pSyncStatus->GetHeaderHeight() * 100 / networkHeight) : 0;
		std::cout << "\nStatus: Syncing Headers (" << percentage << "%)";
	}
	else if (status == ESyncStatus::SYNCING_TXHASHSET)
	{
		const uint64_t downloaded = pSyncStatus->GetDownloaded();
		const uint64_t downloadSize = pSyncStatus->GetDownloadSize();
		const uint64_t percentage = downloadSize > 0 ? (downloaded * 100) / downloadSize : 0;

		std::cout << "\nStatus: Syncing TxHashSet " << downloaded << "/" << downloadSize << "(" << percentage << "%)";
	}
	else if (status == ESyncStatus::PROCESSING_TXHASHSET)
	{
		std::cout << "\nStatus: Validating TxHashSet";
	}
	else if (status == ESyncStatus::TXHASHSET_SYNC_FAILED)
	{
		std::cout << "\nStatus: TxHashSet Sync Failed - Trying Again";
	}
	else if (status == ESyncStatus::SYNCING_BLOCKS)
	{
		std::cout << "\nStatus: Syncing blocks";
	}

	std::cout << "\nNumConnections: " << pSyncStatus->GetNumActiveConnections();
	std::cout << "\nHeader Height: " << pSyncStatus->GetHeaderHeight();
	std::cout << "\nHeader Difficulty: " << pSyncStatus->GetHeaderDifficulty();
	std::cout << "\nBlock Height: " << pSyncStatus->GetBlockHeight();
	std::cout << "\nBlock Difficulty: " << pSyncStatus->GetBlockDifficulty();
	std::cout << "\nNetwork Height: " << pSyncStatus->GetNetworkHeight();
	std::cout << "\nNetwork Difficulty: " << pSyncStatus->GetNetworkDifficulty();
	std::cout << "\n\nPress Ctrl-C to exit...";

	IO::Flush();
}