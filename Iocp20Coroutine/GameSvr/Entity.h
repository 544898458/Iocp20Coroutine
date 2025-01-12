#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "��λ.h"

//#include "PlayerComponent.h"
class Space;
class GameSvrSession;
class GameSvr;
class PlayerComponent;
class PlayerNickNameComponent;
class MonsterComponent;
class BuildingComponent;
class AttackComponent;
class DefenceComponent;
class �ɼ�Component;
class ��ԴComponent;
class �ر�Component;
class ��Component;
class ��ʱ�赲Component;
class ����λComponent;
class �콨��Component;
class AoiComponent;
class PlayerGateSession_Game;


class Entity final : public std::enable_shared_from_this<Entity>//���빫�м̳У�������Ч
{
public:
	Entity(const Position& pos, Space& refSpace, ��λ���� ����, const ��λ::��λ����& ref����);
	Entity(const Entity&) = delete;
	~Entity();
	void Save(std::ofstream& refOf);
	static bool Load(Space& refSpace, char(&buf)[1024], const uint16_t u16Size);
	void Update();
	bool IsDead()const;
	bool NeedDelete()const { return m_bNeedDelete; }
	bool DistanceLessEqual(const Entity& refEntity, float fDistance);
	float DistancePow2(const Entity& refEntity)const;
	float Distance(const Entity& refEntity)const;
	void OnDestroy();
	void BroadcastLeave();
	const std::string& NickName();
	void BroadcastEnter();
	void BroadcastNotifyPos();
	void BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop = true);
	CoTaskBool CoDelayDelete();
	template<class T> void Broadcast(const T& msg);
	bool IsEnemy(const Entity& refEntity);
	const Position& Pos()const { return m_Pos; }
	void SetPos(const Position& refNewPos);
	int m_eulerAnglesY = 0;
	//CoTask<int> m_coWaitDelete;
	FunCancel m_cancelDelete;

	const uint64_t Id;
	bool m_bNeedDelete = false;
	float m_�ٶ�ÿ֡�ƶ����� = 0.5f;//ÿ֡������
	float ��������()const;
	float �������()const;

	const ��λ���� m_����;
	��λ::��λ���� m_����;

	//��̬ECS��û�л���ǿת����
	std::shared_ptr<PlayerComponent> m_spPlayer;
	std::shared_ptr<PlayerNickNameComponent> m_spPlayerNickName;
	std::shared_ptr<AttackComponent> m_spAttack;
	std::shared_ptr<DefenceComponent> m_spDefence;
	std::shared_ptr<MonsterComponent> m_spMonster;
	std::shared_ptr<BuildingComponent> m_spBuilding;
	std::shared_ptr<�ɼ�Component> m_sp�ɼ�;
	std::shared_ptr<��ԴComponent> m_sp��Դ;
	std::shared_ptr<�ر�Component> m_sp�ر�;
	std::shared_ptr<��Component> m_sp��;
	std::shared_ptr<��ʱ�赲Component> m_sp��ʱ�赲;
	std::shared_ptr<����λComponent> m_sp����λ;
	std::shared_ptr<�콨��Component> m_sp�콨��;
	std::unique_ptr<AoiComponent> m_upAoi;
	/// <summary>
	/// �ر��������
	/// </summary>
	WpEntity m_wpOwner;
	//private:
	Space& m_refSpace;
private:
	Position m_Pos;

};

//x,y��������ϵ�е����꣬�磨4��4���𰸾���45��
inline int CalculateAngle(float x, float y)
{
	auto angleRad = std::atan2(x, y); // ���㻡��
	auto angleDeg = angleRad * 180.0f / 3.14159265f; // ������ת��Ϊ�Ƕ�
	return (int)angleDeg; // ���ؽǶ�
}

inline int CalculateAngle(Position from, Position to)
{
	return CalculateAngle(to.x - from.x, to.z - from.z);
}

