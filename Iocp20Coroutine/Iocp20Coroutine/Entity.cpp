#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "CoTimer.h"

Entity::Entity(float x, Space& m_space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun) :m_space(m_space), Id((uint64_t)this)
{
	//����һ��Э�̣������߶�
	this->m_Pos.x = x;
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_cancel);
	m_coWalk.Run();
}

void Entity::ReplaceCo(std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun)
{
	//m_coStop = true;
	if (m_cancel)
		m_cancel();
	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_cancel);
}

CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, std::function<void()> &cancel)
{
	KeepCancel kc(cancel);

	{
		MsgChangeSkeleAnim msg(pEntity, "attack");//���Ź�������
		Broadcast(msg);
	}
	
	if (co_await CoTimer::Wait(3000ms, cancel))//��3��	ǰҡ
		co_return 0;
	pDefencer->Hurt(1);//��һ���öԷ���1������
	if (co_await CoTimer::Wait(500ms, cancel))//��0.5��
		co_return 0;
	pDefencer->Hurt(3);//�ڶ����öԷ���3������
	if(co_await CoTimer::Wait(500ms, cancel))//��0.5��
		co_return 0;
	pDefencer->Hurt(10);//�������öԷ���10������
	if(co_await CoTimer::Wait(3000ms, cancel))//��3��	��ҡ
		co_return 0;
	{
		MsgChangeSkeleAnim msg(pEntity, "idle");//�������д�������
		Broadcast(msg);
	}

	if(co_await CoTimer::Wait(5000ms, cancel))//��5��	������ȴ
		co_return 0;
	
	co_return 0;
}

void Entity::Update()
{
	if (!m_coAttack.Finished())
	{
		return;
	}
	//if (!m_coWalk.Finished())
	//{
	//	m_coWalk.Run();
	//	return;
	//}
	for (const auto pENtity : m_space.setEntity)
	{
		if (pENtity == this)
			continue;

		//m_coStop = true;
		if (m_cancel)
		{
			LOG(INFO) << "����m_cancel";
			m_cancel();
		}
		else
		{
			LOG(WARNING) << "m_cancel�ǿյ�";
		}

		m_coWalk.Run();
		if (!m_coWalk.Finished())//20240205
		{
			LOG(ERROR) << "Э��ȡ��ʧ��";
		}
		//m_coStop = false;
	
		m_coAttack = Attack(this, this, this->m_Pos.x, this->m_Pos.z, m_cancel);
		m_coAttack.Run();
		return;
	}
}
