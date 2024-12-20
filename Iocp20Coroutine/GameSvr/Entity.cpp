#include "pch.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "GameSvr.h"
#include "AiCo.h"
#include <cmath>
#include "PlayerComponent.h"
#include "AiCo.h"
#include "../IocpNetwork/StrConv.h"
#include "MonsterComponent.h"
#include "AttackComponent.h"
#include "BuildingComponent.h"
#include "PlayerGateSession_Game.h"
#include "�ɼ�Component.h"
#include "����λComponent.h"
#include "DefenceComponent.h"
#include "�ر�Component.h"
#include "��Component.h"
#include "�콨��Component.h"

using namespace std;

Entity::Entity(const Position& pos, Space& space, const std::string& strPrefabName, const std::string& strEntityName) :
	Id((uint64_t)this), m_strPrefabName(strPrefabName), m_refSpace(space), m_Pos(pos), m_strEntityName(strEntityName)
{
}

bool Entity::DistanceLessEqual(const Entity& refEntity, float fDistance)
{
	return this->m_Pos.DistanceLessEqual(refEntity.m_Pos, fDistance);
}

float Entity::DistancePow2(const Entity& refEntity)const
{
	return this->m_Pos.DistancePow2(refEntity.m_Pos);
}
float Entity::Distance(const Entity& refEntity)const
{
	return std::sqrtf(DistancePow2(refEntity));
}

bool Entity::IsDead() const
{
	if (!m_spDefence)
		return false;
	return m_spDefence->IsDead();
}

Entity::~Entity()
{
	LOG(INFO) << Id;
}

//���̵߳��߳�ִ��
void Entity::Update()
{
	if (m_spAttack)
		m_spAttack->Update();

	if (m_sp�ر�)
		m_sp�ر�->Update();
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_spPlayer && !refEntity.m_spPlayer)
		return false;//���ǹ�

	if (!m_spPlayer || !refEntity.m_spPlayer)
		return true;//��һ���ǹ�

	//������ҵ�λ
	return &m_spPlayer->m_refSession != &refEntity.m_spPlayer->m_refSession;
}

float Entity::��������() const
{
	if (m_spAttack)
		return m_spAttack->m_f��������;

	return 0;
}

void Entity::OnDestroy()
{
	BroadcastLeave();

	//Ӧ����proxy��ͬ���������ģ��������
	if (m_spAttack)
		m_spAttack->TryCancel();

	if (m_sp����λ)
		m_sp����λ->TryCancel(*this);

	if (m_sp�ɼ�)
		m_sp�ɼ�->m_TaskCancel.TryCancel();

	if (m_sp��)
		m_sp��->TryCancel();

	if (m_sp�ر�)
		m_sp�ر�->OnDestroy();

	if (m_sp��ʱ�赲)
		m_sp��ʱ�赲.reset();

	if (m_sp�콨��)
		m_sp�콨��->TryCancel();

	if (m_cancelDelete)
	{
		LOG(INFO) << "ȡ��ɾ��Э��";
		m_cancelDelete();
	}
	else
	{
		LOG(INFO) << "û��ɾ��Э��";
	}
}

void Entity::BroadcastLeave()
{
	LOG(INFO) << NickName() << "����Entity::BroadcastLeave," << Id;
	Broadcast(MsgDelRoleRet(Id));
}

const std::string& Entity::NickName()
{
	if (m_spPlayer)
		return m_spPlayer->m_refSession.NickName();

	if (m_sp��Դ)
	{
		static const std::string str("��Դ");
		return str;
	}

	{
		static const std::string str("����");
		return str;
	}
}


void Entity::BroadcastEnter()
{
	LOG(INFO) << NickName() << "����Entity::BroadcastEnter," << Id;
	Broadcast(MsgAddRoleRet(*this));//�Լ��㲥������
	BroadcastNotifyPos();
	CoEvent<MyEvent::AddEntity>::OnRecvEvent(false, { weak_from_this() });
}

void Entity::BroadcastNotifyPos()
{
	Broadcast(MsgNotifyPos(*this));
}

void Entity::BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop)
{
	Broadcast(MsgChangeSkeleAnim(*this, refAniClipName, loop));//������������
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	m_refSpace.Broadcast(msg);
}

CoTaskBool Entity::CoDelayDelete()
{
	LOG(INFO) << "��ʼɾ����ʱ�Լ���Э��";
	//assert(!m_cancelDelete);//���ɲ���
	if (m_cancelDelete)
		co_return false;

	if (m_sp�ر�)
		m_sp�ر�->OnBeforeDelayDelete();

	using namespace std;
	if (co_await CoTimer::Wait(3s, m_cancelDelete))//�������������̴߳�ѭ����ÿ��ѭ������һ��
	{
		LOG(INFO) << "WaitDeleteЭ��ȡ����";
		co_return true;
	}

	m_bNeedDelete = true;
	co_return false;
}

template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);
template void Entity::Broadcast(const MsgEntity����& msg);
template void Entity::Broadcast(const Msg��������& msg);