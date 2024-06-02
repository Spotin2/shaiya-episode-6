#include <map>
#include <random>
#include <ranges>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <include/main.h>
#include <include/shaiya/include/CClientToDBAgent.h>
#include <include/shaiya/include/CClientToGameLog.h>
#include <include/shaiya/include/CGameData.h>
#include <include/shaiya/include/CItem.h>
#include <include/shaiya/include/CObjectMgr.h>
#include <include/shaiya/include/CUser.h>
#include <include/shaiya/include/CZone.h>
#include <include/shaiya/include/ItemInfo.h>
#include <include/shaiya/include/SConnectionTBaseReconnect.h>
#include <include/shaiya/include/Synthesis.h>
#include <shaiya/include/common/SConnection.h>
#include <shaiya/include/item/ItemEffect.h>
#include <shaiya/include/item/ItemType.h>
#include <shaiya/include/item/MakeType.h>
#include <shaiya/include/network/dbAgent/incoming/0700.h>
#include <shaiya/include/network/game/incoming/0800.h>
#include <shaiya/include/network/game/outgoing/0200.h>
#include <shaiya/include/network/game/outgoing/0800.h>
#include <shaiya/include/network/gameLog/incoming/0400.h>
#include <util/include/util.h>
using namespace shaiya;

namespace packet_gem
{
    int find_available_slot(CUser* user, int bag)
    {
        for (int slot = 0; slot < max_inventory_slot; ++slot)
        {
            if (!user->inventory[bag][slot])
                return slot;
        }

        return -1;
    }

    bool item_remove(CUser* user, ItemInfo* itemInfo, UINT8 count)
    {
        for (const auto& [bag, items] : std::views::enumerate(
            std::as_const(user->inventory)))
        {
            if (!bag)
                continue;

            for (const auto& [slot, item] : std::views::enumerate(
                std::as_const(items)))
            {
                if (!item)
                    continue;

                if (item->itemInfo->itemId != itemInfo->itemId || item->count < count)
                    continue;

                item->count -= count;

                DBAgentItemRemoveIncoming packet(user->userId, bag, slot, count);
                SConnectionTBaseReconnect::Send(&g_pClientToDBAgent->connection, &packet, sizeof(DBAgentItemRemoveIncoming));

                GameLogItemRemoveIncoming log(user, item, bag, slot, count);
                SConnectionTBaseReconnect::Send(&g_pClientToGameLog->connection, &log, sizeof(GameLogItemRemoveIncoming));

                if (!item->count)
                {
                    ItemRemoveOutgoing outgoing(bag, slot, 0, 0, 0);
                    SConnection::Send(&user->connection, &outgoing, sizeof(ItemRemoveOutgoing));

                    CObjectMgr::FreeItem(item);
                    user->inventory[bag][slot] = nullptr;
                }
                else
                {
                    ItemRemoveOutgoing outgoing(bag, slot, item->type, item->typeId, item->count);
                    SConnection::Send(&user->connection, &outgoing, sizeof(ItemRemoveOutgoing));
                }

                return true;
            }
        }

        return false;
    }

    bool enable_perfect_enchant_step(CItem* lapisian, CItem* item)
    {
        constexpr int max_enchant_step = 20;

        auto enchantStep = CItem::GetEnchantStep(item);
        if (enchantStep >= max_enchant_step)
            return false;

        auto successRate = lapisian->itemInfo->reqRec;
        if (successRate != 10000)
            return false;

        auto lapisianLv = lapisian->itemInfo->attackTime;
        if (!lapisianLv || lapisianLv > max_enchant_step)
            return false;

        // Hot Time Lapisia +1
        if (enchantStep < 10 && lapisian->itemInfo->itemId == 95005)
            return true;

        auto lapisianStep = lapisian->itemInfo->range;
        auto lapisianType = static_cast<UINT8>(lapisian->itemInfo->country);

        if (CItem::IsWeapon(item))
        {
            // Weapon Lapisia +1
            if (enchantStep < 10 && lapisian->itemInfo->itemId == 95004)
                return true;
            else if (enchantStep == lapisianStep && lapisianType == 0)
                return true;
        }
        else
        {
            // Armor Lapisia +1
            if (enchantStep < 10 && lapisian->itemInfo->itemId == 95009)
                return true;
            else if (enchantStep == lapisianStep && lapisianType == 1)
                return true;    
        }

        return false;
    }

    void item_rune_combine_handler(CUser* user, ItemRuneCombineIncoming* incoming)
    {
        if (!incoming->runeBag || incoming->runeBag > user->bagsUnlocked || incoming->runeSlot >= max_inventory_slot)
            return;

        auto& rune = user->inventory[incoming->runeBag][incoming->runeSlot];
        if (!rune)
            return;

        if (!incoming->vialBag || incoming->vialBag > user->bagsUnlocked || incoming->vialSlot >= max_inventory_slot)
            return;

        auto& vial = user->inventory[incoming->vialBag][incoming->vialSlot];
        if (!vial)
            return;

        ItemRuneCombineOutgoing outgoing{};
        outgoing.result = ItemRuneCombineResult::Failure;

        if (rune->count < 2 || rune->itemInfo->effect != ItemEffect::ItemCompose)
        {
            SConnection::Send(&user->connection, &outgoing, 3);
            return;
        }

        ItemInfo* itemInfo = nullptr;

        switch (vial->itemInfo->effect)
        {
        case ItemEffect::ItemRemakeStr:
            itemInfo = CGameData::GetItemInfo(101, 1);
            break;
        case ItemEffect::ItemRemakeDex:
            itemInfo = CGameData::GetItemInfo(101, 2);
            break;
        case ItemEffect::ItemRemakeInt:
            itemInfo = CGameData::GetItemInfo(101, 3);
            break;
        case ItemEffect::ItemRemakeWis:
            itemInfo = CGameData::GetItemInfo(101, 4);
            break;
        case ItemEffect::ItemRemakeRec:
            itemInfo = CGameData::GetItemInfo(101, 5);
            break;
        case ItemEffect::ItemRemakeLuc:
            itemInfo = CGameData::GetItemInfo(101, 6);
            break;
        default:
            break;
        }

        if (!itemInfo)
        {
            SConnection::Send(&user->connection, &outgoing, 3);
            return;
        }

        auto result = ItemRuneCombineResult::Success;
        UINT8 bag = 1;

        while (bag <= user->bagsUnlocked)
        {
            auto slot = find_available_slot(user, bag);

            if (slot != -1)
            {
                if (!CUser::ItemCreate(user, itemInfo, 1))
                    break;

                ItemRuneCombineOutgoing outgoing(result, bag, slot, itemInfo->type, itemInfo->typeId, 1);
                SConnection::Send(&user->connection, &outgoing, sizeof(ItemRuneCombineOutgoing));

                CUser::ItemUseNSend(user, incoming->runeBag, incoming->runeSlot, false);
                CUser::ItemUseNSend(user, incoming->runeBag, incoming->runeSlot, false);
                CUser::ItemUseNSend(user, incoming->vialBag, incoming->vialSlot, false);
                break;
            }

            ++bag;
        }
    }

    void item_lapisian_combine_handler(CUser* user, ItemLapisianCombineIncoming* incoming)
    {
        if (!incoming->cubeBag || incoming->cubeBag > user->bagsUnlocked || incoming->cubeSlot >= max_inventory_slot)
            return;

        auto& cube = user->inventory[incoming->cubeBag][incoming->cubeSlot];
        if (!cube)
            return;

        // no effect :/
        if (cube->itemInfo->itemId != 101101)
            return;

        if (incoming->lapisianType != std::to_underlying(ItemType::Lapisian))
            return;

        auto itemInfo = CGameData::GetItemInfo(incoming->lapisianType, incoming->lapisianTypeId);
        if (!itemInfo)
            return;

        auto successRate = itemInfo->reqRec;
        if (successRate != 10000)
            return;

        auto lapisianLv = itemInfo->attackTime;
        if (lapisianLv < 6 || lapisianLv > 19)
            return;

        int requiredCount = 0;
        if (lapisianLv >= 6 && lapisianLv <= 10)
            requiredCount = 3;
        else if (lapisianLv >= 11 && lapisianLv <= 15)
            requiredCount = 4;
        else if (lapisianLv >= 16 && lapisianLv <= 19)
            requiredCount = 5;
        
        if (!requiredCount)
            return;

        auto createType = incoming->lapisianType;
        auto createTypeId = incoming->lapisianTypeId + 1;

        auto createInfo = CGameData::GetItemInfo(createType, createTypeId);
        if (!createInfo)
            return;

        CUser::ItemUseNSend(user, incoming->cubeBag, incoming->cubeSlot, false);

        bool hasMaterials = false;
        for (int i = 0; i < requiredCount; ++i)
            hasMaterials = item_remove(user, itemInfo, 1);

        if (hasMaterials)
        {
            auto result = ItemLapisianCombineResult::Success;
            UINT8 bag = 1;

            while (bag <= user->bagsUnlocked)
            {
                auto slot = find_available_slot(user, bag);

                if (slot != -1)
                {
                    if (!CUser::ItemCreate(user, createInfo, 1))
                        break;

                    ItemLapisianCombineOutgoing outgoing(result, bag, slot, createInfo->type, createInfo->typeId, 1);
                    SConnection::Send(&user->connection, &outgoing, sizeof(ItemLapisianCombineOutgoing));
                    break;
                }

                ++bag;
            }
        }
    }

    void item_compose_handler(CUser* user, ItemComposeIncoming* incoming)
    {
        if (!incoming->runeBag || incoming->runeBag > user->bagsUnlocked || incoming->runeSlot >= max_inventory_slot)
            return;

        auto& rune = user->inventory[incoming->runeBag][incoming->runeSlot];
        if (!rune)
            return;

        if (incoming->itemBag > user->bagsUnlocked || incoming->itemSlot >= max_inventory_slot)
            return;

        auto& item = user->inventory[incoming->itemBag][incoming->itemSlot];
        if (!item)
            return;

        ItemComposeOutgoing outgoing{};
        outgoing.result = ItemComposeResult::Failure;

        if (!item->itemInfo->composeCount)
        {
            SConnection::Send(&user->connection, &outgoing, 3);
            return;
        }

        if (item->itemInfo->reqWis <= 0 || item->itemInfo->reqWis > 99)
        {
            SConnection::Send(&user->connection, &outgoing, 3);
            return;
        }

        // optional
        if (item->makeType == MakeType::QuestResult)
        {
            SConnection::Send(&user->connection, &outgoing, 3);
            return;
        }

        auto oldItemUid = item->uniqueId;
        auto oldItemId = item->itemInfo->itemId;
        auto oldCraftName = item->craftName;

        std::random_device seed;
        std::mt19937 eng(seed());

        std::uniform_int_distribution<int> uni(1, item->itemInfo->reqWis);
        UINT8 bonus = uni(eng);
        std::string text = std::to_string(bonus);

        if (bonus < 10)
            text.insert(text.begin(), 1, '0');

        auto maxHealth = user->maxHealth;
        auto maxMana = user->maxHealth;
        auto maxStamina = user->maxHealth;

        switch (rune->itemInfo->effect)
        {
        case ItemEffect::ItemCompose:
            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);
                CItem::ReGenerationCraftExpansion(item, true);
                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            CItem::ReGenerationCraftExpansion(item, true);

            break;
        case ItemEffect::ItemComposeStr:
            if (!item->craftStrength)
                return;

            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);

                item->craftStrength = bonus;
                item->craftName[0] = text[0];
                item->craftName[1] = text[1];

                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            item->craftStrength = bonus;
            item->craftName[0] = text[0];
            item->craftName[1] = text[1];

            break;
        case ItemEffect::ItemComposeDex:
            if (!item->craftDexterity)
                return;

            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);

                item->craftDexterity = bonus;
                item->craftName[2] = text[0];
                item->craftName[3] = text[1];

                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            item->craftDexterity = bonus;
            item->craftName[2] = text[0];
            item->craftName[3] = text[1];

            break;
        case ItemEffect::ItemComposeInt:
            if (!item->craftIntelligence)
                return;

            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);

                item->craftIntelligence = bonus;
                item->craftName[6] = text[0];
                item->craftName[7] = text[1];

                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            item->craftIntelligence = bonus;
            item->craftName[6] = text[0];
            item->craftName[7] = text[1];

            break;
        case ItemEffect::ItemComposeWis:
            if (!item->craftWisdom)
                return;

            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);

                item->craftWisdom = bonus;
                item->craftName[8] = text[0];
                item->craftName[9] = text[1];

                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            item->craftWisdom = bonus;
            item->craftName[8] = text[0];
            item->craftName[9] = text[1];

            break;
        case ItemEffect::ItemComposeRec:
            if (!item->craftReaction)
                return;

            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);

                item->craftReaction = bonus;
                item->craftName[4] = text[0];
                item->craftName[5] = text[1];

                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            item->craftReaction = bonus;
            item->craftName[4] = text[0];
            item->craftName[5] = text[1];

            break;
        case ItemEffect::ItemComposeLuc:
            if (!item->craftLuck)
                return;

            if (!incoming->itemBag)
            {
                CUser::ItemEquipmentOptionRem(user, item);

                item->craftLuck = bonus;
                item->craftName[10] = text[0];
                item->craftName[11] = text[1];

                CUser::ItemEquipmentOptionAdd(user, item);

                if (!user->initEquipment)
                {
                    if (maxHealth != user->maxHealth)
                        CUser::SendMaxHP(user);

                    if (maxMana != user->maxMana)
                        CUser::SendMaxMP(user);

                    if (maxStamina != user->maxStamina)
                        CUser::SendMaxSP(user);
                }

                CUser::SetAttack(user);
                break;
            }

            item->craftLuck = bonus;
            item->craftName[10] = text[0];
            item->craftName[11] = text[1];

            break;
        default:
            SConnection::Send(&user->connection, &outgoing, 3);
            return;
        }

        outgoing.result = ItemComposeResult::Success;
        outgoing.bag = incoming->itemBag;
        outgoing.slot = incoming->itemSlot;
        outgoing.craftName = item->craftName;
        SConnection::Send(&user->connection, &outgoing, sizeof(ItemComposeOutgoing));

        DBAgentItemCraftNameIncoming packet(user->userId, incoming->itemBag, incoming->itemSlot, item->craftName);
        SConnectionTBaseReconnect::Send(&g_pClientToDBAgent->connection, &packet, sizeof(DBAgentItemCraftNameIncoming));

        GameLogItemComposeIncoming log(user, item, oldItemUid, oldItemId, oldCraftName);
        SConnectionTBaseReconnect::Send(&g_pClientToGameLog->connection, &log, sizeof(GameLogItemComposeIncoming));

        CUser::ItemUseNSend(user, incoming->runeBag, incoming->runeSlot, false);
    }

    void item_synthesis_list_handler(CUser* user, ItemSynthesisListIncoming* incoming)
    {
        if (user->status == UserStatus::Death)
            return;

        if (!incoming->squareBag || incoming->squareBag > user->bagsUnlocked || incoming->squareSlot >= max_inventory_slot)
            return;

        auto& square = user->inventory[incoming->squareBag][incoming->squareSlot];
        if (!square)
            return;

        if (square->itemInfo->effect != ItemEffect::ChaoticSquare)
            return;

        auto synthesis = g_synthesis.find(square->itemInfo->itemId);
        if (synthesis == g_synthesis.end())
            return;

        user->recallItemBag = incoming->squareBag;
        user->recallItemSlot = incoming->squareSlot;

        CUser::CancelActionExc(user);
        MyShop::Ended(&user->myShop);

        ItemSynthesisListOutgoing outgoing{};
        outgoing.goldPerPercentage = synthesis_min_money;

        int index = 0;
        for (const auto& synthesis : synthesis->second)
        {
            outgoing.createType[index] = synthesis.createType;
            outgoing.createTypeId[index] = synthesis.createTypeId;

            ++index;

            if (index < 10)
                continue;
            else
            {
                SConnection::Send(&user->connection, &outgoing, sizeof(ItemSynthesisListOutgoing));

                outgoing.createType.fill(0);
                outgoing.createTypeId.fill(0);
                index = 0;
            }
        }

        if (!index)
            return;

        SConnection::Send(&user->connection, &outgoing, sizeof(ItemSynthesisListOutgoing));
    }

    void item_synthesis_material_handler(CUser* user, ItemSynthesisMaterialIncoming* incoming)
    {
        auto& square = user->inventory[user->recallItemBag][user->recallItemSlot];
        if (!square)
            return;

        if (square->itemInfo->effect != ItemEffect::ChaoticSquare)
            return;

        auto it = g_synthesis.find(square->itemInfo->itemId);
        if (it == g_synthesis.end())
            return;

        if (incoming->index >= it->second.size())
            return;

        auto& synthesis = it->second[incoming->index];
        if (incoming->createType != synthesis.createType || incoming->createTypeId != synthesis.createTypeId)
            return;

        ItemSynthesisMaterialOutgoing outgoing(synthesis);
        SConnection::Send(&user->connection, &outgoing, sizeof(ItemSynthesisMaterialOutgoing));
    }

    void item_synthesis_handler(CUser* user, ItemSynthesisIncoming* incoming)
    {
        if (!incoming->squareBag || incoming->squareBag > user->bagsUnlocked || incoming->squareSlot >= max_inventory_slot)
            return;

        auto& square = user->inventory[incoming->squareBag][incoming->squareSlot];
        if (!square)
            return;

        if (square->itemInfo->effect != ItemEffect::ChaoticSquare)
            return;

        if (incoming->money > user->money)
            return;

        auto it = g_synthesis.find(square->itemInfo->itemId);
        if (it == g_synthesis.end())
            return;

        if (incoming->index >= it->second.size())
            return;

        auto& synthesis = it->second[incoming->index];
        auto itemInfo = CGameData::GetItemInfo(synthesis.createType, synthesis.createTypeId);
        if (!itemInfo)
            return;

        auto money = (incoming->money > synthesis_max_money) ? synthesis_max_money : incoming->money;
        auto successRate = synthesis.successRate;

        if (money >= synthesis_min_money)
            successRate += (money / synthesis_min_money) * 100;

        if (incoming->hammerBag > user->bagsUnlocked || incoming->hammerSlot >= max_inventory_slot)
            return;

        if (incoming->hammerBag > 0)
        {
            auto& hammer = user->inventory[incoming->hammerBag][incoming->hammerSlot];
            if (!hammer)
                return;

            if (hammer->itemInfo->effect != ItemEffect::CraftingHammer)
                return;

            successRate += hammer->itemInfo->reqVg * 100;
            CUser::ItemUseNSend(user, incoming->hammerBag, incoming->hammerSlot, false);
        }

        CUser::ItemUseNSend(user, incoming->squareBag, incoming->squareSlot, false);

        user->money -= money;
        CUser::SendDBMoney(user);

        int randomRate = 0;
        if (successRate < synthesis_max_success_rate)
        {
            std::random_device seed;
            std::mt19937 eng(seed());

            std::uniform_int_distribution<int> uni(
                synthesis_min_success_rate, 
                synthesis_max_success_rate
            );

            randomRate = uni(eng);
        }

        const auto& materials = std::ranges::views::zip(
            std::as_const(synthesis.materialType),
            std::as_const(synthesis.materialTypeId),
            std::as_const(synthesis.materialCount)
        );

        bool hasMaterials = false;
        for (const auto& [type, typeId, count] : materials)
        {
            auto itemInfo = CGameData::GetItemInfo(type, typeId);
            if (!itemInfo || !count)
                continue;

            hasMaterials = item_remove(user, itemInfo, count);
        }

        ItemSynthesisOutgoing outgoing{};
        outgoing.result = ItemSynthesisResult::Failure;

        if (hasMaterials && randomRate <= successRate)
        {
            if (CUser::ItemCreate(user, itemInfo, synthesis.createCount))
                outgoing.result = ItemSynthesisResult::Success;
        }

        SConnection::Send(&user->connection, &outgoing, sizeof(ItemSynthesisOutgoing));
    }

    void item_free_synthesis_handler(CUser* user, ItemFreeSynthesisIncoming* incoming)
    {
    }

    void item_ability_transfer_handler(CUser* user, ItemAbilityTransferIncoming* incoming)
    {
        constexpr int base_success_rate = 30;
        constexpr int max_success_rate = 100;

        if (!incoming->cubeBag || incoming->cubeBag > user->bagsUnlocked || incoming->cubeSlot >= max_inventory_slot)
            return;

        auto& cube = user->inventory[incoming->cubeBag][incoming->cubeSlot];
        if (!cube)
            return;

        if (cube->itemInfo->effect != ItemEffect::TransferRubiksCube)
            return;

        if (incoming->fromBag > user->bagsUnlocked || incoming->fromSlot >= max_inventory_slot)
            return;

        auto& from = user->inventory[incoming->fromBag][incoming->fromSlot];
        if (!from)
            return;

        if (incoming->toBag > user->bagsUnlocked || incoming->toSlot >= max_inventory_slot)
            return;

        auto& to = user->inventory[incoming->toBag][incoming->toSlot];
        if (!to)
            return;

        if (to->itemInfo->realType != from->itemInfo->realType)
            return;

        if (to->itemInfo->reqLevel < from->itemInfo->reqLevel)
            return;

        if (to->itemInfo->slotCount < from->itemInfo->slotCount)
            return;

        if (!to->itemInfo->composeCount || to->itemInfo->composeCount < from->itemInfo->composeCount)
            return;

        if (to->itemInfo->reqWis <= 0 || to->itemInfo->reqWis > 99 || to->itemInfo->reqWis < from->itemInfo->reqWis)
            return;

        if (!incoming->catalystBag || incoming->catalystBag > user->bagsUnlocked)
            return;

        int successRate = base_success_rate;

        if (incoming->catalystSlot != 255)
        {
            if (incoming->catalystSlot >= max_inventory_slot)
                return;

            auto& catalyst = user->inventory[incoming->catalystBag][incoming->catalystSlot];
            if (!catalyst)
                return;

            if (catalyst->itemInfo->effect != ItemEffect::Catalyst)
                return;

            successRate += catalyst->itemInfo->reqVg;
            CUser::ItemUseNSend(user, incoming->catalystBag, incoming->catalystSlot, false);
        }

        CUser::ItemUseNSend(user, incoming->cubeBag, incoming->cubeSlot, false);

        int randomRate = 0;
        if (successRate < max_success_rate)
        {
            std::random_device seed;
            std::mt19937 eng(seed());

            std::uniform_int_distribution<int> uni(1, max_success_rate);
            randomRate = uni(eng);
        }

        auto toOldItemUid = to->uniqueId;
        auto toOldItemId = to->itemInfo->itemId;
        auto toOldCraftName = to->craftName;

        auto fromOldItemUid = from->uniqueId;
        auto fromOldItemId = from->itemInfo->itemId;
        auto fromOldCraftName = from->craftName;

        auto result = ItemAbilityTransferResult::Failure;
        ItemAbilityTransferOutgoing outgoing(result, incoming->fromBag, incoming->fromSlot, incoming->toBag, incoming->toSlot);

        if (randomRate <= successRate)
        {
            outgoing.result = ItemAbilityTransferResult::Success;

            auto maxHealth = user->maxHealth;
            auto maxMana = user->maxHealth;
            auto maxStamina = user->maxHealth;

            if (!incoming->toBag)
            {
                CUser::ItemEquipmentOptionRem(user, to);

                to->gems = from->gems;
                to->craftName = from->craftName;
                to->craftStrength = from->craftStrength;
                to->craftDexterity = from->craftDexterity;
                to->craftReaction = from->craftReaction;
                to->craftIntelligence = from->craftIntelligence;
                to->craftWisdom = from->craftWisdom;
                to->craftLuck = from->craftLuck;
                to->craftHealth = from->craftHealth;
                to->craftMana = from->craftMana;
                to->craftStamina = from->craftStamina;
                to->craftAttackPower = from->craftAttackPower;
                to->craftAbsorption = from->craftAbsorption;

                CUser::ItemEquipmentOptionAdd(user, to);
            }
            else
            {
                to->gems = from->gems;
                to->craftName = from->craftName;
                to->craftStrength = from->craftStrength;
                to->craftDexterity = from->craftDexterity;
                to->craftReaction = from->craftReaction;
                to->craftIntelligence = from->craftIntelligence;
                to->craftWisdom = from->craftWisdom;
                to->craftLuck = from->craftLuck;
                to->craftHealth = from->craftHealth;
                to->craftMana = from->craftMana;
                to->craftStamina = from->craftStamina;
                to->craftAttackPower = from->craftAttackPower;
                to->craftAbsorption = from->craftAbsorption;
            }
            
            if (!incoming->fromBag)
            {
                CUser::ItemEquipmentOptionRem(user, from);

                from->gems.fill(0);
                CItem::InitCraftExpansion(from);
                from->craftStrength = 0;
                from->craftDexterity = 0;
                from->craftReaction = 0;
                from->craftIntelligence = 0;
                from->craftWisdom = 0;
                from->craftLuck = 0;
                from->craftHealth = 0;
                from->craftMana = 0;
                from->craftStamina = 0;
                from->craftAttackPower = 0;
                from->craftAbsorption = 0;

                CUser::ItemEquipmentOptionAdd(user, from);
            }
            else
            {
                from->gems.fill(0);
                CItem::InitCraftExpansion(from);
                from->craftStrength = 0;
                from->craftDexterity = 0;
                from->craftReaction = 0;
                from->craftIntelligence = 0;
                from->craftWisdom = 0;
                from->craftLuck = 0;
                from->craftHealth = 0;
                from->craftMana = 0;
                from->craftStamina = 0;
                from->craftAttackPower = 0;
                from->craftAbsorption = 0;
            }

            if (!user->initEquipment)
            {
                if (maxHealth != user->maxHealth)
                    CUser::SendMaxHP(user);

                if (maxMana != user->maxMana)
                    CUser::SendMaxMP(user);

                if (maxStamina != user->maxStamina)
                    CUser::SendMaxSP(user);
            }

            CUser::SetAttack(user);

            DBAgentItemCraftNameIncoming packet1(user->userId, incoming->toBag, incoming->toSlot, to->craftName);
            SConnectionTBaseReconnect::Send(&g_pClientToDBAgent->connection, &packet1, sizeof(DBAgentItemCraftNameIncoming));

            DBAgentItemGemsIncoming packet2(user->userId, incoming->toBag, incoming->toSlot, to->gems, user->money);
            SConnectionTBaseReconnect::Send(&g_pClientToDBAgent->connection, &packet2, sizeof(DBAgentItemGemsIncoming));

            GameLogItemComposeIncoming log1(user, to, toOldItemUid, toOldItemId, toOldCraftName);
            SConnectionTBaseReconnect::Send(&g_pClientToGameLog->connection, &log1, sizeof(GameLogItemComposeIncoming));

            DBAgentItemCraftNameIncoming packet3(user->userId, incoming->fromBag, incoming->fromSlot, from->craftName);
            SConnectionTBaseReconnect::Send(&g_pClientToDBAgent->connection, &packet3, sizeof(DBAgentItemCraftNameIncoming));

            DBAgentItemGemsIncoming packet4(user->userId, incoming->fromBag, incoming->fromSlot, from->gems, user->money);
            SConnectionTBaseReconnect::Send(&g_pClientToDBAgent->connection, &packet4, sizeof(DBAgentItemGemsIncoming));

            GameLogItemComposeIncoming log2(user, from, fromOldItemUid, fromOldItemId, fromOldCraftName);
            SConnectionTBaseReconnect::Send(&g_pClientToGameLog->connection, &log2, sizeof(GameLogItemComposeIncoming));
        }

        SConnection::Send(&user->connection, &outgoing, sizeof(ItemAbilityTransferOutgoing));
    }
}

unsigned u0x479FBC = 0x479FBC;
void __declspec(naked) naked_0x479FB4()
{
    __asm
    {
        movzx eax,word ptr[esi]
#ifdef SHAIYA_EP6_BLACKSMITH
        cmp eax,0x80D
        je case_0x80D
#endif
        cmp eax,0x80E
        je case_0x80E
        cmp eax,0x811
        je case_0x811
        cmp eax,0x830
        je case_0x830
        cmp eax,0x831
        je case_0x831
        cmp eax,0x832
        je case_0x832
        cmp eax,0x833
        je case_0x833

        // original
        add eax,-0x801
        jmp u0x479FBC

#ifdef SHAIYA_EP6_BLACKSMITH
        case_0x80D:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_rune_combine_handler
        add esp,0x8
        
        popad

        jmp exit_switch
#endif

        case_0x80E:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_lapisian_combine_handler
        add esp,0x8
        
        popad

        jmp exit_switch

        case_0x811:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_ability_transfer_handler
        add esp,0x8

        popad

        jmp exit_switch

        // chaotic squares

        case_0x830:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_synthesis_list_handler
        add esp,0x8
        
        popad

        jmp exit_switch

        case_0x831:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_synthesis_material_handler
        add esp,0x8
        
        popad

        jmp exit_switch

        case_0x832:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_synthesis_handler
        add esp,0x8
        
        popad

        jmp exit_switch

        case_0x833:
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_free_synthesis_handler
        add esp,0x8
        
        popad

        exit_switch:
        pop edi
        pop esi
        pop ecx
        retn
    }
}

unsigned u0x47A00C = 0x47A00C;
void __declspec(naked) naked_0x47A003()
{
    __asm
    {
        pushad

        push esi // packet
        push edi // user
        call packet_gem::item_compose_handler
        add esp,0x8
        
        popad

        jmp u0x47A00C
    }
}

unsigned u0x47AAE0 = 0x47AAE0;
unsigned u0x46CCF5 = 0x46CCF5;
unsigned u0x46CD83 = 0x46CD83;
void __declspec(naked) naked_0x46CCF0()
{
    __asm
    {
        // original
        call u0x47AAE0

        pushad

        mov eax,[esp+0x3C]
        push eax // item
        push esi // lapisian
        call packet_gem::enable_perfect_enchant_step
        add esp,0x8
        test al,al

        popad

        jne add_enchant_step
        jmp u0x46CCF5

        add_enchant_step:
        // isWeapon
        movzx ecx,byte ptr[esp+0x18]
        // enchant step
        movzx edx,byte ptr[esp+0x13]
        lea edx,[ecx+edx*2]
        jmp u0x46CD83
    }
}

void hook::packet_gem()
{
    // CUser::PacketGem
    util::detour((void*)0x479FB4, naked_0x479FB4, 8);

#ifdef SHAIYA_EP6_BLACKSMITH
    // CUser::PacketGem case 0x806
    util::detour((void*)0x47A003, naked_0x47A003, 9);
    // CUser::ItemLapisianAdd
    util::detour((void*)0x46CCF0, naked_0x46CCF0, 5);
#endif
}
