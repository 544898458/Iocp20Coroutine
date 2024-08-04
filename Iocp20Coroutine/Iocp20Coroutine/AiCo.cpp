#include "StdAfx.h"
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

	CoTask<int> Idle(SpEntity spEntity, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))
			{
				//LOG(INFO) << "�������ֶ�ȡ����Э��Idle";
				co_return 0;
			}
		}
	}


	CoTask<int> Attack(SpEntity spThis, SpEntity spDefencer, FunCancel& cancel)
	{
		KeepCancel kc(cancel);

		if (spThis->IsDead())
			co_return 0;//�Լ�����

		if (spDefencer->IsDead())
			co_return 0;//Ŀ������

		spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "attack"));//���Ź�������

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

			if (spThis->IsDead())
				co_return 0;//�Լ�������Э��ȡ��

			if (!spThis->DistanceLessEqual(*spDefencer, spThis->m_f��������))
				break;//Ҫִ�к�ҡ

			if (spDefencer->IsDead())
				break;//Ҫִ�к�ҡ

			spDefencer->Hurt(std::get<1>(wait_hurt));//��n���öԷ���1
		}

		if (co_await CoTimer::Wait(3000ms, cancel))//��3��	��ҡ
			co_return 0;//Э��ȡ��

		if (!spThis->IsDead())
		{
			spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "idle"));//�������д�������
		}

		co_return 0;//Э�������˳�
	}
	bool MoveStep(Entity &refThis, const Position localTarget)
	{
		const float step = refThis.m_f�ƶ��ٶ�;
		const auto oldPos = refThis.m_Pos;//���ƶ��󣬲�������
		float& x = refThis.m_Pos.x;
		float& z = refThis.m_Pos.z;

		if (std::abs(localTarget.x - x) < step && std::abs(localTarget.z - z) < step) 
		{
			//LOG(INFO) << "���ߵ�" << localTarget.x << "," << localTarget.z << "������Э�������˳�";
			refThis.Broadcast(MsgChangeSkeleAnim(refThis, "idle"));
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

		refThis.m_eulerAnglesY = CalculateAngle(oldPos, refThis.m_Pos);
		refThis.Broadcast(MsgNotifyPos(refThis));

		return true;
	}
	CoTask<int>WalkToPos(SpEntity spThis, const Position& posTarget, MyServer* pServer, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTarget = posTarget;
		auto pLocalServer = pServer;
		spThis->Broadcast(MsgChangeSkeleAnim(*spThis, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << localTarget << "��Э��ȡ����";
				co_return 0;
			}
			if (spThis->IsDead())
			{
				LOG(INFO) << "�Լ�����������" << localTarget << "��Э��ȡ����";
				if (spThis->m_spPlayer)
					spThis->m_spPlayer->m_pSession->Send(MsgSay(StrConv::GbkToUtf8("�Լ�����")));

				co_return 0;
			}

			if (!MoveStep(*spThis, localTarget))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << posTarget;
	}

	CoTask<int> WalkToTarget(SpEntity spThis, SpEntity spTarget, MyServer* pServer, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(*spTarget, "run"));

		while (true)
		{
			if (co_await CoTimer::WaitNextUpdate(funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
			{
				LOG(INFO) << "����" << spTarget << "��Э��ȡ����";
				co_return 0;
			}
			if (!spThis->DistanceLessEqual(*spTarget, spThis->m_f�������))
			{
				LOG(INFO) << "�뿪�Լ��ľ������" << spTarget << "��Э��ȡ����";
				co_return 0;
			}
			if (spThis->DistanceLessEqual(*spTarget, spThis->m_f��������))
			{
				//LOG(INFO) << "���ߵ�" << spTarget << "������Э�������˳�";
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(*spTarget, "idle"));
				co_return 0;
			}
			if (!MoveStep(*spThis, spTarget->m_Pos))
			{
				co_return 0;
			}
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << spThis->m_Pos;
	}
	CoTask<int> WaitDelete(SpEntity spThis, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;
		if (co_await CoTimer::Wait(3s,funCancel))//�������������̴߳�ѭ����ÿ��ѭ������һ��
		{
			LOG(INFO) << "WaitDeleteЭ��ȡ����";
		}
		spThis->m_bNeedDelete = true;
		co_return 0;
	}
	CoTask<int> SpawnMonster(Space& refSpace, FunCancel& funCancel)
	{
		KeepCancel kc(funCancel);
		using namespace std;

		while (!co_await CoTimer::Wait(3s, funCancel))
		{
			SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const std::string& >({ -30.0 }, refSpace, "altman-red");
			spEntityMonster->AddComponentMonster();
			spEntityMonster->m_f������� = 20;
			spEntityMonster->m_f�ƶ��ٶ� = 0.2f;
			refSpace.setEntity.insert(spEntityMonster);
			spEntityMonster->BroadcastEnter();
		}
		LOG(INFO) << "ֹͣˢ��Э��";
		co_return 0;
	}
}