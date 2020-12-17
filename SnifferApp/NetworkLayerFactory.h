#pragma once
#include "BasePacket.h"
#include "Ip4Packet.h"

class NetworkLayerFactory {
public:
	static std::shared_ptr<BasePacket> GetPacket(int type, PUINT8 data)
	{
		switch (type)
		{
		case Ip4Packet::Type:
		{
			return std::make_shared<Ip4Packet>(data);
		}
		}
		return nullptr;
	}
};