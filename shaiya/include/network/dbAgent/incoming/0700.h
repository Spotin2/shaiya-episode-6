#pragma once
#include <shaiya/include/common.h>
#include <shaiya/include/item/MakeType.h>

// CUser::PacketUserItem

namespace shaiya
{
    #pragma pack(push, 1)
    struct DBAgentItemCreateIncoming
    {
        UINT16 opcode{ 0x701 };
        ULONG userId;
        UINT64 uniqueId;
        UINT32 itemId;
        UINT8 bag;
        UINT8 slot;
        UINT8 type;
        UINT8 typeId;
        UINT8 count;
        UINT16 quality;
        Gems gems;
        CraftName craftName;
        ULONG makeTime;
        MakeType makeType;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    // inventory
    struct DBAgentItemRemoveIncoming
    {
        UINT16 opcode{ 0x702 };
        ULONG userId;
        UINT8 bag;
        UINT8 slot;
        UINT8 count;

        DBAgentItemRemoveIncoming() = default;

        DBAgentItemRemoveIncoming(ULONG userId, UINT8 bag, UINT8 slot, UINT8 count)
            : userId(userId), bag(bag), slot(slot), count(count)
        {
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemBagToBankIncoming
    {
        UINT16 opcode{ 0x704 };
        ULONG userId;
        UINT8 srcBag;
        UINT8 srcSlot;
        UINT8 srcCount;
        UINT8 destSlot;
        UINT8 destCount;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemBankToBagIncoming
    {
        UINT16 opcode{ 0x705 };
        ULONG userId;
        UINT8 srcSlot;
        UINT8 srcCount;
        UINT8 destBag;
        UINT8 destSlot;
        UINT8 destCount;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemBankToBankIncoming
    {
        UINT16 opcode{ 0x706 };
        ULONG userId;
        UINT8 srcSlot;
        UINT8 srcCount;
        UINT8 destSlot;
        UINT8 destCount;

        DBAgentItemBankToBankIncoming() = default;

        DBAgentItemBankToBankIncoming(ULONG userId, UINT8 srcSlot, UINT8 srcCount, UINT8 destSlot, UINT8 destCount)
            : userId(userId), srcSlot(srcSlot), srcCount(srcCount), destSlot(destSlot), destCount(destCount)
        {
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemCountIncoming
    {
        UINT16 opcode{ 0x707 };
        ULONG userId;
        UINT8 bag;
        UINT8 slot;
        UINT8 count;

        DBAgentItemCountIncoming() = default;

        DBAgentItemCountIncoming(ULONG userId, UINT8 bag, UINT8 slot, UINT8 count)
            : userId(userId), bag(bag), slot(slot), count(count)
        {
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemGemRemoveAllIncoming
    {
        UINT16 opcode{ 0x709 };
        ULONG userId;
        UINT8 bag;
        UINT8 slot;
        UINT16 quality;
        UINT32 money;

        DBAgentItemGemRemoveAllIncoming() = default;

        DBAgentItemGemRemoveAllIncoming(ULONG userId, UINT8 bag, UINT8 slot, UINT16 quality, UINT32 money)
            : userId(userId), bag(bag), slot(slot), quality(quality), money(money)
        {
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    // aka bank teller
    struct DBAgentItemBoxToBagIncoming
    {
        UINT16 opcode{ 0x710 };
        ULONG userId;
        UINT8 srcSlot;
        UINT8 destBag;
        UINT8 destSlot;
        UINT64 uniqueId;
        UINT32 itemId;
        UINT8 type;
        UINT8 typeId;
        UINT8 count;
        UINT16 quality;
        Gems gems;
        CraftName craftName;
        ULONG makeTime;
        MakeType makeType;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemGemsIncoming
    {
        UINT16 opcode{ 0x711 };
        ULONG userId;
        UINT8 bag;
        UINT8 slot;
        Gems gems;
        UINT32 money;

        DBAgentItemGemsIncoming() = default;

        DBAgentItemGemsIncoming(ULONG userId, UINT8 bag, UINT8 slot, Gems& gems, UINT32 money)
            : userId(userId), bag(bag), slot(slot), gems(gems), money(money)
        {
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemGemRemovePosIncoming
    {
        UINT16 opcode{ 0x715 };
        ULONG userId;
        UINT8 bag;
        UINT8 slot;
        UINT8 gemPos;
        UINT16 quality;
        UINT32 money;

        DBAgentItemGemRemovePosIncoming() = default;

        DBAgentItemGemRemovePosIncoming(ULONG userId, UINT8 bag, UINT8 slot, UINT8 gemPos, UINT16 quality, UINT32 money)
            : userId(userId), bag(bag), slot(slot), gemPos(gemPos), quality(quality), money(money)
        {
        }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct DBAgentItemCraftNameIncoming
    {
        UINT16 opcode{ 0x717 };
        ULONG userId;
        UINT8 bag;
        UINT8 slot;
        CraftName craftName;

        DBAgentItemCraftNameIncoming() = default;

        DBAgentItemCraftNameIncoming(ULONG userId, UINT8 bag, UINT8 slot, CraftName& craftName)
            : userId(userId), bag(bag), slot(slot), craftName(craftName)
        {
        }
    };
    #pragma pack(pop)
}