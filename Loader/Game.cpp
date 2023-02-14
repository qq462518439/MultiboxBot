#include "Game.h"
#include "Functions.h"
#include "Client.h"
#include "MemoryManager.h"
#include "ListAI.h"

#include <iostream>
#include <chrono>

static unsigned long long playerGuid = 0;
static unsigned long long pastPlayerGuid = 0;
static std::string playerName = "";

void Game::MainLoop() {
	while (Client::client_running == true) {

		if (Client::bot_running == false) {
			ThreadSynchronizer::RunOnMainThread(
				[]() {
					playerGuid = Functions::GetPlayerGuid();
					if (playerGuid > 0) {
						if (keyTarget) {
							std::string msg = "AssistByName('" + leaderName + "')";
							Functions::LuaCall(msg.c_str());
							keyTarget = false;
						}
						else if (keyHearthstone) {
							Functions::UseItem("Hearthstone");
							keyHearthstone = false;
						}
						else if (keyMount) {
							Functions::UseItem("Bridle");
							keyMount = false;
						}

						if (playerGuid != pastPlayerGuid) {
							pastPlayerGuid = playerGuid;
							playerClass = Functions::UnitClass("player");
							playerName = Functions::UnitName("player");
							std::string msg = ("Name " + playerName + " Class " + playerClass);
							Client::sendMessage(msg);
						}
					}
				}
			);
		}

		while (Client::bot_running == true) {

			// ========================================== //
			// ===========   Initialisation   =========== //
			// ========================================== //

			auto start = std::chrono::high_resolution_clock::now();

			ThreadSynchronizer::RunOnMainThread(
				[]() {
					playerGuid = Functions::GetPlayerGuid();
					if (playerGuid > 0) {
						if (Functions::IsInRaid()) tarType = "raid";
						else tarType = "party";
						NumGroupMembers = Functions::GetNumGroupMembers();
						IsInGroup = Functions::IsInGroup();

						Functions::EnumerateVisibleObjects(0);

						if (localPlayer != NULL) targetUnit = localPlayer->getTarget();

						if (Functions::GetRepairAllCost() > 0) Functions::LuaCall("RepairAllItems()");
						if (Functions::GetMerchantNumItems() > 0) Functions::SellUselessItems();

						if (keyTarget) {
							std::string msg = "AssistByName('" + leaderName + "')";
							Functions::LuaCall(msg.c_str());
							keyTarget = false;
						}
						else if (keyHearthstone) {
							Functions::UseItem("Hearthstone");
							keyHearthstone = false;
						}
						else if (keyMount) {
							Functions::UseItem("Bridle");
							keyMount = false;
						}

						if (playerGuid != pastPlayerGuid) {
							pastPlayerGuid = playerGuid;
							playerClass = Functions::UnitClass("player");
							playerName = Functions::UnitName("player");
							std::string msg = ("Name " + playerName + " Class " + playerClass);
							Client::sendMessage(msg);
						}
					}
				}
			);

			if (playerGuid > 0) { //in-game
				if (localPlayer != NULL) {

					Functions::ClassifyHeal();
					std::tie(nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer) = Functions::countEnemies();

					IsFacing = false;
					if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) IsFacing = localPlayer->isFacing(targetUnit->position, 0.4f);

					distTarget = 0;
					if (targetUnit != NULL) distTarget = localPlayer->position.DistanceTo(targetUnit->position);

					Combat = (localPlayer->flags & UNIT_FLAG_IN_COMBAT) == UNIT_FLAG_IN_COMBAT;

					hasTargetAggro = false;
					if (targetUnit != NULL) {
						for (unsigned int i = 0; i < HasAggro[0].size(); i++) {
							if (targetUnit->Guid == HasAggro[0][i]) hasTargetAggro = true;
						}
					}

					if (playerGuid > 0 && playerGuid != pastPlayerGuid) {
						pastPlayerGuid = playerGuid;
						std::string msg = ("Name " + playerName + " Class " + playerClass);
						Client::sendMessage(msg);
					}
				} else std::cout << "localPlayer NULL !\n";

				auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> float_ms = end - start;
				std::cout << "Initialisation: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;
				
				// ========================================== //
				// ==============   Movements   ============= //
				// ========================================== //

				start = std::chrono::high_resolution_clock::now();

				if (localPlayer != NULL) {
					Position player_pos = localPlayer->position; player_pos.Z = player_pos.Z + 2.25f;
					Position target_pos; if(targetUnit != NULL) target_pos = targetUnit->position; target_pos.Z += 2.25f;
					Position front_pos = Position((cos(localPlayer->facing) * 3) + localPlayer->position.X, (sin(localPlayer->facing) * 3) + localPlayer->position.Y
						, localPlayer->position.Z + 2.25f);
					Position back_pos = Position((cos(localPlayer->facing + (2 * halfPI)) * 3) + localPlayer->position.X, (sin(localPlayer->facing + (2 * halfPI)) * 3) + localPlayer->position.Y
						, localPlayer->position.Z + 2.25f);
					ThreadSynchronizer::RunOnMainThread([=]() {
						obstacle_front = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) != MOVEFLAG_SWIMMING)
							&& ((localPlayer->movement_flags & MOVEFLAG_WATERWALKING) != MOVEFLAG_WATERWALKING) && Functions::GetDepth(front_pos) > 5.0f;
						obstacle_back = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) != MOVEFLAG_SWIMMING)
							&& ((localPlayer->movement_flags & MOVEFLAG_WATERWALKING) != MOVEFLAG_WATERWALKING) && Functions::GetDepth(back_pos) > 5.0f;
						if (targetUnit != NULL) {
							los_target = !Functions::Intersect(player_pos, Position(targetUnit->position.X, targetUnit->position.Y, targetUnit->position.Z + 2.25f));
							if (IsInGroup && (leaderName != "null") && (leaderName != playerName) && targetUnit->attackable && !targetUnit->isdead
								&& ((targetUnit->flags & UNIT_FLAG_IN_COMBAT) != UNIT_FLAG_IN_COMBAT) && (targetUnit->Guid != ListUnits[leaderIndex].targetGuid))
								Functions::LuaCall("ClearTarget()");
						}
						hasDrink = Functions::HasDrink();
					});
					bool playerIsRanged = Functions::PlayerIsRanged();
					int drinkingIDs[15] = { 430, 431, 432, 1133, 1135, 1137, 24355, 25696, 26261, 26402, 26473, 26475, 29007, 10250, 22734 };
					int RaptorStrikeIDs[8] = { 2973, 14260, 14261, 14262, 14263, 14264, 14265, 14266 };

					if (IsSitting && ((localPlayer->prctMana > 80) || Combat || !localPlayer->hasBuff(drinkingIDs, 15) || (localPlayer->speed > 0))) {
						//Stop sitting
						IsSitting = false;
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (!Combat && !IsSitting && (localPlayer->speed == 0) && (localPlayer->movement_flags == MOVEFLAG_NONE) && (localPlayer->prctMana < 33) && (hasDrink > 0)) {
						//Drink
						IsSitting = true;
						ThreadSynchronizer::RunOnMainThread([]() { Functions::UseItem(hasDrink); });
					}
					else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead && ((localPlayer->castInfo == 0) || (playerClass == "Hunter" && localPlayer->isCasting(RaptorStrikeIDs, 8))) && (localPlayer->channelInfo == 0)) {
						if (playerIsRanged) {
							if ((Moving == 4 || Moving == 2 || Moving == 5) && (distTarget < 30.0f || obstacle_front)) {
								//Running and (target < 30 yard || obstacle in front) => stop
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if (distTarget < 12.0f && (Moving == 0 || Moving == 1) && (targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED)
								&& (targetUnit->flags & UNIT_FLAG_CONFUSED || targetUnit->speed == 0)) {
								//Player stunned or rooted and target < 12 yard => Run away
								if (localPlayer->speed == 0) {
									Position oppositeDir = localPlayer->getOppositeDirection(targetUnit->position, 12.0f);
									float angle_oppositeDir = atan2f(oppositeDir.Y - player_pos.Y, oppositeDir.X - player_pos.X);
									if (angle_oppositeDir < 0.0f) angle_oppositeDir += halfPI * 4.0f;
									else if (angle_oppositeDir > halfPI * 4) angle_oppositeDir -= halfPI * 4.0f;
									Position tmp_pos = Position((cos(angle_oppositeDir) * 3) + localPlayer->position.X
										, (sin(angle_oppositeDir) * 3) + localPlayer->position.Y, localPlayer->position.Z + 2.25f);
									ThreadSynchronizer::RunOnMainThread([oppositeDir, tmp_pos, player_pos]() {
										bool obstacle_oppositeDir = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) != MOVEFLAG_SWIMMING)
											&& ((localPlayer->movement_flags & MOVEFLAG_WATERWALKING) != MOVEFLAG_WATERWALKING) && Functions::GetDepth(tmp_pos) > 5.0f;
										if (!Functions::Intersect(player_pos, oppositeDir) && !obstacle_oppositeDir) {
											localPlayer->ClickToMove(Move, targetUnit->Guid, oppositeDir);
											Moving = 1;
										}
										else {
											Functions::MoveLoS(oppositeDir);
											Moving = 1;
										}
									});
								}
							}
							else if ((Moving == 0 || (Moving == 6 && localPlayer->speed == 0)) && !los_target) {
								//Find LoS
								ThreadSynchronizer::RunOnMainThread([target_pos]() {
									Functions::MoveLoS(target_pos);
									Moving = 6;
								});
							}
							else if (distTarget > 30.0f && !IsSitting && (Moving == 0 || Moving == 2 || Moving == 4 || (Moving == 6 && localPlayer->speed == 0))) {
								//Target > 30 yard => Run to it
								float angle_targetPos = atan2f(targetUnit->position.Y - player_pos.Y, targetUnit->position.X - player_pos.X);
								if (angle_targetPos < 0.0f) angle_targetPos += halfPI * 4.0f;
								else if (angle_targetPos > halfPI * 4) angle_targetPos -= halfPI * 4.0f;
								Position tmp_pos = Position((cos(angle_targetPos) * 3) + localPlayer->position.X
									, (sin(angle_targetPos) * 3) + localPlayer->position.Y, localPlayer->position.Z + 2.25f);
								ThreadSynchronizer::RunOnMainThread([tmp_pos]() {
									bool obstacle_targetPos = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) != MOVEFLAG_SWIMMING)
										&& ((localPlayer->movement_flags & MOVEFLAG_WATERWALKING) != MOVEFLAG_WATERWALKING) && Functions::GetDepth(tmp_pos) > 5.0f;
									if (!obstacle_targetPos && los_target) {
										localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position);
										Moving = 2;
									}
									else {
										Functions::MoveLoS(targetUnit->position);
										Moving = 6;
									}
								});
							}
							else if (Moving == 6 && localPlayer->speed == 0 && los_target) {
								//Looking for LoS, found it => stop
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if (Moving == 1 && (distTarget > 12.0f || targetUnit->speed > 4.5)) {
								//Running away and (target > 12 yard || target moving)
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if (Moving == 3 && (obstacle_back || distTarget > 12.0f || (!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && hasTargetAggro)
								|| ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed >= 4.5))) {
								//Walking backward and (target > 12 yard || Creature aggro || target running)
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if ((Moving == 0) && !IsFacing) {
								//Nothing to do: face target
								ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
							}
							else if ((Moving == 0 || (Moving == 3 && localPlayer->speed == 0)) && distTarget < 12.0f && !obstacle_back && ((!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED)
								&& !hasTargetAggro) || ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed <= 4.5))) {
								//(Creature not aggro || Player slowed) && < 12 yard => Walk backward
								Functions::pressKey(0x28);
								Moving = 3;
							}
						}
						else if(leaderName != playerName || tankAutoMove) {
							if ((Moving == 0 || (Moving == 6 && localPlayer->speed == 0)) && !los_target) {
								//Find LoS
								ThreadSynchronizer::RunOnMainThread([target_pos]() {
									Functions::MoveLoS(target_pos);
									Moving = 6;
								});
							}
							else if (distTarget > 5.0f && !IsSitting && (Moving == 0 || Moving == 2 || Moving == 4 || (Moving == 6 && localPlayer->speed == 0))) {
								//Target > 5 yard => Run to it
								float angle_targetPos = atan2f(targetUnit->position.Y - player_pos.Y, targetUnit->position.X - player_pos.X);
								if (angle_targetPos < 0.0f) angle_targetPos += halfPI * 4.0f;
								else if (angle_targetPos > halfPI * 4) angle_targetPos -= halfPI * 4.0f;
								Position tmp_pos = Position((cos(angle_targetPos) * 3) + localPlayer->position.X
									, (sin(angle_targetPos) * 3) + localPlayer->position.Y, localPlayer->position.Z + 2.25f);
								ThreadSynchronizer::RunOnMainThread([tmp_pos]() {
									bool obstacle_targetPos = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) != MOVEFLAG_SWIMMING)
										&& ((localPlayer->movement_flags & MOVEFLAG_WATERWALKING) != MOVEFLAG_WATERWALKING) && Functions::GetDepth(tmp_pos) > 5.0f;
									if (!obstacle_targetPos && los_target) {
										localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position);
										Moving = 2;
									}
									else {
										Functions::MoveLoS(targetUnit->position);
										Moving = 6;
									}
								});
							}
							else if (Moving == 6 && localPlayer->speed == 0 && los_target) {
								//Looking for LoS, found it => stop
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if (Moving == 2 || Moving == 4 || Moving == 5) {
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if (!IsFacing) ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
						}
					}
					else if (Moving == 1 || Moving == 2 || Moving == 6) {
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if ((Moving == 5 && localPlayer->speed == 0) && !los_target && (targetUnit != NULL) && (targetUnit->unitReaction >= Friendly)) {
						//Find LoS (ally)
						ThreadSynchronizer::RunOnMainThread([target_pos]() {
							Functions::MoveLoS(target_pos);
							Moving = 5;
						});
					}
					else if (Moving == 3) {
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (Moving == 5 && (targetUnit == NULL || (targetUnit->unitReaction < Friendly) || ((targetUnit->unitReaction >= Friendly) && los_target))) {
						if((localPlayer->speed > 0)) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
						}
						Moving = 0;
					}
					else if ((leaderName != "null") && (leaderName != playerName) && !Combat && !IsSitting && IsInGroup && (localPlayer->castInfo == 0)
					&& (localPlayer->channelInfo == 0) && (targetUnit == NULL || !targetUnit->attackable || targetUnit->isdead)) {
						//Follow
						Functions::FollowMultibox(playerIsRanged, positionCircle); //(Circle radius, Circle position)
					}
				}

				end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Movement: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;
				
				// ========================================== //
				// ===============   Actions   ============== //
				// ========================================== //

				start = std::chrono::high_resolution_clock::now();

				if (localPlayer != NULL && !IsSitting) {
					if (playerClass == "Druid" && playerSpec == 0) ListAI::DruidBalance();
					else if (playerClass == "Hunter") ListAI::HunterDps();
					else if (playerClass == "Mage") ListAI::MageDps();
					else if (playerClass == "Paladin" && playerSpec == 0) ListAI::PaladinHeal();
					else if (playerClass == "Paladin" && playerSpec == 1) ListAI::PaladinTank();
					else if (playerClass == "Paladin") ListAI::PaladinDps();
					else if (playerClass == "Priest") ListAI::PriestHeal();
					else if (playerClass == "Rogue") ListAI::RogueDps();
					else if (playerClass == "Warlock") ListAI::WarlockDps();
					else if (playerClass == "Warrior" && playerSpec == 2) ListAI::WarriorTank();
				}

				end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Actions: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;
			}
			Sleep(250);
		}
		if (Moving > 0) {
			if (Moving < 3) Functions::pressKey(0x28);
			Functions::releaseKey(0x28);
			Moving = 0;
		}
		Sleep(250);
	}
}

int GroupMembersIndex[40];
std::vector<unsigned long long> HasAggro[40];
bool Combat = false, IsSitting = false, bossFight = false, IsInGroup = false, IsFacing = false, hasTargetAggro = false, tankAutoFocus = false, tankAutoMove = false,
	keyTarget = false, keyHearthstone = false, keyMount = false, los_target = false, obstacle_back = false, obstacle_front = false;
int AoEHeal = 0, nbrEnemy = 0, nbrCloseEnemy = 0, nbrCloseEnemyFacing = 0, nbrEnemyPlayer = 0, Moving = 0, NumGroupMembers = 0, playerSpec = 0, positionCircle = 0, hasDrink = 0, leaderIndex = 0, LastTarget = 0;
float distTarget = 0, halfPI = acosf(0);
std::string tarType = "party", playerClass = "null", tankName = "null", meleeName = "null", leaderName = "null";
std::vector<int> HealTargetArray;
WoWUnit* ccTarget = NULL; WoWUnit* targetUnit = NULL;
time_t current_time = time(0);