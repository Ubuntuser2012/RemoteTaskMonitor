#include "stdafx.h"
#include "../common/types.h"
#include "../common/util.h"

#include "NetworkAdapter.h"

#pragma comment(lib, "Iphlpapi.lib")


NetworkAdapter::NetworkAdapter()
{
}


NetworkAdapter::~NetworkAdapter()
{
}

void NetworkAdapter::IterateAdapters(
	std::function<bool(const IP_ADAPTER_ADDRESSES* adapter_address)>OnMacAddressCb, STRING &err) const
{
	IP_ADAPTER_ADDRESSES *adapter;
	const int WORKING_BUFFER_SIZE = 15000;
	char adapters[WORKING_BUFFER_SIZE];
	unsigned long sz = sizeof(adapters);
	unsigned long err1 = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, 0, (IP_ADAPTER_ADDRESSES*)adapters, &sz);
	if (err1 != ERROR_SUCCESS)
	{
		err.sprintf(TXT("Failed to retrieve network adapter information [error: %i]"), err);
	}
	else
	{
		adapter = (IP_ADAPTER_ADDRESSES*)adapters;
		while (adapter)
		{
			if (!OnMacAddressCb(adapter))
			{
				break;
			}
			adapter = adapter->Next;
		}

	}
}

/*
NetworkAdapter na;
na.IterateIpAddresses([](const SOCKADDR* socket_address, int len) {
		char_t ip[128] = { 0 };
		DWORD length = 128;
		WSAAddressToStringW(const_cast<SOCKADDR*>(socket_address), len, NULL, ip, &length);
		logg(TXT("ip %s\n"), ip);
		return true;
	}, err);
*/
void NetworkAdapter::IterateIpAddresses(std::function<bool(const SOCKADDR *socket_address, int len)>OnIpAddressCb, STRING &err) const
{
	IterateAdapters(
		[=](const IP_ADAPTER_ADDRESSES* adapter_address)
	{
		PIP_ADAPTER_UNICAST_ADDRESS_LH  ip = adapter_address->FirstUnicastAddress;
		while (ip)
		{
			if (!OnIpAddressCb(ip->Address.lpSockaddr, ip->Address.iSockaddrLength))
			{
				return false;
			}
			ip = ip->Next;
		}
		return true;
	}, err);
}

void NetworkAdapter::IterateBroadcastIpAddresses(std::function<bool(struct in_addr ba, STRING &ip)>OnBroadcastIpAddressCb, STRING &err) const
{
	ULONG sz = 0;
	if (GetAdaptersInfo(NULL, &sz) == ERROR_BUFFER_OVERFLOW)
	{
		sz += 1024;
		char* buf = (char*)malloc(sz);
		if (GetAdaptersInfo((IP_ADAPTER_INFO*)buf, &sz) == ERROR_SUCCESS)
		{
			IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO*)buf;
			for (; pAdapterInfo != NULL; pAdapterInfo = pAdapterInfo->Next)
			{
				unsigned long ip = inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
				unsigned long mask = inet_addr(pAdapterInfo->IpAddressList.IpMask.String);
				unsigned long bcip = ip | ~mask;

				struct in_addr ba;
				ba.S_un.S_addr = bcip;
				STRING ip_string;
				ip_string.set(inet_ntoa(ba));
				if (!OnBroadcastIpAddressCb(ba, ip_string))
				{
					break;
				}
			}
		}
		free(buf);
	}
}