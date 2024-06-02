#pragma once
#include <array>
#include <shaiya/include/common.h>
#include <sdev/include/shaiya/include/CItem.h>
#include <sdev/include/shaiya/include/CUser.h>
#include <sdev/include/shaiya/include/GameLogMain.h>
#include <sdev/include/shaiya/include/ItemInfo.h>

namespace shaiya
{
    struct CItem;
    struct CUser;

    enum struct GameLogItemCraftType : UINT8
    {
        None,
        ItemCreate,
        Remake
    };

    #pragma pack(push, 1)
    // ActionType 111
    struct GameLogItemCreateIncoming
    {
        UINT16 opcode{ 0x401 };
        GameLogMain main;
        UINT64 itemUid;
        UINT32 itemId;
        ItemName itemName;
        Gems gems;
        ULONG makeTime;
        CraftName craftName;
        GameLogItemCraftType craftType;
        UINT8 count;

        GameLogItemCreateIncoming() = default;

        GameLogItemCreateIncoming(CUser* user, CItem* item, GameLogItemCraftType craftType, UINT8 count)
            : main{}, craftType(craftType), count(count)
        {
            CUser::SetGameLogMain(user, this);
            this->itemUid = item->uniqueId;
            this->itemId = item->itemInfo->itemId;
            this->itemName = item->itemInfo->itemName;
            this->gems = item->gems;
            this->makeTime = item->makeTime;
            this->craftName = item->craftName;
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    // ActionType 112
    struct GameLogItemRemoveIncoming
    {
        UINT16 opcode{ 0x402 };
        GameLogMain main;
        UINT64 itemUid;
        UINT32 itemId;
        ItemName itemName;
        Gems gems;
        ULONG makeTime;
        CraftName craftName;
        UINT8 bag;
        UINT8 slot;
        UINT8 count;

        GameLogItemRemoveIncoming() = default;

        GameLogItemRemoveIncoming(CUser* user, CItem* item, UINT8 bag, UINT8 slot, UINT8 count)
            : main{}, bag(bag), slot(slot), count(count)
        {
            CUser::SetGameLogMain(user, this);
            this->itemUid = item->uniqueId;
            this->itemId = item->itemInfo->itemId;
            this->itemName = item->itemInfo->itemName;
            this->gems = item->gems;
            this->makeTime = item->makeTime;
            this->craftName = item->craftName;
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    // ActionType 213
    struct GameLogItemComposeIncoming
    {
        UINT16 opcode{ 0x427 };
        GameLogMain main;
        UINT64 itemUid;
        UINT32 itemId;
        ItemName itemName;
        Gems gems;
        ULONG makeTime;
        CraftName craftName;
        UINT64 oldItemUid;
        UINT32 oldItemId;
        CraftName oldCraftName;

        GameLogItemComposeIncoming() = default;

        GameLogItemComposeIncoming(CUser* user, CItem* item, UINT64 oldItemUid, UINT32 oldItemId, CraftName& oldCraftName)
            : main{}, oldItemUid(oldItemUid), oldItemId(oldItemId), oldCraftName(oldCraftName)
        {
            CUser::SetGameLogMain(user, this);
            this->itemUid = item->uniqueId;
            this->itemId = item->itemInfo->itemId;
            this->itemName = item->itemInfo->itemName;
            this->gems = item->gems;
            this->makeTime = item->makeTime;
            this->craftName = item->craftName;
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    // ActionType 215
    struct GameLogItemRemakeIncoming
    {
        UINT16 opcode{ 0x429 };
        GameLogMain main;
        UINT64 itemUid;
        UINT32 itemId;
        ItemName itemName;
        Gems gems;
        ULONG makeTime;
        CraftName craftName;
        UINT64 oldItemUid1;
        UINT64 oldItemUid2;
        UINT64 oldItemUid3;

        GameLogItemRemakeIncoming() = default;

        GameLogItemRemakeIncoming(CUser* user, CItem* item, UINT64 oldItemUid1, UINT64 oldItemUid2, UINT64 oldItemUid3)
            : main{}, oldItemUid1(oldItemUid1), oldItemUid2(oldItemUid2), oldItemUid3(oldItemUid3)
        {
            CUser::SetGameLogMain(user, this);
            this->itemUid = item->uniqueId;
            this->itemId = item->itemInfo->itemId;
            this->itemName = item->itemInfo->itemName;
            this->gems = item->gems;
            this->makeTime = item->makeTime;
            this->craftName = item->craftName;
        }
    };
    #pragma pack(pop)
}
