#include "pch.h"
#include "����ս��.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "EntitySystem.h"

void ��Ҵ��뵥λ�����ķ���ս(Space& refSpace, Entity& ref�ӿ�, const Position &refPos )
{
	const ��λ���� ����(��λ����::���̳�);
	��λ::���λ���� ����;
	��λ::Find���λ����(����, ����);
	SpEntity sp���̳� = refSpace.����λ(ref�ӿ�.m_spPlayer, EntitySystem::GetNickName(ref�ӿ�), refPos, ����, ����);
}

CoTask<int> ����ս��::Co�ķ���ս(Space& refSpace, Entity& ref�ӿ�, FunCancel& funCancel, PlayerGateSession_Game& refGateSession)
{
	struct ��λ���
	{
		Position pos������;
		std::string strNickName;
	};

	const uint16_t u16�������ĵ� = 50;
	��λ��� arr��λ���[] = 
	{
		{{u16�������ĵ�	,		0			},""},
		{{-u16�������ĵ�,		0			},""},
		{{			0	,	u16�������ĵ�	},""},
		{{			0	,	-u16�������ĵ�	},""},
	};

	//��������һ����
	arr��λ���[0].strNickName = refGateSession.NickName();
	auto wpSpace = refGateSession.m_wpSpace;
	��Ҵ��뵥λ�����ķ���ս(refSpace, ref�ӿ�, arr��λ���[0].pos������);
	for (int i = 1; i < _countof(arr��λ���); ++i)
	{
		const auto& [stop, event��ҽ���Space] = co_await CoEvent<MyEvent::��ҽ���Space>::Wait(funCancel, [&refSpace]
		(auto& refPlayer) { return &refPlayer.refSpace == &refSpace; });
		if (stop)
			co_return 0;

		CHECK_WP_CO_RET_0(wpSpace);
		event��ҽ���Space.refPlayerGateSession.EnterSpace(wpSpace);
		arr��λ���[i].strNickName = event��ҽ���Space.refPlayerGateSession.NickName();
		��Ҵ��뵥λ�����ķ���ս(refSpace, event��ҽ���Space.ref�ӿ�, arr��λ���[i].pos������);
	}
	co_return 0;
}
