#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoTimer.h"
#include "MySession.h"
#include "MyServer.h"
#include "PlayerSystem.h"
#include "PlayerComponent.h"
#include "../IocpNetwork/StrConv.h"
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
			if (pEntity->m_spPlayer)
				pEntity->m_spPlayer->m_pSession->m_pServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity));
		}
	}


	CoTask<int> Attack(Entity* pThis, Entity* pDefencer, std::function<void()>& cancel)
	{
		KeepCancel kc(cancel);

		if (pThis->IsDead())
			co_return 0;//�Լ�����

		if (pDefencer->IsDead())
			co_return 0;//Ŀ������

		pThis->Broadcast(MsgChangeSkeleAnim(pThis, "attack"));//���Ź�������

		using namespace std;


		const std::tuple<std::chrono::milliseconds, int> arrWaitHurt[] =
		{	//�����˺�{ÿ��ǰҡʱ�����˺�ֵ}
			{3000ms,1},
			{500ms,3},
			{500ms,10}
		};

		for (auto wait_hurt : arrWaitHurt)
		{
			if (co_await CoTimer::Wait(std::get<0>(wait_hurt), cancel))//��x��	ǰҡ
				co_return 0;//Э��ȡ��

			if (pThis->IsDead())
				co_return 0;//�Լ�������Э��ȡ��

			if (!pThis->DistanceLessEqual(pDefencer, pThis->m_f��������))
				break;//Ҫִ�к�ҡ

			if (pDefencer->IsDead())
				break;//Ҫִ�к�ҡ

			pDefencer->Hurt(std::get<1>(wait_hurt));//��n���öԷ���1
		}

		if (co_await CoTimer::Wait(3000ms, cancel))//��3��	��ҡ
			co_return 0;//Э��ȡ��

		if (!pThis->IsDead())
		{
			pThis->Broadcast(MsgChangeSkeleAnim(pThis, "idle"));//�������д�������
		}

		co_return 0;//Э�������˳�
	}
	bool MoveStep(Entity* pThis, const Position localTarget)
	{
		const float step = pThis->m_f�ƶ��ٶ�;
		const auto oldPos = pThis->m_Pos;//���ƶ��󣬲�������
		float& x = pThis->m_Pos.x;
		float& z = pThis->m_Pos.z;

		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step) 
		{
			LOG(INFO) << "���ߵ�" << localTarget.x << "," << localTarget.z << "������Э�������˳�";
			pThis->Broadcast(MsgChangeSkeleAnim(pThis, "idle"));
			return false;
		}
		
		if (std::abs(localTarget.x - x) >= step)
		{
			x += localTarget.x < x ? -step : step;
		}

		if (std::abs(localTarget.z - z) >= step)
		{
			z += localTarget.z < z ? -step : step;
		}

		pThis->m_eulerAnglesY = CalculateAngle(oldPos, pThis->m_Pos);
		pThis->Broadcast(MsgNotifyPos(pThis));

		return true;
	}
	CoTask<int>WalkToPos(Entity* pThis, float& x, float& z, const float targetX, const float targetZ, MyServer* pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTargetX = targetX;
		const auto localTargetZ = targetZ;
		auto pLocalServer = pServer;
		pThis->Broadcast(MsgChangeSkeleAnim(pThis, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << localTargetX << "," << localTargetZ << "��Э��ȡ����";
				co_return 0;
			}
			if (pThis->IsDead())
			{
				LOG(INFO) << "�Լ�����������" << localTargetX << "," << localTargetZ << "��Э��ȡ����";
				if (pThis->m_spPlayer)
					pThis->m_spPlayer->m_pSession->Send(MsgSay(StrConv::GbkToUtf8("�Լ�����")));

				co_return 0;
			}

			if (!MoveStep(pThis, Position(localTargetX, localTargetZ)))
			{
				co_return 0;
			}
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
			if (!pThis->DistanceLessEqual(pEntity, pThis->m_f�������))
			{
				LOG(INFO) << "�뿪�Լ��ľ������" << pEntity->m_nickName << "��Э��ȡ����";
				co_return 0;
			}
			if (pThis->DistanceLessEqual(pEntity, pThis->m_f��������))
			{
				LOG(INFO) << "���ߵ�" << pEntity->m_nickName << "������Э�������˳�";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
				co_return 0;
			}
			if (!MoveStep(pThis, pEntity->m_Pos))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << pThis->m_Pos.x << "," << pThis->m_Pos.z;
	}
}