#include "ListAI.h"

#include <iostream>

static std::string GetSpellRank(std::string txt) {
	std::string list[5] = { "Major", "Greater", "", "Lesser", "Minor" };
	for (int i = 0; i < 5; i++) {
		if (Functions::IsPlayerSpell(txt + " " + list[i])) return (txt + " " + list[i]);
	}
	return "";
}

bool HasSoulstone() {
	int listID[] = { 5232, 16892, 16893, 16895, 16896 };
	if (Functions::HasItem(listID, 5) > 0) return true;
	else return false;
}

float GetSoulstoneCD() {
	int listID[] = { 5232, 16892, 16893, 16895, 16896 };
	float CD = Functions::GetItemCooldownDuration(listID, 5);
	return CD;
}

void ListAI::WarlockDps() {
	if (localPlayer->castInfo == 0 && localPlayer->channelInfo == 0 && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {

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

			int DemonSkinIDs[7] = { 687, 696, 706, 1086, 11733, 11734, 11735 }; //Demon Armor included
			bool DemonSkinBuff = localPlayer->hasBuff(DemonSkinIDs, 7);
			std::string RankCreateHealthstone = GetSpellRank("Create Healthstone");
			std::string RankCreateSoulstone = GetSpellRank("Create Soulstone");
			if (Combat && (localPlayer->prctHP < 40.0f) && (Functions::GetHealthstoneCD() < 1.25)) {
				//Healthstone
				Functions::UseItem("Healthstone");
			}
			else if (Combat && (localPlayer->prctHP < 35.0f) && (Functions::GetHPotionCD() < 1.25)) {
				//Healing Potion
				Functions::UseItem("Healing Potion");
			}
			else if (Combat && (localPlayer->prctMana < 10.0f) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::UseItem("Mana Potion");
			}
			else if (!DemonSkinBuff && Functions::IsSpellReady("Demon Armor")) {
				//Demon Armor
				Functions::CastSpellByName("Demon Armor");
			}
			else if (!DemonSkinBuff && !Functions::IsPlayerSpell("Demon Armor") && Functions::IsSpellReady("Demon Skin")) {
				//Demon Skin
				Functions::CastSpellByName("Demon Skin");
			}
			if (!Combat && !Functions::HasPetUI() && Functions::IsSpellReady("Summon Imp")) {
				//Summon Imp
				Functions::CastSpellByName("Summon Imp");
			}
			else if (!Combat && (localPlayer->speed == 0) && !Functions::HasHealthstone() && Functions::IsSpellReady(RankCreateHealthstone)) {
				//Create Healthstone
				Functions::CastSpellByName(RankCreateHealthstone);
			}
			else if (!Combat && (localPlayer->speed == 0) && !HasSoulstone() && Functions::IsSpellReady(RankCreateSoulstone)) {
				//Create Soulstone
				Functions::CastSpellByName(RankCreateSoulstone);
			}
			else if (!Combat && (localPlayer->speed == 0) && HasSoulstone() && (GetSoulstoneCD() < 1.0f)) {
				//Use Soulstone
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[Functions::GetHealer()]].Guid);
				Functions::UseItem("Soulstone");
			}
			else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
				bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
				bool targetFear = targetUnit->flags & UNIT_FLAG_FLEEING;
				int nbrSoulShard = Functions::GetItemCount(6265);
				time_t FearTimer = 15 - (time(0) - current_time);
				if (FearTimer < 0) FearTimer = 0;
				int nbrAggro = HasAggro[0].size();
				int CoTonguesIDs[2] = { 1714, 11719 }; bool CoTonguesDebuff = targetUnit->hasDebuff(CoTonguesIDs, 2);
				int CoShadowIDs[2] = { 17862, 17937 }; bool CoShadowDebuff = targetUnit->hasDebuff(CoShadowIDs, 2);
				int CoAgonyIDs[6] = { 980, 1014, 6217, 11711, 11712, 11713 }; bool CoAgonyDebuff = targetUnit->hasDebuff(CoAgonyIDs, 6);
				int CorruptionIDs[7] = { 172, 6222, 6223, 7648, 11671, 11672, 25311 }; bool CorruptionDebuff = targetUnit->hasDebuff(CorruptionIDs, 7);
				int SiphonLifeIDs[4] = { 18265, 18879, 18880, 18881 }; bool SiphonLifeDebuff = targetUnit->hasDebuff(SiphonLifeIDs, 4);
				int ImmolateIDs[8] = { 348, 707, 1094, 2941, 11665, 11667, 11668, 25309 }; bool ImmolateDebuff = targetUnit->hasDebuff(ImmolateIDs, 8);
				//Specific for Rain of Fire cast:
				Position cluster_center = Position(0, 0, 0); int cluster_unit;
				std::tie(cluster_center, cluster_unit) = Functions::getAOETargetPos(25, 30);
				if ((localPlayer->prctHP < 40.0f) && targetPlayer && !targetFear && Functions::IsSpellReady("Death Coil")) {
					//Death Coil (PvP)
					Functions::CastSpellByName("Death Coil");
				}
				else if ((localPlayer->speed == 0) && (nbrAggro >= 2) && (nbrCloseEnemy >= 2) && Functions::IsSpellReady("Howl of Terror")) {
					//Howl of Terror
					Functions::CastSpellByName("Howl of Terror");
				}
				else if (!CoTonguesDebuff && targetPlayer && Functions::UnitIsCaster("target") && Functions::IsSpellReady("Curse of Tongues")) {
					//Curse of Tongues (PvP -> Caster)
					Functions::CastSpellByName("Curse of Tongues");
				}
				else if ((localPlayer->speed == 0) && targetPlayer && !targetFear && (FearTimer == 0) && Functions::IsSpellReady("Fear")) {
					//Fear (PvP)
					Functions::CastSpellByName("Fear");
					if (localPlayer->isCasting()) current_time = time(0);
				}
				else if ((localPlayer->speed == 0) && targetPlayer && (targetUnit->level >= localPlayer->level-10) && Functions::IsSpellReady("Inferno")) {
					//Inferno (PvP)
					if (Functions::HasPetUI()) Functions::LuaCall("PetDismiss()");
					Functions::CastSpellByName("Inferno");
					Functions::ClickAOE(targetUnit->position);
				}
				else if ((localPlayer->speed == 0) && (cluster_unit >= 6) && Functions::IsSpellReady("Inferno")) {
					//Inferno (AoE)
					if (Functions::HasPetUI()) Functions::LuaCall("PetDismiss()");
					Functions::CastSpellByName("Inferno");
					Functions::ClickAOE(cluster_center);
				}
				else if ((localPlayer->speed == 0) && (localPlayer->prctHP > 66.0f) && (nbrCloseEnemy >= 4) && Functions::IsSpellReady("Hellfire")) {
					//Hellfire
					Functions::CastSpellByName("Hellfire");
				}
				else if ((localPlayer->speed == 0) && (cluster_unit >= 4) && Functions::IsSpellReady("Rain of Fire")) {
					//Rain of Fire
					Functions::CastSpellByName("Rain of Fire");
					Functions::ClickAOE(cluster_center);
				}
				else if (!CoShadowDebuff && bossFight && Functions::UnitIsElite("target") && Functions::IsSpellReady("Curse of Shadow")) {
					//Curse of Shadow (Boss)
					Functions::CastSpellByName("Curse of Shadow");
				}
				else if ((localPlayer->speed == 0) && (localPlayer->prctHP < 40.0f) && Functions::IsSpellReady("Drain Life")) {
					//Drain Life
					Functions::CastSpellByName("Drain Life");
				}
				else if (!CoAgonyDebuff && !CoTonguesDebuff && targetPlayer && Functions::IsSpellReady("Curse of Agony")) {
					//Curse of Agony (PvP)
					Functions::CastSpellByName("Curse of Agony");
				}
				else if (!CorruptionDebuff && targetPlayer && Functions::IsSpellReady("Corruption")) {
					//Corruption (PvP)
					Functions::CastSpellByName("Corruption");
				}
				else if (!SiphonLifeDebuff && targetPlayer && Functions::IsSpellReady("Siphon Life")) {
					//Siphon Life (PvP)
					Functions::CastSpellByName("Siphon Life");
				}
				else if ((localPlayer->speed == 0) && !ImmolateDebuff && targetPlayer && Functions::IsSpellReady("Immolate")) {
					//Immolate (PvP)
					Functions::CastSpellByName("Immolate");
				}
				else if ((localPlayer->speed == 0) && (nbrSoulShard < 6) && (targetUnit->prctHP < 15.0f) && Functions::IsSpellReady("Drain Soul")) {
					//Drain Soul
					Functions::CastSpellByName("Drain Soul");
				}
				else if ((localPlayer->speed == 0) && (targetUnit->prctMana > 33.0f) && targetPlayer && Functions::IsSpellReady("Drain Mana")) {
					//Drain Mana (PvP)
					Functions::CastSpellByName("Drain Mana");
				}
				else if (IsFacing && (localPlayer->speed == 0) && Functions::IsSpellReady("Shadow Bolt")) {
					//Shadow Bolt
					Functions::CastSpellByName("Shadow Bolt");
				}
				else if ((localPlayer->speed == 0) && (localPlayer->prctHP > 40.0f) && (localPlayer->prctMana < 10.0f) && Functions::IsSpellReady("Life Tap")) {
					//Life Tap
					Functions::CastSpellByName("Life Tap");
				}
				else if (IsFacing && (localPlayer->speed == 0) && Functions::HasWandEquipped() && !Functions::IsAutoRepeatAction(Functions::GetSlot("Shoot"))) {
					//Wand
					Functions::CastSpellByName("Shoot");
				}
			}
		});
	}
}