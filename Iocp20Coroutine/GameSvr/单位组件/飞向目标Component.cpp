#include "pch.h"
#include "����Ŀ��Component.h"
#include "../Entity.h"
#include "../../CoRoutine/CoTimer.h"

void ����Ŀ��Component::AddComponet(Entity& refEntity, const Position& posĿ��)
{
	refEntity.m_up����Ŀ��.reset(new ����Ŀ��Component(refEntity, posĿ��));
}

����Ŀ��Component::����Ŀ��Component(Entity& ref, const Position& posĿ��) :m_refEntity(ref), m_posĿ��(posĿ��)
{
	m_TaskCancel.co = Co����Ŀ�����б�ը();
}

CoTaskBool ����Ŀ��Component::Co����Ŀ�����б�ը()
{
	while (!co_await CoTimer::WaitNextUpdate(m_TaskCancel.cancel))
	{
		if (m_refEntity.Pos().DistanceLessEqual(m_posĿ��, 1))
		{
			using namespace std;
			m_refEntity.CoDelayDelete(1ms).RunNew();
			co_return false;
		}
		const auto vec���� = (m_posĿ�� - m_refEntity.Pos()).��һ��();
		��λ::ս������ ����;
		CHECK_CO_RET_FALSE(��λ::Findս������(m_refEntity.m_����, ����));
		m_refEntity.SetPos(m_refEntity.Pos() + vec���� * ����.fÿ֡�ƶ�����);
		m_refEntity.BroadcastNotifyPos();
	}
	co_return true;
}