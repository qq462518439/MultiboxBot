#ifndef Functions_H
#define Functions_H
#include <cstdint>
#include "WoWObject.h"

class Functions {
	public:
		static void pressKey(unsigned int key);
		static void releaseKey(unsigned int key);
		static bool Intersect(Position start, Position end, int height);
		static float GetDepth(Position pos, int height);
		static Position ProjectPos(Position pos, int height);
		static unsigned long GetPlayerGuid();
		static void EnumerateVisibleObjects(int filter);
		static uintptr_t GetObjectPtr(unsigned long long guid);
		static void LuaCall(const char* code);
		static uintptr_t GetText(const char* varName);
		static void ClickAOE(Position tpos);
		static std::tuple<Position, int> getAOETargetPos(float range, float range2);
		static void ClassifyHeal();
		static std::tuple<int, int, int, int> countEnemies();
		static int getNbrCreatureType(int range, CreatureType type1, CreatureType type2=Null, CreatureType type3=Null);
		static int GetBuffKey(int* IDs, int size);
		static bool PlayerIsRanged();
		static void MoveLoS(Position target_pos);
		static void FollowMultibox(int ranged = 0, int placement = 0);
			// === Lua Game Functions === //
		static int GetNumGroupMembers();
		static bool IsInGroup();
		static bool IsInRaid();
		static std::string GetTank();
		static int GetMerchantNumItems();
		static int GetRepairAllCost();
		static bool HasPetUI(); static int GetPetHappiness();
		// === Timer/CD === //
		static float GetTime();
		static float GetItemCooldownDuration(int item_id);
		static float GetItemCooldownDuration(int* items_id, int size);
		static float GetActionCooldownDuration(int slot);
		static float GetSpellCooldownDuration(std::string spell_name);
		// === Items === //
		static int GetContainerNumSlots(int slot);
		static std::tuple<std::string, int> GetContainerItemInfo(int bag, int slot);
		static std::string GetContainerItemLink(int bag, int slot);
		static bool IsInventoryFull();
		static int GetItemCount(std::string item_info);
		static int GetItemCount(int item_info);
		static int HasItem(int* item_id, int size);
		static void PickupItem(std::string item_info);
		static void PickupItem(int item_id);
		static void PlaceItem(int slot, std::string item_info);
		static void PlaceItem(int slot, int item_info);
		static void UseItem(std::string itemName);
		static void UseItem(int item_id);
		static int HasDrink(); static int HasMeat();
		static bool HasHPotion(); static bool HasMPotion(); static bool HasHealthstone();
		static float GetHPotionCD(); static float GetMPotionCD(); static float GetHealthstoneCD();
		static void SellUselessItems(); static int GetItemQuality(int bag, int slot);
		// === Buffs / Debuffs === //
		static std::string UnitBuff(std::string target, int index);
		static std::tuple<std::string, int, std::string> UnitDebuff(std::string target, int index);
		static bool GetUnitBuff(std::string target, std::string texture);
		static bool GetUnitDebuff(std::string target, std::string texture);
		static int GetStackDebuff(std::string target, std::string texture);
		static bool GetUnitDispel(std::string target, std::string dispellType1, std::string dispellType2="Null", std::string dispellType3="Null");
		static int GetDispelKey(std::string dispellType1, std::string dispellType2="Null", std::string dispellType3="Null");
		// === Status === //
		static bool IsSlowed(std::string target);
		static bool IsRooted(std::string target);
		static bool IsFeared(std::string target);
		static bool IsCharmed(std::string target);
		// === Spells/Actions === //
		static bool GetShapeshiftFormInfo(int nbr);
		static int GetNumSpellTabs();
		static std::string GetSpellName(int id);
		static std::string GetSpellTexture(int spellID);
		static std::tuple<std::string, std::string, int, int> GetSpellTabInfo(int index);
		static std::tuple<int, int> GetSpellID(std::string spell_name);
		static bool IsPlayerSpell(std::string spell_name);
		static bool IsSpellReady(std::string spell_name);
		static void CastSpellByName(std::string spell_name);
		static void UseAction(int slot, int self=0);
		static bool IsAutoRepeatAction(int slot);
		static bool IsUsableAction(int slot);
		static bool HasAction(int slot);
		static std::string GetActionTexture(int slot);
		static bool IsConsumableAction(int slot);
		static bool IsActionInRange(int slot);
		static int GetSlot(std::string spell_name, std::string slot_type="SPELL");
		// === Unit === //
		static int GetHealer();
		static int UnitStat(std::string target, int nbr);
		static void TargetUnit(std::string target);
		static std::string UnitName(std::string target);
		static bool UnitCanAttack(std::string char1, std::string char2);
		static bool UnitIsDeadOrGhost(std::string char1);
		static bool CheckInteractDistance(std::string char1, int dist);
		static bool UnitAffectingCombat(std::string target);
		static std::string UnitClass(std::string target);
		static bool UnitIsCaster(std::string target);
		static bool UnitIsElite(std::string target);
		static std::string UnitCreatureType(std::string target);
		static bool IsGroupInCombat();
		static int GetGroupDead(int mode = 0);
		static bool IsShieldEquipped();
		static bool HasWandEquipped();
		static int GetComboPoints();
		static int GetTalentInfo(int page, int index);
		static bool IsCurrentAction(int slot);
		static float UnitAttackSpeed(std::string target);
		static void FollowUnit(std::string target);

	private:
		const static uintptr_t OBJECT_TYPE_OFFSET = 0x14;
		const static uintptr_t GET_PLAYER_GUID_FUN_PTR = 0x00468550;
		const static uintptr_t ENUMERATE_VISIBLE_OBJECTS_FUN_PTR = 0x00468380;
		const static uintptr_t GET_OBJECT_PTR_FUN_PTR = 0x00464870;
		const static uintptr_t LUA_CALL_FUN_PTR = 0x00704CD0;
		const static uintptr_t LUA_GET_TEXT_FUN_PTR = 0x00703bf0;
		const static uintptr_t SPELL_C_HANDLETERRAINCLICK_FUN_PTR = 0x006E60F0;
		const static uintptr_t INTERSECT_FUN = 0x00672170;

		static int Callback(unsigned long long guid, int filter);
};

#endif