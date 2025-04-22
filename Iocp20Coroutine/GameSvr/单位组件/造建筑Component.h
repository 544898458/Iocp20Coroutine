#pragma once
#include <unordered_set>
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
#include "SpPlayerComponent.h"

class PlayerGateSession_Game;
class Entity;
enum 单位类型;
enum 单位类型;
struct Position;
class Space;
namespace 单位 
{
	struct 建筑单位配置;
}
/// <summary>
/// 此单位可以造建筑（工程车、工蜂、探机）
/// </summary>
class 造建筑Component final
{
public:
	static void AddComponent(Entity& refEntity);
	static bool 正在建造(const Entity& refEntity);
	static void 根据建筑类型AddComponent(Space& refSpace, const 单位类型 类型, Entity& refNewEntity, UpPlayerComponent& spPlayer, const std::string& strPlayerNickName);
	static WpEntity 创建建筑(Space& refSpace, const Position& pos, const 单位类型 类型, UpPlayerComponent& spPlayer, const std::string& strPlayerNickName);
	造建筑Component(Entity& refEntity);
	~造建筑Component();
	bool 可造(const 单位类型 类型) const;
	void 造建筑(const Position refPos, const 单位类型 类型);
	void OnEntityDestroy(const bool bDestroy);
private:
	CoTaskBool Co造建筑(const Position refPos, const 单位类型 类型);
	CoTaskBool Co建造过程(WpEntity wpEntity建筑, FunCancel& cancel);
	WpEntity AddBuilding(const 单位类型 类型, const Position pos);
	bool 此处有苔蔓吗(const Position pos);
	
	std::unordered_set<单位类型> m_set可造类型;
	FunCancel m_cancel造建筑;
	Entity& m_refEntity;
};

