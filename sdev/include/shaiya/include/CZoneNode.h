#pragma once
#include <shaiya/include/common.h>

namespace shaiya
{
    struct CMap;
    struct CZone;

    #pragma pack(push, 1)
    struct CZoneNode
    {
        CMap* map;              //0x00
        CZone* zone;            //0x04
        CRITICAL_SECTION cs08;  //0x08
        // 0x20
        PAD(120);
        // 0x98
    };
    #pragma pack(pop)
}
