#include "Game.h"
#include "Functions.h"
#include "Client.h"
#include "MemoryManager.h"
#include "ListAI.h"

#include <iostream>
#include <chrono>

static bool infoB = false;
static std::string playerName = "";
static bool obstacle_front = false;
static bool obstacle_back = false;
static bool los_target = false;

void Game::MainLoop() {
	while (Client::client_running == true) {
		if (!infoB && Functions::GetPlayerGuid() > 0) {
			ThreadSynchronizer::RunOnMainThread(
				[]() {
					playerName = Functions::UnitName("player");
					playerClass = Functions::UnitClass("player");
				}
			);
			std::string msg = ("Name " + playerName + " Class " + playerClass);
			Client::sendMessage(msg);
			infoB = true;
		}
		else if (infoB && Functions::GetPlayerGuid() == 0) {
			infoB = false;
			Client::sendMessage("Name Null Class Null");
		}

		if (keyTarget) {
			ThreadSynchronizer::RunOnMainThread([]() {
				std::string msg = "AssistByName('" + tankName + "')";
				Functions::LuaCall(msg.c_str());
				});
			keyTarget = false;
		}
		else if (keyHearthstone) {
			ThreadSynchronizer::RunOnMainThread([]() { Functions::UseItem("Hearthstone"); });
			keyHearthstone = false;
		}
		else if (keyMount) {
			ThreadSynchronizer::RunOnMainThread([]() { Functions::UseItem("Bridle"); });
			keyMount = false;
		}

		while (Client::bot_running == true) {
			if (Functions::GetPlayerGuid() > 0) { //in-game

				// ========================================== //
				// ===========   Initialisation   =========== //
				// ========================================== //

				auto start = std::chrono::high_resolution_clock::now();

				if (keyTarget) {
					ThreadSynchronizer::RunOnMainThread([]() {
						std::string msg = "AssistByName('" + tankName + "')";
						Functions::LuaCall(msg.c_str());
						});
					keyTarget = false;
				}
				else if (keyHearthstone) {
					ThreadSynchronizer::RunOnMainThread([]() { Functions::UseItem("Hearthstone"); });
					keyHearthstone = false;
				}
				else if (keyMount) {
					ThreadSynchronizer::RunOnMainThread([]() { Functions::UseItem("Bridle"); });
					keyMount = false;
				}

				ThreadSynchronizer::RunOnMainThread(
					[]() {
						if (Functions::IsInRaid()) tarType = "raid";
						else tarType = "party";
						NumGroupMembers = Functions::GetNumGroupMembers();
						IsInGroup = Functions::IsInGroup();

						Functions::EnumerateVisibleObjects(0);
						Functions::ClassifyHeal();

						playerClass = Functions::UnitClass("player");

						if (localPlayer != NULL) targetUnit = localPlayer->getTarget();

						if (Functions::GetRepairAllCost() > 0) Functions::LuaCall("RepairAllItems()");
						if (Functions::GetMerchantNumItems() > 0) Functions::SellUselessItems();
					}
				);
				if (localPlayer != NULL) {

					std::tie(nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer) = Functions::countEnemies();

					IsFacing = false;
					if (targetUnit != NULL && targetUnit->unitReaction <= Neutral && !targetUnit->isdead) IsFacing = localPlayer->isFacing(targetUnit->position, 0.4f);

					distTarget = 0;
					if (targetUnit != NULL) distTarget = localPlayer->position.DistanceTo(targetUnit->position);

					int drinkingIDs[15] = { 430, 431, 432, 1133, 1135, 1137, 24355, 25696, 26261, 26402, 26473, 26475, 29007, 10250, 22734 };
					if (localPlayer->hasBuff(drinkingIDs, 15)) IsSitting = true;
					else if (IsSitting) {
						//Stop Drinking
						IsSitting = false;
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}

					Combat = localPlayer->flags & UNIT_FLAG_IN_COMBAT;

					hasTargetAggro = false;
					if (targetUnit != NULL) {
						for (unsigned int i = 0; i < HasAggro[0].size(); i++) {
							if (targetUnit->Guid == HasAggro[0][i]) hasTargetAggro = true;
						}
					}

					tankIndex = Functions::getTankIndex();

					if (!infoB) {
						std::string msg = "Name " + playerName + " Class " + playerClass;
						Client::sendMessage(msg.c_str());
						infoB = true;
					}
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
					float halfPI = acos(0.0);
					Position player_pos = Position(localPlayer->position.X, localPlayer->position.Y, localPlayer->position.Z + 5);
					Position front_pos = Position(cos(localPlayer->facing) * 5 + localPlayer->position.X, sin(localPlayer->facing) * 5 + localPlayer->position.Y, localPlayer->position.Z + 5);
					Position back_pos = Position(cos(localPlayer->facing + (2 * halfPI)) * 5 + localPlayer->position.X, sin(localPlayer->facing + (2 * halfPI)) * 5 + localPlayer->position.Y, localPlayer->position.Z + 5);
					los_target = false;
					ThreadSynchronizer::RunOnMainThread([=]() {
						obstacle_front = Functions::GetDepth(front_pos) > 10;
						obstacle_back = Functions::GetDepth(back_pos) > 10;
						if (targetUnit != NULL) los_target = !Functions::Intersect(player_pos, Position(targetUnit->position.X, targetUnit->position.Y, targetUnit->position.Z + 5));
					});
					if (IsSitting && ((localPlayer->prctMana > 80) || Combat)) {
						//Stop Drinking
						IsSitting = false;
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (targetUnit != NULL && Functions::PlayerIsRanged() && ((localPlayer->castInfo == 0) || (playerClass == "Hunter" && distTarget < 11.0f)) && (localPlayer->channelInfo == 0) && (targetUnit->unitReaction <= Neutral) && (!targetUnit->isdead)) {
						if ((Moving == 4 || Moving == 2 || Moving == 5) && ((distTarget < 30.0f && los_target) || obstacle_front)) {
							//Running and ((target < 30 yard && LoS) || obstacle in front) => stop
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if ((distTarget < 11.0f) && !obstacle_back && (Moving == 0 || Moving == 1 || Moving == 4) && (targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && (targetUnit->flags & UNIT_FLAG_CONFUSED || targetUnit->speed == 0)) {
							//Player stunned or rooted < 11 yard => Run away
							if (localPlayer->speed == 0) {
								Position oppositeDir = localPlayer->getOppositeDirection(targetUnit->position);
								ThreadSynchronizer::RunOnMainThread([oppositeDir]() { localPlayer->ClickToMove(Move, targetUnit->Guid, oppositeDir); });
							}
							Moving = 1;
						}
						else if ((distTarget < 11.0f) && !obstacle_back && (Moving == 0) && ((!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && !hasTargetAggro && playerClass == "Hunter") || ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed <= 4.5))) {
							//(Creature not aggro && Hunter) || Player slowed < 11 yard => Walk backward
							Functions::pressKey(0x28);
							Moving = 3;
						}
						else if ((distTarget > 30.0f || !los_target) && !obstacle_front && !IsSitting && (Moving == 0 || Moving == 2 || Moving == 5)) {
							//(Target > 30 yard || LoS lost) and no obstacle => Run to it
							ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position); });
							Moving = 2;
						}
						else if (Moving == 1 && (obstacle_front || distTarget > 11.0f || !(targetUnit->flags & UNIT_FLAG_CONFUSED) || targetUnit->speed > 0)) {
							//Running away and (target > 11 yard || target confused || target moving)
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if (Moving == 3 && (obstacle_back || (distTarget > 11.0f) || (!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && hasTargetAggro) || ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed >= 7))) {
							//Walking backward and (target > 11 yard || Creature aggro || target running)
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if ((Moving == 0) && !IsFacing) {
							//Nothing to do: face target
							ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
						}
					}
					else if (targetUnit != NULL && !Functions::PlayerIsRanged() && (tankName != playerName || tankAutoMove) && (localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && (targetUnit->unitReaction <= Neutral) && !targetUnit->isdead) {
						if (distTarget > 5.0f && !IsSitting && !obstacle_front) {
							ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position); });
							Moving = 2;
						}
						else if (Moving == 2 || Moving == 5 || (Moving == 4 && obstacle_front)) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
							Moving = 0;
						}
						else if (!IsFacing) ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
					}
					else if ((Moving > 0 && Moving < 4) || (Moving == 4 && obstacle_front)) {
						if (Moving < 3 || Moving == 4) Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (Moving == 5 && (obstacle_front || targetUnit == NULL || ((targetUnit->unitReaction > Neutral) && (distTarget < 30.0f)))) {
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
					else if (playerClass == "Paladin" && playerSpec == 0) ListAI::PaladinHeal();
					else if (playerClass == "Paladin" && playerSpec == 1) ListAI::PaladinTank();
					else if (playerClass == "Paladin") ListAI::PaladinDps();
					else if (playerClass == "Rogue") ListAI::RogueDps();
				}

				end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Actions: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;
			}
			else if (infoB) {
				infoB = false;
				Client::sendMessage("Name Null Class Null");
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
bool Combat = false, IsSitting = false, bossFight = false, IsInGroup = false, IsFacing = false, hasTargetAggro = false, tankAutoFocus = false, tankAutoMove = false,
	keyTarget = false, keyHearthstone = false, keyMount = false;
int AoEHeal = 0, nbrEnemy = 0, nbrCloseEnemy = 0, nbrCloseEnemyFacing = 0, nbrEnemyPlayer = 0, Moving = 0, NumGroupMembers = 0, playerSpec = 3, tankIndex = 0;
float distTarget = 0;
std::string tarType = "party", playerClass = "null", tankName = "null", meleeName = "null";
std::vector<int> HealTargetArray;
std::vector<int> listIndexCloseEnemies;
WoWUnit* ccTarget = NULL; WoWUnit* targetUnit = NULL;