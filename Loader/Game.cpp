#include "Game.h"
#include "Functions.h"
#include "Client.h"
#include "MemoryManager.h"
#include "ListAI.h"

#include <iostream>
#include <chrono>

void Game::MainLoop() {
	while (Client::client_running == true) {
		while (Client::bot_running == true) {
			if (Functions::GetPlayerGuid() > 0) { //in-game

				// ========================================== //
				// ===========   Initialisation   =========== //
				// ========================================== //

				auto start = std::chrono::high_resolution_clock::now();

				ThreadSynchronizer::RunOnMainThread(
					[]() {
						if (Functions::IsInRaid()) tarType = "raid";
						else tarType = "party";
						NumGroupMembers = Functions::GetNumGroupMembers();
						IsInGroup = Functions::IsInGroup();

						Functions::EnumerateVisibleObjects(0);

						playerClass = Functions::UnitClass("player");
						playerRole = Functions::GetPlayerRole();

						if(localPlayer != NULL) targetUnit = localPlayer->getTarget();

						if (Functions::GetRepairAllCost() > 0) Functions::LuaCall("RepairAllItems()");
						if (Functions::GetMerchantNumItems() > 0) Functions::SellUselessItems();
					}
				);
				if (localPlayer != NULL) {

					Functions::ClassifyHeal();
					std::tie(nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer) = Functions::countEnemies();

					IsFacing = false;
					if (targetUnit != NULL && targetUnit->unitReaction <= Neutral && targetUnit->prctHP > 0) IsFacing = localPlayer->isFacing(targetUnit->position, 0.4f);

					distTarget = 0;
					if (targetUnit != NULL) distTarget = localPlayer->position.DistanceTo(targetUnit->position);

					IsSitting = false;
					int drinkingIDs[15] = { 430, 431, 432, 1133, 1135, 1137, 24355, 25696, 26261, 26402, 26473, 26475, 29007, 10250, 22734 };
					if (localPlayer->hasBuff(drinkingIDs, 15)) IsSitting = true;
					else if (IsSitting) {
						//Stop Drinking
						IsSitting = false;
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
					}

					Combat = localPlayer->flags & UNIT_FLAG_IN_COMBAT;

					hasTargetAggro = false;
					if (targetUnit != NULL) {
						for (unsigned int i = 0; i < HasAggro[0].size(); i++) {
							if (targetUnit->Guid == HasAggro[0][i]) hasTargetAggro = true;
						}
					}

					tankIndex = Functions::getTankIndex();

				}
				else std::cout << "localPlayer NULL !\n";

				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> float_ms = end - start;
				std::cout << "Initialisation: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;

				// ========================================== //
				// ==============   Movements   ============= //
				// ========================================== //

				start = std::chrono::high_resolution_clock::now();

				if (localPlayer != NULL) {

					if (IsSitting && ((localPlayer->prctMana > 80) || Combat)) {
						//Stop Drinking
						IsSitting = false;
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
					}
					else if (targetUnit != NULL && Functions::PlayerIsRanged() && ((localPlayer->castInfo == 0) || (playerClass == "Hunter" && distTarget < 10.0f)) && (localPlayer->channelInfo == 0) && (targetUnit->unitReaction <= Neutral) && (!targetUnit->isdead)) {
						if (Moving == 4 && (distTarget < 30.0f)) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if ((distTarget < 12.0f) && (Moving == 0 || Moving == 1) && (targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && (targetUnit->flags & UNIT_FLAG_CONFUSED || targetUnit->speed == 0)) {
							if (localPlayer->speed == 0) {
								Position oppositeDir = localPlayer->getOppositeDirection(targetUnit->position);
								ThreadSynchronizer::RunOnMainThread([oppositeDir]() { localPlayer->ClickToMove(Move, targetUnit->Guid, oppositeDir); });
							}
							Moving = 1;
						}
						else if ((Moving == 0) && !IsFacing) {
							ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
						}
						else if ((distTarget < 10.0f) && (Moving == 0) && ((!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && !hasTargetAggro) || ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed <= 4.5))) {
							Functions::pressKey(0x28);
							Moving = 3;
						}
						else if (distTarget > 30.0f && !IsSitting && (Moving == 0 || Moving == 2 || Moving == 5)) {
							ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position); });
							Moving = 2;
						}
						else if (Moving == 1 && (distTarget > 12.0f || !(targetUnit->flags & UNIT_FLAG_CONFUSED) || targetUnit->speed > 0)) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if ((Moving == 2 || Moving == 5) && (distTarget < 30.0f)) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if (Moving == 3 && ((distTarget > 10.0f) || (!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && hasTargetAggro) || ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed >= 7))) {
							Functions::releaseKey(0x28);
							Moving = 0;
						}
					}
					else if (targetUnit != NULL && !Functions::PlayerIsRanged() && (localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && (targetUnit->unitReaction <= Neutral) && !targetUnit->isdead) {
						if (distTarget > 5.0f && !IsSitting) {
							ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position); });
							Moving = 2;
						}
						else if (Moving == 2 || Moving == 5) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if (!IsFacing) ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
					}
					else if (Moving > 0 && Moving < 4) {
						if (Moving < 3) Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (Moving == 5 && (targetUnit == NULL || ((targetUnit->unitReaction > Neutral) && (distTarget < 30.0f)))) {
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
				}

				end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Movement: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;

				// ========================================== //
				// ===============   Actions   ============== //
				// ========================================== //

				start = std::chrono::high_resolution_clock::now();

				if (localPlayer != NULL) {
					if (playerClass == "Hunter") ListAI::HunterDps();
					else if (playerClass == "Mage") ListAI::MageDps();
					else if (playerClass == "Paladin" && playerRole == 1) ListAI::PaladinHeal();
					else if (playerClass == "Paladin" && playerRole == 2) ListAI::PaladinTank();
					else if (playerClass == "Paladin") ListAI::PaladinDps();
					else if (playerClass == "Rogue") ListAI::RogueDps();
				}

				end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Actions: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;
			}
			Sleep(300);
		}
		if (Moving > 0) {
			if (Moving < 3) Functions::pressKey(0x28);
			Functions::releaseKey(0x28);
			Moving = 0;
		}
		Sleep(300);
	}
}

int GroupMembersIndex[40];
std::vector<unsigned long long> HasAggro[40];
bool Combat = false, IsSitting = false, bossFight = false, IsInGroup = false, IsFacing = false, hasTargetAggro = false;
int AoEHeal = 0, nbrEnemy = 0, nbrCloseEnemy = 0, nbrCloseEnemyFacing = 0, nbrEnemyPlayer = 0, Moving = 0, NumGroupMembers = 0, playerRole = 3, tankIndex = 0;
float distTarget = 0;
std::string tarType = "party", playerClass = "null";
std::vector<int> HealTargetArray;
std::vector<int> listIndexCloseEnemies;
WoWUnit* ccTarget = NULL; WoWUnit* targetUnit = NULL;