#include "ListAI.h"

#include <iostream>

static void PaladinAttack() {
	if ((Leader == NULL) || (Leader->Guid != localPlayer->Guid) || tankAutoFocus) {
		bool targetFocusingTank = false;
		if (targetUnit != NULL) {
			for (int i = 1; i <= NumGroupMembers; i++) {
				if (GroupMember[i] != NULL && targetUnit->targetGuid == GroupMember[i]->Guid && GroupMember[i]->role == 0) {
					targetFocusingTank = true;
					break;
				}
			}
		}
		if (targetUnit == NULL || targetUnit->isdead || !targetUnit->attackable || targetFocusingTank) {
			bool foundTarget = false;
			for (int i = NumGroupMembers; i >= 0; i--) {
				if (GroupMember[i] != NULL && GroupMember[i]->role != 0 && HasAggro[i].size() > 0) {
					localPlayer->SetTarget(HasAggro[i][0]->Guid);
					foundTarget = true;
					break;
				}
			}
			if ((targetUnit == NULL || targetUnit->isdead || !targetUnit->attackable) && !foundTarget) {
				for (int i = NumGroupMembers; i >= 0; i--) { //Tank also
					if (HasAggro[i].size() > 0) {
						localPlayer->SetTarget(HasAggro[i][0]->Guid);
						break;
					}
				}
			}
		}
	}
	if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
		bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
		bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
		int SoRIDs[9] = { 20154, 21084, 20287, 20288, 20289, 20290, 20291, 20292, 20293 };
		bool SoRBuff = localPlayer->hasBuff(SoRIDs, 9);
		int SoWIDs[3] = { 20166, 20356, 20357 };
		bool SoWBuff = localPlayer->hasBuff(SoWIDs, 3);
		bool SealBuff = (SoRBuff || SoWBuff);
		int SoWDebuffIDs[3] = { 20186, 20354, 20355 };
		bool SoWDebuff = targetUnit->hasDebuff(SoWDebuffIDs, 3);
		if (!FunctionsLua::IsCurrentAction(FunctionsLua::GetSlot("Attack"))) FunctionsLua::CastSpellByName("Attack");
		if ((localPlayer->prctMana < 33) && !SealBuff && FunctionsLua::IsSpellReady("Seal of Wisdom")) {
			//Seal of Wisdom
			FunctionsLua::CastSpellByName("Seal of Wisdom");
		}
		else if (!SealBuff && FunctionsLua::IsSpellReady("Seal of Righteousness")) {
			//Seal of Righteousness
			FunctionsLua::CastSpellByName("Seal of Righteousness");
		}
		else if ((Functions::getNbrCreatureType(20, Undead, Demon) >= 4) && FunctionsLua::IsSpellReady("Holy Wrath")) {
			//Holy Wrath
			FunctionsLua::CastSpellByName("Holy Wrath");
		}
		else if ((nbrCloseEnemy >= 2) && FunctionsLua::IsSpellReady("Consecration")) {
			//Consecration
			FunctionsLua::CastSpellByName("Consecration");
		}
		else if (SealBuff && (distTarget < 10) && ((localPlayer->prctMana > 20) || (SoWBuff && !SoWDebuff && FunctionsLua::UnitIsElite("target"))) && FunctionsLua::IsSpellReady("Judgement")) {
			//Judgement
			FunctionsLua::CastSpellByName("Judgement");
		}
		else if (!targetStunned && !targetConfused && (distTarget < 10) && FunctionsLua::IsSpellReady("Hammer of Justice")) {
			//Hammer of Justice
			FunctionsLua::CastSpellByName("Hammer of Justice");
		}
		else if (((localPlayer->prctMana > 33) || (nbrCloseEnemy <= 1)) && (distTarget < 30) && ((targetUnit->creatureType == Undead) || (targetUnit->creatureType == Demon)) && FunctionsLua::IsSpellReady("Exorcism")) {
			//Exorcism
			FunctionsLua::CastSpellByName("Exorcism");
		}
		else if ((distTarget < 30) && (targetUnit->prctHP < 20) && (localPlayer->prctMana > 33) && FunctionsLua::IsSpellReady("Hammer of Wrath")) {
			//Hammer of Wrath
			FunctionsLua::CastSpellByName("Hammer of Wrath");
		}
	}
}

static int HealGroup(unsigned int indexP) { //Heal Players and Npcs
	float HpRatio = ListUnits[indexP].prctHP;
	unsigned long long healGuid = ListUnits[indexP].Guid;
	bool isPlayer = (healGuid == localPlayer->Guid);
	int ForbearanceID[1] = { 25771 };
	bool ForbearanceDebuff = ListUnits[indexP].hasDebuff(ForbearanceID, 1);
	int BoSIDs[2] = { 6940, 20729 };
	bool BoSacrificeBuff = ListUnits[indexP].hasBuff(BoSIDs, 2);
	bool isParty = false;
	int nbrAggro = HasAggro[0].size();
	if (!isPlayer) {
		for (int i = 1; i <= NumGroupMembers; i++) {
			if ((GroupMember[i] != NULL) && GroupMember[i]->Guid == ListUnits[indexP].Guid) isParty = true;
		}
	}
	float distAlly = localPlayer->position.DistanceTo(ListUnits[indexP].position);
	if (Combat && isParty && (distAlly < 40.0f) && (HpRatio < 20) && FunctionsLua::IsSpellReady("Lay on Hands")) {
		//Lay on Hands
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Lay on Hands");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if (isPlayer && Combat && (localPlayer->prctHP < 25) && !ForbearanceDebuff && FunctionsLua::IsSpellReady("Divine Protection")) {
		//Divine Protection / Divine Shield
		FunctionsLua::CastSpellByName("Divine Protection"); FunctionsLua::CastSpellByName("Divine Shield");
		return 0;
	}
	else if (isPlayer && Combat && (localPlayer->prctHP < 40) && (FunctionsLua::GetHealthstoneCD() < 1.25)) {
		//Healthstone
		FunctionsLua::UseItem("Healthstone");
		return 0;
	}
	else if (isPlayer && Combat && (localPlayer->prctHP < 35) && (FunctionsLua::GetHPotionCD() < 1.25)) {
		//Healing Potion
		FunctionsLua::UseItem("Healing Potion");
		return 0;
	}
	else if (Combat && isParty && (distAlly < 30.0f) && (HpRatio < 33) && !ForbearanceDebuff && FunctionsLua::IsSpellReady("Blessing of Protection")) {
		//Blessing of Protection
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Blessing of Protection");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if (Combat && isParty && (distAlly < 30.0f) && (HpRatio < 60) && !BoSacrificeBuff && FunctionsLua::IsSpellReady("Blessing of Sacrifice")) {
		//Blessing of Sacrifice
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Blessing of Sacrifice");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if ((!Combat || nbrAggro == 0) && (distAlly < 40.0f) && (HpRatio < 50) && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && FunctionsLua::IsSpellReady("Flash of Light")) {
		//Flash of Light
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Flash of Light");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	else if ((!Combat || nbrAggro == 0) && (distAlly < 40.0f) && (HpRatio < 33) && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && FunctionsLua::IsSpellReady("Holy Light")) {
		//Holy Light
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Holy Light");
		LastTarget = indexP;
		if (Combat) Functions::LuaCall("TargetLastEnemy()");
		return 0;
	}
	return 1;
}

void ListAI::PaladinTank() {
	int FoLIDs[6] = { 19750, 19939, 19940, 19941, 19942, 19943 };
	int holyLightIDs[9] = { 635, 639, 647, 1026, 1042, 3472, 10328, 10329, 25292 };
	if ((ListUnits.size() > LastTarget) && ((localPlayer->isCasting(holyLightIDs, 9) && (ListUnits[LastTarget].prctHP > 80))
		|| (localPlayer->isCasting(FoLIDs, 6) && (ListUnits[LastTarget].prctHP > 95)))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			int BoSanctuaryIDs[4] = { 20911, 20912, 20913, 20914 };
			bool BoSanctuaryBuff = localPlayer->hasBuff(BoSanctuaryIDs, 4);
			int BoKingsIDs[1] = { 20217 };
			bool BoKingsBuff = localPlayer->hasBuff(BoKingsIDs, 1);
			WoWUnit* BoKingsTarget = Functions::GetMissingBuff(BoKingsIDs, 1);
			int RetributionAuraIDs[5] = { 7294, 10298, 10299, 10300, 10301 };
			bool RetributionAuraBuff = localPlayer->hasBuff(RetributionAuraIDs, 5);
			int RighteousFuryIDs[1] = { 25780 };
			bool RighteousFuryBuff = localPlayer->hasBuff(RighteousFuryIDs, 1);
			int BoSalvationIDs[1] = { 1038 };
			WoWUnit* BoSalvationTarget = Functions::GetMissingBuff(BoSalvationIDs, 1);
			WoWUnit* PurifyTarget = FunctionsLua::GetGroupDispel("Disease", "Poison");
			WoWUnit* CleanseTarget = FunctionsLua::GetGroupDispel("Disease", "Poison", "Magic");
			WoWUnit* deadPlayer = Functions::GetGroupDead(1);
			if (!RetributionAuraBuff && FunctionsLua::IsPlayerSpell("Retribution Aura")) {
				//Retribution Aura
				FunctionsLua::CastSpellByName("Retribution Aura");
			}
			else if (!RighteousFuryBuff && FunctionsLua::IsSpellReady("Righteous Fury")) {
				//Righteous Fury
				FunctionsLua::CastSpellByName("Righteous Fury");
			}
			else if (!Combat && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && (deadPlayer != NULL) && FunctionsLua::IsSpellReady("Redemption")) {
				//Redemption
				localPlayer->SetTarget(deadPlayer->Guid);
				FunctionsLua::CastSpellByName("Redemption");
			}
			else if (!BoKingsBuff && FunctionsLua::IsSpellReady("Blessing of Kings")) {
				//Blessing of Kings (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Blessing of Kings");
			}
			else if ((BoKingsTarget != NULL) && FunctionsLua::IsSpellReady("Blessing of Kings")) {
				//Blessing of Kings (Group)
				localPlayer->SetTarget(BoKingsTarget->Guid);
				FunctionsLua::CastSpellByName("Blessing of Kings");
			}
			else if (!BoSanctuaryBuff && FunctionsLua::IsSpellReady("Blessing of Sanctuary")) {
				//Blessing of Sanctuary (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Blessing of Sanctuary");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if ((BoSalvationTarget != NULL) && FunctionsLua::IsSpellReady("Blessing of Salvation")) {
				//Blessing of Salvation (Groupe)
				localPlayer->SetTarget(BoSalvationTarget->Guid);
				FunctionsLua::CastSpellByName("Blessing of Salvation");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (FunctionsLua::GetMPotionCD() < 1.25)) {
				//Mana Potion
				FunctionsLua::UseItem("Mana Potion");
			}
			else if ((localPlayer->prctMana > 25) && FunctionsLua::GetUnitDispel("player", "Disease", "Poison") && FunctionsLua::IsSpellReady("Purify")) {
				//Purify (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Purify");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if ((PurifyTarget != NULL) && (localPlayer->prctMana > 25) && FunctionsLua::IsSpellReady("Purify")) {
				//Purify (Groupe)
				localPlayer->SetTarget(PurifyTarget->Guid);
				FunctionsLua::CastSpellByName("Purify");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if ((localPlayer->prctMana > 25) && FunctionsLua::GetUnitDispel("player", "Disease", "Poison", "Magic") && FunctionsLua::IsSpellReady("Cleanse")) {
				//Cleanse (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Cleanse");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else if ((CleanseTarget != NULL) && (localPlayer->prctMana > 25) && FunctionsLua::IsSpellReady("Cleanse")) {
				//Cleanse (Groupe)
				localPlayer->SetTarget(CleanseTarget->Guid);
				FunctionsLua::CastSpellByName("Cleanse");
				if (Combat) Functions::LuaCall("TargetLastEnemy()");
			}
			else {
				int tmp = 1; unsigned int index = 0;
				while (tmp == 1 and index < HealTargetArray.size()) {
					tmp = HealGroup(HealTargetArray[index]);
					index = index + 1;
				}
				if (tmp == 1 && !passiveGroup) PaladinAttack();
			}
		});
	}
}