#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "CoTimer.h"
using namespace std;
Entity::Entity(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun) :m_space(m_space), Id((uint64_t)this)
{
	//创建一个协程，来回走动
	this->m_Pos.x = x;
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_cancel);
	m_coWalk.Run();
}

void Entity::ReplaceCo(std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun)
{
	//m_coStop = true;
	if (m_cancel)
		m_cancel();
	else if (!m_coWalk.Finished() || !m_coAttack.Finished())
	{
		LOG(ERROR) << "协程没结束，却提前清空了m_cancel"  ;
	}
	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_cancel);
}

CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, std::function<void()> &cancel)
{
	KeepCancel kc(cancel);

	Broadcast(MsgChangeSkeleAnim(pEntity, "attack"));//播放攻击动作
		
	if (co_await CoTimer::Wait(3000ms, cancel))//等3秒	前摇
		co_return 0;//协程取消

	pDefencer->Hurt(1);//第一次让对方伤1点生命

	if (co_await CoTimer::Wait(500ms, cancel))//等0.5秒
		co_return 0;//协程取消

	pDefencer->Hurt(3);//第二次让对方伤3点生命

	if(co_await CoTimer::Wait(500ms, cancel))//等0.5秒
		co_return 0;//协程取消

	pDefencer->Hurt(10);//第三次让对方伤10点生命

	if(co_await CoTimer::Wait(3000ms, cancel))//等3秒	后摇
		co_return 0;//协程取消

	Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));//播放休闲待机动作
	
	if(co_await CoTimer::Wait(5000ms, cancel))//等5秒	公共冷却
		co_return 0;//协程取消
	
	co_return 0;//协程正常退出
}

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
	for (const auto pENtity : m_space.setEntity)
	{
		if (pENtity == this)
			continue;

		//m_coStop = true;
		if (m_cancel)
		{
			LOG(INFO) << "调用m_cancel";
			m_cancel();
		}
		else
		{
			LOG(WARNING) << "m_cancel是空的";
		}

		m_coWalk.Run();
		if (!m_coWalk.Finished())//20240205
		{
			LOG(ERROR) << "协程取消失败";
		}
		//m_coStop = false;
	
		m_coAttack = Attack(this, this, this->m_Pos.x, this->m_Pos.z, m_cancel);
		m_coAttack.Run();
		return;
	}
}
