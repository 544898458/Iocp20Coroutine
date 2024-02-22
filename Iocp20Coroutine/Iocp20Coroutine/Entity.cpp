#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "CoTimer.h"

Entity::Entity(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun) :m_space(m_space), Id((uint64_t)this)
{
	//创建一个协程，来回走动
	this->m_Pos.x = x;
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_coStop);
	m_coWalk.Run();
}

void Entity::ReplaceCo(std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun)
{
	m_coStop = true;
	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	m_coStop = false;
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_coStop);
}

CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, bool& stop)
{
	{
		MsgChangeSkeleAnim msg(pEntity, "attack");//播放攻击动作
		Broadcast(msg);
	}

	co_await CoTimer::Wait(3000ms);//等3秒	前摇
	pDefencer->Hurt(1);//第一次让对方伤1点生命
	co_await CoTimer::Wait(500ms);//等0.5秒
	pDefencer->Hurt(3);//第二次让对方伤3点生命
	co_await CoTimer::Wait(500ms);//等0.5秒
	pDefencer->Hurt(10);//第三次让对方伤10点生命
	co_await CoTimer::Wait(3000ms);//等3秒	后摇
	{
		MsgChangeSkeleAnim msg(pEntity, "idle");//播放休闲待机动作
		Broadcast(msg);
	}

	co_await CoTimer::Wait(5000ms);//等5秒	公共冷却
	
	co_return 0;
}

void Entity::Update()
{
	//if (!m_coAttack.Finished())
	//{
	//	m_coAttack.Run();
	//	return;
	//}
	//if (!m_coWalk.Finished())
	//{
	//	m_coWalk.Run();
	//	return;
	//}
	for (const auto pENtity : m_space.setEntity)
	{
		if (pENtity == this)
			continue;

		m_coStop = true;
		m_coWalk.Run();
		assert(m_coWalk.Finished());//20240205
		m_coStop = false;

		m_coAttack = Attack(this, this, this->m_Pos.x, this->m_Pos.z, m_coStop);
		m_coAttack.Run();
		return;
	}
}
