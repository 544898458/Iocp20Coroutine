#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "MySession.h"
#include "MyServer.h"

namespace AiCo
{

	CoTask<int> Idle(Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		bool stop = false;
		funCancel = [&stop]() {stop = true; };
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))
			{
				LOG(INFO) << "�������ֶ�ȡ����Э��Idle";
				co_return 0;
			}
			if (stop)
			{

				LOG(INFO) << "IdleЭ�������˳�";
				co_return 0;
			}
			//x -= 0.1f;

			pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z, pEntity->m_eulerAnglesY, pEntity->m_hp));
		}
	}


	CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, std::function<void()>& cancel)
	{
		using namespace std;
		KeepCancel kc(cancel);
		
		if (pEntity->IsDead())
			co_return 0;//�Լ�����

		pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "attack"));//���Ź�������

		if (co_await CoTimer::Wait(3000ms, cancel))//��3��	ǰҡ
			co_return 0;//Э��ȡ��

		if (pEntity->IsDead())
			co_return 0;//�Լ�����

		if(pDefencer->IsDead())
			co_return 0;//Ŀ������

		pDefencer->Hurt(1);//��һ���öԷ���1������

		if (co_await CoTimer::Wait(500ms, cancel))//��0.5��
			co_return 0;//Э��ȡ��

		if (pEntity->IsDead())
			co_return 0;//�Լ�����

		if (pDefencer->IsDead())
			co_return 0;//Ŀ������

		pDefencer->Hurt(3);//�ڶ����öԷ���3������

		if (co_await CoTimer::Wait(500ms, cancel))//��0.5��
			co_return 0;//Э��ȡ��

		if (pEntity->IsDead())
			co_return 0;//�Լ�����

		if (pDefencer->IsDead())
			co_return 0;//Ŀ������

		pDefencer->Hurt(10);//�������öԷ���10������

		if (co_await CoTimer::Wait(3000ms, cancel))//��3��	��ҡ
			co_return 0;//Э��ȡ��

		if (!pEntity->IsDead())
		{
			pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));//�������д�������
		}

		if (co_await CoTimer::Wait(5000ms, cancel))//��5��	������ȴ
			co_return 0;//Э��ȡ��

		co_return 0;//Э�������˳�
	}

	CoTask<int>WalkToPos(Entity* pThis, float& x, float& z, const float targetX, const float targetZ, MyServer *pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTargetX = targetX;
		const auto localTargetZ = targetZ;
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pThis, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << localTargetX << "," << localTargetZ << "��Э��ȡ����";
				co_return 0;
			}

			const auto step = 0.5f;
			if (std::abs(localTargetX - x) < step && std::abs(localTargetZ - z) < step) {
				LOG(INFO) << "���ߵ�" << localTargetX << "," << localTargetZ << "������Э�������˳�";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pThis, "idle"));
				co_return 0;
			}
			const auto oldPos = pThis->m_Pos;
			if (std::abs(localTargetX - x) >= step)
			{
				x += localTargetX < x ? -step : step;
			}

			if (std::abs(localTargetZ - z) >= step)
			{
				z += localTargetZ < z ? -step : step;
			}

			pThis->m_eulerAnglesY = CalculateAngle(oldPos, pThis->m_Pos);
			pLocalServer->m_Sessions.Broadcast(MsgNotifyPos(pThis, x, z, pThis->m_eulerAnglesY, pThis->m_hp));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << targetX << "," << targetX;
	}

	CoTask<int> WalkToTarget(Entity* pThis, Entity* pEntity, MyServer* pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << pEntity->m_nickName << "��Э��ȡ����";
				co_return 0;
			}

			if (pThis->DistanceLessEqual(pEntity, pThis->m_fAttackDistance))
			{
				LOG(INFO) << "���ߵ�" << pEntity->m_nickName << "������Э�������˳�";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
				co_return 0;
			}

			const auto step = 0.5f;
			const auto oldPos = pThis->m_Pos;
			pThis->m_Pos.x += pEntity->m_Pos.x < pThis->m_Pos.x ? -step : step;
			pThis->m_Pos.z += pEntity->m_Pos.z< pThis->m_Pos.z ? -step : step;

			pThis->m_eulerAnglesY = CalculateAngle(oldPos, pThis->m_Pos);

			pLocalServer->m_Sessions.Broadcast(MsgNotifyPos(pThis, pThis->m_Pos.x, pThis->m_Pos.z, pThis->m_eulerAnglesY, pEntity->m_hp));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << pThis->m_Pos.x << "," << pThis->m_Pos.z;
	}
}