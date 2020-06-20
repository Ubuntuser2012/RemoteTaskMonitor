#pragma once

#include <functional>
#include "../ds/ds.h"


class NetworkAdapter
{
public:
	NetworkAdapter();
	~NetworkAdapter();

	void IterateAdapters(std::function<bool(const IP_ADAPTER_ADDRESSES* adapter_address)>OnMacAddressCb, STRING &err) const;
	void IterateIpAddresses(std::function<bool(const SOCKADDR *socket_address, int len)>OnIpAddressCb, STRING &err) const;
	void IterateBroadcastIpAddresses(std::function<bool(struct in_addr ba, STRING &ip)>OnBroadcastIpAddressCb, STRING &err) const;
};

