#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "CoTimer.h"

Entity::Entity(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun) :m_space(m_space), Id((uint64_t)this)
{
	//创建一个协程，来回走动
	this->x = x;
	m_coWalk = fun(this, this->x, this->z, m_coStop);
	m_coWalk.Run();
}

void Entity::ReplaceCo(std::function< CoTask<int>(Entity*, float&, float&, bool&)> fun)
{
	m_coStop = true;
	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	m_coStop = false;
	m_coWalk = fun(this, this->x, this->z, m_coStop);
}

CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, bool& stop)
{
	//while (true)
	{
		//co_yield 0;
		//if (stop)
		//{
		//	LOG(INFO) << "Entity协程正常退出";
		//	co_return 0;
		//}
		{
			MsgChangeSkeleAnim msg(pEntity, "attack");
			Broadcast(msg);
		}
		co_await CoTimer::Wait(3000ms);
		{
			MsgChangeSkeleAnim msg(pEntity, "idle");
			Broadcast(msg);
		}
		co_await CoTimer::Wait(3000ms);
	}
	co_return 0;
}

void Entity::Update()
{
	if (!m_coAttack.Finished())
	{
		m_coAttack.Run();
		return;
	}
	if (!m_coWalk.Finished())
	{
		m_coWalk.Run();
		return;
	}
	for (const auto pENtity : m_space.setEntity)
	{
		if (pENtity == this)
			continue;

		m_coStop = true;
		m_coWalk.Run();
		assert(m_coWalk.Finished());//20240205
		m_coStop = false;

		m_coAttack = Attack(this, this, this->x, this->z, m_coStop);
		//m_coAttack.Run();
		return;
	}
}
