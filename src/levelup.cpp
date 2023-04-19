/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "list"
#include "string"
#include "random"

using namespace std;

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "Configuration/Config.h"

using namespace Acore::ChatCommands;

// Add player scripts
class levelup : public PlayerScript
{
public:
    levelup() : PlayerScript("levelup") {}

    void OnLogin(Player* player) override
    {

    }
    void OnLevelChanged(Player* player, uint8 oldLevel) override {

        if (sConfigMag->GetOption<bool>("levelup.Enable", true)) {

            QueryResult result = WorldDatabase.Query("SELECT entry FROM item_template");

            list<int> itemlist;
            if (result) {
                do {
                    int entry = result->Fetch()[0].Get<uint32>();
                    itemlist.emplace_back(entry);
                } while (result->NextRow());
            }

            int size = itemlist.size();
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> distr(0, size);
            auto itemlistFront = itemlist.begin();
            advance(itemlistFront, distr(gen));
            int item = *itemlistFront;

            QueryResult stackResult = WorldDatabase.Query("SELECT stackable FROM item_template as t WHERE t.entry = " +
                                                          to_string(item));

            list<int> stackList;
            if (stackResult) {
                do {
                    int entry = stackResult->Fetch()[0].Get<uint32>();
                    stackList.emplace_back(entry);
                } while (stackResult->NextRow());
            }
            auto stacklistFront = stackList.begin();
            int stackSize = *stacklistFront;

            player->AddItem(item, stackSize);

            return;

        }
    }
};

class levelupCommand : public CommandScript
{
public:
    levelupCommand(): CommandScript("levelupCommand") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable levelupCommandTable =
                {
                        {"itemsel", HandlelevelupItemSelCommand, SEC_ADMINISTRATOR, Console::Yes},
                };
        static ChatCommandTable levelupCommandBaseTable =
                {
                        {"lu", levelupCommandTable},
                };
        return levelupCommandBaseTable;
    }

    static bool HandlelevelupItemSelCommand(ChatHandler *handler)
    {
        Player *player = handler->GetSession()->GetPlayer();

        if(!player)
            return false;

        if (player->IsInCombat())
        {
            handler->SendSysMessage("You can't use this command while in combat!");
            handler->SetSentErrorMessage(true);
            return false;
        }
        QueryResult result = WorldDatabase.Query("SELECT entry FROM item_template");

        if (!result)
        {
            handler->SendSysMessage("Query Failed");
            handler->SetSentErrorMessage(true);
            return false;
        }
        list<int> itemlist;
        if (result)
        {
            do
            {
                int entry = result->Fetch()[0].Get<uint32>();
                itemlist.emplace_back(entry);
            } while (result->NextRow());
        }

        int size = itemlist.size();
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distr(0, size);
        auto itemlistFront = itemlist.begin();
        advance(itemlistFront, distr(gen));
        int item = *itemlistFront;

        QueryResult stackResult = WorldDatabase.Query("SELECT stackable FROM item_template as t WHERE t.entry = " +
                                                              to_string(item));
        if (!stackResult)
        {
            handler->SendSysMessage("Query Failed");
            handler->SetSentErrorMessage(true);
            return false;
        }
        list<int> stackList;
        if (stackResult)
        {
            do
            {
                int entry = stackResult->Fetch()[0].Get<uint32>();
                stackList.emplace_back(entry);
            } while (stackResult->NextRow());
        }
        auto stacklistFront = stackList.begin();
        int stackSize = *stacklistFront;

        handler->SendSysMessage(std::to_string(item));
        player->AddItem(item, stackSize);

        return true;
    }
};


// Add all scripts in one
void AddlevelupScripts()
{
    new levelup();
    new levelupCommand();
}
