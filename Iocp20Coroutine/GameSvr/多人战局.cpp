#include "pch.h"
#include "����ս��.h"
#include "Space.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoEvent.h"
#include "MyEvent.h"
#include "EntitySystem.h"
#include "��λ���/��ԴComponent.h"

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);

void ��Ҵ��뵥λ�����ķ���ս(Space& refSpace, const Position &refPos, PlayerGateSession_Game& refGateSession)
{
	{
		const ��λ���� ����(��λ����::���̳�);
		��λ::���λ���� ����;
		��λ::Find���λ����(����, ����);
		CHECK_WP_RET_VOID(refGateSession.m_wp�ӿ�);
		SpEntity sp���̳� = refSpace.����λ(refGateSession.m_wp�ӿ�.lock()->m_spPlayer, refGateSession.NickName(), refPos, ����, ����);
		refGateSession.Send�����ӿ�(*sp���̳�);
	}

	��ԴComponent::Add(refSpace, �����, { refPos.x,		refPos.z - 20 });
	��ԴComponent::Add(refSpace, �����, { refPos.x,		refPos.z + 20 });
	��ԴComponent::Add(refSpace, �����, { refPos.x - 20,	refPos.z	});
	��ԴComponent::Add(refSpace, ȼ����, { refPos.x + 20,	refPos.z	});

}

CoTask<int> ����ս��::Co�ķ���ս(Space& refSpace, FunCancel& funCancel, const std::string strPlayerNickName)
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
	arr��λ���[0].strNickName = strPlayerNickName;
	auto spOwnerPlayerSession = GetPlayerGateSession(strPlayerNickName);
	CHECK_WP_CO_RET_0(spOwnerPlayerSession);
	��Ҵ��뵥λ�����ķ���ս(refSpace, arr��λ���[0].pos������, *spOwnerPlayerSession.lock());

	for (int i = 1; i < _countof(arr��λ���); )
	{
		const auto& [stop, event��ҽ���Space] = co_await CoEvent<MyEvent::��ҽ���Space>::Wait(funCancel, 
			[&refSpace](auto& refPlayer) { return &*refPlayer.wpSpace.lock() == &refSpace; });
		if (stop)
		{
			LOG(INFO) << "Co�ķ���ս,�ж��˳�";
			co_return 0;
		}

		CHECK_WP_CO_RET_0(event��ҽ���Space.wpPlayerGateSession);
		CHECK_WP_CO_RET_0(event��ҽ���Space.wpSpace);
		CHECK_WP_CO_RET_0(event��ҽ���Space.wp�ӿ�);

		{
			auto iterEnd = arr��λ��� + _countof(arr��λ���);
			if (iterEnd != std::find_if(arr��λ���, iterEnd, 
				[&event��ҽ���Space](const ��λ���& ref��λ���) 
				{return ref��λ���.strNickName == event��ҽ���Space.wpPlayerGateSession.lock()->NickName(); }))
			{
				LOG(INFO) << "�ظ����룺" << event��ҽ���Space.wpPlayerGateSession.lock()->NickName();
				continue;
			}
		}
		//event��ҽ���Space.wpPlayerGateSession.lock()->EnterSpace(wpSpace);
		arr��λ���[i].strNickName = event��ҽ���Space.wpPlayerGateSession.lock()->NickName();
		��Ҵ��뵥λ�����ķ���ս(refSpace, arr��λ���[i].pos������, *event��ҽ���Space.wpPlayerGateSession.lock());
		++i;
	}
	co_return 0;
}
