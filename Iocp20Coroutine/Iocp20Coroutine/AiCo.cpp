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
				LOG(INFO) << "�������ֶ�ȡ����Э��TraceEnemy";
				co_return 0;
			}
			if (stop)
			{

				LOG(INFO) << "TraceEnemyЭ�������˳�";
				co_return 0;
			}
			//x -= 0.1f;

			pEntity->m_pSession->m_pServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z, pEntity->m_hp));
		}
	}


	CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, std::function<void()>& cancel)
	{
		using namespace std;
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

	CoTask<int>WalkToPos(Entity* pEntity, float& x, float& z, const float targetX, const float targetZ, MyServer *pServer, std::function<void()>& funCancel)
	{
		KeepCancel kc(funCancel);
		const auto localTargetX = targetX;
		const auto localTargetZ = targetZ;
		auto pLocalServer = pServer;
		pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "run"));

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
				pLocalServer->m_Sessions.Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
				co_return 0;
			}

			x += localTargetX < x ? -step : step;
			z += localTargetZ < z ? -step : step;

			pLocalServer->m_Sessions.Broadcast(MsgNotifyPos(pEntity, x, z, pEntity->m_hp));
		}
		LOG(INFO) << "����Ŀ��Э�̽���:" << targetX << "," << targetX;
	}
}