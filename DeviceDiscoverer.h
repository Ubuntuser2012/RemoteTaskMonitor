#pragma once

#include <functional>

extern "C" {
	bool DiscoveryServerInit(unsigned short port);
	bool DiscoveryServerServe(char *sender_id, int sender_id_sz, char *server_key, int server_id_sz);
}

class DeviceDiscoverer
{
public:
	DeviceDiscoverer();
	~DeviceDiscoverer();

	bool Init(const DISCOVERY_PING& msg);
	void Discover(std::function<void(sockaddr_in addr, DISCOVERY_PONG &pong)>OnDiscovered, unsigned short port);

private:
	LIST m_broadcast_ips;
	DISCOVERY_PING m_broadcast_msg;
	int m_sock;

	void GetBroadcastIpAddresses();
};

