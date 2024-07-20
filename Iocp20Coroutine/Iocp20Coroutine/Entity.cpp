#include "Entity.h"
#include "MySession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "AiCo.h"

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

	for (const auto pENtity : m_space->setEntity)
	{
		if (pENtity == this)//���ҵ��ˣ����������˶��ǵ���
			continue;

		//m_coStop = true;
		TryCancel();

		m_coWalk.Run();
		if (!m_coWalk.Finished())//20240205
		{
			LOG(ERROR) << "Э��ȡ��ʧ��";
		}
		//m_coStop = false;

		m_coAttack = AiCo::Attack(this, pENtity, this->m_Pos.x, this->m_Pos.z, m_cancel);
		m_coAttack.Run();
		return;
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
	}
}

void Entity::OnDestroy()
{
	LOG(INFO) << "����Entity::OnDestroy";
	TryCancel();
}
