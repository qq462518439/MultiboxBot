#include "Functions.h"

#include <iostream>

#include "MemoryManager.h"
#include "Game.h"
#include "FunctionsLua.h"

// Use them OUTSIDE main thread !
void Functions::pressKey(unsigned int key) {
	SendMessageW(ThreadSynchronizer::windowHandle, WM_KEYDOWN, key, 0);
}

void Functions::releaseKey(unsigned int key) {
	SendMessageW(ThreadSynchronizer::windowHandle, WM_KEYUP, key, 0);
}

bool Functions::Intersect(Position start, Position end, float height, float height2) {
	//Need height variable because LOS is based on the position of the eyes of the char
	if (height2 == 0.0f) height2 = height;
	typedef bool __fastcall func(Position* p1, Position* p2, int ignore, Position* intersection, float* distance, unsigned int flags);
	func* function = (func*)INTERSECT_FUN_PTR;
	Position p1 = Position(start.X, start.Y, start.Z+height);
	Position p2 = Position(end.X, end.Y, end.Z+height2);
	Position intersection = Position(0, 0, 0);
	float distance = float(start.DistanceTo(end));
	bool result = function(&p1, &p2, 0, &intersection, &distance, 0x00100111);
	return result;
}

float Functions::GetDepth(Position pos, float height) {
	typedef bool __fastcall func(Position* p1, Position* p2, int ignore, Position* intersection, float* distance, unsigned int flags);
	func* function = (func*)INTERSECT_FUN_PTR;
	Position p1 = Position(pos.X, pos.Y, pos.Z+height);
	Position p2 = Position(pos.X, pos.Y, pos.Z - 100);
	Position intersection = Position(0, 0, 0);
	float distance = float(pos.DistanceTo(p2));
	bool res = function(&p1, &p2, 0, &intersection, &distance, 0x00100111);
	float result = pos.DistanceTo(intersection);
	return result;
}

Position Functions::ProjectPos(Position pos, float height) {
	typedef bool __fastcall func(Position* p1, Position* p2, int ignore, Position* intersection, float* distance, unsigned int flags);
	func* function = (func*)INTERSECT_FUN_PTR;
	Position p1 = Position(pos.X, pos.Y, pos.Z+height);
	Position p2 = Position(pos.X, pos.Y, pos.Z - 100);
	Position intersection = Position(0, 0, 0);
	float distance = float(pos.DistanceTo(p2));
	bool res = function(&p1, &p2, 0, &intersection, &distance, 0x00100111);
	if (res) {
		Position result = Position(pos.X, pos.Y, intersection.Z);
		return result;
	}
	else return pos;
}

unsigned long Functions::GetPlayerGuid() {
	typedef unsigned long func();
	func* function = (func*)GET_PLAYER_GUID_FUN_PTR;
	unsigned long guid = function();
	return guid;
}

int Functions::GetPositionCircle() {
	int tmp_pos = 1;
	if (localPlayer == NULL || Leader == NULL) return 1;
	for (unsigned int i = 0; i < leaderInfos.size(); i++) {
		if (Leader->name != get<0>(leaderInfos[i])) { //not Leader
			for (unsigned int z = 0; z < ListUnits.size(); z++) {
				if (ListUnits[z].name == get<0>(leaderInfos[i])) {
					if (get<0>(leaderInfos[i]) == localPlayer->name) return tmp_pos;
					tmp_pos++;
					break;
				}
			}
		}
	}
	return 1;
}

void Functions::EnumerateVisibleObjects(int filter) {
	for (int i = 0; i < 40; i++) GroupMember[i] = NULL;
	ListUnits.clear();
	ListGameObjects.clear();
	typedef int (*EnumerateVisibleObjectsCallback)(unsigned long long, int);
	EnumerateVisibleObjectsCallback callback = &Callback;
	void* callbackPtr = (void*&)callback;

	typedef void __fastcall func(uintptr_t, int);
	func* function = (func*)ENUMERATE_VISIBLE_OBJECTS_FUN_PTR;
	try {
		function((uintptr_t)callbackPtr, filter);
	}
	catch (...) {}

	Leader = GetLeader();

	positionCircle = GetPositionCircle();

	for (unsigned int i = 0; i < ListUnits.size(); i++) {
		ListUnits[i].unitReaction = localPlayer->getUnitReaction(ListUnits[i].Pointer);
		ListUnits[i].attackable = localPlayer->canAttack(ListUnits[i].Pointer);
	}
}

uintptr_t Functions::GetObjectPtr(unsigned long long guid) {
	typedef uintptr_t __stdcall func(unsigned long long);
	func* function = (func*)GET_OBJECT_PTR_FUN_PTR;
	uintptr_t ptr = function(guid);
	return ptr;
}

int Functions::Callback(unsigned long long guid, int filter) {
	uintptr_t pointer = Functions::GetObjectPtr(guid);
	if (pointer != 0) {
		ObjectType objectType = (*(ObjectType*)(pointer + OBJECT_TYPE_OFFSET));
		if (objectType == GameObject) {
			WoWGameObject gameObject = WoWGameObject(pointer, guid, objectType);
			ListGameObjects.push_back(gameObject);
		}
		else if (objectType == Unit || objectType == Player) {
			WoWUnit unit = WoWUnit(pointer, guid, objectType);
			ListUnits.push_back(unit);
			if (objectType == Player) {
				for (unsigned int y = 0; y < leaderInfos.size(); y++) {
					if (unit.name == get<0>(leaderInfos[y])) {
						ListUnits.back().role = get<1>(leaderInfos[y]);
						break;
					}
				}
				if (guid == GetPlayerGuid()) {
					if (localPlayer != NULL) {
						delete(localPlayer);
						localPlayer = NULL;
					}
					localPlayer = new LocalPlayer(pointer, guid, objectType);
					GroupMember[0] = &ListUnits.back();
				}
				else {
					for (int i = 1; i <= NumGroupMembers; i++) {
						if (unit.name == FunctionsLua::UnitName(tarType + std::to_string(i))) {
							GroupMember[i] = &ListUnits.back();
							break;
						}
					}
				}
			}
		}
	}
	return 1;
}

void Functions::LuaCall(const char* code) {
	typedef void __fastcall func(const char* code, const char* unused);
	func* function = (func*)LUA_CALL_FUN_PTR;

	function(code, "Unused");
}

uintptr_t Functions::GetText(const char* varName) {
	typedef uintptr_t __fastcall func(const char* varName, unsigned int nonSense, int zero);
	func* f = (func*)LUA_GET_TEXT_FUN_PTR;
	return f(varName, -1, 0);
}

void Functions::ClickAOE(Position position) {
	float xyz[3] = { position.X, position.Y, position.Z };
	typedef void __fastcall func(float*);
	func* function = (func*)SPELL_C_HANDLETERRAINCLICK_FUN_PTR;
	function(xyz);
}

void Functions::LootUnit(uintptr_t target, int autoloot) {
	typedef void(__thiscall* func)(uintptr_t target, int autoloot);
	func function = (func)RIGHT_CLICK_UNIT_FUN_PTR;
	function(target, autoloot);
}

void Functions::InteractObject(uintptr_t object_ptr, int autoloot) {
	typedef void(__thiscall* func)(uintptr_t object_ptr, int autoloot);
	func function = (func)INTERACT_OBJECT_FUN_PTR;
	function(object_ptr, autoloot);
}

void Functions::FollowMultibox(int placement) {
	if (Leader == NULL) return;
	float range = 2.0f; float cst = 0.30f;
	if (placement == 1) cst = 0.30f;
	else if (placement == 2) cst = -0.30f;
	else if (placement == 3) { range = 4.0f; cst = 0.15f; }
	else if (placement == 4) { range = 4.0f; cst = -0.15f; }
	float dist = localPlayer->position.DistanceTo(Leader->position);
	if (dist < 60.0f && dist > range + 1) {
		ThreadSynchronizer::RunOnMainThread([=]() {
			float halfPI = acosf(0);
			Position target_pos = Position((cos(Leader->facing + (halfPI * 2) + cst) * range) + Leader->position.X
				, (sin(Leader->facing + (halfPI * 2) + cst) * range) + Leader->position.Y, Leader->position.Z);
			bool swimming = ((localPlayer->movement_flags & MOVEFLAG_SWIMMING) || (localPlayer->movement_flags & MOVEFLAG_WATERWALKING)
				|| (Leader->movement_flags & MOVEFLAG_SWIMMING) || (Leader->movement_flags & MOVEFLAG_WATERWALKING));
			if (swimming) localPlayer->ClickToMove(Move, Leader->Guid, target_pos);
			else {
				target_pos = Functions::ProjectPos(target_pos, 2.00f);
				Functions::MoveObstacle(target_pos);
			}
		});
	}
}

bool MoveObstacle_tmp(Position target_pos, Position last_pos) {
	float angle_targetPos = atan2f(target_pos.Y - last_pos.Y, target_pos.X - last_pos.X);
	float halfPI = acosf(0);
	if (angle_targetPos < 0.0f) angle_targetPos += halfPI * 4.0f;
	else if (angle_targetPos > halfPI * 4) angle_targetPos -= halfPI * 4.0f;
	Position tmp_pos, next_pos = last_pos;
	while (next_pos.DistanceTo(target_pos) > 2.5f) {
		tmp_pos = Position((cos(angle_targetPos) * 2.5f) + last_pos.X
			, (sin(angle_targetPos) * 2.5f) + last_pos.Y, last_pos.Z);
		next_pos = Functions::ProjectPos(tmp_pos, 2.00f);
		if (Functions::Intersect(last_pos, next_pos, 2.00f) || (Functions::GetDepth(tmp_pos, 2.00f) > 2.00f)) return false;
		last_pos = next_pos;
	}
	return true;
}

bool Functions::MoveObstacle(Position target_pos) {
	float angle_targetPos = atan2f(target_pos.Y - localPlayer->position.Y, target_pos.X - localPlayer->position.X);
	float halfPI = acosf(0);
	if (angle_targetPos < 0.0f) angle_targetPos += halfPI * 4.0f;
	else if (angle_targetPos > halfPI * 4) angle_targetPos -= halfPI * 4.0f;
	int j = 0;
	for (int i = 0; i < 6; i++) { //Front, left, right...
		for (int z = 0; z < 2; z++) {
			if (z == 0) j = 16 - i;
			else {
				if (i == 0) break;
				j = i;
			}
			Position last_pos = localPlayer->position; //Take into account the difference in altitude at each point
			for (int w = 0; w < 12; w++) { //Every 2.5 yards up to 30 check for LoS point
				Position tmp_pos = Position((cos(angle_targetPos + (j * halfPI / 4)) * 2.5f) + last_pos.X
					, (sin(angle_targetPos + (j * halfPI / 4)) * 2.5f) + last_pos.Y, last_pos.Z);
				Position next_pos = Functions::ProjectPos(tmp_pos, 2.00f);
				if (!Functions::Intersect(last_pos, next_pos, 2.00f) && (Functions::GetDepth(tmp_pos, 2.00f) < 2.00f)) {
					if (MoveObstacle_tmp(target_pos, next_pos)) {
						if(i > 0) localPlayer->ClickToMove(Move, localPlayer->Guid, next_pos);
						else localPlayer->ClickToMove(Move, localPlayer->Guid, target_pos);
						return true;
					}
					else { last_pos = next_pos; continue; }
				}
				else break; //There is an obstacle on this path, we need to change
			}
		}
	}
	return false;
}

void Functions::MoveLoS(Position target_pos) {
	/*
		Check every directions for a position where you have line of sight of target_pos,
		if there is an obstacle on the path check a new direction
	*/
	float angle_targetPos = atan2f(target_pos.Y - localPlayer->position.Y, target_pos.X - localPlayer->position.X);
	float halfPI = acosf(0);
	if (angle_targetPos < 0.0f) angle_targetPos += halfPI * 4.0f;
	else if (angle_targetPos > halfPI * 4) angle_targetPos -= halfPI * 4.0f;
	int j = 0;
	for (int i = 1; i < 6; i++) { //Left, right...
		for (int z = 0; z < 2; z++) {
			Position last_pos = localPlayer->position; //Take into account the difference in altitude at each point
			if (z == 0) j = 16 - i; else j = i; //Pendulum direction choice
			for (int w = 0; w < 12; w++) { //Every 2.5 yards up to 30 check for LoS point
				Position tmp_pos = Position((cos(angle_targetPos + (j * halfPI / 4)) * 2.5f) + last_pos.X
					, (sin(angle_targetPos + (j * halfPI / 4)) * 2.5f) + last_pos.Y, last_pos.Z);
				Position next_pos = Functions::ProjectPos(tmp_pos, 2.00f);
				if (!Functions::Intersect(last_pos, next_pos, 2.00f) && (Functions::GetDepth(tmp_pos, 2.00f) < 2.00f)) {
					if (!Functions::Intersect(next_pos, target_pos, 2.00f)) {
						localPlayer->ClickToMove(Move, localPlayer->Guid, next_pos);
						Moving = 6;
						return;
					}
					else { last_pos = next_pos; continue; }
				}
				else break; //There is an obstacle on this path, we need to change
			}
		}
	}
}

Position meanPos(std::vector<Position> posArr) {
	Position clusterCenter = Position(0, 0, 0);
	for (unsigned int i = 0; i < posArr.size(); i++) {
		clusterCenter.X = clusterCenter.X + posArr[i].X;
		clusterCenter.Y = clusterCenter.Y + posArr[i].Y;
		clusterCenter.Z = clusterCenter.Z + posArr[i].Z;
	}
	clusterCenter.X = clusterCenter.X / posArr.size();
	clusterCenter.Y = clusterCenter.Y / posArr.size();
	clusterCenter.Z = clusterCenter.Z / posArr.size();
	return clusterCenter;
}

unsigned int Functions::GetMapID() {
	typedef int __fastcall func();
	func* function = (func*)GETMAPID_FUN_PTR;
	return function();
}

//======================================================================//
//======================   Non-memory Functions   ======================//
//======================================================================//
//(They don't use Lua calls and memory pointers)

void Functions::ClassifyHeal() {
	//Heal all friendly players or NPC within 60 yards
	std::vector<float> PrctHp;
	HealTargetArray.clear();
	AoEHeal = 0;
	for (unsigned int i = 0; i < ListUnits.size(); i++) {
		if ((ListUnits[i].unitReaction > Neutral) && !ListUnits[i].isdead) {
			float dist = localPlayer->position.DistanceTo(ListUnits[i].position);
			if (dist < 60) {
				PrctHp.push_back(ListUnits[i].prctHP);
				if (ListUnits[i].objectType == Player && ListUnits[i].prctHP < 60) AoEHeal = AoEHeal + 1;
				HealTargetArray.push_back(i);
			}
		}
	}
	for (int i = PrctHp.size(); i > 0; i--) {
		for (int y = 0; y < i - 1; y++) {
			if (PrctHp[y] > PrctHp[y + 1]) {
				float tmp = PrctHp[y];
				PrctHp[y] = PrctHp[y + 1];
				PrctHp[y + 1] = tmp;
				int tmp2 = HealTargetArray[y];
				HealTargetArray[y] = HealTargetArray[y + 1];
				HealTargetArray[y + 1] = tmp2;
			}
		}
	}
}

std::tuple<Position, int> Functions::getAOETargetPos(float range, float range2) {
	//Execution: ~0.05ms
	//Hierarchical Clustering implementation to find position
	std::vector<std::vector<Position>> clustersArr;
	std::vector<Position> clusters_center;
	//1- Chaque position est un cluster
	for (unsigned int i = 0; i < ListUnits.size(); i++) {
		if ((ListUnits[i].flags & UNIT_FLAG_IN_COMBAT) && ListUnits[i].attackable && ListUnits[i].speed <= 4.5f) {
			std::vector<Position> cluster;
			cluster.push_back(ListUnits[i].position);
			clustersArr.push_back(cluster);
			clusters_center.push_back(ListUnits[i].position);
		}
	}
	if (clusters_center.size() > 0) {
		//2- On rassemble les clusters les + proche et distance < range/2
		int min_cluster1, min_cluster2;
		do {
			float distMin = INFINITY;
			min_cluster1 = 0; min_cluster2 = 0;
			for (unsigned int i = 0; i < clustersArr.size(); i++) {
				for (unsigned int y = 0; y < clustersArr.size(); y++) {
					if (i != y) {
						float distCluster = clusters_center[i].DistanceTo(clusters_center[y]);
						if (distCluster < distMin && distCluster < (range / 2)) {
							distMin = distCluster;
							min_cluster1 = i;
							min_cluster2 = y;
						}
					}
				}
			}
			if (min_cluster1 != 0 || min_cluster2 != 0) {
				//On ajoute les positions du cluster_min2 dans cluster_min1
				for (unsigned int i = 0; i < clustersArr[min_cluster2].size(); i++) {
					clustersArr[min_cluster1].push_back(clustersArr[min_cluster2][i]);
				}
				clusters_center[min_cluster1] = meanPos(clustersArr[min_cluster1]);
				//On d�truit les valeurs du cluster_min2
				clustersArr.erase(clustersArr.begin() + min_cluster2);
				clusters_center.erase(clusters_center.begin() + min_cluster2);
			}
		} while (min_cluster1 != 0 || min_cluster2 != 0);
		//3- On choisit le cluster avec le plus d'unit�s
		unsigned int max_cluster_unit = 0; int index = 0;
		for (unsigned int i = 0; i < clustersArr.size(); i++) {
			if (clustersArr[i].size() > max_cluster_unit && clusters_center[i].DistanceTo(localPlayer->position) < range2) {
				max_cluster_unit = clustersArr[i].size();
				index = i;
			}
		}
		return std::make_tuple(clusters_center[index], max_cluster_unit);
	}
	else return std::make_tuple(Position(0, 0, 0), 0);
}

std::tuple<int, int, int, int> Functions::countEnemies() {
	for (int i = 0; i < 40; i++) {
		HasAggro[i].clear();
	}
	int nbr = 0, nbrClose = 0, nbrCloseFacing = 0, nbrEnemyPlayer = 0;
	ccTarget = NULL;
	for (unsigned int i = 0; i < ListUnits.size(); i++) {
		if (ListUnits[i].attackable) { //Hostile
			bool aggroed = false;
			for (int y = 0; y <= NumGroupMembers; y++) {
				if ((GroupMember[y] != NULL) && (GroupMember[y]->Guid == ListUnits[i].targetGuid) && (ListUnits[i].flags & UNIT_FLAG_IN_COMBAT)) {
					HasAggro[y].push_back(&ListUnits[i]); //Check if Group Member has aggro
					if(y == 0) aggroed = true;
					break;
				}
			}
			bool groupMember = false;
			if (ListUnits[i].flags & UNIT_FLAG_PLAYER_CONTROLLED) { //Enemy player
				for (int y = 0; y <= NumGroupMembers; y++) {
					if ((GroupMember[y] != NULL) && (GroupMember[y]->Guid == ListUnits[i].Guid)) {
						groupMember = true;
						break;
					}
				}
				if(!groupMember) nbrEnemyPlayer++;
				if((targetUnit != NULL) && (ListUnits[i].Guid != targetUnit->Guid)) {
					float dist = localPlayer->position.DistanceTo(ListUnits[i].position);
					if (dist < 30) ccTarget = &ListUnits[i]; //2nd Target for CC
				}
			}
			if (!groupMember && (ListUnits[i].flags & UNIT_FLAG_IN_COMBAT)) { //Combat
				if(!(ListUnits[i].flags & UNIT_FLAG_CONFUSED)) nbr++;
				if (localPlayer->position.DistanceTo(ListUnits[i].position) < 7.5f) {
					if (!(ListUnits[i].flags & UNIT_FLAG_CONFUSED)) nbrClose++;
					if (localPlayer->isFacing(ListUnits[i].position, 0.5f)) {
						nbrCloseFacing++;
					}
				}
			}
		}
	}
	return std::make_tuple(nbr, nbrClose, nbrCloseFacing, nbrEnemyPlayer);
}

int Functions::getNbrCreatureType(int range, CreatureType type1, CreatureType type2, CreatureType type3) {
	CreatureType args[3] = { type1, type2, type3 };
	int nbr = 0;
	for (unsigned int i = 0; i < ListUnits.size(); i++) {
		float dist = localPlayer->position.DistanceTo(ListUnits[i].position);
		if (dist < range) {
			for (int y = 0; y < 3; y++) {
				if (ListUnits[i].creatureType == args[y]) nbr++;
			}
		}
	}
	return nbr;
}

bool Functions::PlayerIsRanged() {
	if(playerClass == "Mage" || playerClass == "Priest" || playerClass == "Warlock" || playerClass == "Hunter"
		|| (playerClass == "Druid" && (playerSpec == 0 || playerSpec == 3)) || (playerClass == "Shaman" && (playerSpec == 0 || playerSpec == 2))) return true;
	else return false;
}

WoWUnit* Functions::GetGroupDead(int mode) {
	if (mode == 0) {
		for (int i = 1; i <= NumGroupMembers; i++) {
			if ((GroupMember[i] != NULL) && GroupMember[i]->isdead) return GroupMember[i];
		}
	}
	else {
		for (int i = NumGroupMembers; i >= 1; i--) {
			if ((GroupMember[i] != NULL) && GroupMember[i]->isdead) return GroupMember[i];
		}
	}
	return NULL;
}

WoWUnit* Functions::GetLeader() {
	if (localPlayer == NULL) return NULL;
	for (unsigned int y = 0; y < 2; y++) {
		for (unsigned int i = 0; i < leaderInfos.size(); i++) {
			if (get<1>(leaderInfos[i]) == y) {
				for (unsigned int z = 0; z < ListUnits.size(); z++) {
					if (ListUnits[z].name == get<0>(leaderInfos[i]) && localPlayer->position.DistanceTo(ListUnits[z].position) < 60.0f) {
						return &(ListUnits[z]);
					}
				}
			}
		}
	}
	return NULL;
}

WoWUnit* Functions::GetMissingBuff(int* IDs, int size) {
	//Retourne le joueur auquel il manque le buff
	for (int i = 1; i <= NumGroupMembers; i++) {
		if ((GroupMember[i] != NULL) && (GroupMember[i]->unitReaction > Neutral)
			&& !GroupMember[i]->isdead && (localPlayer->position.DistanceTo(GroupMember[i]->position) < 40.0f)
			&& !Functions::Intersect(localPlayer->position, GroupMember[i]->position, 2.00f) && !GroupMember[i]->hasBuff(IDs, size)) return GroupMember[i];
	}
	return NULL;
}