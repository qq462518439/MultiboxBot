#include "ListAI.h"

#include <iostream>
#include <ctime>

static time_t current_time = time(0);
static float SliceDiceTimer = 0;
static float SliceDiceDuration = 0; //Depends on talent and combo points

void ListAI::RogueDps() {
	SliceDiceTimer = SliceDiceDuration - (time(0) - current_time);
	if (SliceDiceTimer < 0) SliceDiceTimer = 0;
	if (localPlayer->castInfo == 0 && localPlayer->channelInfo == 0 && !localPlayer->isdead) {
		int nbrAggro = HasAggro[0].size();
		bool IsStunned = localPlayer->flags & UNIT_FLAG_STUNNED;
		bool IsConfused = localPlayer->flags & UNIT_FLAG_CONFUSED;
		int StealthIDs[4] = { 1784, 1785, 1786, 1787 };
		bool StealthBuff = localPlayer->hasBuff(StealthIDs, 4);
		int SliceDiceIDs[2] = { 5171, 6774 };
		bool SliceDiceBuff = localPlayer->hasBuff(SliceDiceIDs, 2);

		ThreadSynchronizer::RunOnMainThread([=]() {
			int ComboPoints = Functions::GetComboPoints();
			int SprintTalent = Functions::GetTalentInfo(2, 9);

			if (IsInGroup && tankName != "null" && (targetUnit == NULL || targetUnit->isdead || targetUnit->unitReaction > Neutral)) {
				std::string msg = "AssistByName('" + tankName + "')";
				Functions::LuaCall(msg.c_str());
			}
			else if (Combat && (targetUnit == NULL || targetUnit->isdead || targetUnit->unitReaction > Neutral) && (HasAggro[0].size() > 0)) {
				localPlayer->SetTarget(HasAggro[0][0]);
			}

			if (Combat && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
				//Healthstone
				Functions::UseItem("Healthstone");
			}
			else if (Combat && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
				//Healing Potion
				Functions::UseItem("Healing Potion");
			}
			else if (targetUnit != NULL && targetUnit->unitReaction <= Neutral && !targetUnit->isdead) {
				bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
				bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
				bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
				int GougeIDs[5] = { 1776, 1777, 8629, 11285, 11286 };
				bool GougeDebuff = targetUnit->hasDebuff(GougeIDs, 5);
				bool stopAttack = false;
				if ((StealthBuff || GougeDebuff || targetConfused) && Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
				if (GougeDebuff || targetConfused) stopAttack = true; else stopAttack = false;
				if (!Combat && !StealthBuff && Functions::IsSpellReady("Stealth")) {
					//Stealth
					Functions::CastSpellByName("Stealth");
				}
				else if (StealthBuff && Functions::IsSpellReady("Cheap Shot")) {
					//Cheap Shot
					Functions::CastSpellByName("Cheap Shot");
				}
				else if (stopAttack) { } //Do nothing
				else if (Combat && (distTarget > 12) && (localPlayer->speed < 7) && (SprintTalent > 0) && Functions::IsSpellReady("Sprint")) {
					//Sprint
					Functions::CastSpellByName("Sprint");
				}
				else if (Combat && !StealthBuff && (Functions::GetItemCount(5140) > 0) && (localPlayer->speed < 7) && Functions::IsSpellReady("Vanish")) {
					//Vanish
					Functions::CastSpellByName("Vanish");
				}
				else if (Combat && !StealthBuff && targetPlayer && (distTarget > 5) && (Functions::GetItemCount(5530) > 0) && (localPlayer->speed < 7) && Functions::IsSpellReady("Blind")) {
					//Blind
					Functions::CastSpellByName("Blind");
				}
				else if (Combat && !StealthBuff && SliceDiceBuff && Functions::UnitIsElite("target") && Functions::IsSpellReady("Adrenaline Rush")) {
					//Adrenaline Rush
					Functions::CastSpellByName("Adrenaline Rush");
				}
				else if (Combat && !StealthBuff && ((nbrCloseEnemyFacing >= 2) || targetPlayer) && (SliceDiceTimer >= 15) && Functions::IsSpellReady("Blade Flurry")) {
					//Blade Flurry
					Functions::CastSpellByName("Blade Flurry");
				}
				else if (!StealthBuff && (((ComboPoints >= 3) && !SliceDiceBuff) || (SliceDiceTimer < 8 && (ComboPoints >= 5))) && Functions::IsSpellReady("Slice and Dice")) {
					//Slice and Dice
					int SliceDiceTalent = Functions::GetTalentInfo(1, 6);
					SliceDiceDuration = (6 + (3 * ComboPoints)) * (1 + (0.15 * SliceDiceTalent));
					current_time = time(0);
					Functions::CastSpellByName("Slice and Dice");
				}
				else if (IsFacing && !StealthBuff && (ComboPoints >= 3) && targetPlayer && Functions::IsSpellReady("Kidney Shot")) {
					//Kidney Shot
					Functions::CastSpellByName("Kidney Shot");
				}
				else if (IsFacing && !StealthBuff && (ComboPoints >= 5) && (SliceDiceTimer >= 8) && Functions::IsSpellReady("Eviscerate")) {
					//Eviscerate
					Functions::CastSpellByName("Eviscerate");
				}
				else if (IsFacing && !StealthBuff && !targetStunned && Functions::UnitIsCaster("target") && Functions::IsSpellReady("Kick")) {
					//Kick
					Functions::CastSpellByName("Kick");
				}
				else if (IsFacing && !StealthBuff && Functions::IsSpellReady("Sinister Strike")) {
					//Sinister Strike
					Functions::CastSpellByName("Sinister Strike");
				}
			}
			else if (!Combat && !IsSitting && IsInGroup) {
				if (Functions::FollowMultibox(0, 0)) Moving = 4;
			}
		});
	}
}