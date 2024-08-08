#include "Game.h"
#include "FunctionsLua.h"
#include "Client.h"
#include "MemoryManager.h"
#include "ListAI.h"

#include <iostream>
//#include <chrono>

static unsigned long long playerGuid = 0;
static unsigned long long pastPlayerGuid = 0;

void Game::MainLoop() {
	while (Client::client_running == true) {
		
		ThreadSynchronizer::RunOnMainThread(
			[]() {
				playerGuid = Functions::GetPlayerGuid();
				if (playerGuid > 0) {
					if (FunctionsLua::IsInRaid()) tarType = "raid";
					else tarType = "party";
					NumGroupMembers = FunctionsLua::GetNumGroupMembers();
					IsInGroup = FunctionsLua::IsInGroup();

					Functions::EnumerateVisibleObjects(0);

					if (localPlayer != NULL) targetUnit = localPlayer->getTarget();

					if (FunctionsLua::GetRepairAllCost() > 0) Functions::LuaCall("RepairAllItems()");
					if (FunctionsLua::GetMerchantNumItems() > 0) FunctionsLua::SellUselessItems();

					if (keyTarget && Leader != NULL) {
						std::string msg = "AssistByName('" + (std::string)Leader->name + "')";
						Functions::LuaCall(msg.c_str());
						keyTarget = false;
					}
					else if (keyHearthstone) {
						FunctionsLua::UseItem("Hearthstone");
						keyHearthstone = false;
					}
					else if (keyMount) {
						FunctionsLua::UseItem("Bridle");
						keyMount = false;
					}

					if (playerGuid != pastPlayerGuid) {
						pastPlayerGuid = playerGuid;
						playerClass = FunctionsLua::UnitClass("player");
						std::string playerName = FunctionsLua::UnitName("player");
						std::string msg = ("Name " + playerName + " Class " + playerClass);
						Client::sendMessage(msg);
					}
				}
			}
		);

		while (Client::bot_running == true) {

			ThreadSynchronizer::RunOnMainThread(
				[]() {
					playerGuid = Functions::GetPlayerGuid();
					if (playerGuid > 0) {
						if (FunctionsLua::IsInRaid()) tarType = "raid";
						else tarType = "party";
						NumGroupMembers = FunctionsLua::GetNumGroupMembers();
						IsInGroup = FunctionsLua::IsInGroup();

						Functions::EnumerateVisibleObjects(0);

						if (localPlayer != NULL) targetUnit = localPlayer->getTarget();

						if (FunctionsLua::GetRepairAllCost() > 0) Functions::LuaCall("RepairAllItems()");
						if (FunctionsLua::GetMerchantNumItems() > 0) FunctionsLua::SellUselessItems();

						if (keyTarget && Leader != NULL) {
							std::string msg = "AssistByName('" + (std::string)Leader->name + "')";
							Functions::LuaCall(msg.c_str());
							keyTarget = false;
						}
						else if (keyHearthstone) {
							FunctionsLua::UseItem("Hearthstone");
							keyHearthstone = false;
						}
						else if (keyMount) {
							FunctionsLua::UseItem("Bridle");
							keyMount = false;
						}

						if (playerGuid != pastPlayerGuid) {
							pastPlayerGuid = playerGuid;
							playerClass = FunctionsLua::UnitClass("player");
							std::string playerName = FunctionsLua::UnitName("player");
							std::string msg = ("Name " + playerName + " Class " + playerClass);
							Client::sendMessage(msg);
						}
						skinningLevel = FunctionsLua::GetTradingSkill("Skinning");
						miningLevel = FunctionsLua::GetTradingSkill("Mining");
						herbalismLevel = FunctionsLua::GetTradingSkill("Herbalism");
					}
				}
			);

			// ========================================== //
			// ===========   Initialisation   =========== //
			// ========================================== //

			//auto start = std::chrono::high_resolution_clock::now();

			if (playerGuid > 0) { //in-game
				if (localPlayer != NULL) {
					Functions::ClassifyHeal();
					std::tie(nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer) = Functions::countEnemies();

					Combat = (localPlayer->flags & UNIT_FLAG_IN_COMBAT) == UNIT_FLAG_IN_COMBAT;

					IsFacing = false; distTarget = 0; hasTargetAggro = false;
					if(targetUnit != NULL) {
						if(targetUnit->attackable && !targetUnit->isdead) IsFacing = localPlayer->isFacing(targetUnit->position, 0.4f);
						distTarget = localPlayer->position.DistanceTo(targetUnit->position);

						for (unsigned int i = 0; i < HasAggro[0].size(); i++) {
							if (targetUnit->Guid == HasAggro[0][i]->Guid) hasTargetAggro = true;
						}
					}
				} else std::cout << "localPlayer NULL !\n";

				/*auto end = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double, std::milli> float_ms = end - start;
				std::cout << "Initialisation: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;*/
				
				// ========================================== //
				// ==============   Movements   ============= //
				// ========================================== //

				//start = std::chrono::high_resolution_clock::now();

				if (localPlayer != NULL) {
					float halfPI = acosf(0);
					Position back_pos = Position((cos(localPlayer->facing + (2 * halfPI)) * 2.0f) + localPlayer->position.X
						, (sin(localPlayer->facing + (2 * halfPI)) * 2.0f) + localPlayer->position.Y
						, localPlayer->position.Z);
					ThreadSynchronizer::RunOnMainThread([=]() {
						obstacle_back = Functions::Intersect(localPlayer->position, back_pos, 2.00f)
							|| (((localPlayer->movement_flags & MOVEFLAG_SWIMMING) != MOVEFLAG_SWIMMING)
								&& ((localPlayer->movement_flags & MOVEFLAG_WATERWALKING) != MOVEFLAG_WATERWALKING)
								&& Functions::GetDepth(back_pos, 2.00f) > 2.00f);
						los_target = true;
						if (targetUnit != NULL) {
							los_target = !Functions::Intersect(localPlayer->position, targetUnit->position, 2.00f);
							if (IsInGroup && (Leader != NULL) && (Leader->Guid != localPlayer->Guid) && targetUnit->attackable && !targetUnit->isdead
								&& !(targetUnit->flags & UNIT_FLAG_IN_COMBAT) && (targetUnit->Guid != Leader->targetGuid))
								Functions::LuaCall("ClearTarget()");
						}
						hasDrink = FunctionsLua::HasDrink();
						if (IsFacing && distTarget < 5.0f && (float(time(0) - autoAttackCD) >= FunctionsLua::UnitAttackSpeed("player")) && FunctionsLua::IsCurrentAction(FunctionsLua::GetSlot("Attack")))
							autoAttackCD = time(0);
					});
					  //Reset Loot History after 20 sec
					unsigned int ind = 0;
					while (ind < LootHistory.size()) {
						if (float(time(0) - get<1>(LootHistory[ind])) >= 20.0f) LootHistory.erase(LootHistory.begin() + ind);
						else ind++;
					}
					bool playerIsRanged = Functions::PlayerIsRanged();
					int drinkingIDs[15] = { 430, 431, 432, 1133, 1135, 1137, 24355, 25696, 26261, 26402, 26473, 26475, 29007, 10250, 22734 };
					int RaptorStrikeIDs[8] = { 2973, 14260, 14261, 14262, 14263, 14264, 14265, 14266 };
					if (IsSitting && ((localPlayer->prctMana > 85) || Combat || !localPlayer->hasBuff(drinkingIDs, 15) || (localPlayer->speed > 0))) {
						//Stop sitting
						IsSitting = false;
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (!Combat && !IsSitting && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && (localPlayer->movement_flags == MOVEFLAG_NONE) && (localPlayer->prctMana < 50) && (hasDrink > 0)) {
						//Drink
						IsSitting = true;
						ThreadSynchronizer::RunOnMainThread([]() { FunctionsLua::UseItem(hasDrink); });
					}
					else if (!passiveGroup && (targetUnit != NULL) && targetUnit->attackable && !targetUnit->isdead && (((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0)) || playerClass == "Hunter")) {
						if (playerIsRanged) {
							if ((Moving == 4 || Moving == 2 || Moving == 5) && distTarget < 30.0f) {
								//Running and (target < 30 yard) => stop
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if ((Moving == 0 || (Moving == 6 && localPlayer->speed == 0)) && !los_target) {
								//!LoS => Find LoS
								ThreadSynchronizer::RunOnMainThread([]() {
									Functions::MoveLoS(targetUnit->position);
									});
							}
							else if (distTarget > 30.0f && !IsSitting && (Moving == 0 || Moving == 2 || Moving == 4 || (Moving == 6 && localPlayer->speed == 0))) {
								//Target > 30 yard => Run to it
								bool swimming = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) || (localPlayer->movement_flags & MOVEFLAG_WATERWALKING)
									|| (targetUnit->movement_flags & MOVEFLAG_SWIMMING) || (targetUnit->movement_flags & MOVEFLAG_WATERWALKING));
								if (swimming) {
									ThreadSynchronizer::RunOnMainThread([]() {
										localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position);
										});
									Moving = 2;
								}
								else {
									ThreadSynchronizer::RunOnMainThread([]() {
										Functions::MoveObstacle(targetUnit->position);
										});
									Moving = 2;
								}
							}
							else if (Moving == 6 && los_target) {
								//Looking for LoS, found it => stop
								Functions::pressKey(0x28);
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if (Moving == 3 && (distTarget > 12.0f || obstacle_back
								|| ((!(targetUnit->flags & UNIT_FLAG_STUNNED) && !(targetUnit->movement_flags & MOVEFLAG_ROOT))
									&& ((!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && hasTargetAggro)
										|| ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed > 4.5))))) {
								//Walking backward and (target > 12 yard || Creature aggro || target running)
								Functions::releaseKey(0x28);
								Moving = 0;
							}
							else if ((Moving == 0) && !IsFacing) {
								//Nothing to do => face target
								ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
							}
							else if ((Moving == 0 || Moving == 3) && distTarget < 12.0f && !obstacle_back
								&& ((targetUnit->flags & UNIT_FLAG_STUNNED) || (targetUnit->movement_flags & MOVEFLAG_ROOT)
									|| (!(targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && !hasTargetAggro)
									|| ((targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED) && targetUnit->speed <= 4.5 && targetUnit->speed > 0))) {
								//(Creature not aggro || Player slowed) && < 12 yard => Walk backward
								Functions::pressKey(0x28);
								Moving = 3;
							}
						}
						else if (Leader == NULL || ((std::string)Leader->name != localPlayer->name) || tankAutoMove) {
							if ((Moving == 0 || (Moving == 6 && localPlayer->speed == 0)) && !los_target) {
								//Find LoS
								ThreadSynchronizer::RunOnMainThread([]() {
									Functions::MoveLoS(targetUnit->position);
									});
							}
							else if (distTarget > 5.0f && !IsSitting && (Moving == 0 || Moving == 2 || Moving == 4 || (Moving == 6 && localPlayer->speed == 0))) {
								//Target > 5 yard => Run to it
								bool swimming = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) || (localPlayer->movement_flags & MOVEFLAG_WATERWALKING)
									|| (targetUnit->movement_flags & MOVEFLAG_SWIMMING) || (targetUnit->movement_flags & MOVEFLAG_WATERWALKING));
								if (swimming) {
									ThreadSynchronizer::RunOnMainThread([]() {
										localPlayer->ClickToMove(Move, targetUnit->Guid, targetUnit->position);
										});
									Moving = 2;
								}
								else {
									ThreadSynchronizer::RunOnMainThread([]() {
										Functions::MoveObstacle(targetUnit->position);
										});
									Moving = 2;
								}
							}
							else if (Moving == 6 && los_target) {
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
							else if ((Moving == 0) && !IsFacing) ThreadSynchronizer::RunOnMainThread([]() { localPlayer->ClickToMove(FaceTarget, targetUnit->Guid, targetUnit->position); });
						}
					}
					else if (Moving == 1 || Moving == 2 || Moving == 6) {
						Functions::pressKey(0x28);
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if ((Moving == 5 && localPlayer->speed == 0) && !los_target && (targetUnit != NULL) && (targetUnit->unitReaction >= Friendly)) {
						//Find LoS (ally)
						ThreadSynchronizer::RunOnMainThread([]() {
							Functions::MoveLoS(targetUnit->position);
							Moving = 5;
							});
					}
					else if (Moving == 3) {
						Functions::releaseKey(0x28);
						Moving = 0;
					}
					else if (Moving == 5 && (targetUnit == NULL || (targetUnit->unitReaction < Friendly) || ((targetUnit->unitReaction >= Friendly) && los_target))) {
						if ((localPlayer->speed > 0)) {
							Functions::pressKey(0x28);
							Functions::releaseKey(0x28);
						}
						Moving = 0;
					}
					else if (!Combat && !IsSitting && (float(time(0) - gatheringCD) > 5.5f) && (localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) &&
						(Leader == NULL || Leader->Guid != localPlayer->Guid || tankAutoMove) && (targetUnit == NULL || !targetUnit->attackable || targetUnit->isdead || passiveGroup)) {
						//Loot
						bool looted = false;
						for (unsigned int i = 0; i < ListGameObjects.size(); i++) {
							if((IsInGroup && Leader != NULL && Leader->position.DistanceTo(ListGameObjects[i].position) > 40.0f)
								|| (localPlayer->position.DistanceTo(ListGameObjects[i].position) > 40.0f)) continue;
							else if (miningLevel > 0 && ((ListGameObjects[i].displayID == 310) || (miningLevel >= 65 && ListGameObjects[i].displayID == 315)
								|| (miningLevel >= 75 && ListGameObjects[i].displayID == 314) || (miningLevel >= 125 && ListGameObjects[i].displayID == 312)
								|| (miningLevel >= 155 && ListGameObjects[i].displayID == 311) || (miningLevel >= 175 && ListGameObjects[i].displayID == 313)
								|| (miningLevel >= 230 && ListGameObjects[i].displayID == 2571) || (miningLevel >= 245 && ListGameObjects[i].displayID == 3951)
								|| (miningLevel >= 275 && ListGameObjects[i].displayID == 3952) || (miningLevel >= 305 && ListGameObjects[i].displayID == 6650))) {
								Position tmp = ListGameObjects[i].position;
								tmp.Z = tmp.Z + 2.0f;
								if (localPlayer->speed == 0.0f && tmp.DistanceTo(localPlayer->position) < 8.0f) {
									ThreadSynchronizer::RunOnMainThread([=]() {
										Functions::InteractObject(ListGameObjects[i].Pointer, 1);
									});
									gatheringCD = time(0);
								}
								else {
									ThreadSynchronizer::RunOnMainThread([=]() {
										bool hasMoved = Functions::MoveObstacle(tmp);
										if (!hasMoved && (Leader == NULL || Leader->Guid != localPlayer->Guid)) {
											Functions::FollowMultibox(positionCircle); //Follow
										}
									});
									Moving = 4;
								}
								looted = true;
								break;
							}
							else if (herbalismLevel > 0 && ((ListGameObjects[i].displayID == 269) || (ListGameObjects[i].displayID == 270)
								|| (herbalismLevel >= 15 && ListGameObjects[i].displayID == 414) || (herbalismLevel >= 50 && ListGameObjects[i].displayID == 268)
								|| (herbalismLevel >= 70 && ListGameObjects[i].displayID == 271) || (herbalismLevel >= 85 && ListGameObjects[i].displayID == 700)
								|| (herbalismLevel >= 100 && ListGameObjects[i].displayID == 358) || (herbalismLevel >= 115 && ListGameObjects[i].displayID == 371)
								|| (herbalismLevel >= 120 && ListGameObjects[i].displayID == 357) || (herbalismLevel >= 125 && ListGameObjects[i].displayID == 320)
								|| (herbalismLevel >= 150 && ListGameObjects[i].displayID == 677) || (herbalismLevel >= 160 && ListGameObjects[i].displayID == 697)
								|| (herbalismLevel >= 170 && ListGameObjects[i].displayID == 698) || (herbalismLevel >= 185 && ListGameObjects[i].displayID == 701)
								|| (herbalismLevel >= 195 && ListGameObjects[i].displayID == 699) || (herbalismLevel >= 205 && ListGameObjects[i].displayID == 2312)
								|| (herbalismLevel >= 210 && ListGameObjects[i].displayID == 2314) || (herbalismLevel >= 220 && ListGameObjects[i].displayID == 2310)
								|| (herbalismLevel >= 230 && ListGameObjects[i].displayID == 2315) || (herbalismLevel >= 235 && ListGameObjects[i].displayID == 2311)
								|| (herbalismLevel >= 245 && ListGameObjects[i].displayID == 389) || (herbalismLevel >= 250 && ListGameObjects[i].displayID == 2313)
								|| (herbalismLevel >= 260 && ListGameObjects[i].displayID == 4652) || (herbalismLevel >= 270 && ListGameObjects[i].displayID == 4635)
								|| (herbalismLevel >= 280 && ListGameObjects[i].displayID == 4633) || (herbalismLevel >= 285 && ListGameObjects[i].displayID == 4632)
								|| (herbalismLevel >= 290 && ListGameObjects[i].displayID == 4634) || (herbalismLevel >= 300 && ListGameObjects[i].displayID == 4636))) {
								if (localPlayer->speed == 0.0f && ListGameObjects[i].position.DistanceTo(localPlayer->position) < 5.0f) {
									ThreadSynchronizer::RunOnMainThread([=]() {
										Functions::InteractObject(ListGameObjects[i].Pointer, 1);
									});
									gatheringCD = time(0);
								}
								else {
									ThreadSynchronizer::RunOnMainThread([=]() {
										bool hasMoved = Functions::MoveObstacle(ListGameObjects[i].position);
										if (!hasMoved && (Leader == NULL || Leader->Guid != localPlayer->Guid)) {
											Functions::FollowMultibox(positionCircle); //Follow
										}
									});
									Moving = 4;
								}
								looted = true;
								break;
							}
						}
						std::vector<bool> already_looted;
						for (int y = 0; y <= NumGroupMembers; y++) {
							already_looted.push_back(false);
						}
						for (unsigned int i = 0; i < ListUnits.size(); i++) {
							if (looted) break;
							bool lootable = (ListUnits[i].dynamic_flags & DYNAMICFLAG_CANBELOOTED);
							bool skinnable = (ListUnits[i].flags & UNIT_FLAG_SKINNABLE);
							float min_dist = localPlayer->position.DistanceTo(ListUnits[i].position);
							if (min_dist > 40.0f) continue;
							if (lootable) {
								bool skip = false;
								for (unsigned int y = 0; y < LootHistory.size(); y++) {
									if (get<0>(LootHistory[y]) == ListUnits[i].Guid) {
										skip = true;
										break;
									}
								}
								if (skip) continue;
								int player_close = 0;
								for (int y = 1; y <= NumGroupMembers; y++) {
									if (GroupMember[y] == NULL || (already_looted[y] == true) || (Leader != NULL && Leader->Guid == GroupMember[y]->Guid && !tankAutoMove)) continue;
									float dist = GroupMember[y]->position.DistanceTo(ListUnits[i].position);
									if (dist < min_dist) {
										min_dist = dist;
										player_close = y;
									}
								}
								already_looted[player_close] = true;
								if (player_close != 0) continue;
								else if (localPlayer->speed == 0.0f && ListUnits[i].position.DistanceTo(localPlayer->position) < 4.0f) {
									ThreadSynchronizer::RunOnMainThread([=]() {
										Functions::LootUnit(ListUnits[i].Pointer, 1);
									});
									LootHistory.push_back(std::tuple<unsigned long long, time_t>(ListUnits[i].Guid, time(0)));
								}
								else {
									ThreadSynchronizer::RunOnMainThread([=]() {
										bool hasMoved = Functions::MoveObstacle(ListUnits[i].position);
										if (!hasMoved && (Leader == NULL || Leader->Guid != localPlayer->Guid)) {
											Functions::FollowMultibox(positionCircle); //Follow
										}
									});
									Moving = 4;
								}
								looted = true;
								break;
							}
							else if (skinnable && skinningLevel > 0 && (ListUnits[i].level <= 20 && ((ListUnits[i].level-10)*10 <= skinningLevel)
								|| (ListUnits[i].level > 20 && (ListUnits[i].level*5 <= skinningLevel)))) {
								if (localPlayer->speed == 0.0f && ListUnits[i].position.DistanceTo(localPlayer->position) < 4.0f) {
									ThreadSynchronizer::RunOnMainThread([=]() {
										Functions::LootUnit(ListUnits[i].Pointer, 1);
									});
									LootHistory.push_back(std::tuple<unsigned long long, time_t>(ListUnits[i].Guid, time(0)));
								}
								else {
									ThreadSynchronizer::RunOnMainThread([=]() {
										bool hasMoved = Functions::MoveObstacle(ListUnits[i].position);
										if (!hasMoved && (Leader == NULL || Leader->Guid != localPlayer->Guid)) {
											Functions::FollowMultibox(positionCircle); //Follow
										}
									});
									Moving = 4;
								}
								looted = true;
								break;
							}
						}
						if (!looted && (Leader == NULL || Leader->Guid != localPlayer->Guid)) {
							//Follow
							Functions::FollowMultibox(positionCircle);
							Moving = 4;
						}
					}
					if (localPlayer->speed > 0 && Moving > 0 && (Leader == NULL || Leader->Guid != localPlayer->Guid || tankAutoMove) && (playerLastPos.DistanceTo(localPlayer->position) < 0.5f)) {
						//Jump
						Functions::pressKey(0x20);
						Functions::releaseKey(0x20);
					}
				}
				/*end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Movement: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;*/
				
				// ========================================== //
				// ===============   Actions   ============== //
				// ========================================== //

				//start = std::chrono::high_resolution_clock::now();

				if (localPlayer != NULL && !IsSitting) {
					if (playerClass == "Druid" && playerSpec == 0) ListAI::DruidBalance();
					else if (playerClass == "Druid" && playerSpec == 3) ListAI::DruidHeal();
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

				if (localPlayer != NULL) playerLastPos = localPlayer->position;

				/*end = std::chrono::high_resolution_clock::now();
				float_ms = end - start;
				std::cout << "Actions: elapsed time is " << float_ms.count() << " milliseconds" << std::endl;*/
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

std::vector<WoWUnit*> HasAggro[40]; std::vector<std::tuple<unsigned long long, time_t>> LootHistory;
bool Combat = false, IsSitting = false, IsInGroup = false, IsFacing = false, hasTargetAggro = false, tankAutoFocus = false, tankAutoMove = false,
keyTarget = false, keyHearthstone = false, keyMount = false, los_target = false, obstacle_back = false, obstacle_front = false, passiveGroup = false, petAttacking = false;
int AoEHeal = 0, nbrEnemy = 0, nbrCloseEnemy = 0, nbrCloseEnemyFacing = 0, nbrEnemyPlayer = 0, Moving = 0, NumGroupMembers = 0, playerSpec = 0, positionCircle = 0, hasDrink = 0,
skinningLevel = 0, miningLevel = 0, herbalismLevel = 0;
unsigned int LastTarget = 0;
float distTarget = 0;
std::string tarType = "party", playerClass = "null";
std::vector<std::tuple<std::string, int>> leaderInfos;
std::vector<int> HealTargetArray;
WoWUnit* ccTarget = NULL; WoWUnit* targetUnit = NULL; WoWUnit* GroupMember[40]; WoWUnit* Leader = NULL;
time_t current_time = time(0), autoAttackCD = time(0), gatheringCD = time(0);
Position playerLastPos = Position(0, 0, 0);