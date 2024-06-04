#pragma once
#include <strsafe.h>
#include <shaiya/include/common.h>
#include <shaiya/include/common/ChatType.h>
#include <sdev/include/shaiya/include/CUser.h>

namespace shaiya
{
    #pragma pack(push, 1)
    struct GameLogChatIncoming
    {
        UINT16 opcode{ 0x901 };
        ULONG userId;
        ULONG charId;
        UINT16 mapId;
        ChatType chatType;
        UINT8 messageLength;
        ChatMessage message;

        GameLogChatIncoming() = default;

        GameLogChatIncoming(CUser* user, ChatType chatType, const char* message)
            : userId(user->userId), charId(user->id), mapId(user->mapId), chatType(chatType), messageLength(0), message{}
        {
            this->messageLength = static_cast<UINT8>(std::strlen(message) + 1);

            auto result = StringCbCopyA(this->message.data(), this->message.size(), message);
            if (result == STRSAFE_E_INSUFFICIENT_BUFFER)
                this->messageLength = static_cast<UINT8>(this->message.size());
        }

        constexpr int size_without_message() { return 14; }
        constexpr int length() { return size_without_message() + this->messageLength; }
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct GameLogChatToIncoming
    {
        UINT16 opcode{ 0x902 };
        ULONG userId;
        ULONG charId;
        UINT16 mapId;
        ChatType chatType;
        CharName targetName;
        UINT8 messageLength;
        ChatMessage message;

        GameLogChatToIncoming() = default;

        GameLogChatToIncoming(CUser* user, const char* targetName, const char* message)
            : userId(user->userId), charId(user->id), mapId(user->mapId), chatType(ChatType::To), targetName{}, messageLength(0), message{}
        {
            StringCbCopyA(this->targetName.data(), this->targetName.size(), targetName);
            this->messageLength = static_cast<UINT8>(std::strlen(message) + 1);

            auto result = StringCbCopyA(this->message.data(), this->message.size(), message);
            if (result == STRSAFE_E_INSUFFICIENT_BUFFER)
                this->messageLength = static_cast<UINT8>(this->message.size());
        }

        constexpr int size_without_message() { return 35; }
        constexpr int length() { return size_without_message() + this->messageLength; }
    };
    #pragma pack(pop)
}
