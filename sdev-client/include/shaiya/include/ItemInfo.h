#pragma once
#include <include/shaiya/common.h>
#include <shaiya/include/common/Attribute.h>
#include <shaiya/include/common/Country.h>
#include <shaiya/include/common/Grow.h>
#include <shaiya/include/item/ExtDuration.h>
#include <shaiya/include/item/ItemEffect.h>
#include <shaiya/include/item/ItemType.h>
#include <shaiya/include/item/ReqOg.h>

namespace shaiya
{
    #pragma pack(push, 1)
    struct ItemInfo
    {
        char* name;             //0x00
        char* desc;             //0x04
        UINT8 type;             //0x08
        UINT8 typeId;           //0x09
        UINT8 model;            //0x0A
        UINT8 icon;             //0x0B
        UINT16 reqLevel;        //0x0C
        Country3 country;       //0x0E
        bool attackFighter;     //0x0F
        bool defenseFighter;    //0x10
        bool patrolRogue;       //0x11
        bool shootRogue;        //0x12
        bool attackMage;        //0x13
        bool defenseMage;;      //0x14
        Grow grow;              //0x15
        UINT16 reqStr;          //0x16
        UINT16 reqDex;          //0x18
        UINT16 reqRec;          //0x1A
        UINT16 reqInt;          //0x1C
        UINT16 reqWis;          //0x1E
        INT32 reqLuc;           //0x20
        INT16 reqVg;            //0x24
        ReqOg reqOg;            //0x26
        UINT8 reqIg;            //0x27
        UINT16 range;           //0x28
        UINT8 attackTime;       //0x2A
        Attribute attribute;    //0x2B
        ItemEffect effect;      //0x2C
        UINT8 slotCount;        //0x2D
        UINT16 quality;         //0x2E
        UINT16 effect1;         //0x30
        UINT16 effect2;         //0x32
        UINT16 effect3;         //0x34
        UINT16 effect4;         //0x36
        PAD(20);
        UINT32 buy;             //0x4C
        UINT32 sell;            //0x50
        UINT32 dropGrade;       //0x54
        UINT8 composeCount;     //0x58
        UINT8 count;            //0x59
        PAD(2);
        UINT32 duration;        //0x5C
        // 0x60
        ExtDuration extDuration;
        PAD(47);
        UINT8 vehicleModel;     //0x90
        PAD(3);
        UINT8 vehicleSeats;     //0x94
        PAD(3);
        UINT32 vehicleSpeed;    //0x98
        bool isSnowboard;       //0x9C
        PAD(15);
        // 0xAC
    };
    #pragma pack(pop)
}
