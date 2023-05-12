#include "ListAI.h"

#include <iostream>

static std::string GetSpellRank(std::string txt) {
	std::string list[4] = { "Ruby", "Citrine", "Jade", "Agate" };
	for (int i = 0; i < 4; i++) {
		if (Functions::IsPlayerSpell(txt + " " + list[i])) return (txt + " " + list[i]);
	}
	return "";
}

bool HasManaStone() {
	int listID[4] = { 5514, 5513, 8007, 8008 };
	if (Functions::HasItem(listID, 4) > 0) return true;
	else return false;
}

float GetManaStoneCD() {
	int listID[4] = { 5514, 5513, 8007, 8008 };
	for (int i = 0; i < 4; i++) {
		float CD = Functions::GetItemCooldownDuration(listID[i]);
		if(CD < 999) return CD;
	}
	return 999;
}

void ListAI::MageDps() {
	time_t PolymorphTimer = 15 - (time(0) - current_time);
	if (PolymorphTimer < 0) PolymorphTimer = 0;
	if (localPlayer->castInfo == 0 && localPlayer->channelInfo == 0 && !localPlayer->isdead && !passiveGroup) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			bool IsStunned = localPlayer->flags & UNIT_FLAG_STUNNED;
			bool IsConfused = localPlayer->flags & UNIT_FLAG_CONFUSED;
			int FrostArmorIDs[7] = { 168, 7300, 7301, 7302, 7320, 10219, 10220 };
			bool FrostArmorBuff = localPlayer->hasBuff(FrostArmorIDs, 7);
			int MageArmorIDs[3] = { 6117, 22782, 22783 };
			bool MageArmorBuff = localPlayer->hasBuff(MageArmorIDs, 3);
			int IceBarrierIDs[4] = { 11426, 13031, 13032, 13033 };
			bool IceBarrierBuff = localPlayer->hasBuff(IceBarrierIDs, 4);
			int ManaShieldIDs[6] = { 1463, 8494, 8495, 10191, 10192, 10193 };
			bool ManaShieldBuff = localPlayer->hasBuff(ManaShieldIDs, 6);
			int ArcaneIntellectIDs[5] = { 1459, 1460, 1461, 10156, 10157 };
			bool ArcaneIntellectBuff = localPlayer->hasBuff(ArcaneIntellectIDs, 5);
			int PolymorphIDs[6] = { 118, 12824, 12825, 12826, 28271, 28272 };
			bool PolymorphDebuff = false;
			if (targetUnit != NULL) PolymorphDebuff = targetUnit->hasDebuff(PolymorphIDs, 6);
			int ArcaneIntellectKey = Functions::GetBuffKey(ArcaneIntellectIDs, 5);

			//Specific for Blizzard cast:
			Position cluster_center = Position(0, 0, 0); int cluster_unit;
			std::tie(cluster_center, cluster_unit) = Functions::getAOETargetPos(25, 30);

			int RemoveCurseKey = Functions::GetDispelKey("Curse");
			std::string RankConjureMana = GetSpellRank("Conjure Mana");

			if (targetUnit == NULL || targetUnit->isdead || !targetUnit->attackable) {
				if (leaderName != "null" && (ListUnits[leaderIndex].targetGuid != 0)) { //Leader has target
					localPlayer->SetTarget(ListUnits[leaderIndex].targetGuid);
				}
				else {
					for (int i = 0; i <= NumGroupMembers; i++) {
						if (HasAggro[i].size() > 0) {
							localPlayer->SetTarget(HasAggro[i][0]);
							break;
						}
					}
				}
			}

			if ((IsStunned || Moving == 1) && Functions::IsSpellReady("Blink")) {
				//Blink
				Functions::CastSpellByName("Blink");
			}
			else if ((IsStunned || IsConfused) && Functions::IsSpellReady("Ice Block")) {
				//Ice Block
				Functions::CastSpellByName("Ice Block");
			}
			else if (!FrostArmorBuff && (nbrEnemyPlayer > 0 || !Functions::IsPlayerSpell("Mage Armor")) && Functions::IsSpellReady("Frost Armor")) {
				//Frost|Ice Armor (PvP)
				Functions::CastSpellByName("Ice Armor");
				Functions::CastSpellByName("Frost Armor");
			}
			else if (!MageArmorBuff && (nbrEnemyPlayer == 0) && Functions::IsSpellReady("Mage Armor")) {
				//Mage Armor (PvE)
				Functions::CastSpellByName("Mage Armor");
			}
			else if (!Combat && !ArcaneIntellectBuff && Functions::IsSpellReady("Arcane Intellect")) {
				//Arcane Intellect (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Arcane Intellect");
			}
			else if (!Combat && (ArcaneIntellectKey > 0) && (GroupMembersIndex[ArcaneIntellectKey] > -1) && Functions::IsSpellReady("Arcane Intellect")) {
				//Arcane Intellect (group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[ArcaneIntellectKey]].Guid);
				Functions::CastSpellByName("Arcane Intellect");
			}
			else if (!Combat && (localPlayer->speed == 0) && !HasManaStone() && Functions::IsSpellReady(RankConjureMana)) {
				//Conjure Mana (stone)
				Functions::CastSpellByName(RankConjureMana);
			}
			else if (!Combat && (localPlayer->speed == 0) && (hasDrink == 0) && Functions::IsSpellReady("Conjure Water")) {
				//Conjure Water
				Functions::CastSpellByName("Conjure Water");
			}
			else if (!IceBarrierBuff && Functions::IsSpellReady("Ice Barrier")) {
				//Ice Barrier
				Functions::CastSpellByName("Ice Barrier");
			}
			else if ((localPlayer->prctHP < 25) && (localPlayer->prctMana > 50) && !ManaShieldBuff && Functions::IsSpellReady("Mana Shield")) {
				//Mana Shield
				Functions::CastSpellByName("Mana Shield");
			}
			else if (Combat && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
				//Healthstone
				Functions::UseItem("Healthstone");
			}
			else if (Combat && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
				//Healing Potion
				Functions::UseItem("Healing Potion");
			}
			else if (Combat && (localPlayer->prctMana < 15) && (GetManaStoneCD() < 1.25)) {
				//Mana Stone
				Functions::UseItem("Mana ");
			}
			else if (Combat && (localPlayer->speed == 0) && (localPlayer->prctMana < 15) && (nbrCloseEnemy == 0) && Functions::IsSpellReady("Evocation")) {
				//Evocation
				Functions::CastSpellByName("Evocation");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::UseItem("Mana Potion");
			}
			else if (Functions::GetUnitDispel("player", "Curse") && Functions::IsSpellReady("Remove Lesser Curse")) {
				//Remove Lesser Curse (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Remove Lesser Curse");
				if(Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if ((RemoveCurseKey > 0) && (GroupMembersIndex[RemoveCurseKey] > -1) && Functions::IsSpellReady("Remove Lesser Curse")) {
				//Remove Lesser Curse (group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[RemoveCurseKey]].Guid);
				Functions::CastSpellByName("Remove Lesser Curse");
				if(Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
				bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
				bool targetStunned = targetUnit->flags & UNIT_FLAG_CONFUSED;
				bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
				if ((playerSpec == 2) && targetPlayer && Functions::IsSpellReady("Cold Snap") && !Functions::IsSpellReady("Frost Nova") && !Functions::IsSpellReady("Ice Block")) {
					Functions::CastSpellByName("Cold Snap");
				}
				else if ((nbrCloseEnemy >= 3 || (nbrCloseEnemyFacing >= 1 && targetPlayer) || (nbrCloseEnemy >= 1 && !IsInGroup)) && Functions::IsSpellReady("Frost Nova")) {
					//Frost Nova
					Functions::CastSpellByName("Frost Nova");
				}
				else if ((playerSpec == 1) && (nbrCloseEnemy >= 3 || (nbrCloseEnemyFacing >= 1 && targetPlayer) || (nbrCloseEnemy >= 1 && !IsInGroup)) && Functions::IsSpellReady("Blast Wave")) {
					//Blast Wave
					Functions::CastSpellByName("Blast Wave");
				}
				else if ((nbrCloseEnemyFacing >= 3 || (nbrCloseEnemyFacing >= 1 && targetPlayer) || (nbrCloseEnemyFacing >= 1 && !IsInGroup)) && Functions::IsSpellReady("Cone of Cold")) {
					//Cone of Cold
					Functions::CastSpellByName("Cone of Cold");
				}
				else if (IsFacing && targetUnit->channelInfo > 0 && Functions::IsSpellReady("Counterspell")) {
					//Counter Spell
					Functions::CastSpellByName("Counterspell");
				}
				else if ((ccTarget != NULL) && (PolymorphTimer == 0) && !(ccTarget->flags & UNIT_FLAG_CONFUSED) && Functions::IsSpellReady("Polymorph")) {
					//Polymorph (second target)
					WoWUnit* firstTarget = targetUnit;
					localPlayer->SetTarget(ccTarget->Guid);
					Functions::CastSpellByName("Polymorph");
					if(localPlayer->isCasting()) current_time = time(0);
					localPlayer->SetTarget(firstTarget->Guid);
				}
				else if ((localPlayer->speed == 0) && (cluster_unit >= 4) && (playerSpec == 1 || localPlayer->level < 20) && Functions::IsSpellReady("Flamestrike")) {
					//Flamestrike
					Functions::CastSpellByName("Flamestrike");
					Functions::ClickAOE(cluster_center);
				}
				else if ((localPlayer->speed == 0) && (cluster_unit >= 4) && Functions::IsSpellReady("Blizzard")) {
					//Blizzard
					Functions::CastSpellByName("Blizzard");
					Functions::ClickAOE(cluster_center);
				}
				else if ((localPlayer->speed > 0 || localPlayer->level < 20) && (nbrCloseEnemy >= 4) && Functions::IsSpellReady("Arcane Explosion")) {
					//Arcane Explosion
					Functions::CastSpellByName("Arcane Explosion");
				}
				else if (IsFacing && (localPlayer->speed > 0) && Functions::IsSpellReady("Fire Blast")) {
					//Fire Blast (Movement)
					Functions::CastSpellByName("Fire Blast");
				}
				else if (IsFacing && (localPlayer->speed == 0) && (playerSpec == 1) && (Functions::GetStackDebuff("target", "Interface\\Icons\\Spell_Fire_Soulburn") < 5) && Functions::IsSpellReady("Scorch")) {
					//Scorch
					Functions::CastSpellByName("Scorch");
				}
				else if ((playerSpec == 1) && Functions::UnitIsElite("target") && Functions::IsSpellReady("Combustion")) {
					//Combustion
					Functions::CastSpellByName("Combustion");
				}
				else if ((playerSpec == 1) && Functions::GetUnitBuff("player", "Interface\\Icons\\Spell_Fire_SealOfFire") && Functions::IsSpellReady("Pyroblast")) {
					//Pyroblast
					Functions::CastSpellByName("Pyroblast");
				}
				else if (IsFacing && (localPlayer->speed == 0) && (playerSpec == 1) && Functions::IsSpellReady("Fireball")) {
					//Fireball
					Functions::CastSpellByName("Fireball");
				}
				else if (IsFacing && (localPlayer->speed == 0) && Functions::IsSpellReady("Frostbolt")) {
					//Frostbolt
					Functions::CastSpellByName("Frostbolt");
				}
				else if (IsFacing && (localPlayer->speed == 0) && Functions::HasWandEquipped() && !Functions::IsAutoRepeatAction(Functions::GetSlot("Shoot"))) {
					//Wand
					Functions::CastSpellByName("Shoot");
				}
			}
		} );
	}
}