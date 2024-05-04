#pragma once
#include <include/shaiya/common.h>

namespace shaiya
{
    static Address g_ItemDrop = 0x1091878;

    struct ItemInfo;

    #pragma pack(push, 1)
    struct CItemDrop
    {
        static ItemInfo* GetItem(int grade/*eax*/);
        static ItemInfo* GetItemPlz(int grade/*eax*/);
    };
    #pragma pack(pop)
}
