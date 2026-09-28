#include "ABGlobalScript.h"

#include "ABConfig.h"
#include "ABMapInfo.h"
#include "ABUtils.h"
#include "Bot/PlayerbotAI.h"

void AutoBalance_GlobalScript::OnAfterUpdateEncounterState(Map* map, EncounterCreditType type, uint32 /*creditEntry*/, Unit* /*source*/, Difficulty /*difficulty_fixed*/, DungeonEncounterList const* /*encounters*/, uint32 dungeonCompleted, bool updated)
{
    if (!updated || !map->IsDungeon())
        return;

    // --- Timewalking mode: reward only real players in TW maps ---
    if (timewalkingMode)
    {
        if (!timewalkingBossToken || timewalkingMapIds.empty())
            return;
        if (!timewalkingMapIds.count(map->GetId()))
            return;
        AutoBalanceMapInfo* twMapInfo = GetMapInfo(map);
        if (!twMapInfo->isLFGInstance)
            return;

        Map::PlayerList const& playerList = map->GetPlayers();
        if (playerList.IsEmpty())
            return;

        for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
        {
            Player* player = itr->GetSource();
            if (!player || player->IsGameMaster() || !IsRealPlayer(player))
                continue;

            // 1 token per boss kill
            if (type == ENCOUNTER_CREDIT_KILL_CREATURE)
                player->AddItem(timewalkingBossToken, 1);

            // 5 tokens (configurable) for full dungeon completion
            if (dungeonCompleted && timewalkingCompletionTokens)
                player->AddItem(timewalkingBossToken, timewalkingCompletionTokens);
        }
        return;
    }

    // --- Original reward logic ---
    if (!rewardEnabled || type != ENCOUNTER_CREDIT_KILL_CREATURE)
        return;

    AutoBalanceMapInfo* mapABInfo = GetMapInfo(map);

    if (mapABInfo->adjustedPlayerCount < MinPlayerReward)
        return;

    if (!LevelScaling || mapABInfo->mapLevel <= 70 || mapABInfo->lfgMinLevel <= 70)
        return;

    Map::PlayerList const& playerList = map->GetPlayers();
    if (playerList.IsEmpty())
        return;

    uint32 reward = map->ToInstanceMap()->GetMaxPlayers() > 5 ? rewardRaid : rewardDungeon;
    if (!reward)
        return;

    uint8 difficulty = map->GetDifficulty();

    for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
    {
        if (!itr->GetSource() || itr->GetSource()->IsGameMaster() || itr->GetSource()->GetLevel() < DEFAULT_MAX_LEVEL)
            continue;

        itr->GetSource()->AddItem(reward, 1 + difficulty);
    }
}
