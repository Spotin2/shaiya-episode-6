#pragma once
#include <shaiya/include/common.h>
#include <shaiya/include/common/SConnection.h>

namespace shaiya
{
    #pragma pack(push, 1)
    struct CClientToDBAgent
    {
        SConnection connection;  //0x00
        PAD(24);
        // 0xE8
    };
    #pragma pack(pop)

    // 0x58796C
    static auto g_pClientToDBAgent = (CClientToDBAgent*)0x10A2628;
}
