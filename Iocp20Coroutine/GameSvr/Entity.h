#pragma once
#include "../CoRoutine/CoTask.h"
#include <functional>
#include "MyMsgQueue.h"
#include "SpEntity.h"
#include "��λ.h"
#include "../proxy/proxy.h"

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
class �泲Component;
class ����Ŀ��Component;
class ������λComponent;
class ������λ����Component;
class ҽ�Ʊ�Component;
class ��Ŀ���߹�ȥComponent;
class BuffComponent;
class ̦��Component;
class ̦������Component;
class ��̦���ͳ�����ѪComponent;
class ̫��Component;

PRO_DEF_MEM_DISPATCH(EntityDestroy, OnEntityDestroy);
struct Component : pro::facade_builder
	::add_convention<EntityDestroy, void(bool)>
	::build {};


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
	void OnLoad();
	void BroadcastLeave();
	const std::string& ͷ��Name()const;
	void BroadcastEnter();
	void BroadcastNotifyPos();
	void BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop = true);
	void DelayDelete(const std::chrono::system_clock::duration& dura = std::chrono::seconds(3));
	template<class T> void Broadcast(const T& msg);
	bool IsEnemy(const Entity& refEntity);
	const Position& Pos()const { return m_Pos; }
	void SetPos(const Position& refNewPos);
	enum FindType
	{
		����,
		�з�,
		�ѷ�,
	};
	WpEntity Get�����Entity֧�ֵر��еĵ�λ(FindType bFindEnemy, std::function<bool(const Entity&)> fun��������);
	WpEntity Get�����Entity(FindType bFindEnemy, std::function<bool(const Entity&)> fun��������);
	WpEntity Get�����Entity(FindType findType);
	WpEntity Get�����Entity(const FindType bFindEnemy, const ��λ���� Ŀ������);
	template<class T>
	bool AddComponentOnDestroy(std::unique_ptr<T> Entity::* pMem, T* pNew)
	{
		CHECK_RET_FALSE(pNew);
		const std::string str������� = typeid(T).name();

		std::unique_ptr<T>& up��Ա = this->*pMem;
		if (up��Ա)
		{
			LOG(ERROR) << "�����ظ���" << str�������;
			_ASSERT(!"�����ظ������");
			return false;// *refEntity.m_upAttack;
		}
		up��Ա.reset(pNew);
		CHECK_RET_FALSE(m_mapComponentOnEntityDstroy.end() == m_mapComponentOnEntityDstroy.find(str�������));
		//pro::proxy<Component> pro = pro::make_proxy<Component>(up��Ա.get());
		m_mapComponentOnEntityDstroy.insert({ str�������, pNew });
		return true;
	}

	int m_eulerAnglesY = 0;
	//CoTask<int> m_coWaitDelete;
	FunCancel m_cancelDelete;

	const uint64_t Id;
	bool m_bNeedDelete = false;
	float ��������()const;
	float �������()const;

	const ��λ���� m_����;
	��λ::��λ���� m_����;

	//��̬ECS��û�л���ǿת����
	std::unique_ptr<PlayerComponent> m_upPlayer;
	std::unique_ptr<PlayerNickNameComponent> m_upPlayerNickName;
	std::unique_ptr<AttackComponent> m_upAttack;
	std::unique_ptr<DefenceComponent> m_upDefence;
	//std::unique_ptr<MonsterComponent> m_spMonster;
	std::unique_ptr<BuildingComponent> m_upBuilding;
	std::unique_ptr<�ɼ�Component> m_up�ɼ�;
	std::unique_ptr<��ԴComponent> m_up��Դ;
	std::unique_ptr<�ر�Component> m_up�ر�;
	std::unique_ptr<��Component> m_up��;
	std::unique_ptr<��ʱ�赲Component> m_up��ʱ�赲;
	std::unique_ptr<����λComponent> m_up����λ;
	std::unique_ptr<�콨��Component> m_up�콨��;
	std::unique_ptr<�泲Component> m_up�泲;
	std::unique_ptr<AoiComponent> m_upAoi;
	std::unique_ptr<����Ŀ��Component> m_up����Ŀ��;
	std::unique_ptr<������λComponent> m_up������λ;
	std::unique_ptr<������λ����Component> m_up������λ����;
	std::unique_ptr<ҽ�Ʊ�Component> m_upҽ�Ʊ�;
	std::unique_ptr<��Ŀ���߹�ȥComponent> m_up��Ŀ���߹�ȥ;
	std::unique_ptr<BuffComponent> m_upBuff;
	std::unique_ptr<̦��Component> m_up̦��;
	std::unique_ptr<̦������Component> m_up̦������;
	std::unique_ptr<��̦���ͳ�����ѪComponent> m_up��̦���ͳ�����Ѫ;
	std::unique_ptr<̫��Component> m_up̫��;


	/// <summary>
	/// �ر��������
	/// </summary>
	WpEntity m_wpOwner;
	//private:
	Space& m_refSpace;
private:
	CoTaskBool CoDelayDelete(const std::chrono::system_clock::duration& dura = std::chrono::seconds(3));
	Position m_Pos;
	std::map<std::string, pro::proxy<Component> > m_mapComponentOnEntityDstroy;

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

