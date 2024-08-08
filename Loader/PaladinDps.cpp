#include "Client.h"
#include "ListAI.h"

#include <iostream>

static void PaladinAttack() {
	if (targetUnit == NULL || targetUnit->isdead || !targetUnit->attackable) {
		if ((Leader != NULL) && (Leader->targetGuid != 0)) { //Leader has target
			localPlayer->SetTarget(Leader->targetGuid);
		}
		else {
			for (int i = 0; i <= NumGroupMembers; i++) {
				if (HasAggro[i].size() > 0) {
					localPlayer->SetTarget(HasAggro[i][0]->Guid);
					break;
				}
			}
		}
	}
	else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
		bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
		bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
		int SotCIDs[6] = { 21082, 20162, 20305, 20306, 20307, 20308 };
		bool SotCBuff = localPlayer->hasBuff(SotCIDs, 6);
		int SotCDebuffIDs[6] = {21183, 20188, 20300, 20301, 20302, 20303};
		bool SotCDebuff = targetUnit->hasDebuff(SotCDebuffIDs, 6);
		int SoRIDs[9] = { 20154, 21084, 20287, 20288, 20289, 20290, 20291, 20292, 20293 };
		bool SoRBuff = localPlayer->hasBuff(SoRIDs, 9);
		int SoCIDs[5] = { 20375, 20915, 20918, 20919, 20920 };
		bool SoCBuff = localPlayer->hasBuff(SoCIDs, 5);
		int SoLIDs[1] = { 20165 };
		bool SoLBuff = localPlayer->hasBuff(SoLIDs, 1);
		int SoLDebuffIDs[1] = { 20167 };
		bool SoLDebuff = targetUnit->hasDebuff(SoLDebuffIDs, 1);
		bool SealBuff = (SoRBuff || SotCBuff || SoCBuff || SoLBuff);
		if (!FunctionsLua::IsCurrentAction(FunctionsLua::GetSlot("Attack"))) FunctionsLua::CastSpellByName("Attack");
		if (!SealBuff && !SoLDebuff && FunctionsLua::UnitIsElite("target") && FunctionsLua::IsSpellReady("Seal of Light")) {
			//Seal of Light
			FunctionsLua::CastSpellByName("Seal of Light");
		}
		else if (!SealBuff && !SotCDebuff && FunctionsLua::UnitIsElite("target") && FunctionsLua::IsSpellReady("Seal of the Crusader")) {
			//Seal of the Crusader
			FunctionsLua::CastSpellByName("Seal of the Crusader");
		}
		else if (!SealBuff && !FunctionsLua::IsPlayerSpell("Seal of Command") && FunctionsLua::IsSpellReady("Seal of Righteousness")) {
			//Seal of Righteousness
			FunctionsLua::CastSpellByName("Seal of Righteousness");
		}
		else if (!SealBuff && FunctionsLua::IsSpellReady("Seal of Command")) {
			//Seal of Command
			FunctionsLua::CastSpellByName("Seal of Command");
		}
		else if ((localPlayer->prctMana > 33) && (nbrCloseEnemy >= 4) && FunctionsLua::IsSpellReady("Consecration")) {
			//Consecration
			FunctionsLua::CastSpellByName("Consecration");
		}
		else if (SealBuff && (distTarget < 10) && FunctionsLua::IsSpellReady("Judgement")) {
			//Judgement
			FunctionsLua::CastSpellByName("Judgement");
		}
		else if (!targetStunned && !targetConfused && (distTarget < 10) && FunctionsLua::IsSpellReady("Hammer of Justice")) {
			//Hammer of Justice
			FunctionsLua::CastSpellByName("Hammer of Justice");
		}
		else if ((Functions::getNbrCreatureType(20, Undead, Demon) >= 4) && FunctionsLua::IsSpellReady("Holy Wrath")) {
			//Holy Wrath
			FunctionsLua::CastSpellByName("Holy Wrath");
		}
		else if ((localPlayer->prctMana > 33) && (distTarget < 30) && ((targetUnit->creatureType == Undead) || (targetUnit->creatureType == Demon)) && FunctionsLua::IsSpellReady("Exorcism")) {
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
	bool isTank = (Leader != NULL && ListUnits[indexP].name == (std::string)Leader->name);
	bool DivineProtectionBuff = false;
	if (!isPlayer) {
		for (int i = 1; i <= NumGroupMembers; i++) {
			if ((GroupMember[i] != NULL) && GroupMember[i]->Guid == ListUnits[indexP].Guid) isParty = true;
		}
	}
	else {
		int DivineProtectionIDs[4] = { 498, 5573, 642, 1020 };
		DivineProtectionBuff = ListUnits[indexP].hasBuff(DivineProtectionIDs, 4);
	}
	float distAlly = localPlayer->position.DistanceTo(ListUnits[indexP].position);
	if (Combat && isParty && (distAlly < 40.0f) && (HpRatio < 20) && FunctionsLua::IsSpellReady("Lay on Hands")) {
		//Lay on Hands
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Lay on Hands");
		LastTarget = indexP;
		bool los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if (isPlayer && Combat && (localPlayer->prctHP < 40) && !ForbearanceDebuff && FunctionsLua::IsSpellReady("Divine Protection")) {
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
	else if (Combat && (distAlly < 30.0f) && isParty && !isTank && (HpRatio < 33) && !ForbearanceDebuff && FunctionsLua::IsSpellReady("Blessing of Protection")) {
		//Blessing of Protection
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Blessing of Protection");
		LastTarget = indexP;
		bool los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if (Combat && (distAlly < 30.0f) && isParty && (HpRatio < 60) && !BoSacrificeBuff && FunctionsLua::IsSpellReady("Blessing of Sacrifice")) {
		//Blessing of Sacrifice
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Blessing of Sacrifice");
		LastTarget = indexP;
		bool los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 50) && (distAlly < 40.0f) && (localPlayer->prctMana > 33) && ((float(time(0) - autoAttackCD) < 2.5f) || DivineProtectionBuff) && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && FunctionsLua::IsSpellReady("Holy Light")) {
		//Holy Light
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Holy Light");
		LastTarget = indexP;
		if (localPlayer->isCasting()) current_time = time(0);
		bool los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 85) && (distAlly < 40.0f) && (localPlayer->prctMana > 33) && ((float(time(0) - autoAttackCD) < 1.5f) || DivineProtectionBuff) && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && FunctionsLua::IsSpellReady("Flash of Light")) {
		//Flash of Light
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Flash of Light");
		LastTarget = indexP;
		if (localPlayer->isCasting()) current_time = time(0);
		bool los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
		if (!los_heal) Moving = 5;
		return 0;
	}
	return 1;
}

void ListAI::PaladinDps() {
	int FoLIDs[6] = { 19750, 19939, 19940, 19941, 19942, 19943 };
	int holyLightIDs[9] = { 635, 639, 647, 1026, 1042, 3472, 10328, 10329, 25292 };
	if ((ListUnits.size() > LastTarget) && ((localPlayer->isCasting(holyLightIDs, 9) && (ListUnits[LastTarget].prctHP > 80))
		|| (localPlayer->isCasting(FoLIDs, 6) && (ListUnits[LastTarget].prctHP > 95)))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			int BoWisdomIDs[6] = { 19742, 19850, 19852, 19853, 19854, 25290 };
			bool BoWisdomBuff = localPlayer->hasBuff(BoWisdomIDs, 6);
			int SanctityAuraIDs[1] = { 20218 };
			bool SanctityAuraBuff = localPlayer->hasBuff(SanctityAuraIDs, 1);
			WoWUnit* BoWisdomTarget = Functions::GetMissingBuff(BoWisdomIDs, 6);
			WoWUnit* PurifyTarget = FunctionsLua::GetGroupDispel("Disease", "Poison");
			WoWUnit* CleanseTarget = FunctionsLua::GetGroupDispel("Disease", "Poison", "Magic");
			WoWUnit* deadPlayer = Functions::GetGroupDead(1);
			if (!SanctityAuraBuff && FunctionsLua::IsPlayerSpell("Sanctity Aura")) {
				//Sanctity Aura
				FunctionsLua::CastSpellByName("Sanctity Aura");
			}
			else if (!Combat && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && (deadPlayer != NULL) && FunctionsLua::IsSpellReady("Redemption")) {
				//Redemption
				localPlayer->SetTarget(deadPlayer->Guid);
				FunctionsLua::CastSpellByName("Redemption");
			}
			else if (!BoWisdomBuff && FunctionsLua::IsSpellReady("Blessing of Wisdom")) {
				//Blessing of Wisdom (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Blessing of Wisdom");
			}
			else if ((BoWisdomTarget != NULL) && FunctionsLua::IsSpellReady("Blessing of Wisdom")) {
				//Blessing of Wisdom (Groupe)
				localPlayer->SetTarget(BoWisdomTarget->Guid);
				FunctionsLua::CastSpellByName("Blessing of Wisdom");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (FunctionsLua::GetMPotionCD() < 1.25)) {
				//Mana Potion
				FunctionsLua::UseItem("Mana Potion");
			}
			else if ((localPlayer->prctMana > 25) && FunctionsLua::GetUnitDispel("player", "Disease", "Poison") && FunctionsLua::IsSpellReady("Purify")) {
				//Purify (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Purify");
			}
			else if ((PurifyTarget != NULL) && (localPlayer->prctMana > 25) && FunctionsLua::IsSpellReady("Purify")) {
				//Purify (Groupe)
				localPlayer->SetTarget(PurifyTarget->Guid);
				FunctionsLua::CastSpellByName("Purify");
			}
			else if ((localPlayer->prctMana > 25) && FunctionsLua::GetUnitDispel("player", "Disease", "Poison", "Magic") && FunctionsLua::IsSpellReady("Cleanse")) {
				//Cleanse (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Cleanse");
			}
			else if ((CleanseTarget != NULL) && (localPlayer->prctMana > 25) && FunctionsLua::IsSpellReady("Cleanse")) {
				//Cleanse (Groupe)
				localPlayer->SetTarget(CleanseTarget->Guid);
				FunctionsLua::CastSpellByName("Cleanse");
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