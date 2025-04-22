#pragma once
#include <unordered_set>
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"
#include "SpPlayerComponent.h"

class PlayerGateSession_Game;
class Entity;
enum ��λ����;
enum ��λ����;
struct Position;
class Space;
namespace ��λ 
{
	struct ������λ����;
}
/// <summary>
/// �˵�λ�����콨�������̳������䡢̽����
/// </summary>
class �콨��Component final
{
public:
	static void AddComponent(Entity& refEntity);
	static bool ���ڽ���(const Entity& refEntity);
	static void ���ݽ�������AddComponent(Space& refSpace, const ��λ���� ����, Entity& refNewEntity, UpPlayerComponent& spPlayer, const std::string& strPlayerNickName);
	static WpEntity ��������(Space& refSpace, const Position& pos, const ��λ���� ����, UpPlayerComponent& spPlayer, const std::string& strPlayerNickName);
	�콨��Component(Entity& refEntity);
	~�콨��Component();
	bool ����(const ��λ���� ����) const;
	void �콨��(const Position refPos, const ��λ���� ����);
	void OnEntityDestroy(const bool bDestroy);
private:
	CoTaskBool Co�콨��(const Position refPos, const ��λ���� ����);
	CoTaskBool Co�������(WpEntity wpEntity����, FunCancel& cancel);
	WpEntity AddBuilding(const ��λ���� ����, const Position pos);
	bool �˴���̦����(const Position pos);
	
	std::unordered_set<��λ����> m_set��������;
	FunCancel m_cancel�콨��;
	Entity& m_refEntity;
};

