#include "ListAI.h"

#include <iostream>
#include <ctime>

void ListAI::WarriorTank() {
	if (localPlayer->castInfo == 0 && localPlayer->channelInfo == 0 && !localPlayer->isdead) {
		ThreadSynchronizer::RunOnMainThread([=]() {

			if (tankAutoFocus && (targetUnit == NULL || targetUnit->isdead || !targetUnit->attackable)) {
				for (int i = NumGroupMembers; i >= 0; i--) {
					if (HasAggro[i].size() > 0) {
						localPlayer->SetTarget(HasAggro[i][0]);
						break;
					}
				}
			}

			int BattleShoutIDs[7] = { 6673, 5242, 6192, 11549, 11550, 11551, 25289 }; bool BattleShoutBuff = localPlayer->hasBuff(BattleShoutIDs, 7);
			int DemoralizingShoutIDs[5] = { 1160, 6190, 11554, 11555, 11556 }; bool DemoralizingShoutDebuff = localPlayer->hasBuff(DemoralizingShoutIDs, 5);
			if (Combat && (localPlayer->prctHP < 40) && (Functions::GetHealthstoneCD() < 1.25)) {
				//Healthstone
				Functions::UseItem("Healthstone");
			}
			else if (Combat && (localPlayer->prctHP < 35) && (Functions::GetHPotionCD() < 1.25)) {
				//Healing Potion
				Functions::UseItem("Healing Potion");
			}
			else if (Combat && (localPlayer->prctHP < 40) && Functions::IsSpellReady("Last Stand")) {
				//Last Stand
				Functions::CastSpellByName("Last Stand");
			}
			else if (Combat && (localPlayer->prctHP < 25) && Functions::IsSpellReady("Shield Wall")) {
				//Shield Wall
				Functions::CastSpellByName("Shield Wall");
			}
			else if ((nbrCloseEnemy >= 4) && Functions::IsSpellReady("Intimidating Shout")) {
				//Intimidating Shout
				Functions::CastSpellByName("Intimidating Shout");
			}
			else if (!BattleShoutBuff && Functions::IsSpellReady("Battle Shout")) {
				//Battle Shout
				Functions::CastSpellByName("Battle Shout");
			}
			else if ((nbrCloseEnemy >= 3) && !DemoralizingShoutDebuff && Functions::IsSpellReady("Demoralizing Shout")) {
				//Demoralizing Shout
				Functions::CastSpellByName("Demoralizing Shout");
			}
			else if (targetUnit != NULL && targetUnit->attackable && !targetUnit->isdead) {
				bool targetPlayer = targetUnit->flags & UNIT_FLAG_PLAYER_CONTROLLED;
				bool targetStunned = targetUnit->flags & UNIT_FLAG_STUNNED;
				bool targetConfused = targetUnit->flags & UNIT_FLAG_CONFUSED;
				bool BattleStance = Functions::GetShapeshiftFormInfo(1);
				bool DefensiveStance = Functions::GetShapeshiftFormInfo(2);
				bool BerserkerStance = Functions::GetShapeshiftFormInfo(3);
				if (!Functions::IsCurrentAction(Functions::GetSlot("Attack"))) Functions::CastSpellByName("Attack");
				if (BattleStance) {
					int HamstringIDs[3] = { 1715, 7372, 7373 };
					bool HamstringDebuff = targetUnit->hasDebuff(HamstringIDs, 3);
					if ((distTarget < 25.0f) && Functions::IsSpellReady("Charge")) {
						//Charge
						Functions::CastSpellByName("Charge");
					}
					else if (!hasTargetAggro && !targetPlayer && Functions::IsSpellReady("Mocking Blow")) {
						//Mocking Blow
						Functions::CastSpellByName("Mocking Blow");
					}
					else if ((localPlayer->rage < 25) && Functions::IsSpellReady("Charge")) {
						//Bloodrage
						Functions::CastSpellByName("Charge");
					}
					else if (IsFacing && !targetStunned && Functions::UnitIsCaster("target") && Functions::IsSpellReady("Shield Bash")) {
						//Shield Bash (Caster)
						Functions::CastSpellByName("Shield Bash");
					}
					else if (targetPlayer && !HamstringDebuff && Functions::IsSpellReady("Hamstring")) {
						//Hamstring (PvP)
						Functions::CastSpellByName("Hamstring");
					}
					else if ((nbrCloseEnemy >= 3) && Functions::IsSpellReady("Thunder Clap")) {
						//Thunder Clap
						Functions::CastSpellByName("Thunder Clap");
					}
					else if (Functions::IsSpellReady("Execute")) {
						//Execute
						Functions::CastSpellByName("Execute");
					}
					else if (Functions::IsSpellReady("Overpower")) {
						//Overpower
						Functions::CastSpellByName("Overpower");
					}
					else if (Functions::IsSpellReady("Defensive Stance")) {
						//Defensive Stance
						Functions::CastSpellByName("Defensive Stance");
					}
				}
				else if (DefensiveStance) {
					int ShieldBlockIDs[1] = { 2565 }; bool ShieldBlockBuff = localPlayer->hasBuff(ShieldBlockIDs, 1);
					int nbrAggroParty = 0; for (int i = 1; i < NumGroupMembers; i++) { nbrAggroParty += HasAggro[i].size(); }
					if (!Combat && (distTarget > 5.0f) && (Functions::GetSpellCooldownDuration("Charge") < 1.0f) && Functions::IsSpellReady("Battle Stance")) {
						//Battle Stance
						Functions::CastSpellByName("Battle Stance");
					}
					else if ((nbrAggroParty >= 4) && Functions::IsSpellReady("Challenging Shout")) {
						//Challenging Shout
						Functions::CastSpellByName("Challenging Shout");
					}
					else if (!hasTargetAggro && !targetPlayer && Functions::IsSpellReady("Taunt")) {
						//Taunt
						Functions::CastSpellByName("Taunt");
					}
					else if ((localPlayer->rage < 25) && Functions::IsSpellReady("Charge")) {
						//Bloodrage
						Functions::CastSpellByName("Charge");
					}
					else if (IsFacing && !targetStunned && Functions::UnitIsCaster("target") && Functions::IsSpellReady("Shield Bash")) {
						//Shield Bash (Caster)
						Functions::CastSpellByName("Shield Bash");
					}
					else if ((nbrCloseEnemyFacing >= 1) && !ShieldBlockBuff && Functions::IsSpellReady("Shield Block")) {
						//Shield Block
						Functions::CastSpellByName("Shield Block");
					}
					else if (Functions::IsSpellReady("Revenge")) {
						//Revenge
						Functions::CastSpellByName("Revenge");
					}
					else if (!targetStunned && !targetConfused && Functions::IsSpellReady("Concussion Blow")) {
						//Concussion Blow
						Functions::CastSpellByName("Concussion Blow");
					}
					else if (Functions::IsSpellReady("Shield Slam")) {
						//Shield Slam
						Functions::CastSpellByName("Shield Slam");
					}
					else if ((localPlayer->rage >= 25) && (nbrCloseEnemyFacing >= 2) && Functions::IsSpellReady("Cleave")) {
						//Cleave (dump excessive rage)
						Functions::CastSpellByName("Cleave");
					}
					else if (Functions::IsSpellReady("Sunder Armor")) {
						//Sunder Armor (threat generator)
						Functions::CastSpellByName("Sunder Armor");
					}
					else if ((localPlayer->rage >= 25) && Functions::IsSpellReady("Heroic Strike")) {
						//Heroic Strike (dump excessive rage)
						Functions::CastSpellByName("Heroic Strike");
					}
				}
				else if (BerserkerStance) {
					int HamstringIDs[3] = { 1715, 7372, 7373 };
					bool HamstringDebuff = targetUnit->hasDebuff(HamstringIDs, 3);
					if ((distTarget < 25.0f) && Functions::IsSpellReady("Intercept")) {
						//Intercept
						Functions::CastSpellByName("Intercept");
					}
					else if ((localPlayer->rage < 25) && Functions::IsSpellReady("Charge")) {
						//Bloodrage
						Functions::CastSpellByName("Charge");
					}
					else if (Functions::IsSpellReady("Berserker Rage")) {
						//Berserker Rage
						Functions::CastSpellByName("Berserker Rage");
					}
					else if (IsFacing && !targetStunned && Functions::UnitIsCaster("target") && Functions::IsSpellReady("Pummel")) {
						//Pummel (Caster)
						Functions::CastSpellByName("Pummel");
					}
					else if (targetPlayer && !HamstringDebuff && Functions::IsSpellReady("Hamstring")) {
						//Hamstring (PvP)
						Functions::CastSpellByName("Hamstring");
					}
					else if ((nbrCloseEnemy >= 3) && Functions::IsSpellReady("Whirlwind")) {
						//Whirlwind
						Functions::CastSpellByName("Whirlwind");
					}
					else if (Functions::IsSpellReady("Execute")) {
						//Execute
						Functions::CastSpellByName("Execute");
					}
					else if(Functions::IsSpellReady("Defensive Stance")) {
						//Defensive Stance
						Functions::CastSpellByName("Defensive Stance");
					}
				}
			}
		});
	}
}