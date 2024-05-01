#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"

using namespace std;
Entity::Entity() :Id((uint64_t)this)
{

}
void Entity::Init(float x, Space& space, std::function< CoTask<int>(Entity*, float&, float&, std::function<void()>&)> fun, MySession* pSession)
{
	m_pSession = pSession;
	m_space = &space;

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
		LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
	}
	m_coWalk.Run();
	assert(m_coWalk.Finished());//20240205
	assert(m_coAttack.Finished());//20240205
	/*m_coStop = false;*/
	m_coWalk = fun(this, this->m_Pos.x, this->m_Pos.z, m_cancel);
}

CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, std::function<void()>& cancel)
{
	KeepCancel kc(cancel);

	pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "attack"));//���Ź�������

	if (co_await CoTimer::Wait(3000ms, cancel))//��3��	ǰҡ
		co_return 0;//Э��ȡ��

	pDefencer->Hurt(1);//��һ���öԷ���1������

	if (co_await CoTimer::Wait(500ms, cancel))//��0.5��
		co_return 0;//Э��ȡ��

	pDefencer->Hurt(3);//�ڶ����öԷ���3������

	if (co_await CoTimer::Wait(500ms, cancel))//��0.5��
		co_return 0;//Э��ȡ��

	pDefencer->Hurt(10);//�������öԷ���10������

	if (co_await CoTimer::Wait(3000ms, cancel))//��3��	��ҡ
		co_return 0;//Э��ȡ��

	pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));//�������д�������

	if (co_await CoTimer::Wait(5000ms, cancel))//��5��	������ȴ
		co_return 0;//Э��ȡ��

	co_return 0;//Э�������˳�
}

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
	for (const auto pENtity : m_space->setEntity)
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
