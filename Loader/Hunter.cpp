#include "ListAI.h"

#include <iostream>

void ListAI::HunterDps() {
	int FeignDeathIDs[1] = { 5384 };
	int RaptorStrikeIDs[8] = { 2973, 14260, 14261, 14262, 14263, 14264, 14265, 14266 };
	if (localPlayer->hasBuff(FeignDeathIDs, 1) || ((localPlayer->castInfo == 0 || localPlayer->isCasting(RaptorStrikeIDs, 8)) && (localPlayer->channelInfo == 0) && !localPlayer->isdead)) {
		int nbrAggro = HasAggro[0].size();
		bool IsStunned = localPlayer->flags & UNIT_FLAG_STUNNED;
		bool IsConfused = localPlayer->flags & UNIT_FLAG_CONFUSED;
		int TrueshotAuraIDs[1] = { 19506 };
		bool TrueshotAuraBuff = localPlayer->hasBuff(TrueshotAuraIDs, 1);
		int AspectMonkeyIDs[1] = { 13163 };
		bool AspectMonkeyBuff = localPlayer->hasBuff(AspectMonkeyIDs, 1);
		int AspectHawkIDs[7] = { 13165, 14318, 14319, 14320, 14321, 14322, 25296 };
		bool AspectHawkBuff = localPlayer->hasBuff(AspectHawkIDs, 7);

		//Specific for Volley cast:
		Position cluster_center = Position(0, 0, 0); int cluster_unit;
		std::tie(cluster_center, cluster_unit) = Functions::getAOETargetPos(30, 35);

		ThreadSynchronizer::RunOnMainThread([=]() {
			bool FeedingBuff = Functions::GetUnitBuff("pet", "Interface\\Icons\\Ability_Hunter_BeastTraining");

			if (IsInGroup && (tankIndex > 0) && (GroupMembersIndex[tankIndex] > -1) && ListUnits[GroupMembersIndex[tankIndex]].targetGuid != 0)
				localPlayer->SetTarget(ListUnits[GroupMembersIndex[tankIndex]].targetGuid);
			else if (Combat && (targetUnit == NULL || targetUnit->unitReaction > Neutral) && (HasAggro[0].size() > 0)) {
				if (listIndexCloseEnemies.size() > 0) localPlayer->SetTarget(ListUnits[listIndexCloseEnemies[0]].Guid);
			}

			if (!Functions::HasPetUI() && Functions::IsPlayerSpell("Call Pet")) {
				//Call Pet
				Functions::CastSpellByName("Call Pet");
			}
			else if (Functions::UnitIsDeadOrGhost("pet") && Functions::IsSpellReady("Revive Pet")) {
				//Revive Pet
				Functions::CastSpellByName("Revive Pet");
			}
			else if (!Combat && !FeedingBuff && Functions::HasPetUI() && (Functions::GetPetHappiness() < 3) && !Functions::UnitIsDeadOrGhost("pet") && Functions::HasMeat()) {
				//Feed Pet
				Functions::CastSpellByName("Feed Pet");
				Functions::PlaceItem(120, Functions::HasMeat()); Functions::UseAction(120);
			}
			else if (!TrueshotAuraBuff && Functions::IsSpellReady("Trueshot Aura")) {
				//Trueshot Aura
				Functions::CastSpellByName("Trueshot Aura");
			}
			else if (Combat && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
				//Healthstone
				Functions::UseItem("Healthstone");
			}
			else if (Combat && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
				//Healing Potion
				Functions::UseItem("Healing Potion");
			}
			else if (Combat && (localPlayer->prctMana < 10) && (Functions::GetMPotionCD() < 1.25)) {
				//Mana Potion
				Functions::UseItem("Mana Potion");
			}
			else if ((nbrEnemyPlayer == 0) && (nbrAggro > 0) && IsInGroup && Functions::IsSpellReady("Feign Death")) {
				//Feign Death (Aggro PvE)
				Functions::CastSpellByName("Feign Death");
			}
			else if (targetUnit != NULL && targetUnit->unitReaction <= Neutral && !targetUnit->isdead) {
				bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
				bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
				bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
				int WingClipIDs[3] = { 2974, 14267, 14268 };
				bool WingClipDebuff = targetUnit->hasDebuff(WingClipIDs, 3);
				int SerpentStingIDs[9] = { 1978, 13549, 13550, 13551, 13552, 13553, 13554, 13555, 25295 };
				bool SerpentStingDebuff = targetUnit->hasDebuff(SerpentStingIDs, 9);
				int HunterMarkIDs[4] = { 1130,  14323, 14324, 14325 };
				bool HunterMarkDebuff = targetUnit->hasDebuff(HunterMarkIDs, 4);
				int FreezingTrapIDs[3] = { 1499,  14310, 14311 };
				bool FreezingTrapDebuff = targetUnit->hasDebuff(FreezingTrapIDs, 3);
				if (FreezingTrapDebuff || targetConfused) { if (Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack"); }
				else if ((distTarget < 8) && !Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
				if ((distTarget > 8) && !Functions::IsAutoRepeatAction(Functions::GetSlot("Auto Shot"))) Functions::CastSpellByName("Auto Shot");
				if (targetUnit->flags & UNIT_FLAG_IN_COMBAT) Functions::LuaCall("PetAttack()");
				int distMove = 15; if (FreezingTrapDebuff) distMove = 30;
				if ((distTarget < 5) && (localPlayer->prctMana > 10) && targetPlayer && Functions::IsSpellReady("Feign Death")) {
					//Feign Death
					Functions::CastSpellByName("Feign Death");
				}
				else if (!Combat && (distTarget < 5) && targetPlayer && Functions::IsSpellReady("Freezing Trap")) {
					//Freezing Trap
					Functions::CastSpellByName("Freezing Trap");
				}
				else if (!Combat && (nbrCloseEnemy >= 4) && Functions::IsSpellReady("Explosive Trap")) {
					//Explosive trap (AoE)
					Functions::CastSpellByName("Explosive Trap");
				}
				else if ((distTarget < 15) && targetPlayer && !FreezingTrapDebuff && Functions::IsSpellReady("Scatter Shot")) {
					//Scatter Shot
					Functions::CastSpellByName("Scatter Shot");
				}
				else if ((distTarget < 5) && targetPlayer && !WingClipDebuff && !FreezingTrapDebuff && Functions::IsSpellReady("Wing Clip")) {
					//Wing Clip
					Functions::CastSpellByName("Wing Clip");
				}
				else if (IsFacing && targetUnit->channelInfo > 0 && (distTarget < 15) && Functions::IsSpellReady("Scatter Shot")) {
					//Scatter Shot (Silence)
					Functions::CastSpellByName("Scatter Shot");
				}
				else if ((distTarget < 5) && (nbrAggro > 0) && !AspectMonkeyBuff && Functions::IsSpellReady("Aspect of the Monkey")) {
					//Aspect of the Monkey
					Functions::CastSpellByName("Aspect of the Monkey");
				}
				else if ((((distTarget > 8) && !targetPlayer) || ((distTarget > 20) && targetPlayer)) && !AspectHawkBuff && Functions::IsSpellReady("Aspect of the Hawk")) {
					//Aspect of the Hawk
					Functions::CastSpellByName("Aspect of the Hawk");
				}
				else if (IsFacing && (distTarget > 8) && targetPlayer && Functions::IsSpellReady("Concussive Shot")) {
					//Concussive Shot (PvP)
					Functions::CastSpellByName("Concussive Shot");
				}
				else if (IsFacing && !targetPlayer && hasTargetAggro && (distTarget < 5) && Functions::IsSpellReady("Disengage")) {
					//Disengage
					Functions::CastSpellByName("Disengage");
				}
				else if (IsFacing && (distTarget < 5) && Functions::IsSpellReady("Mongoose Bite")) {
					//Mongoose Bite
					Functions::CastSpellByName("Mongoose Bite");
				}
				else if (IsFacing && (distTarget < 5) && Functions::IsSpellReady("Raptor Strike")) {
					//Raptor Strike
					Functions::CastSpellByName("Raptor Strike");
				}
				else if ((localPlayer->speed == 0) && (cluster_unit >= 4) && Functions::IsSpellReady("Volley")) {
					//Volley
					Functions::CastSpellByName("Volley");
					Functions::ClickAOE(cluster_center);
				}
				else if (!HunterMarkDebuff && Functions::UnitIsElite("target") && Functions::IsSpellReady("Hunter's Mark")) {
					//Hunter's Mark
					Functions::CastSpellByName("Hunter's Mark");
				}
				else if ((localPlayer->speed == 0) && (((distTarget > 8) && !targetPlayer) || ((distTarget > 20) && targetPlayer)) && Functions::UnitIsElite("target") && Functions::IsSpellReady("Rapid Fire")) {
					//Rapid Fire
					Functions::CastSpellByName("Rapid Fire");
				}
				else if (IsFacing && (distTarget > 8) && targetPlayer && !SerpentStingDebuff && Functions::IsSpellReady("Serpent Sting")) {
					//Serpent Sting (PvP)
					Functions::CastSpellByName("Serpent Sting");
				}
				else if (IsFacing && (distTarget > 8) && (localPlayer->speed > 0) && Functions::IsSpellReady("Arcane Shot")) {
					//Arcane Shot (Movement)
					Functions::CastSpellByName("Arcane Shot");
				}
				else if (IsFacing && (((distTarget > 8) && !targetPlayer) || ((distTarget > 20) && targetPlayer)) && (localPlayer->speed == 0) && Functions::IsSpellReady("Aimed Shot")) {
					//Aimed Shot
					Functions::CastSpellByName("Aimed Shot");
				}
				else if (IsFacing && (distTarget > 8) && (localPlayer->speed == 0) && Functions::IsSpellReady("Multi-Shot")) {
					//Multi-Shot
					Functions::CastSpellByName("Multi-Shot");
				}
			}
			else if (!Combat && !IsSitting && IsInGroup) {
				Functions::FollowMultibox("Eydis");
				Moving = 4;
			}
		});
	}
}