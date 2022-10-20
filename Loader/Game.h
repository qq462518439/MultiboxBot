#ifndef Game_H
#define Game_H
#include <vector>
#include <string>

#include "WoWObject.h"

class Game {
public:
	static void MainLoop();
};

//Global Variables
extern bool Combat, IsSitting, bossFight, IsInGroup, IsFacing, hasTargetAggro;
extern float distTarget;
extern WoWUnit* targetUnit;
extern int GroupMembersIndex[40];
extern std::vector<unsigned long long> HasAggro[40];
extern std::vector<int> listIndexCloseEnemies; extern std::vector<int> HealTargetArray;
extern int AoEHeal, nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer, Moving, NumGroupMembers, playerSpec, tankIndex;
extern std::string tarType, playerClass, tankName, meleeName;
extern WoWUnit* ccTarget;

#endif