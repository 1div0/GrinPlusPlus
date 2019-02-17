#pragma once

#include "../civetweb/include/civetweb.h"
#include <string>

class PeersAPI
{
public:
	static int GetAllPeers_Handler(struct mg_connection* conn, void* pServerContainer);
	static int GetConnectedPeers_Handler(struct mg_connection* conn, void* pServerContainer);
	static int Peer_Handler(struct mg_connection* conn, void* pServerContainer);

private:
	static std::string ParseIPAddress(const std::string& request);
	static std::string ParsePort(const std::string& request);
	static std::string ParseCommand(const std::string& request);
};