#pragma once
#include "BasePacket.h"
#include "TransportLayerFactory.h"
#include <sstream>
#include <iomanip>
#pragma comment(lib, "Ws2_32.lib")

inline std::wstring GetIpv4(PBYTE address);

class Ip4Packet : public BasePacket
{
public: 
	static const UINT16 Type = 0x0800;

	explicit Ip4Packet(PUINT8 data)
	{
		PIP_HEADER_V4 pHeader = (PIP_HEADER_V4)data;

		int headerLength = pHeader->headerLength << 2;

		source = GetIpv4(pHeader->pSourceAddress);
		destination = GetIpv4(pHeader->pDestinationAddress);

		nextLayerData = TransportLayerFactory::GetPacket(pHeader->typeOfService, data + headerLength);
	}

	// Inherited via BasePacket
	 std::wstring Protocol() const final
	{
		if (nextLayerData == nullptr)
		{
			return L"IPv4";
		}
		else
		{
			return nextLayerData->Destination();
		}
	}
	 std::wstring Source() const final
	{
		if (nextLayerData == nullptr)
		{
			return source;
		}
		else
		{
			return nextLayerData->Source();
		}
	}

	 std::wstring Destination() const final
	{
		if (nextLayerData == nullptr)
		{
			return destination;
		}
		else
		{
			return nextLayerData->Destination();
		}
	}

	 SIZE_T Length() const final
	{
		return 0;
	}

private:
	std::shared_ptr<BasePacket> nextLayerData;
	std::wstring source;
	std::wstring destination;
};

std::wstring GetIpv4(PBYTE pAddress)
{
	IP_ADDRESS address;
	address.address = ntohl(*((UINT32*)pAddress));

	std::wstringstream s;
	for (int i = 0; i < IPV4_LENGTH; ++i)
	{
		if (i != 0)
		{
			s << L'.';
		}
		s << address.bytes[i];
	}

	return s.str();
}

