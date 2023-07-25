#include "ListAI.h"

#include <iostream>

static void DruidAttack() {
	int MoonkinFormIDs[1] = { 24858 }; bool MoonkinFormBuff = localPlayer->hasBuff(MoonkinFormIDs, 1);
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
		time_t EntanglingRootsTimer = 15 - (time(0) - current_time);
		if (EntanglingRootsTimer < 0) EntanglingRootsTimer = 0;
		//Specific for Hurricane cast:
		Position cluster_center = Position(0, 0, 0); int cluster_unit;
		std::tie(cluster_center, cluster_unit) = Functions::getAOETargetPos(25, 30);
		int MoonkinFormIDs[1] = { 24858 }; bool MoonkinFormBuff = localPlayer->hasBuff(MoonkinFormIDs, 1);
		int MoonfireIDs[10] = { 8921, 8924, 8925, 8926, 8927, 8928, 8929, 9833, 9834, 9835 };
		bool MoonfireDebuff = targetUnit->hasDebuff(MoonfireIDs, 10);
		bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
		if (!Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
		if (!MoonkinFormBuff && Functions::IsSpellReady("Moonkin Form")) {
			//Moonkin Form
			Functions::CastSpellByName("Moonkin Form");
		}
		else if ((localPlayer->speed == 0) && (cluster_unit >= 4) && Functions::IsSpellReady("Hurricane")) {
			//Hurricane
			Functions::CastSpellByName("Hurricane");
			Functions::ClickAOE(cluster_center);
		}
		else if (IsFacing && !MoonfireDebuff && Functions::IsSpellReady("Moonfire") && ((localPlayer->prctMana > 50.0f) || (MoonkinFormBuff))) {
			//Moonfire
			Functions::CastSpellByName("Moonfire");
		}
		else if ((localPlayer->speed == 0) && targetPlayer && (EntanglingRootsTimer == 0) && Functions::IsSpellReady("Entangling Roots")) {
			//Entangling Roots (PvP)
			Functions::CastSpellByName("Entangling Roots");
			if (localPlayer->isCasting()) current_time = time(0);
		}
		else if (IsFacing && (localPlayer->speed == 0) && Functions::IsSpellReady("Wrath") && ((localPlayer->prctMana > 50.0f) || (MoonkinFormBuff))) {
			//Wrath
			Functions::CastSpellByName("Wrath");
		}
	}
	else if (!Combat && (localPlayer->prctMana > 33.0f) && MoonkinFormBuff) {
		//Disable Moonkin Form
		Functions::CastSpellByName("Moonkin Form");
	}
}

static int HealGroup(unsigned int indexP) { //Heal Players and Npcs
	unsigned long long healGuid = ListUnits[indexP].Guid;
	bool isPlayer = (healGuid == localPlayer->Guid);
	bool isParty = false;
	if (!isPlayer) {
		for (int i = 1; i <= NumGroupMembers; i++) {
			if ((GroupMembersIndex[i] > -1) && ListUnits[GroupMembersIndex[i]].Guid == ListUnits[indexP].Guid) isParty = true;
		}
		if (!isParty) return 1;
	}
	bool los_heal = true; if (isParty) los_heal = !Functions::Intersect(localPlayer->position, ListUnits[indexP].position, 2.00f);
	float HpRatio = ListUnits[indexP].prctHP;
	float distAlly = localPlayer->position.DistanceTo(ListUnits[indexP].position);
	int RejuvenationIDs[11] = { 774, 1058, 1430, 2090, 2091, 3627, 8910, 9839, 9840, 9841, 25299 };
	bool RejuvenationBuff = ListUnits[indexP].hasBuff(RejuvenationIDs, 11);
	int RegrowthIDs[9] = { 8936, 8938, 8939, 8940, 8941, 9750, 9856, 9857, 9858 };
	bool RegrowthBuff = ListUnits[indexP].hasBuff(RegrowthIDs, 9);
	int MoonkinFormIDs[1] = { 24858 }; bool MoonkinFormBuff = localPlayer->hasBuff(MoonkinFormIDs, 1);
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
	else if (isPlayer && Combat && (localPlayer->prctHP < 40) && Functions::IsSpellReady("Barkskin")) {
		//Barkskin
		Functions::CastSpellByName("Barkskin");
		return 0;
	}
	else if (MoonkinFormBuff && (HpRatio < 40.0f) && (localPlayer->prctMana > 33.0f)) {
		//Disable Moonkin Form
		Functions::CastSpellByName("Moonkin Form");
	}
	else if ((AoEHeal >= 4) && (distAlly < 40.0f) && Functions::IsSpellReady("Tranquility")) {
		//Tranquility
		Functions::CastSpellByName("Tranquility");
		return 0;
	}
	else if ((HpRatio < 50) && (distAlly < 40.0f) && !RegrowthBuff && Functions::IsSpellReady("Regrowth")) {
		//Regrowth
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Regrowth");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 40) && (distAlly < 40.0f) && Functions::IsSpellReady("Healing Touch")) {
		//Healing Touch
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Healing Touch");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	else if ((HpRatio < 70) && (distAlly < 40.0f) && !RejuvenationBuff && Functions::IsSpellReady("Rejuvenation")) {
		//Rejuvenation
		localPlayer->SetTarget(healGuid);
		Functions::CastSpellByName("Rejuvenation");
		LastTarget = indexP;
		if (!los_heal) Moving = 5;
		return 0;
	}
	return 1;
}

void ListAI::DruidBalance() {
	int HealingTouchIDs[11] = { 5185, 5186, 5187, 5188, 5189, 6778, 8903, 9758, 9888, 9889, 25297 };
	int RegrowthIDs[9] = { 8936, 8938, 8939, 8940, 8941, 9750, 9856, 9857, 9858 };
	if ((ListUnits.size() > LastTarget) && ((localPlayer->isCasting(HealingTouchIDs, 11) && (ListUnits[LastTarget].prctHP > 80))
		|| (localPlayer->isCasting(RegrowthIDs, 9) && (ListUnits[LastTarget].prctHP > 80)))) {
		Functions::pressKey(0x28);
		Functions::releaseKey(0x28);
	}
	else if ((localPlayer->castInfo == 0) && (localPlayer->channelInfo == 0) && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			int MotWIDs[9] = { 1126, 5232, 6756, 5234, 8907, 9884, 9885, 21849, 21850 }; //GotW included
			bool MotWBuff = localPlayer->hasBuff(MotWIDs, 9);
			int MotWKey = Functions::GetBuffKey(MotWIDs, 9);
			int ThornsIDs[6] = { 467, 782, 1075, 8914, 9756, 9910 };
			bool ThornsBuff = localPlayer->hasBuff(ThornsIDs, 6);
			int ThornsKey = Functions::GetBuffKey(ThornsIDs, 6);
			int RemoveCurseKey = Functions::GetDispelKey("Curse");
			int CurePoisonKey = Functions::GetDispelKey("Poison");
			if ((localPlayer->speed == 0) && Functions::IsSpellReady("Rebirth") && (Functions::GetGroupDead(1) > 0)) {
				//Rebirth
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[Functions::GetGroupDead()]].Guid);
				Functions::CastSpellByName("Rebirth");
			}
			else if ((MotWKey > 0) && (GroupMembersIndex[MotWKey] > -1) && Functions::IsSpellReady("Gift of the Wild")) {
				//Gift of the Wild (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[MotWKey]].Guid);
				Functions::CastSpellByName("Gift of the Wild");
			}
			else if (!MotWBuff && Functions::IsSpellReady("Mark of the Wild")) {
				//Mark of the Wild (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Mark of the Wild");
			}
			else if ((MotWKey > 0) && (GroupMembersIndex[MotWKey] > -1) && Functions::IsSpellReady("Mark of the Wild")) {
				//Mark of the Wild (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[MotWKey]].Guid);
				Functions::CastSpellByName("Mark of the Wild");
			}
			else if (!ThornsBuff && Functions::IsSpellReady("Thorns")) {
				//Thorns (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Thorns");
			}
			else if ((ThornsKey > 0) && (GroupMembersIndex[ThornsKey] > -1) && Functions::IsSpellReady("Thorns")) {
				//Thorns (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[ThornsKey]].Guid);
				Functions::CastSpellByName("Thorns");
			}
			else if (Combat && (localPlayer->prctMana < 10) && Functions::IsSpellReady("Innervate")) {
				//Innervate
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Innervate");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::UseItem("Mana Potion");
			}
			else if ((localPlayer->prctMana > 25) && Functions::GetUnitDispel("player", "Curse") && Functions::IsSpellReady("Remove Curse")) {
				//Remove Curse (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Remove Curse");
			}
			else if ((RemoveCurseKey > 0) && (GroupMembersIndex[RemoveCurseKey] > -1) && (localPlayer->prctMana > 25) && Functions::IsSpellReady("Remove Curse")) {
				//Remove Curse (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[RemoveCurseKey]].Guid);
				Functions::CastSpellByName("Remove Curse");
			}
			else if ((localPlayer->prctMana > 25) && Functions::GetUnitDispel("player", "Poison") && Functions::IsSpellReady("Cure Poison")) {
				//Cure Poison (self)
				localPlayer->SetTarget(localPlayer->Guid);
				Functions::CastSpellByName("Cure Poison");
			}
			else if ((CurePoisonKey > 0) && (GroupMembersIndex[CurePoisonKey] > -1) && (localPlayer->prctMana > 25) && Functions::IsSpellReady("Cure Poison")) {
				//Cure Poison (Group)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[CurePoisonKey]].Guid);
				Functions::CastSpellByName("Cure Poison");
			}
			else {
				int tmp = 1; unsigned int index = 0;
				while (tmp == 1 and index < HealTargetArray.size()) {
					tmp = HealGroup(HealTargetArray[index]);
					index = index + 1;
				}
				if (tmp == 1 && !passiveGroup) DruidAttack();
			}
		});
	}
}