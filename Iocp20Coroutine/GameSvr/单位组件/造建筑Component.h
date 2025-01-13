#pragma once
#include <unordered_set>
#include "../CoRoutine/CoTask.h"
#include "SpEntity.h"

class PlayerGateSession_Game;
class Entity;
enum ��λ����;
enum ��λ����;
struct Position;
class Space;
class PlayerComponent;
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
	static void AddComponent(Entity& refEntity, const ��λ���� ����);
	static bool ���ڽ���(Entity& refEntity);
	static void ���ݽ�������AddComponent(Space& refSpace, const ��λ���� ����, Entity& refNewEntity, std::shared_ptr<PlayerComponent> spPlayer,
		const std::string& strNickName, const ��λ::������λ����& ����);
	�콨��Component(Entity& refEntity, const ��λ���� ����);
	~�콨��Component();
	bool ����(const ��λ���� ����) const;
	CoTaskBool Co�콨��(const Position refPos, const ��λ���� ����);
	void TryCancel();
private:
	CoTaskBool Co�������(WpEntity wpEntity����, FunCancel& cancel);
	CoTask<SpEntity> CoAddBuilding(const ��λ���� ����, const Position pos);
	
	std::unordered_set<��λ����> m_set��������;
	FunCancel m_cancel�콨��;
	Entity& m_refEntity;
};

