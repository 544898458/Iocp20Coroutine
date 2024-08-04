#include "StdAfx.h"
#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "AiCo.h"
#include <cmath>
#include "PlayerComponent.h"
#include "AiCo.h"
#include "../IocpNetwork/StrConv.h"
#include "MonsterComponent.h"

using namespace std;

Entity::Entity(const Position& pos, Space& space, const std::string& strPrefabName) :
	Id((uint64_t)this), m_strPrefabName(strPrefabName), m_space(space), m_Pos(pos)
{
}

void Entity::WalkToPos(const Position& posTarget, MyServer* pServer)
{
	if (IsDead())
	{
		if (m_spPlayer)
			m_spPlayer->m_pSession->Send(MsgSay(StrConv::GbkToUtf8("�Լ�����,������")));

		return;
	}
	//m_coStop = true;

	TryCancel();

	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk = AiCo::WalkToPos(shared_from_this(), posTarget, pServer, m_cancel);
	m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
}

bool Entity::DistanceLessEqual(const Entity& refEntity, float fDistance)
{
	const float fExponent = 2.0f;
	return std::pow(this->m_Pos.x - refEntity.m_Pos.x, fExponent) + std::pow(this->m_Pos.z - refEntity.m_Pos.z, fExponent) <= std::pow(fDistance, fExponent);
}

void Entity::Hurt(int hp)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;

	this->Broadcast(MsgNotifyPos(*this));
	if (IsDead())
	{
		this->Broadcast(MsgChangeSkeleAnim(*this, "died", false));//������������
		m_coWaitDelete = AiCo::WaitDelete(shared_from_this(), m_cancel);
		m_coWaitDelete.Run();
	}
}

//���̵߳��߳�ִ��
void Entity::Update()
{
	if (!m_coAttack.Finished())
	{
		return;//��ʾ��������
	}
	if (!m_coWalk.Finished())
	{
		return;//��ʾ��������
	}

	if (IsDead())
	{
		return;
	}

	for (const auto& spEntity : m_space.setEntity)
	{
		if (spEntity.get() == this)//���ҵ��ˣ����������˶��ǵ���
			continue;

		if (spEntity->IsDead())
			continue;

		if (!spEntity->IsEnemy(*this))
			continue;

		if (DistanceLessEqual(*spEntity, this->m_f��������))
		{
			TryCancel();

			m_coAttack = AiCo::Attack(this->shared_from_this(), spEntity, m_cancel);
			m_coAttack.Run();
			return;
		}
		else if (DistanceLessEqual(*spEntity, this->m_f�������))
		{
			TryCancel();

			//m_coWalk.Run();
			assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			m_coWalk = AiCo::WalkToTarget(shared_from_this(), spEntity, this->m_space.m_pServer, m_cancel);
			m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
			return;
		}
	}

	if (!m_spPlayer)//��
	{
		TryCancel();
		assert(m_coWalk.Finished());//20240205
		assert(m_coAttack.Finished());//20240205

		auto posTarget = m_Pos;
		posTarget.x += std::rand() % 11 - 5;//�漴��
		posTarget.z += std::rand() % 11 - 5;
		m_coWalk = AiCo::WalkToPos(shared_from_this(), posTarget, this->m_space.m_pServer, m_cancel);
		m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await

	}
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_spPlayer && !refEntity.m_spPlayer)
		return false;//���ǹ�

	if (!m_spPlayer || !refEntity.m_spPlayer)
		return true;//��һ���ǹ�

	//������ҵ�λ
	return m_spPlayer->m_pSession != refEntity.m_spPlayer->m_pSession;
}

void Entity::TryCancel()
{
	if (m_cancel)
	{
		LOG(INFO) << "����m_cancel";
		m_cancel();
	}
	else
	{
		LOG(INFO) << "m_cancel�ǿյģ�û��ȡ����Э��";
		if (!m_coWalk.Finished() || !m_coAttack.Finished() || (m_spMonster && !m_spMonster->m_coIdle.Finished()))
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
		}
	}

	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205

}

void Entity::OnDestroy()
{
	LOG(INFO) << "����Entity::OnDestroy";
	TryCancel();
}

const std::string& Entity::NickName()
{
	if (m_spPlayer)
		return m_spPlayer->m_pSession->m_nickName;

	static const std::string str��("��");
	return str��;
}


void Entity::BroadcastEnter()
{
	Broadcast(MsgAddRoleRet((uint64_t)this, StrConv::GbkToUtf8(NickName()), m_strPrefabName));//�Լ��㲥������
	Broadcast(MsgNotifyPos(*this));
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	m_space.m_pServer->m_Sessions.Broadcast(msg);
}

template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);