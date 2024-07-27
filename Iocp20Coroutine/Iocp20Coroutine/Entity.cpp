#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "AiCo.h"
#include <cmath>
#include "PlayerComponent.h"

using namespace std;
Entity::Entity() :Id((uint64_t)this)
{

}

void Entity::Init(float x, Space& space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun, const std::string &strPrefabName)
{
	m_strPrefabName = strPrefabName;

	m_space = &space;

	this->m_Pos.x = x;
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_cancel);
	m_coWalk.Run();

}

void Entity::WalkToPos(const float targetX, const float targetZ, MyServer* pServer)
{
	if (IsDead())
	{
		return;
	}
	//m_coStop = true;

	TryCancel();

	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk = AiCo::WalkToPos(this, this->m_Pos.x, this->m_Pos.z, targetX, targetZ, pServer, m_cancel);
	m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}

bool Entity::DistanceLessEqual(Entity* pEntity, float fDistance)
{
	const float fExponent = 2.0f;
	return std::pow(this->m_Pos.x - pEntity->m_Pos.x, fExponent) + std::pow(this->m_Pos.z - pEntity->m_Pos.z, fExponent) <= std::pow(fDistance, fExponent);
}

void Entity::Hurt(int hp)
{
	CHECK_GE(hp, 0);
	if (IsDead())
		return;

	this->m_hp -= hp;

	this->Broadcast(MsgNotifyPos(this, this->m_Pos.x, this->m_Pos.z, this->m_eulerAnglesY, this->m_hp));
	if (IsDead())
	{
		this->Broadcast(MsgChangeSkeleAnim(this, "died", false));//播放死亡动作
	}
}

//主线程单线程执行
void Entity::Update()
{
	if (!m_coAttack.Finished())
	{
		return;//表示不允许打断
	}
	if (!m_coWalk.Finished())
	{
		return;//表示不允许打断
	}

	if (IsDead())
	{
		return;
	}

	for (const auto pEntity : m_space->setEntity)
	{
		if (pEntity == this)//查找敌人，所有其他人都是敌人
			continue;

		if(pEntity->IsDead())
			continue;

		if (DistanceLessEqual(pEntity, this->m_fAttackDistance))
		{
			TryCancel();
			
			m_coAttack = AiCo::Attack(this, pEntity, m_cancel);
			m_coAttack.Run();
			return;
		}
		else
		{
			TryCancel();

			//m_coWalk.Run();
			assert(m_coWalk.Finished());//20240205
			assert(m_coAttack.Finished());//20240205
			/*m_coStop = false;*/
			m_coWalk = AiCo::WalkToTarget(this, pEntity, this->m_space->m_pServer, m_cancel);
			m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
		}
	}
}

void Entity::TryCancel()
{
	if (m_cancel)
	{
		LOG(INFO) << "调用m_cancel";
		m_cancel();
	}
	else
	{
		LOG(WARNING) << "m_cancel是空的";
		if (!m_coWalk.Finished() || !m_coAttack.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
		}
	}

	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205

}

void Entity::OnDestroy()
{
	LOG(INFO) << "调用Entity::OnDestroy";
	TryCancel();
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