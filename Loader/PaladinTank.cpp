#include "ListAI.h"

#include <iostream>

static int LastTarget = 0;

static void PaladinAttack() {
	if (tankAutoFocus && (targetUnit == NULL || targetUnit->isdead || !targetUnit->attackable)) {
		for (int i = NumGroupMembers; i >= 0; i--) {
			if (HasAggro[i].size() > 0) {
				localPlayer->SetTarget(HasAggro[i][0]);
				break;
			}
		}
	}
	else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
		bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
		bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
		int SoRIDs[9] = { 20154, 21084, 20287, 20288, 20289, 20290, 20291, 20292, 20293 };
		bool SoRBuff = localPlayer->hasBuff(SoRIDs, 9);
		int SoWIDs[3] = { 20166, 20356, 20357 };
		bool SoWBuff = localPlayer->hasBuff(SoWIDs, 3);
		bool SealBuff = (SoRBuff || SoWBuff);
		int SoWDebuffIDs[3] = { 20186, 20354, 20355 };
		bool SoWDebuff = targetUnit->hasDebuff(SoWDebuffIDs, 3);
		if (!Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
		if ((localPlayer->prctMana < 33) && !SealBuff && Functions::IsSpellReady("Seal of Wisdom")) {
			//Seal of Wisdom
			Functions::CastSpellByName("Seal of Wisdom");
		}
		else if (!SealBuff && Functions::IsSpellReady("Seal of Righteousness")) {
			//Seal of Righteousness
			Functions::CastSpellByName("Seal of Righteousness");
		}
		else if ((nbrCloseEnemy >= 2) && Functions::IsSpellReady("Consecration")) {
			//Consecration
			Functions::CastSpellByName("Consecration");
		}
		else if (SealBuff && (distTarget < 10) && ((localPlayer->prctMana > 20) || (SoWBuff && !SoWDebuff && Functions::UnitIsElite("target"))) && Functions::IsSpellReady("Judgement")) {
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
	int nbrAggro = HasAggro[0].size();
	if (!isPlayer) {
		for (int i = 1; i < NumGroupMembers; i++) {
			if ((GroupMembersIndex[i] > -1) && ListUnits[GroupMembersIndex[i]].Guid == ListUnits[indexP].Guid) isParty = true;
		}
	}
	float distAlly = localPlayer->position.DistanceTo(ListUnits[indexP].position);
	if (Combat && (distAlly < 40.0f) && (HpRatio < 20) && Functions::IsSpellReady("Lay on Hands")) {
		//Lay on Hands
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Lay on Hands");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if (Combat && (localPlayer->prctHP < 25) && !ForbearanceDebuff && Functions::IsSpellReady("Divine Protection")) {
		//Divine Protection / Divine Shield
		Functions::CastSpellByName("Divine Protection"); Functions::CastSpellByName("Divine Shield");
		return 0;
	}
	else if (Combat && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
		//Healthstone
		Functions::UseItem("Healthstone");
		return 0;
	}
	else if (Combat && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
		//Healing Potion
		Functions::UseItem("Healing Potion");
		return 0;
	}
	else if (Combat && isParty && (distAlly < 30.0f) && (HpRatio < 20) && !ForbearanceDebuff && Functions::IsSpellReady("Blessing of Protection")) {
		//Blessing of Protection
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Blessing of Protection");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if (Combat && isParty && (distAlly < 30.0f) && (HpRatio < 50) && !BoSacrificeBuff && Functions::IsSpellReady("Blessing of Sacrifice")) {
		//Blessing of Sacrifice
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Blessing of Sacrifice");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if (!Combat && (distAlly < 40.0f) && (HpRatio < 80) && (localPlayer->speed == 0) && Functions::IsSpellReady("Flash of Light")) {
		//Flash of Light
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Flash of Light");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if (!Combat && (distAlly < 40.0f) && (HpRatio < 50) && (localPlayer->speed == 0) && Functions::IsSpellReady("Holy Light")) {
		//Holy Light
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Holy Light");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	return 1;
}

void ListAI::PaladinTank() {
	int FoLIDs[6] = { 19750, 19939, 19940, 19941, 19942, 19943 };
	int holyLightIDs[9] = { 635, 639, 647, 1026, 1042, 3472, 10328, 10329, 25292 };
	if ((localPlayer->isCasting(holyLightIDs, 9) && (ListUnits[LastTarget].prctHP > 80)) || (localPlayer->isCasting(FoLIDs, 6) && (ListUnits[LastTarget].prctHP > 95))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			int BoSanctuaryIDs[4] = { 20911, 20912, 20913, 20914 };
			bool BoSanctuaryBuff = localPlayer->hasBuff(BoSanctuaryIDs, 4);
			int RetributionAuraIDs[5] = { 7294, 10298, 10299, 10300, 10301 };
			bool RetributionAuraBuff = localPlayer->hasBuff(RetributionAuraIDs, 5);
			int RighteousFuryIDs[1] = { 25780 };
			bool RighteousFuryBuff = localPlayer->hasBuff(RighteousFuryIDs, 1);
			int BoSalvationIDs[1] = { 1038 };
			int BoSalvationKey = Functions::GetBuffKey(BoSalvationIDs, 1);
			int PurifyDispelKey = Functions::GetDispelKey("Disease", "Poison");
			int CleanseDispelKey = Functions::GetDispelKey("Disease", "Poison", "Magic");
			if (!RetributionAuraBuff && Functions::IsPlayerSpell("Retribution Aura")) {
				//Retribution Aura
				Functions::CastSpellByName("Retribution Aura");
			}
			else if (!RighteousFuryBuff && Functions::IsSpellReady("Righteous Fury")) {
				//Righteous Fury
				Functions::CastSpellByName("Righteous Fury");
			}
			else if (!Combat && (localPlayer->speed == 0) && Functions::IsSpellReady("Redemption") && (Functions::GetGroupDead(1) > 0)) {
				//Redemption
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[Functions::GetGroupDead(1)]].Guid);
				Functions::CastSpellByName("Redemption");
			}
			else if (!BoSanctuaryBuff && Functions::IsSpellReady("Blessing of Sanctuary")) {
				//Blessing of Sanctuary (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Blessing of Sanctuary");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if ((BoSalvationKey > 0) && (GroupMembersIndex[BoSalvationKey] > -1) && Functions::IsSpellReady("Blessing of Salvation")) {
				//Blessing of Salvation (Groupe)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[BoSalvationKey]].Guid);
				Functions::CastSpellByName("Blessing of Salvation");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::UseItem("Mana Potion");
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