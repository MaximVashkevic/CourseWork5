#pragma once
#include "BasePacket.h"

class TransportLayerFactory {
public:
	static std::shared_ptr<BasePacket> GetPacket(int type, PUINT8 data)
	{
		switch (type)
		{
		}
		return nullptr;
	}
};