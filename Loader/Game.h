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
extern bool Combat, IsSitting, bossFight, IsInGroup, IsFacing, hasTargetAggro, tankAutoFocus, tankAutoMove,
	keyTarget, keyHearthstone, keyMount, los_target, obstacle_front, obstacle_back, passiveGroup;
extern float distTarget, halfPI;
extern WoWUnit* targetUnit;
extern int GroupMembersIndex[40];
extern std::vector<unsigned long long> HasAggro[40];
extern std::vector<int> HealTargetArray;
extern int AoEHeal, nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer, Moving, NumGroupMembers, playerSpec, positionCircle, hasDrink, leaderIndex;
extern unsigned int LastTarget;
extern std::string tarType, playerClass, tankName, meleeName, leaderName;
extern WoWUnit* ccTarget;
extern time_t current_time;

#endif