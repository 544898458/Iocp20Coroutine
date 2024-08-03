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

using namespace std;
Entity::Entity() :Id((uint64_t)this)
{

}

void Entity::Init(float x, Space& space, const std::string &strPrefabName)
{
	m_strPrefabName = strPrefabName;

	m_space = &space;

	this->m_Pos.x = x;
	m_coIdle = AiCo::Idle(shared_from_this(), this->m_Pos.x, this->m_Pos.z, m_cancel);
	m_coIdle.Run();

}

void Entity::WalkToPos(const Position &posTarget, MyServer* pServer)
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
	m_coWalk = AiCo::WalkToPos(shared_from_this(), this->m_Pos.x, this->m_Pos.z, posTarget, pServer, m_cancel);
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

	for (const auto &spEntity : m_space->setEntity)
	{
		if (spEntity.get() == this)//���ҵ��ˣ����������˶��ǵ���
			continue;

		if(spEntity->IsDead())
			continue;

		if (DistanceLessEqual(*spEntity, this->m_f��������))
		{
			TryCancel();
			
			m_coAttack = AiCo::Attack(this->shared_from_this(), spEntity, m_cancel);
			m_coAttack.Run();
			return;
		}
		else if(DistanceLessEqual(*spEntity, this->m_f�������))
		{
			TryCancel();

			//m_coWalk.Run();
			assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			m_coWalk = AiCo::WalkToTarget(shared_from_this(), spEntity, this->m_space->m_pServer, m_cancel);
			m_coWalk.Run();//Э���뿪��ʼ���У����е���һ��co_await
		}
	}
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
		LOG(WARNING) << "m_cancel�ǿյ�";
		if (!m_coWalk.Finished() || !m_coAttack.Finished() || !m_coIdle.Finished() )
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

	return "��";
}

void Entity::AddComponent(MySession* pSession)
{
	CHECK_NOTNULL(pSession);
	if (nullptr == pSession)
		return;
	
	m_spPlayer = std::make_shared<PlayerComponent>();
	m_spPlayer->m_pSession = pSession;
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	m_space->m_pServer->m_Sessions.Broadcast(msg);
}

template void Entity::Broadcast(const MsgLoginRet& msg);