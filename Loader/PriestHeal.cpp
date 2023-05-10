#include "ListAI.h"

#include <iostream>

	//Local variables //
static int LesserHealRank = 0; static float LesserHealValue[3]; static int LesserHealLevel[3] = {1, 4, 10};
static int RenewRank = 0; static float RenewValue[10]; static int RenewLevel[10] = { 8, 14, 20, 26, 32, 38, 44, 50, 56, 60 };
static int HealRank = 0; static float HealValue[4]; static int HealLevel[4] = { 16, 22, 28, 34 };
static int GreaterHealRank = 0; static float GreaterHealValue[5]; static int GreaterHealLevel[5] = { 40, 46, 52, 58, 60 };
	//================//

static void GetSpellBonusHealing() {
	float tmp[3] = { 53, 84, 154 }; for (int i = 0; i < 3; i++) { LesserHealValue[i] = tmp[i]; }
	float tmp2[10] = { 45, 100, 175, 245, 315, 400, 510, 650, 810, 970 }; for (int i = 0; i < 10; i++) { RenewValue[i] = tmp2[i]; }
	float tmp3[4] = { 330, 476, 624, 781 }; for (int i = 0; i < 4; i++) { HealValue[i] = tmp3[i]; }
	float tmp4[5] = { 982, 1248, 1556, 1917, 2080 }; for (int i = 0; i < 5; i++) { GreaterHealValue[i] = tmp4[i]; }
	int RenewTalentRank = Functions::GetTalentInfo(2, 2);
	int SpiritualHealingRank = Functions::GetTalentInfo(2, 15);
	int spirit = Functions::UnitStat("player", 5);
	int SpiritualGuidance = Functions::GetTalentInfo(2, 14);
	float bonusHealing = spirit * 0.05f * SpiritualGuidance;
	std::tie(std::ignore, LesserHealRank) = Functions::GetSpellID("Lesser Heal");
	std::tie(std::ignore, RenewRank) = Functions::GetSpellID("Renew");
	std::tie(std::ignore, HealRank) = Functions::GetSpellID("Heal");
	std::tie(std::ignore, GreaterHealRank) = Functions::GetSpellID("Greater Heal");
	//====================================================//
	float SubLevel20PENALTY = 1.0f;
	if (RenewLevel[RenewRank] < 20.0f) SubLevel20PENALTY = 1.0f - (20.0f - RenewLevel[RenewRank]) * 0.0375f;
	RenewValue[RenewRank] = (RenewValue[RenewRank] + bonusHealing * SubLevel20PENALTY) * (1.0f + (0.05f * RenewTalentRank)) * (1.0f + (0.02f * SpiritualHealingRank));
	SubLevel20PENALTY = 1.0f - (20.0f - LesserHealLevel[LesserHealRank]) * 0.0375f;
	LesserHealValue[LesserHealRank] = (LesserHealValue[LesserHealRank] + bonusHealing * (2.5f / 3.5f) * SubLevel20PENALTY) * (1.0f + (0.02f * SpiritualHealingRank));
	SubLevel20PENALTY = 1.0f;
	if (HealLevel[HealRank] < 20.0f) SubLevel20PENALTY = 1.0f - (20.0f - HealLevel[HealRank]) * 0.0375f;
	HealValue[HealRank] = (HealValue[HealRank] + bonusHealing * (3.0f / 3.5f) * SubLevel20PENALTY) * (1.0f + (0.02f * SpiritualHealingRank));
	GreaterHealValue[GreaterHealRank] = (GreaterHealValue[GreaterHealRank] + bonusHealing * (3.0f / 3.5f)) * (1.0f + (0.02f * SpiritualHealingRank));
}

static void PriestAttack() {
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
	else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
		bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
		int ShadowWordPainIDs[8] = { 589, 594, 970, 992, 2767, 10892, 10893, 10894 };
		bool ShadowWordPainDebuff = targetUnit->hasDebuff(ShadowWordPainIDs, 8);
		int HolyFireIDs[8] = { 14914, 15262, 15263, 15264, 15265, 15266, 15267, 15261 };
		bool HolyFireDebuff = targetUnit->hasDebuff(HolyFireIDs, 8);
		if (!Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
		if ((nbrCloseEnemy >= 4) && Functions::IsSpellReady("Holy Nova")) {
			//Holy Nova
			Functions::CastSpellByName("Holy Nova");
		}
		else if (IsFacing && (localPlayer->prctMana > 66) && targetPlayer && !ShadowWordPainDebuff && Functions::IsSpellReady("Shadow Word: Pain")) {
			//Shadow Word: Pain	(PvP)
			Functions::CastSpellByName("Shadow Word: Pain");
		}
		else if (IsFacing && (localPlayer->speed == 0) && (localPlayer->prctMana > 66) && targetPlayer && !HolyFireDebuff && Functions::IsSpellReady("Holy Fire")) {
			//Holy Fire (PvP)
			Functions::CastSpellByName("Holy Fire");
		}
		else if (IsFacing && (localPlayer->speed == 0) && (localPlayer->prctMana > 66) && Functions::IsSpellReady("Mind Blast")) {
			//Mind Blast
			Functions::CastSpellByName("Mind Blast");
		}
		else if (IsFacing && (localPlayer->speed == 0) && (localPlayer->prctMana > 66) && Functions::IsSpellReady("Smite")) {
			//Smite
			Functions::CastSpellByName("Smite");
		}
		else if (IsFacing && (localPlayer->speed == 0) && Functions::HasWandEquipped() && !Functions::IsAutoRepeatAction(Functions::GetSlot("Shoot"))) {
			//Wand
			Functions::CastSpellByName("Shoot");
		}
	}
}

static int HealGroup(int indexP) { //Heal Players and Npcs
	float HpRatio = ListUnits[indexP].prctHP;
	int HpLost = ListUnits[indexP].hpLost;
	unsigned long long healGuid = ListUnits[indexP].Guid;
	bool isPlayer = (healGuid == localPlayer->Guid);
	int RenewIDs[10] = { 139, 6074, 6075, 6076, 6077, 6078, 10927, 10928, 10929, 25315 };
	bool RenewBuff = ListUnits[indexP].hasBuff(RenewIDs, 10);
	int PWShieldIDs[10] = { 17, 592, 600, 3747, 6065, 6066, 10898, 10899, 10900, 10901 };
	bool PWShieldBuff = ListUnits[indexP].hasBuff(PWShieldIDs, 10);
	int WeakenedSoulID[1] = { 6788 }; bool WeakenedSoulDebuff = ListUnits[indexP].hasDebuff(WeakenedSoulID, 1);
	bool isParty = false;
	if (!isPlayer) {
		for (int i = 1; i < NumGroupMembers; i++) {
			if ((GroupMembersIndex[i] > -1) && ListUnits[GroupMembersIndex[i]].Guid == ListUnits[indexP].Guid) isParty = true;
		}
	}
	float distAlly = localPlayer->position.DistanceTo(ListUnits[indexP].position);
	if (isPlayer && Combat && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
		//Healthstone
		Functions::UseItem("Healthstone");
		return 0;
	}
	else if (isPlayer && Combat && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
		//Healing Potion
		Functions::UseItem("Healing Potion");
		return 0;
	}
	else if (isPlayer && Combat && (localPlayer->prctHP < 35) && Functions::IsSpellReady("Desperate Prayer")) {
		//Desperate Prayer
		Functions::CastSpellByName("Desperate Prayer");
		return 0;
	}
	else if ((AoEHeal >= 3) && (distAlly < 40.0f) && (localPlayer->speed == 0) && Functions::IsSpellReady("Prayer of Healing")) {
		//Prayer of Healing
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Prayer of Healing");
		LastTarget = indexP;
		return 0;
	}
	else if ((HpRatio < 30) && !PWShieldBuff && !WeakenedSoulDebuff && (distAlly < 40.0f) && Functions::IsSpellReady("Power Word: Shield")) {
		//Power Word: Shield
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Power Word: Shield");
		LastTarget = indexP;
		return 0;
	}
	else if ((HpRatio < 30) && (distAlly < 40.0f) && (localPlayer->speed == 0) && Functions::IsSpellReady("Flash Heal")) {
		//Flash Heal
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Flash Heal");
		LastTarget = indexP;
		return 0;
	}
	else if ((HpLost > GreaterHealValue[GreaterHealRank]) && (distAlly < 40.0f) && (localPlayer->speed == 0) && Functions::IsSpellReady("Greater Heal")) {
		//Greater Heal
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Greater Heal");
		LastTarget = indexP;
		return 0;
	}
	else if ((HpLost > HealValue[HealRank]) && (distAlly < 40.0f) && (localPlayer->speed == 0) && Functions::IsSpellReady("Heal")) {
		//Heal
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Heal");
		LastTarget = indexP;
		return 0;
	}
	else if ((localPlayer->level < 40) && (HpLost > LesserHealValue[LesserHealRank]) && (distAlly < 40.0f) && (localPlayer->speed == 0) && Functions::IsSpellReady("Lesser Heal")) {
		//Lesser Heal
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Lesser Heal");
		LastTarget = indexP;
		return 0;
	}
	else if ((HpLost > RenewValue[RenewRank]) && !RenewBuff && (distAlly < 40.0f) && Functions::IsSpellReady("Renew")) {
		//Renew
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Renew");
		LastTarget = indexP;
		return 0;
	}
	return 1;
}

void ListAI::PriestHeal() {
	int LesserHealIDs[3] = { 2050, 2052, 2053 }; int FlashHealIDs[7] = { 2061, 9472, 9473, 9474, 10915, 10916, 10917 };
	int GreaterHealIDs[5] = { 2060, 10963, 10964, 10965, 25314 }; int HealIDs[4] = { 2054, 2055, 6093, 6064 };
	if ((localPlayer->isCasting(LesserHealIDs, 3) && (ListUnits[LastTarget].hpLost < LesserHealValue[LesserHealRank] * 0.9))
		|| (localPlayer->isCasting(HealIDs, 4) && (ListUnits[LastTarget].hpLost < HealValue[HealRank] * 0.9))
		|| (localPlayer->isCasting(GreaterHealIDs, 5) && (ListUnits[LastTarget].hpLost < GreaterHealValue[GreaterHealRank] * 0.9))
		|| (localPlayer->isCasting(FlashHealIDs, 7) && (ListUnits[LastTarget].prctHP > 80))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			float SpellCalculTimer = 30.0f - (time(0) - current_time);
			if (SpellCalculTimer <= 0) {
				GetSpellBonusHealing();
				current_time = time(0);
			}
			int nbrAggro = HasAggro[0].size();
			int PWFortitudeIDs[8] = { 1243, 1244, 1245, 2791, 10937, 10938, 21562, 21564 }; //Include PoFortitude
			bool PWFortitudeBuff = localPlayer->hasBuff(PWFortitudeIDs, 8);
			int PWFortitudeKey = Functions::GetBuffKey(PWFortitudeIDs, 8);
			int DivineSpiritIDs[5] = { 14752, 14818, 14819, 27841, 27681 }; //Include PoSpirit
			bool DivineSpiritBuff = localPlayer->hasBuff(DivineSpiritIDs, 5);
			int DivineSpiritKey = Functions::GetBuffKey(DivineSpiritIDs, 5);
			int InnerFireIDs[6] = { 588, 7128, 602, 1006, 10951, 10952 };
			bool InnerFireBuff = localPlayer->hasBuff(InnerFireIDs, 6);
			int DispelMagicKey = Functions::GetDispelKey("Magic");
			int CureDiseaseKey = Functions::GetDispelKey("Disease");
			if (!Combat && (localPlayer->speed == 0) && Functions::IsSpellReady("Resurrection") && (Functions::GetGroupDead(0) > 0)) {
				//Resurrection
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[Functions::GetGroupDead()]].Guid);
				Functions::CastSpellByName("Resurrection");
			}
			else if (!Combat && (PWFortitudeKey > 0) && (GroupMembersIndex[PWFortitudeKey] > -1) && Functions::IsSpellReady("Prayer of Fortitude")) {
				//Prayer of Fortitude (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[PWFortitudeKey]].Guid);
				Functions::CastSpellByName("Prayer of Fortitude");
			}
			else if (!Combat && (DivineSpiritKey > 0) && (GroupMembersIndex[DivineSpiritKey] > -1) && Functions::IsSpellReady("Prayer of Spirit")) {
				//Prayer of Spirit (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[DivineSpiritKey]].Guid);
				Functions::CastSpellByName("Prayer of Spirit");
			}
			else if (!InnerFireBuff && Functions::IsPlayerSpell("Inner Fire")) {
				//Inner Fire (self)
				Functions::CastSpellByName("Inner Fire");
			}
			else if (!Combat && !PWFortitudeBuff && Functions::IsPlayerSpell("Power Word: Fortitude")) {
				//Power Word: Fortitude (self)
				Functions::CastSpellByName("Power Word: Fortitude");
			}
			else if (!Combat && (PWFortitudeKey > 0) && (GroupMembersIndex[PWFortitudeKey] > -1) && Functions::IsSpellReady("Power Word: Fortitude")) {
				//Power Word: Fortitude (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[PWFortitudeKey]].Guid);
				Functions::CastSpellByName("Power Word: Fortitude");
			}
			else if (!Combat && !DivineSpiritBuff && Functions::IsPlayerSpell("Divine Spirit")) {
				//Divine Spirit (self)
				Functions::CastSpellByName("Divine Spirit");
			}
			else if (!Combat && (DivineSpiritKey > 0) && (GroupMembersIndex[DivineSpiritKey] > -1) && Functions::IsSpellReady("Divine Spirit")) {
				//Divine Spirit (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[DivineSpiritKey]].Guid);
				Functions::CastSpellByName("Divine Spirit");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::UseItem("Mana Potion");
			}
			else if ((nbrAggro > 0) && Functions::IsSpellReady("Fade")) {
				//Fade (Aggro)
				Functions::CastSpellByName("Fade");
			}
			else if ((nbrCloseEnemy >= 4) && Functions::IsSpellReady("Psychic Scream")) {
				//Psychic Scream
				Functions::CastSpellByName("Psychic Scream");
			}
			else if ((localPlayer->prctMana > 25) && Functions::GetUnitDispel("player", "Disease") && Functions::IsSpellReady("Cure Disease")) {
				//Cure Disease (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Cure Disease");
			}
			else if ((CureDiseaseKey > 0) && (GroupMembersIndex[CureDiseaseKey] > -1) && (localPlayer->prctMana > 25) && Functions::IsSpellReady("Cure Disease")) {
				//Cure Disease (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[CureDiseaseKey]].Guid);
				Functions::CastSpellByName("Cure Disease");
			}
			else if ((localPlayer->prctMana > 25) && Functions::GetUnitDispel("player", "Magic") && Functions::IsSpellReady("Dispel Magic")) {
				//Dispel Magic (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Dispel Magic");
			}
			else if ((DispelMagicKey > 0) && (GroupMembersIndex[DispelMagicKey] > -1) && (localPlayer->prctMana > 25) && Functions::IsSpellReady("Dispel Magic")) {
				//Dispel Magic (Groupe)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[DispelMagicKey]].Guid);
				Functions::CastSpellByName("Dispel Magic");
			}
			else {
				int tmp = 1; unsigned int index = 0;
				while (tmp == 1 and index < HealTargetArray.size()) {
					tmp = HealGroup(HealTargetArray[index]);
					index = index + 1;
				}
				if (tmp == 1) PriestAttack();
			}
		});
	}
}