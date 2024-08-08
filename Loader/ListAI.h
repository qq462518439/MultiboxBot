#ifndef ListAI_H
#define ListAI_H
#include "MemoryManager.h"
#include "WoWObject.h"
#include "FunctionsLua.h"
#include "Game.h"

class ListAI {
public:
	static void DruidBalance();
	static void DruidHeal();
	static void HunterDps();
	static void MageDps();
	static void PaladinHeal();
	static void PaladinTank();
	static void PaladinDps();
	static void PriestHeal();
	static void RogueDps();
	static void WarlockDps();
	static void WarriorTank();
};

#endif