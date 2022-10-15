#ifndef ListAI_H
#define ListAI_H
#include "MemoryManager.h"
#include "WoWObject.h"
#include "Functions.h"
#include "Game.h"

class ListAI {
public:
	static void HunterDps();
	static void MageDps();
	static void PaladinHeal();
	static void PaladinTank();
	static void PaladinDps();
	static void RogueDps();
	static void WarlockDps();
};

#endif