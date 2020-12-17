#pragma once
#include "BasePacket.h"
#include <memory>
#include "NetworkLayerFactory.h"
#include <stdio.h>
#include <sstream>
#include <iomanip>

inline std::wstring GetMac(UINT8* pData)
{
    std::wstringstream s;

    for (int i = 0; i < MAC_LENGTH; ++i)
    {
        s << std::setfill(L'0') << std::setw(2) << std::hex << pData[i];
        if (i != MAC_LENGTH - 1)
        {
            s << L':';
        }
    }

    return s.str();
}

class EthernetPacket :
    virtual public BasePacket
{
public:
    EthernetPacket(SIZE_T length, PUINT8 data)
        : BasePacket()
    {
        PETHERNET_II_HEADER header = (PETHERNET_II_HEADER)data;

        this->source = GetMac(header->pSourceAddress);
        this->destination = GetMac(header->pDestinationAddress);

        this->length = length;
        this->pData = data;

        nextLayerData = NetworkLayerFactory::GetPacket(ntohs(header->type), data + sizeof(ETHERNET_II_HEADER));
    }
    ~EthernetPacket()
    {
        delete[] pData;
    }


private:
    SIZE_T length;
    std::shared_ptr<BasePacket> nextLayerData;
    std::wstring source;
    std::wstring destination;
    PUINT8 pData;

    // Inherited via BasePacket
    std::wstring Protocol() const final
    {
        if (nextLayerData == nullptr)
        {
            return L"Ethernet";
        }
        else
        {
            return nextLayerData->Protocol();
        }
    }

    std::wstring Source() const final {
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
        return length;
    }

    std::wstring Bytes() const
    {
        std::wstringstream s;

        for (int i = 0; i < length; ++i)
        {
            if (i != 0 && i % 16 == 0)
            {
                s << std::endl;
            }

            if (i % 16 == 0)
            {
                s << std::setfill(L'0') << std::setw(4) << std::hex << i << L':';
            }
            s << std::setfill(L'0') << std::setw(2) << std::hex << pData[i];
        }

        return s.str();
    }

};

