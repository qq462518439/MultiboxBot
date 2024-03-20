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
extern bool Combat, IsSitting, IsInGroup, IsFacing, hasTargetAggro, tankAutoFocus, tankAutoMove,
	keyTarget, keyHearthstone, keyMount, los_target, obstacle_front, obstacle_back, passiveGroup, petAttacking,
	enemy_close;
extern float distTarget, halfPI;
extern std::vector<WoWUnit *> HasAggro[40];
extern std::vector<int> HealTargetArray;
extern int AoEHeal, nbrEnemy, nbrCloseEnemy, nbrCloseEnemyFacing, nbrEnemyPlayer, Moving, NumGroupMembers, playerSpec, positionCircle, hasDrink;
extern unsigned int LastTarget;
extern std::string tarType, playerClass;
extern std::vector<std::tuple<std::string, int, int>> leaderInfos;
extern WoWUnit* ccTarget; extern WoWUnit* targetUnit; extern WoWUnit* GroupMember[40]; extern WoWUnit* Leader;
extern time_t current_time;
extern std::vector<Position> path;
#endif