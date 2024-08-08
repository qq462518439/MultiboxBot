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
		int SoLIDs[4] = { 20165, 20347, 20348, 20349 };
		bool SoLBuff = localPlayer->hasBuff(SoLIDs, 4);
		int SolDebuffIDs[4] = { 20185, 20344, 20345, 20346 };
		bool SoLDebuff = targetUnit->hasDebuff(SolDebuffIDs, 4);
		if (!FunctionsLua::IsCurrentAction(FunctionsLua::GetSlot("Attack"))) FunctionsLua::CastSpellByName("Attack");
		if (!SoLBuff && !SoLDebuff && FunctionsLua::UnitIsElite("target") && FunctionsLua::IsSpellReady("Seal of Light")) {
			//Seal of Light
			FunctionsLua::CastSpellByName("Seal of Light");
		}
		else if ((localPlayer->prctMana > 33) && (nbrCloseEnemy >= 4) && FunctionsLua::IsSpellReady("Consecration")) {
			//Consecration
			FunctionsLua::CastSpellByName("Consecration");
		}
		else if (SoLBuff && (distTarget < 10) && FunctionsLua::UnitIsElite("target") && FunctionsLua::IsSpellReady("Judgement")) {
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
	unsigned long long healGuid = ListUnits[indexP].Guid;
	bool isPlayer = (healGuid == localPlayer->Guid);
	bool isParty = false;
	if (!isPlayer) {
		for (int i = 1; i <= NumGroupMembers; i++) {
			if ((GroupMember[i] != NULL) && GroupMember[i]->Guid == ListUnits[indexP].Guid) isParty = true;
		}
		if (!isParty) return 1;
	}
	bool los_heal = true; if (isParty) los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
	float HpRatio = ListUnits[indexP].prctHP;
	bool isTank = (Leader != NULL && ListUnits[indexP].name == (std::string)Leader->name);
	float distAlly = localPlayer->position.DistanceTo(ListUnits[indexP].position);
	int ForbearanceID[1] = { 25771 };
	bool ForbearanceDebuff = ListUnits[indexP].hasDebuff(ForbearanceID, 1);
	int BoSIDs[2] = { 6940, 20729 };
	bool BoSacrificeBuff = ListUnits[indexP].hasBuff(BoSIDs, 2);
	if (Combat && isParty && (distAlly < 40.0f) && (HpRatio < 20) && FunctionsLua::IsSpellReady("Lay on Hands")) {
		//Lay on Hands
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Lay on Hands");
		LastTarget = indexP;
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
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if (Combat && (distAlly < 30.0f) && isParty && (HpRatio < 60) && !BoSacrificeBuff && FunctionsLua::IsSpellReady("Blessing of Sacrifice")) {
		//Blessing of Sacrifice
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Blessing of Sacrifice");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 50) && (distAlly < 20.0f) && FunctionsLua::IsSpellReady("Holy Shock")) {
		//Holy Shock
		localPlayer->SetTarget(healGuid);
		if (FunctionsLua::IsSpellReady("Divine Favor")) FunctionsLua::CastSpellByName("Divine Favor");
		FunctionsLua::CastSpellByName("Holy Shock");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 50) && (distAlly < 40.0f) && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && FunctionsLua::IsSpellReady("Holy Light")) {
		//Holy Light
		localPlayer->SetTarget(healGuid);
		if (FunctionsLua::IsSpellReady("Divine Favor")) FunctionsLua::CastSpellByName("Divine Favor");
		FunctionsLua::CastSpellByName("Holy Light");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 85) && (distAlly < 40.0f) && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && FunctionsLua::IsSpellReady("Flash of Light")) {
		//Flash of Light
		localPlayer->SetTarget(healGuid);
		FunctionsLua::CastSpellByName("Flash of Light");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	return 1;
}

void ListAI::PaladinHeal() {
	int FoLIDs[6] = { 19750, 19939, 19940, 19941, 19942, 19943 };
	int holyLightIDs[9] = { 635, 639, 647, 1026, 1042, 3472, 10328, 10329, 25292 };
	if ((ListUnits.size() > LastTarget) && ((localPlayer->isCasting(holyLightIDs, 9) && (ListUnits[LastTarget].prctHP > 80))
		|| (localPlayer->isCasting(FoLIDs, 6) && (ListUnits[LastTarget].prctHP > 95)))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			int BoMightIDs[7] = { 19740, 19834, 19835, 19836, 19837, 19838, 25291 };
			bool BoMightBuff = localPlayer->hasBuff(BoMightIDs, 7);
			WoWUnit* BoMightTarget = Functions::GetMissingBuff(BoMightIDs, 7);
			int BoKingsIDs[1] = { 20217 };
			bool BoKingsBuff = localPlayer->hasBuff(BoKingsIDs, 1);
			WoWUnit* BoKingsTarget = Functions::GetMissingBuff(BoKingsIDs, 1);
			int DevotionAuraIDs[7] = { 465, 10290, 643, 10291, 1032, 10292, 10293 };
			bool DevotionAuraBuff = localPlayer->hasBuff(DevotionAuraIDs, 7);
			WoWUnit* PurifyTarget = FunctionsLua::GetGroupDispel("Disease", "Poison");
			WoWUnit* CleanseTarget = FunctionsLua::GetGroupDispel("Disease", "Poison", "Magic");
			WoWUnit* deadPlayer = Functions::GetGroupDead();
			if (!DevotionAuraBuff && FunctionsLua::IsPlayerSpell("Devotion Aura")) {
				//Devotion Aura
				FunctionsLua::CastSpellByName("Devotion Aura");
			}
			else if (!Combat && (localPlayer->speed == 0) && (Moving == 0 || Moving == 4) && (deadPlayer != NULL) && FunctionsLua::IsSpellReady("Redemption")) {
				//Redemption
				localPlayer->SetTarget(deadPlayer->Guid);
				FunctionsLua::CastSpellByName("Redemption");
			}
			else if (!BoMightBuff && !FunctionsLua::IsPlayerSpell("Blessing of Kings") && FunctionsLua::IsSpellReady("Blessing of Might")) {
				//Blessing of Might (self)
				localPlayer->SetTarget(localPlayer->Guid);
				FunctionsLua::CastSpellByName("Blessing of Might");
			}
			else if ((BoMightTarget != NULL) && !FunctionsLua::IsPlayerSpell("Blessing of Kings") && FunctionsLua::IsSpellReady("Blessing of Might")) {
				//Blessing of Might (Group)
				localPlayer->SetTarget(BoMightTarget->Guid);
				FunctionsLua::CastSpellByName("Blessing of Might");
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
				//Purify (Group)
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