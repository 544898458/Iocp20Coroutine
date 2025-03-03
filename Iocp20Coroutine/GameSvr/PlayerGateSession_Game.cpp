#include "pch.h"
#include "PlayerGateSession_Game.h"
#include "Space.h"
#include "Entity.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoEvent.h"
#include "AiCo.h"
#include "��λ���/AttackComponent.h"
#include "��λ���/DefenceComponent.h"
#include "��λ���/�ɼ�Component.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "��λ���/BuildingComponent.h"
#include <unordered_map>
#include "��λ.h"
#include "���˾���.h"
#include "����ս��.h"
#include <sstream>
#include "��λ���/����λComponent.h"
#include "��λ���/�콨��Component.h"
#include "��λ���/�ر�Component.h"
#include "��λ���/��Component.h"
#include "��λ���/PlayerComponent.h"
#include "��λ���/��ԴComponent.h"
#include "../CoRoutine/CoTimer.h"
#include "EntitySystem.h"
#include "��λ���/PlayerNickNameComponent.h"
#include "MyEvent.h"
#include "../�������ļ�/Try��Ini���ػ���ר��.h"
std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);


/// <summary>
/// GameSvrͨ��GateSvr͸������Ϸ�ͻ���
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="ref"></param>
template<class T>
void PlayerGateSession_Game::Send(const T& ref)
{
	++m_snSend;
	ref.msg.sn = (m_snSend);

	MsgPack::SendMsgpack(ref, [this](const void* buf, int len)
		{
			MsgGateת�� msg(buf, len, m_idPlayerGateSession, m_snSend);
			MsgPack::SendMsgpack(msg, [this](const void* bufת��, int lenת��)
				{
					this->m_refGameSvrSession.SendToGate(bufת��, lenת��);
				});
		}, false);
}


template void PlayerGateSession_Game::Send(const MsgAddRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyPos&);
template void PlayerGateSession_Game::Send(const MsgChangeSkeleAnim&);
template void PlayerGateSession_Game::Send(const MsgSay&);
template void PlayerGateSession_Game::Send(const MsgDelRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyMoney&);
template void PlayerGateSession_Game::Send(const Msg��ʾ����&);
template void PlayerGateSession_Game::Send(const MsgEntity����&);
template void PlayerGateSession_Game::Send(const Msg������Ч&);


void PlayerGateSession_Game::OnDestroy()
{
	�뿪Space(false);
}

void PlayerGateSession_Game::�뿪Space(const bool b������)
{
	if (!m_wpSpace.expired())
	{
		auto wp�ҵĵ��� = Space::GetSpace����(NickName());
		m_wpSpace.lock()->m_mapPlayer[NickName()].OnDestroy(b������ && !wp�ҵĵ���.expired(), *m_wpSpace.lock(), NickName());
	}

	for (auto& sp : m_vecFunCancel)
	{
		auto& fun = *sp;
		if (fun)
			fun();
	}
	m_vecFunCancel.clear();
	/*m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
		{
		});*/

	if (m_funCancel����ͼ)
	{
		m_funCancel����ͼ();
		m_funCancel����ͼ = nullptr;
	}

	const bool b�뿪 = !m_wpSpace.expired();
	m_wpSpace.reset();
	//if (m_spSpace���˾��鸱��)
	//{
	//	m_spSpace���˾��鸱��->OnDestory();
	//	m_spSpace���˾��鸱��.reset();
	//}
	if (b������)
	{
		Space::DeleteSpace����(NickName());
	}
	//if (m_spSpace����ս��)
	//{
	//	m_spSpace����ս��->OnDestory();
	//	m_spSpace����ս��.reset();
	//}
	if (b�뿪)
		Send<Msg�뿪Space>({});
}

void PlayerGateSession_Game::Say(const std::string& str, const SayChannel channel)
{
	Send<MsgSay>({ .content = StrConv::GbkToUtf8(str),.channel = channel });
}

void PlayerGateSession_Game::Sayϵͳ(const std::string& refStrNickName, const std::string& str)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->Sayϵͳ(str);

}

void PlayerGateSession_Game::Say������ʾ(const std::string& refStrNickName, const std::string& str)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->Say(str, ������ʾ);

}

void PlayerGateSession_Game::Sayϵͳ(const std::string& str)
{
	Say(str, SayChannel::ϵͳ);
}

//void PlayerGateSession_Game::Say������ʾ(const std::string& str)
//{
//	Say(str, SayChannel::����);
//}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	bool bOK(false);
	ForEachSelected([this, &msg, &bOK](Entity& ref)
		{
			if (!ref.m_sp����λ)return;
			if (!ref.m_sp����λ->����(msg.����))return;
			ref.m_sp����λ->���(*this, msg.����);
			bOK = true;
		});

	if (bOK)
		return;

	//�Զ��ұ���ȥ��
	CHECK_WP_RET_VOID(m_wpSpace);
	Space& refSpace = *m_wpSpace.lock();
	std::vector<WpEntity> vecWp����;
	for (auto [_, wp] : refSpace.m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntiy = *wp.lock();
		if (!refEntiy.m_spBuilding)continue;
		if (!refEntiy.m_spBuilding->�����())continue;
		if (!refEntiy.m_sp����λ)continue;
		if (!refEntiy.m_sp����λ->����(msg.����))continue;
		vecWp����.push_back(wp);
	}

	//���������̵���ǰ��
	std::sort(vecWp����.begin(), vecWp����.end(), [](const WpEntity& wp��, const WpEntity& wp��)->bool
		{
			CHECK_WP_RET_FALSE(wp��);
			CHECK_WP_RET_FALSE(wp��);
			CHECK_NOTNULL_RET_FALSE(wp��.lock()->m_sp����λ);
			CHECK_NOTNULL_RET_FALSE(wp��.lock()->m_sp����λ);
			return wp��.lock()->m_sp����λ->�ȴ���Count() < wp��.lock()->m_sp����λ->�ȴ���Count();
		});
	for (auto wp : vecWp����)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntiy = *wp.lock();
		refEntiy.m_sp����λ->���(*this, msg.����);
		return;
	}

	{
		std::string str��ʾ("������һ��");
		const auto _ = msg.����;
		switch (_)
		{
		case ���̳�:str��ʾ += "����"; break;
		case �ɻ�:str��ʾ += "����"; break;
		case ��ɫ̹��:str��ʾ += "�ع���"; break;
		default:str��ʾ += "����"; break;
		}
		��������Buzz(str��ʾ);
	}
}

void PlayerGateSession_Game::OnRecv(const Msg�ɼ�& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			CHECK_VOID(!m_wpSpace.expired());
			auto wpEntity = m_wpSpace.lock()->GetEntity((int64_t)msg.idĿ����Դ);
			CHECK_WP_RET_VOID(wpEntity);
			auto& ref��Դ = *wpEntity.lock();
			if (���� == ref.m_���� || ������ == ref.m_����)
			{
				CHECK_RET_VOID(ref.m_sp����λ);
				ref.m_sp����λ->m_pos����� = ref��Դ.Pos();
				Sayϵͳ("���趨�ɼ���Դ");
				return;
			}

			if (!ref.m_sp�ɼ�)
			{
				��������Buzz("�˵�λ�޷��ɼ���Դ");
				return;
			}

			ref.m_sp�ɼ�->�ɼ�(wpEntity);
		});
}

void PlayerGateSession_Game::OnRecv(const Msg���ر�& msg)
{
	CHECK_VOID(!m_wpSpace.expired());
	auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.id�ر�);
	//msg.vecId�ر��ڵ�λ.size();
	CHECK_RET_VOID(!wpTarget.expired());
	auto spTarget = wpTarget.lock();
	if (!spTarget->m_sp�ر�)
	{
		Sayϵͳ("Ŀ�겻�ǵر�");
		return;
	}
	spTarget->m_sp�ر�->ȫ�����ر�();
}

void PlayerGateSession_Game::OnRecv(const Msg���ر�& msg)
{
	CHECK_VOID(!m_wpSpace.expired());
	auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.idĿ��ر�);
	CHECK_RET_VOID(!wpTarget.expired());
	auto& refTarget�ر� = *wpTarget.lock();
	if (!refTarget�ر�.m_sp�ر�)
	{
		��������Buzz("Ŀ�겻�ǵر�");
		return;
	}
	if (EntitySystem::GetNickName(refTarget�ر�) != NickName())
	{
		//��������Buzz("���ܽ����˵ĵر�");
		MsgMove msg = { .pos = refTarget�ر�.Pos(),.b���������Զ����� = true };
		OnRecv(msg);

		return;
	}

	std::list<std::function<void()>> listFun;
	ForEachSelected([this, &msg, &listFun, &wpTarget](Entity& ref)
		{
			if (!ref.m_spAttack || !EntitySystem::Is�ɽ��ر�(ref.m_����))
			{
				//Sayϵͳ("�˵�λ���ɽ���ر�");
				��������Buzz("�˵�λ���ɽ���ر�");
				return;
			}

			listFun.emplace_back([&ref, wpTarget, this]()
				{
					if (ref.m_sp��)
						ref.m_sp��->�߽��ر�(wpTarget);
				});
		});

	for (auto& fun : listFun)
	{
		fun();
	}
}

CoTaskBool PlayerGateSession_Game::Co������������ͼ(WpEntity wp�ӿ�)
{
	{
		CHECK_WP_CO_RET_FALSE(m_wpSpace);
		CHECK_WP_CO_RET_FALSE(wp�ӿ�);
		auto& refSpace = *m_wpSpace.lock();
		auto& ref�ӿ� = *wp�ӿ�.lock();
		auto pos���� = Position(std::rand() % 100 - 50.f, std::rand() % 100 - 50.f);
		refSpace.����λ(ref�ӿ�, NickName(), ��λ����::���̳�, pos����, true);
		refSpace.����λ(ref�ӿ�, NickName(), ��λ����::��, { pos����.x, pos����.z + 6 });
		refSpace.����λ(ref�ӿ�, NickName(), ��λ����::��ɫ̹��, { pos����.x + 6, pos����.z });
		refSpace.����λ(ref�ӿ�, NickName(), ��λ����::����, { pos����.x + 6, pos����.z + 6 });
		refSpace.����λ(ref�ӿ�, NickName(), ��λ����::��ս��, { pos����.x - 6, pos����.z });
		refSpace.����λ(ref�ӿ�, NickName(), ��λ����::�ɻ�, { pos����.x - 6, pos����.z - 6 });

		//auto [stop, msgResponce] = co_await AiCo::ChangeMoney(*this, 0, true, m_funCancel����ͼ);
		//if (stop)
		//	co_return true;

		const uint16_t u16��ʼ�����(1000);
		auto& refSpacePlayer = refSpace.GetSpacePlayer(ref�ӿ�);
		if (refSpacePlayer.m_u32����� < u16��ʼ�����)
		{
			refSpacePlayer.m_u32����� += u16��ʼ�����;
			refSpacePlayer.m_u32ȼ���� += u16��ʼ�����;
			PlayerComponent::Send��Դ(ref�ӿ�);
		}
		//if (msgResponce.finalMoney < u16��ʼ�����)
		//{
			//if (std::get<0>(co_await AiCo::ChangeMoney(*this, u16��ʼ�����, true, m_funCancel����ͼ)))
			//	co_return true;
		//}
	}
	using namespace std;
	const auto seconds��Ϣ��� = 10s;
	Say("����ÿ����Ҷ��������ɹ�ͬ����ĳ������ֲ���һЩ��Դ�������Ĺ֣���Դ�������ٶȺ���", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("������ֻ��һ�����̳������̳����Խ��콨���������п��Բ������λ", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("һ�����˳��˳�������ߺ����ڴ˳����е����е�λ���ܻ���Ϊȱ��ָ�Ӷ��⵽����", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("������������ĵ��ˣ����ǿ��ܻ�����������", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("������ƶ����������������˳��˳����ٴν��룬���л��ᶫɽ����", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("��ʩչ����ָ�������ɣ����ͣ�", SayChannel::ϵͳ);

	co_return false;
}
void PlayerGateSession_Game::OnRecv(const Msg��Space& msg)
{
	�뿪Space(true);
	LOG(INFO) << "ϣ����Space:" << msg.idSapce;
	auto wp = Space::GetSpace(msg.idSapce);
	CHECK_WP_RET_VOID(wp);
	EnterSpace(wp);

	if (m_funCancel����ͼ)
	{
		m_funCancel����ͼ();
		m_funCancel����ͼ = nullptr;
	}

	Co������������ͼ(m_wp�ӿ�).RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg�뿪Space& msg)
{
	LOG(INFO) << "ϣ���뿪Space:";// << msg.idSapce;
	�뿪Space(true);
}

std::unordered_map<����ID, ��������> g_map�������� =
{
	{ѵ��ս,{"all_tiles_tilecache.bin",		"sceneս��",	���˾���::Coѵ��ս,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse.mp3"}},
	{����ս,{"����ս.bin",					"scene����ս",	���˾���::Co����ս,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse_2.mp3"}},
	{����ս,{"����ս.bin",					"scene����ս",	���˾���::Co����ս,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse.mp3"}},
	{����������ͼ,{"all_tiles_tilecache.bin","sceneս��",	{},	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse.mp3"}},
	{�ķ���ս,{"�ķ���ս.bin",				"scene�ķ���ս",����ս��::Co�ķ���ս,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse_2.mp3"}},
};

bool Get��������(const ����ID id, ��������& refOut)
{
	const auto itFind = g_map��������.find(id);
	if (itFind == g_map��������.end())
	{
		LOG(ERROR) << "";
		_ASSERT(false);
		return false;
	}

	refOut = itFind->second;
	return true;
}

void PlayerGateSession_Game::OnRecv(const Msg�����˾��鸱��& msg)
{
	CHECK_VOID(msg.id == ѵ��ս || msg.id == ����ս || msg.id == ����ս);
	�������� ����;
	{
		const auto ok = Get��������(msg.id, ����);
		CHECK_RET_VOID(ok);
	}

	//m_spSpace���˾��鸱�� = std::make_shared<Space, const ��������&>(����);
	auto wpOld = Space::GetSpace����(NickName());
	if (!wpOld.expired() && wpOld.lock()->m_����.strSceneName != ����.strSceneName)
	{
		Space::DeleteSpace����(NickName());
	}
	auto [b��, wpSpace] = Space::GetSpace����(NickName(), ����);
	//m_wpSpace���˾��鸱�� = wpSpace;
	CHECK_WP_RET_VOID(wpSpace);
	EnterSpace(wpSpace);
	CHECK_WP_RET_VOID(m_wp�ӿ�);
	if (b��)
		����.funCo����(*wpSpace.lock(), wpSpace.lock()->m_funCancel����, NickName()).RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg��������ս��& msg)
{
	CHECK_VOID(msg.id == �ķ���ս);
	�������� ����;
	{
		const auto ok = Get��������(msg.id, ����);
		CHECK_RET_VOID(ok);
	}

	//m_spSpace����ս�� = std::make_shared<Space, const ��������&>(����);
	auto wpOld = Space::GetSpace����(NickName());
	if (!wpOld.expired() && wpOld.lock()->m_����.strSceneName != ����.strSceneName)
	{
		Space::DeleteSpace����(NickName());
	}
	auto [b��, wpSpace] = Space::GetSpace����(NickName(), ����);
	//m_wpSpace���˾��鸱�� = wpSpace;
	CHECK_WP_RET_VOID(wpSpace);
	m_wp�ӿ� = EnterSpace(wpSpace);
	CHECK_WP_RET_VOID(m_wp�ӿ�);
	����.funCo����(*wpSpace.lock(), wpSpace.lock()->m_funCancel����, NickName()).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.pos;
	const auto pos = msg.pos;
	const auto b���ֶ��� = !pos.DistanceLessEqual(m_pos�ϴε����·Ŀ��, 5);
	m_pos�ϴε����·Ŀ�� = pos;
	if (m_wpSpace.expired())
	{
		Sayϵͳ("��û����ͼ");
		return;
	}
	auto& refSpace = *m_wpSpace.lock();

	std::vector<WpEntity> vecWp;
	bool b�Ѳ�������(false);
	Position pos���ĵ� = { 0 };
	ForEachSelected([this, msg, &b�Ѳ�������, &vecWp, &pos���ĵ�](Entity& ref)
		{
			if (!ref.m_sp��)
				return;

			if (�콨��Component::���ڽ���(ref))
			{
				PlayerComponent::��������Buzz(ref, "���ڽ��죬�����ƶ�");
				return;
			}

			if (ref.m_sp�ɼ�)
				ref.m_sp�ɼ�->m_TaskCancel.TryCancel();

			if (ref.m_spAttack)
				ref.m_spAttack->TryCancel();

			vecWp.push_back(ref.weak_from_this());

			pos���ĵ� += ref.Pos();
		});

	if (vecWp.empty())
	{
		LOG(WARNING) << "vecWp";
		return;
	}
	pos���ĵ�.x /= vecWp.size();
	pos���ĵ�.z /= vecWp.size();
	for (auto& wp : vecWp)
	{
		auto& ref = *wp.lock();
		auto posƫ�� = ref.Pos() - pos���ĵ�;

		if (ref.m_spAttack)
		{
			const auto f�������ĵ�Max = b���ֶ��� ? ref.m_spAttack->m_ս������.f������� : ref.m_spAttack->m_ս������.f�������� / 5;

			if (std::abs(posƫ��.x) > f�������ĵ�Max)
				posƫ��.x = posƫ��.x / std::abs(posƫ��.x) * f�������ĵ�Max;

			if (std::abs(posƫ��.z) > f�������ĵ�Max)
				posƫ��.z = posƫ��.z / std::abs(posƫ��.z) * f�������ĵ�Max;
		}

		auto posĿ�� = msg.pos + posƫ��;

		if (!refSpace.CrowdTool��վ��(posĿ��))
		{
			LOG(INFO) << posĿ�� << "����վ�����Ҹ����Ŀ�վ����";
			if (!refSpace.CrowdToolFindNerestPos(posĿ��))
			{
				LOG(WARNING) << posĿ�� << "����û�п�վ���ĵ�";
				continue;
			}
			LOG(INFO) << "�ҵ������ĵ�:" << posĿ��;
		}

		��Component::Cancel���а�����·��Э��(ref);
		if (ref.m_spDefence)
			ref.m_spDefence->m_map�����˺�.clear();

		if (msg.b���������Զ�����)
			ref.m_sp��->WalkToPos(posĿ��);
		else
			ref.m_sp��->WalkToPos�ֶ�����(posĿ��);

		if (b�Ѳ�������)
			continue;

		if (ref.m_spAttack)
		{
			switch (ref.m_����)
			{
			case ��:��������(msg.b���������Զ����� ? "����/������������" : "����/��������������", ""); break;//Standing by. ������
			case ��ս��:��������("tfbYes03", ""); break;//Checked up and good to go. �����ϣ�׼������
			case ���̳�:��������(msg.b���������Զ����� ? "����/��Ů���ɰ���" : "����/����Ů���ɰ���", ""); break;
			case ��ɫ̹��:��������("����/ttayes01", ""); break;
			case �ɻ�:��������(msg.b���������Զ����� ? "��Ч/�ɻ���" : "����/�ɻ�����", ""); break;
			default:
				continue;
				break;
			}

			b�Ѳ������� = true;
		}
	}
}

void PlayerGateSession_Game::��������(const std::string& refStrNickName, const std::string& refStr����, const std::string& str�ı�)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->��������(refStr����, str�ı�);
}

void PlayerGateSession_Game::��������Buzz(const std::string& str�ı�)
{
	��������("BUZZ", str�ı�);
}
void PlayerGateSession_Game::��������(const std::string& refStr����, const std::string& str�ı�)
{
	Send<Msg��������>({ .str���� = StrConv::GbkToUtf8(refStr����), .str�ı� = StrConv::GbkToUtf8(str�ı�) });
}

void PlayerGateSession_Game::Send�����ӿ�(const Entity& refEntity)
{
	Send<Msg�����ӿ�>({ .pos�ӿ� = refEntity.Pos() });
}

void PlayerGateSession_Game::ɾ��ѡ��(const uint64_t id)
{
	auto temp = m_vecSelectedEntity;
	temp.erase(std::remove(temp.begin(), temp.end(), id), temp.end());
	ѡ�е�λ(temp);
}

void PlayerGateSession_Game::ForEachSelected(std::function<void(Entity& ref)> fun)
{
	CHECK_VOID(!m_wpSpace.expired());
	auto sp = m_wpSpace.lock();
	auto temp = m_vecSelectedEntity;//��ֹѭ�����޸�����
	for (const auto id : temp)
	{
		auto itFind = sp->m_mapEntity.find(id);
		if (itFind == sp->m_mapEntity.end())
		{
			LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
			//_ASSERT(false);
			continue;
		}
		auto& spEntity = itFind->second;
		auto& refMap = sp->m_mapPlayer[NickName()].m_mapWpEntity;
		if (refMap.end() == std::find_if(refMap.begin(), refMap.end(), [&spEntity](const auto& kv)
			{
				auto& wp = kv.second;
				CHECK_WP_RET_FALSE(wp);
				//_ASSERT(!wp.expired());
				auto sp = wp.lock();
				return sp == spEntity;
			}))
		{
			LOG(ERROR) << id << "�����Լ��ĵ�λ�����ܲ���";
			continue;
		}

		fun(*spEntity);

	}

	//CoAddRole().RunNew();
}

template<class T> void SendToWorldSvr(const T& msg, const uint64_t idGateSession);


bool ����(const Entity& refEntiy, const ��λ���� ����λ����)
{
	if (refEntiy.m_���� != ���̳� && refEntiy.m_���� != ����) return false;
	if (�콨��Component::���ڽ���(refEntiy))return false;

	CHECK_FALSE(refEntiy.m_spAttack);
	if (refEntiy.m_spAttack->m_cancelAttack)return false;
	if (refEntiy.m_spAttack->m_TaskCancel.cancel)return false;

	CHECK_FALSE(refEntiy.m_sp�콨��);
	if (����λ���� > ��λ����_Invalid_0 && !refEntiy.m_sp�콨��->����(����λ����))return false;

	return true;
}
void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//CoAddBuilding(msg.����, msg.pos).RunNew();
	bool b����OK(false);
	ForEachSelected([this, msg, &b����OK](Entity& refEntiy)
		{
			if (!����(refEntiy, msg.����))
				return;

			��Component::Cancel���а�����·��Э��(refEntiy); //TryCancel();
			refEntiy.m_sp�콨��->Co�콨��(msg.pos, msg.����).RunNew();
			b����OK = true;
		});

	if (b����OK)
		return;

	//�Զ���һ�����ʵĹ��̳�ȥ��
	auto vecWp = Get���й��̳�(msg.����, true);
	if (vecWp.empty())
	{
		if (������ == msg.����)
			��������Buzz("û�ҵ����еĹ���");
		else
			��������Buzz("û�ҵ����еĹ��̳�");

		return;
	}

	std::sort(vecWp.begin(), vecWp.end(), [&msg](WpEntity& wp��, WpEntity& wp��) {
		CHECK_WP_RET_FALSE(wp��);
		CHECK_WP_RET_FALSE(wp��);
		auto& ref�� = *wp��.lock();
		auto& ref�� = *wp��.lock();
		const auto b��_�ڲɼ� = �ɼ�Component::���ڲɼ�(ref��);
		const auto b��_�ڲɼ� = �ɼ�Component::���ڲɼ�(ref��);
		if (b��_�ڲɼ� && !b��_�ڲɼ�)
			return false;

		if (!b��_�ڲɼ� && b��_�ڲɼ�)
			return true;

		return msg.pos.DistancePow2(ref��.Pos()) < msg.pos.DistancePow2(ref��.Pos());
		});

	CHECK_WP_RET_VOID(*vecWp.begin());
	auto& wp��һ�� = *vecWp.begin();
	auto& ref��һ�� = *wp��һ��.lock();
	��Component::Cancel���а�����·��Э��(ref��һ��);
	ref��һ��.m_sp�콨��->Co�콨��(msg.pos, msg.����).RunNew();

}

std::vector<WpEntity> PlayerGateSession_Game::Get���й��̳�(const ��λ���� ����λ����, bool b�����ɼ��еĹ��̳�)
{
	CHECK_WP_RET_DEFAULT(m_wpSpace);
	Space& refSpace = *m_wpSpace.lock();
	std::vector<WpEntity> vecWp;
	for (auto [_, wp] : refSpace.m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntity = *wp.lock();
		if (!����(refEntity, ����λ����))
			continue;

		if (!b�����ɼ��еĹ��̳� && �ɼ�Component::���ڲɼ�(refEntity))
			continue;

		vecWp.push_back(wp);
	}
	return vecWp;
}

WpEntity PlayerGateSession_Game::EnterSpace(WpSpace wpSpace)
{
	CHECK_RET_DEFAULT(m_wpSpace.expired());
	CHECK_WP_RET_DEFAULT(wpSpace);
	m_wpSpace = wpSpace;
	auto& refSpace = *m_wpSpace.lock();

	Send<Msg��Space>({ .idSapce = 1 });
	{
		auto& sencePlayer = refSpace.m_mapPlayer[NickName()];
		for (auto [id, wp] : sencePlayer.m_mapWpEntity)
		{
			if (wp.expired())
				continue;

			auto sp = wp.lock();
			PlayerComponent::AddComponent(*sp, *this);
			//refSpace.m_mapPlayer[NickName()].m_mapWpEntity.insert({ sp->Id ,sp });
		}
		//mapOld.clear();


	}
	for (const auto& [id, spEntity] : refSpace.m_mapEntity)//���е�ͼ�ϵ�ʵ�巢���Լ�
	{
		LOG(INFO) << spEntity->ͷ��Name() << ",��������," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
		if (spEntity->m_spBuilding && !spEntity->m_spBuilding->�����())
			Send<MsgEntity����>({ .idEntity = spEntity->Id, .str���� = StrConv::GbkToUtf8(std::format("�������{0}%",spEntity->m_spBuilding->m_n������Ȱٷֱ�)) });
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����&>(
		{ 0.0 }, refSpace, �ӿ�, { "�ӿ�","smoke", "" });
	refSpace.m_mapPlayer[NickName()].m_mapWpEntity[spEntityViewPort->Id] = (spEntityViewPort);
	PlayerComponent::AddComponent(*spEntityViewPort, *this);
	{
		const auto [k, ok] = refSpace.m_map�ӿ�.insert({ spEntityViewPort->Id ,spEntityViewPort });
		CHECK_RET_DEFAULT(ok);
	}
	refSpace.AddEntity(spEntityViewPort, 500);
	spEntityViewPort->BroadcastEnter();
	m_wp�ӿ� = spEntityViewPort;
	CoEvent<MyEvent::��ҽ���Space>::OnRecvEvent({ this->weak_from_this(), spEntityViewPort, wpSpace });
	spEntityViewPort->Broadcast<MsgSay>({ .content = StrConv::GbkToUtf8(NickName() + " ������") });

	auto& playerSpace = refSpace.GetSpacePlayer(NickName());
	if (!playerSpace.m_msg�ϴη���ǰ�˵ľ���Ի�.str�Ի�����.empty())
	{
		Send(playerSpace.m_msg�ϴη���ǰ�˵ľ���Ի�);
		playerSpace.m_msg�ϴη���ǰ�˵ľ���Ի�.str�Ի�����.clear();
	}
	{
		auto strHttps = refSpace.m_����.strHttps����;
		Try��Ini���ػ���ר��(strHttps, "PlayerGateSession_Game", refSpace.m_����.strSceneName + "_����");
		Send<Msg������������>({ .strHttpsMp3 = strHttps });
	}
	return spEntityViewPort;
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	MsgSay msg������ = msg;
	auto strGbk = StrConv::Utf8ToGbk(msg������.content);
	LOG(INFO) << "�յ�����:" << strGbk << ",channel:" << msg.channel;
	msg������.content = StrConv::GbkToUtf8(NickName() + " ˵:\n" + strGbk);
	msg������.channel = ����;
	SendToWorldSvr<MsgSay>(msg������, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	if (m_wpSpace.expired()) {
		LOG(ERROR) << "m_wpSpace";
		return;
	}
	LOG(INFO) << "�յ�ѡ��:" << msg.ids.size();
	std::vector<uint64_t> listSelectedEntity;
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(listSelectedEntity), [](const double& id) {return uint64_t(id); });
	ѡ�е�λ(listSelectedEntity);
}

template<class T_Msg>
void PlayerGateSession_Game::RecvMsg(const msgpack::object& obj)
{
	try
	{
		const auto msg = obj.as<T_Msg>();
		OnRecv(msg);
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << typeid(T_Msg).name() << ",�����л�ʧ��," << error.what();
		_ASSERT(false);
		return;
	}
}

PlayerGateSession_Game::PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession, const std::string& strNickName) :
	m_refGameSvrSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_strNickName(strNickName)
{

}

void PlayerGateSession_Game::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::��Space:RecvMsg<Msg��Space>(obj); break;
	case MsgId::�뿪Space:RecvMsg<Msg�뿪Space>(obj); break;
	case MsgId::�����˾��鸱��:RecvMsg<Msg�����˾��鸱��>(obj); break;
	case MsgId::��������ս��:RecvMsg<Msg��������ս��>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::�ɼ�:RecvMsg<Msg�ɼ�>(obj); break;
	case MsgId::���ر�:RecvMsg<Msg���ر�>(obj); break;
	case MsgId::���ر�:RecvMsg<Msg���ر�>(obj); break;
	case MsgId::��ѡ:RecvMsg<Msg��ѡ>(obj); break;
	case MsgId::��Ҹ���ս���б�:RecvMsg<Msg��Ҹ���ս���б�>(obj); break;
	case MsgId::��������Ҹ���ս��:RecvMsg<Msg��������Ҹ���ս��>(obj); break;
	case MsgId::��Ҷ���ս���б�:RecvMsg<Msg��Ҷ���ս���б�>(obj); break;
	case MsgId::��������Ҷ���ս��:RecvMsg<Msg��������Ҷ���ս��>(obj); break;
	case MsgId::�л����й��̳�:RecvMsg<Msg�л����й��̳�>(obj); break;
	case MsgId::����Ի��ѿ���:RecvMsg<Msg����Ի��ѿ���>(obj); break;
	case MsgId::�����������λ�ļ����:RecvMsg<Msg�����������λ�ļ����>(obj); break;
	case MsgId::Gateת��:
		LOG(ERROR) << "������ת��";
		_ASSERT(false);
		break;
	default:
		LOG(ERROR) << "û����msgId:" << idMsg;
		_ASSERT(false);
		break;
	}
}

void PlayerGateSession_Game::Process()
{
	{
		const auto oldSize = m_vecFunCancel.size();
		std::erase_if(m_vecFunCancel, [](std::shared_ptr<FunCancel>& sp)->bool
			{
				return !(*sp).operator bool();
			});
		const auto newSize = m_vecFunCancel.size();
		if (oldSize != newSize)
		{
			LOG(INFO) << "oldSize:" << oldSize << ",newSize:" << newSize;
		}
	}

	//if (m_spSpace���˾��鸱��)
		//m_spSpace���˾��鸱��->Update();

	//if (m_spSpace����ս��)
		//m_spSpace����ս��->Update();
}


void PlayerGateSession_Game::Send��Դ()
{
	CHECK_WP_RET_VOID(m_wpSpace);
	auto& ref = m_wpSpace.lock()->m_mapPlayer[NickName()];
	Send<Msg��Դ>({ .����� = ref.m_u32�����,
					.ȼ���� = ref.m_u32ȼ����,
					.���λ = ���λ������������е�(),
					.���λ���� = ���λ����() });
}

uint16_t PlayerGateSession_Game::���λ����() const
{
	if (m_wpSpace.expired())
		return 0;

	uint16_t result = 0;
	for (const auto& [_, wp] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		const auto& refEntity = *wp.lock();
		if (!refEntity.m_spBuilding)continue;
		if (!refEntity.m_spBuilding->�����())continue;

		switch (refEntity.m_����)
		{
		case ��:result += 8; break;
		case ����:result += 6; break;
		default:break;
		}
	}
	return result;
}

uint16_t PlayerGateSession_Game::���λ������������е�() const
{
	if (m_wpSpace.expired())
	{
		return 0;
	}

	uint16_t ��������еĵ�λ = 0;
	for (const auto& [_, wp] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		const auto& refEntity = *wp.lock();
		if (EntitySystem::Is�ӿ�(refEntity))
		{
			continue;//���ɹ������Լ��ĵ�λ�����ӿ�
		}
		if (refEntity.m_sp����λ)
		{
			��������еĵ�λ += (uint16_t)refEntity.m_sp����λ->�ȴ���Count();//m_i�ȴ������;

		}

		if (refEntity.m_spBuilding)
			continue;//��

		++��������еĵ�λ;
	}

	return ��������еĵ�λ;

}


void PlayerGateSession_Game::OnRecv(const Msg��ѡ& msg)
{
	if (m_wpSpace.expired())
		return;

	const Position pos����(std::min(msg.pos��ʼ.x, msg.pos����.x), std::min(msg.pos��ʼ.z, msg.pos����.z));
	const Position pos����(std::max(msg.pos��ʼ.x, msg.pos����.x), std::max(msg.pos��ʼ.z, msg.pos����.z));
	const Rect rect = { pos����,pos���� };
	std::vector<uint64_t> vec;
	for (const auto [k, wpEntity] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
	{
		if (wpEntity.expired())
		{
			LOG(ERROR) << "";
			continue;
		}

		auto& refEntity = *wpEntity.lock();
		if (rect.�����˵�(refEntity.Pos()))
			vec.push_back(refEntity.Id);
	}
	ѡ�е�λ(vec);
}

void PlayerGateSession_Game::ѡ�е�λ(const std::vector<uint64_t>& vecId)
{
	m_vecSelectedEntity.clear();
	bool b�ѷ���ѡ����Ч(false);

	if (m_wpSpace.expired())
	{
		LOG(ERROR) << "";
		return;
	}
	auto& refSpace = *m_wpSpace.lock();
	for (const auto id : vecId)
	{
		auto wpEntity = refSpace.GetEntity(id);
		if (wpEntity.expired())
		{
			LOG(WARNING) << "����ѡ�����ѽ��ر��ı�";
			continue;
		}
		auto spEntity = wpEntity.lock();
		if (!spEntity->m_wpOwner.expired())
			continue;//�ر���

		if (spEntity->m_spBuilding && vecId.size() > 1)
			continue;//������λĿǰֻ�ܵ�ѡ

		if (EntitySystem::Is�ӿ�(*spEntity))
			continue;

		if (!spEntity->m_spPlayer)
			continue;

		if (&spEntity->m_spPlayer->m_refSession != this)//�����Լ��ĵ�λ
		{
			if (vecId.size() == 1)//��ѡһ���з���λ�������߹�ȥ��
			{
				MsgMove msg = { .pos = spEntity->Pos(),.b���������Զ����� = true };
				OnRecv(msg);
				return;
			}
			continue;
		}

		m_vecSelectedEntity.push_back(spEntity->Id);
		if (!b�ѷ���ѡ����Ч)
		{
			b�ѷ���ѡ����Ч = true;
			Sendѡ����Ч(*spEntity);
		}
	}
	Sendѡ�е�λResponce();
}

void PlayerGateSession_Game::Sendѡ����Ч(const Entity& refEntity)
{
	if (refEntity.m_spAttack)
	{
		��������(refEntity.m_����.strѡ����Ч, "");
	}
	else if (refEntity.m_spBuilding)
	{
		//switch (refEntity.m_spBuilding->m_����)
		//{
		//case ����:��������("tcsWht00"); break;
		//case ����:��������("tacWht00"); break;
		//case ��:��������("tclWht00"); break;
		//default:
		//	break;
		//}
		��������(refEntity.m_����.strѡ����Ч, "");
	}
	else if (refEntity.m_sp��Դ) {
		switch (refEntity.m_sp��Դ->m_����)
		{
		case �����:
		case ȼ����:
		default:
			��������("��Ч/BUTTON", "");
			break;
		}
	}
}

void PlayerGateSession_Game::Sendѡ�е�λResponce()
{
	MsgSelectRoles msgResponse;
	msgResponse.ids.insert(msgResponse.ids.end(), m_vecSelectedEntity.begin(), m_vecSelectedEntity.end());
	Send(msgResponse);
}

void PlayerGateSession_Game::OnRecv(const Msg��Ҹ���ս���б�& msg)
{
	Msg��Ҹ���ս���б�Responce msgResponce;
	//for (const auto [id, sp] : m_refGameSvrSession.m_mapPlayerGateSession)
	for (auto [strNickName, sp] : Space::����ս��())
	{
		//if (!sp->m_spSpace���˾��鸱��)
		//	continue;

		if (sp->m_����.strSceneName == "scene�ķ���ս")
			continue;

		msgResponce.vec����ս���е����.push_back(
			{
				StrConv::GbkToUtf8(strNickName),
				StrConv::GbkToUtf8(sp->m_����.strSceneName)
			});
	}
	Send(msgResponce);
}
void PlayerGateSession_Game::OnRecv(const Msg��Ҷ���ս���б�& msg)
{
	Msg��Ҷ���ս���б�Responce msgResponce;
	//for (const auto [id, sp] : m_refGameSvrSession.m_mapPlayerGateSession)
	for (auto [strNickName, sp] : Space::����ս��())
	{
		//if (!sp->m_spSpace����ս��)
			//continue;
		if (sp->m_����.strSceneName != "scene�ķ���ս")
			continue;

		msgResponce.vec����ս���е�Host���.push_back(
			{
				StrConv::GbkToUtf8(strNickName),
				StrConv::GbkToUtf8(sp->m_����.strSceneName)//sp->m_spSpace����ս��->m_����.strSceneName)
			});
	}
	Send(msgResponce);
}

void PlayerGateSession_Game::OnRecv(const Msg��������Ҹ���ս��& msg)
{
	const auto strGbk = StrConv::Utf8ToGbk(msg.nickName�������);
	//auto iterFind = std::find_if(m_refGameSvrSession.m_mapPlayerGateSession.begin(), m_refGameSvrSession.m_mapPlayerGateSession.end(),
	//	[&strGbk](const auto& pair)->bool
	//	{
	//		return pair.second->NickName() == strGbk;
	//	});
	auto wpSpace = Space::GetSpace����(strGbk);
	CHECK_WP_RET_VOID(wpSpace);
	EnterSpace(wpSpace);
}

void PlayerGateSession_Game::OnRecv(const Msg��������Ҷ���ս��& msg)
{
	const auto strGbk = StrConv::Utf8ToGbk(msg.nickName�������);
	//auto iterFind = std::find_if(m_refGameSvrSession.m_mapPlayerGateSession.begin(), m_refGameSvrSession.m_mapPlayerGateSession.end(),
	//	[&strGbk](const auto& pair)->bool
	//	{
	//		return pair.second->NickName() == strGbk;
	//	});
	//CHECK_RET_VOID(iterFind != m_refGameSvrSession.m_mapPlayerGateSession.end());
	//auto& refSp = iterFind->second->m_spSpace����ս��;
	auto wpSpace = Space::GetSpace����(strGbk);
	CHECK_WP_RET_VOID(wpSpace);
	EnterSpace(wpSpace);
}

void PlayerGateSession_Game::OnRecv(const Msg�л����й��̳�& msg)
{
	auto vecWp = Get���й��̳�(��λ����_Invalid_0, false);
	if (vecWp.empty())
	{
		��������Buzz("û�п��еĹ��̳�");
		return;
	}
	m_idx�л����̳� = m_idx�л����̳� % vecWp.size();
	auto& wp = vecWp[m_idx�л����̳�];
	CHECK_WP_RET_VOID(wp);
	Entity& refEntity = *wp.lock();
	ѡ�е�λ({ refEntity.Id });
	Send�����ӿ�(refEntity);
	++m_idx�л����̳�;
}

void PlayerGateSession_Game::OnRecv(const Msg����Ի��ѿ���& msg)
{
	CoEvent<MyEvent::���Ķ�����Ի�>::OnRecvEvent({ .wpPlayerGateSession = weak_from_this() });
}
void PlayerGateSession_Game::����Ի��ѿ���()
{
	Send<Msg����Ի��ѿ���>({});
}

void PlayerGateSession_Game::OnRecv(const Msg�����������λ�ļ����& msg)
{
	CHECK_WP_RET_VOID(m_wpSpace);
	auto& refSpace = *m_wpSpace.lock();
	auto& refSpacePlayer = refSpace.GetSpacePlayer(NickName());

	CHECK_RET_VOID(!m_vecSelectedEntity.empty());
	auto iterFind = refSpacePlayer.m_mapWpEntity.find(*m_vecSelectedEntity.begin());
	CHECK_RET_VOID(refSpacePlayer.m_mapWpEntity.end() != iterFind);

	auto& wp�ҷ����� = iterFind->second;
	CHECK_WP_RET_VOID(wp�ҷ�����);

	auto& refEntity�ҷ����� = *wp�ҷ�����.lock();
	CHECK_RET_VOID(EntitySystem::Is����(refEntity�ҷ�����.m_����));
	CHECK_RET_VOID(refEntity�ҷ�����.m_sp����λ);

	refEntity�ҷ�����.m_sp����λ->m_pos����� = msg.pos;
	Sayϵͳ("�޸�����Ч���˽����²����Ļ��λ�����Զ�����˼���㡣");
}