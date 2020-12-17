#pragma once
#include <Windows.h>

static const int MAC_LENGTH = 6;
static const int IPV4_LENGTH = 4;


typedef struct _ETHERNET_II_HEADER_
{
    UINT8   pDestinationAddress[6];
    UINT8   pSourceAddress[6];
    UINT16 type;
}ETHERNET_II_HEADER, * PETHERNET_II_HEADER;

typedef struct _IP_HEADER_V4_
{
    union
    {
        UINT8 versionAndHeaderLength;
        struct
        {
            UINT8 headerLength : 4;
            UINT8 version : 4;
        };
    };
    union
    {
        UINT8  typeOfService;
        UINT8  differentiatedServicesCodePoint;
        struct
        {
            UINT8 explicitCongestionNotification : 2;
            UINT8 typeOfService : 6;
        };
    };
    UINT16 totalLength;
    UINT16 identification;
    union
    {
        UINT16 flagsAndFragmentOffset;
        struct
        {
            UINT16 fragmentOffset : 13;
            UINT16 flags : 3;
        };
    };
    UINT8  timeToLive;
    UINT8  protocol;
    UINT16 checksum;
    BYTE   pSourceAddress[sizeof(UINT32)];
    BYTE   pDestinationAddress[sizeof(UINT32)];
}IP_HEADER_V4, * PIP_HEADER_V4;

typedef struct _IPV4
{
    union
    {
        UINT32 address;
        struct
        {
            UINT8 bytes[4];
        };
    };
} IP_ADDRESS;