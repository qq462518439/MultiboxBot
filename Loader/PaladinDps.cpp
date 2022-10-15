#include "ListAI.h"

#include <iostream>

static int LastTarget = 0;
static time_t current_time = time(0);
static float HealInnerTimer = 0;

static void PaladinAttack() {

	if (IsInGroup && (tankIndex > 0) && (GroupMembersIndex[tankIndex] > -1) && ListUnits[GroupMembersIndex[tankIndex]].targetGuid != 0)
		localPlayer->SetTarget(ListUnits[GroupMembersIndex[tankIndex]].targetGuid);
	else if (Combat && (targetUnit == NULL || targetUnit->unitReaction > Neutral) && (HasAggro[0].size() > 0)) {
		if (listIndexCloseEnemies.size() > 0) localPlayer->SetTarget(ListUnits[listIndexCloseEnemies[0]].Guid);
	}

	if (targetUnit != NULL && (targetUnit->unitReaction <= Neutral) && !targetUnit->isdead) {
		bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
		bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
		int SotCIDs[6] = { 21082, 20162, 20305, 20306, 20307, 20308 };
		bool SotCBuff = localPlayer->hasBuff(SotCIDs, 6);
		int SoRIDs[9] = { 20154, 21084, 20287, 20288, 20289, 20290, 20291, 20292, 20293 };
		bool SoRBuff = localPlayer->hasBuff(SoRIDs, 9);
		int SoCIDs[5] = { 20375, 20915, 20918, 20919, 20920 };
		bool SoCBuff = localPlayer->hasBuff(SoCIDs, 5);
		int SotCDebuffIDs[6] = { 21183, 20188, 20300, 20301, 20302, 20303 };
		bool SotCDebuff = targetUnit->hasDebuff(SotCDebuffIDs, 6);
		bool SealBuff = (SoRBuff || SotCBuff || SoCBuff);
		if (!Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
		if (!SealBuff && !SotCDebuff && Functions::UnitIsElite("target") && Functions::IsSpellReady("Seal of the Crusader")) {
			//Seal of the Crusader
			Functions::CastSpellByName("Seal of the Crusader");
		}
		else if (!SealBuff && !Functions::IsPlayerSpell("Seal of Command") && Functions::IsSpellReady("Seal of Righteousness")) {
			//Seal of Righteousness
			Functions::CastSpellByName("Seal of Righteousness");
		}
		else if (!SealBuff && Functions::IsSpellReady("Seal of Command")) {
			//Seal of Command
			Functions::CastSpellByName("Seal of Command");
		}
		else if ((localPlayer->prctMana > 33) && (nbrCloseEnemy >= 4) && Functions::IsSpellReady("Consecration")) {
			//Consecration
			Functions::CastSpellByName("Consecration");
		}
		else if (SealBuff && (distTarget < 10) && Functions::IsSpellReady("Judgement")) {
			//Judgement
			Functions::CastSpellByName("Judgement");
		}
		else if (!targetStunned && !targetConfused && (distTarget < 10) && Functions::IsSpellReady("Hammer of Justice")) {
			//Hammer of Justice
			Functions::CastSpellByName("Hammer of Justice");
		}
		else if ((Functions::getNbrCreatureType(20, Undead, Demon) >= 4) && Functions::IsSpellReady("Holy Wrath")) {
			//Holy Wrath
			Functions::CastSpellByName("Holy Wrath");
		}
		else if ((localPlayer->prctMana > 33) && (distTarget < 30) && (targetUnit->creatureType == Undead) || (targetUnit->creatureType == Demon) && Functions::IsSpellReady("Exorcism")) {
			//Exorcism
			Functions::CastSpellByName("Exorcism");
		}
		else if ((distTarget < 30) && (targetUnit->prctHP < 20) && (localPlayer->prctMana > 33) && Functions::IsSpellReady("Hammer of Wrath")) {
			//Hammer of Wrath
			Functions::CastSpellByName("Hammer of Wrath");
		}
	}
	else if (Moving == 0 && !Combat && !IsSitting && IsInGroup) {
		Functions::FollowMultibox("Nihal");
	}
}

static int HealGroup(int indexP) { //Heal Players and Npcs
	float HpRatio = ListUnits[indexP].prctHP;
	unsigned long long healGuid = ListUnits[indexP].Guid;
	bool isPlayer = (healGuid == Functions::GetPlayerGuid());
	int ForbearanceID[1] = { 25771 };
	bool ForbearanceDebuff = ListUnits[indexP].hasDebuff(ForbearanceID, 1);
	int BoSIDs[2] = { 6940, 20729 };
	bool BoSacrificeBuff = ListUnits[indexP].hasBuff(BoSIDs, 2);
	bool isParty = false;
	if (!isPlayer) {
		for (int i = 1; i < NumGroupMembers; i++) {
			if ((GroupMembersIndex[i] > -1) && ListUnits[GroupMembersIndex[i]].Guid == ListUnits[indexP].Guid) isParty = true;
		}
	}
	if (Combat && (HpRatio < 20) && Functions::IsSpellReady("Lay on Hands")) {
		//Lay on Hands
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Lay on Hands");
		return 0;
	}
	else if (Combat && isPlayer && (HpRatio < 25) && !ForbearanceDebuff && Functions::IsSpellReady("Divine Protection")) {
		//Divine Protection / Divine Shield
		Functions::CastSpellByName("Divine Protection"); Functions::CastSpellByName("Divine Shield");
		return 0;
	}
	else if (Combat && isPlayer && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
		//Healthstone
		Functions::PlaceItem(120, "Healthstone");
		Functions::UseAction(120);
		return 0;
	}
	else if (Combat && isPlayer && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
		//Healing Potion
		Functions::PlaceItem(120, "Healing Potion");
		Functions::UseAction(120);
		return 0;
	}
	else if (Combat && isParty && (HpRatio < 20) && !ForbearanceDebuff && Functions::IsSpellReady("Blessing of Protection")) {
		//Blessing of Protection
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Blessing of Protection");
		return 0;
	}
	else if (Combat && isParty && (HpRatio < 50) && !BoSacrificeBuff && Functions::IsSpellReady("Blessing of Sacrifice")) {
		//Blessing of Sacrifice
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Blessing of Sacrifice");
		return 0;
	}
	else if ((HpRatio < 50) && (localPlayer->prctMana > 33) && (HealInnerTimer == 0) && (localPlayer->speed == 0) && Functions::IsSpellReady("Holy Light")) {
		//Holy Light
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Holy Light");
		LastTarget = indexP;
		if (localPlayer->isCasting()) current_time = time(0);
		return 0;
	}
	else if ((HpRatio < 85) && (localPlayer->prctMana > 33) && (HealInnerTimer == 0) && (localPlayer->speed == 0) && Functions::IsSpellReady("Flash of Light")) {
		//Flash of Light
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Flash of Light");
		LastTarget = indexP;
		if (localPlayer->isCasting()) current_time = time(0);
		return 0;
	}
	return 1;
}

void ListAI::PaladinDps() {
	HealInnerTimer = 5.0 - (time(0) - current_time);
	if (HealInnerTimer < 0) HealInnerTimer = 0;
	int FoLIDs[6] = { 19750, 19939, 19940, 19941, 19942, 19943 };
	int holyLightIDs[9] = { 635, 639, 647, 1026, 1042, 3472, 10328, 10329, 25292 };
	if ((localPlayer->isCasting(holyLightIDs, 9) && (ListUnits[LastTarget].prctHP > 80)) || (localPlayer->isCasting(FoLIDs, 6) && (ListUnits[LastTarget].prctHP > 95))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		int BoWisdomIDs[6] = { 19742, 19850, 19852, 19853, 19854, 25290 };
		bool BoWisdomBuff = localPlayer->hasBuff(BoWisdomIDs, 6);
		int SanctityAuraIDs[1] = { 20218 };
		bool SanctityAuraBuff = localPlayer->hasBuff(SanctityAuraIDs, 1);
		int BoWisdomKey = Functions::GetBuffKey(BoWisdomIDs, 6);
		ThreadSynchronizer::RunOnMainThread([=]() {
			int PurifyDispelKey = Functions::GetDispelKey("Disease", "Poison");
			int CleanseDispelKey = Functions::GetDispelKey("Disease", "Poison", "Magic");
			if (!SanctityAuraBuff && Functions::IsPlayerSpell("Sanctity Aura")) {
				//Sanctity Aura
				Functions::CastSpellByName("Sanctity Aura");
			}
			else if (!Combat && (localPlayer->speed == 0) && Functions::IsSpellReady("Redemption") && (Functions::GetGroupDead(1) > 0)) {
				//Redemption
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[Functions::GetGroupDead()]].Guid);
				Functions::CastSpellByName("Redemption");
			}
			else if (!BoWisdomBuff && Functions::IsSpellReady("Blessing of Wisdom")) {
				//Blessing of Wisdom (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Blessing of Wisdom");
			}
			else if ((BoWisdomKey > 0) && (GroupMembersIndex[BoWisdomKey] > -1) && Functions::IsSpellReady("Blessing of Wisdom")) {
				//Blessing of Wisdom (Groupe)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[BoWisdomKey]].Guid);
				Functions::CastSpellByName("Blessing of Wisdom");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::PlaceItem(120, "Mana Potion");
				Functions::UseAction(120);
			}
			else if ((localPlayer->prctMana > 25) && Functions::GetUnitDispel("player", "Disease", "Poison") && Functions::IsSpellReady("Purify")) {
				//Purify (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Purify");
			}
			else if ((PurifyDispelKey > 0) && (GroupMembersIndex[PurifyDispelKey] > -1) && (localPlayer->prctMana > 25) && Functions::IsSpellReady("Purify")) {
				//Purify (Groupe)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[PurifyDispelKey]].Guid);
				Functions::CastSpellByName("Purify");
			}
			else if ((localPlayer->prctMana > 25) && Functions::GetUnitDispel("player", "Disease", "Poison", "Magic") && Functions::IsSpellReady("Cleanse")) {
				//Cleanse (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Cleanse");
			}
			else if ((CleanseDispelKey > 0) && (GroupMembersIndex[CleanseDispelKey] > -1) && (localPlayer->prctMana > 25) && Functions::IsSpellReady("Cleanse")) {
				//Cleanse (Groupe)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[CleanseDispelKey]].Guid);
				Functions::CastSpellByName("Cleanse");
			}
			else {
				int tmp = 1; unsigned int index = 0;
				while (tmp == 1 and index < HealTargetArray.size()) {
					tmp = HealGroup(HealTargetArray[index]);
					index = index + 1;
				}
				if (tmp == 1) PaladinAttack();
			}
			});
	}
}