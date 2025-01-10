#include "pch.h"
#include "PlayerGateSession_Game.h"
#include "Space.h"
#include "Entity.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoEvent.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "DefenceComponent.h"
#include "�ɼ�Component.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "BuildingComponent.h"
#include <unordered_map>
#include "��λ.h"
#include "���˾���.h"
#include <sstream>
#include "����λComponent.h"
#include "�콨��Component.h"
#include "�ر�Component.h"
#include "��Component.h"
#include "PlayerComponent.h"
#include "��ԴComponent.h"
#include "../CoRoutine/CoTimer.h"
#include "EntitySystem.h"
#include "PlayerNickNameComponent.h"
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


void PlayerGateSession_Game::OnDestroy()
{
	if (!m_wpSpace.expired())
		m_wpSpace.lock()->m_mapPlayer[NickName()].OnDestroy((bool)m_spSpace���˾��鸱��, *m_wpSpace.lock(), NickName());

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
		m_funCancel����ͼ();

	const bool b�뿪 = !m_wpSpace.expired();
	m_wpSpace.reset();
	if (m_spSpace���˾��鸱��)
	{
		m_spSpace���˾��鸱��->OnDestory();
		m_spSpace���˾��鸱��.reset();
	}

	if (b�뿪)
		Send<Msg�뿪Space>({});
}



void PlayerGateSession_Game::Say(const std::string& str, const SayChannel channel)
{
	Send< MsgSay>({ .content = StrConv::GbkToUtf8(str),.channel = channel });
}
void PlayerGateSession_Game::Sayϵͳ(const std::string& str)
{
	Say(str, SayChannel::ϵͳ);
}

void PlayerGateSession_Game::Say������ʾ(const std::string& str)
{
	Say(str, SayChannel::������ʾ);
}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			if (!ref.m_sp����λ)
			{
				PlayerComponent::��������(ref, "BUZZ", "�첻��");
				return;
			}

			ref.m_sp����λ->���(*this, ref, msg.����);
		});
}

void PlayerGateSession_Game::OnRecv(const Msg�ɼ�& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			CHECK_VOID(!m_wpSpace.expired());
			auto wpEntity = m_wpSpace.lock()->GetEntity((int64_t)msg.idĿ����Դ);
			CHECK_RET_VOID(!wpEntity.expired());

			if (!ref.m_sp�ɼ�)
			{
				Sayϵͳ("�˵�λ�޷��ɼ���Դ");
				return;
			}

			ref.m_sp�ɼ�->�ɼ�(*this, wpEntity);
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
	std::list<std::function<void()>> listFun;
	ForEachSelected([this, &msg, &listFun](Entity& ref)
		{
			CHECK_VOID(!m_wpSpace.expired());
			auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.idĿ��ر�);
			CHECK_RET_VOID(!wpTarget.expired());
			auto spTarget = wpTarget.lock();
			if (!spTarget->m_sp�ر�)
			{
				Sayϵͳ("Ŀ�겻�ǵر�");
				return;
			}

			if (!ref.m_spAttack)
			{
				Sayϵͳ("�˵�λ���ɽ���ر�");
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
		auto pos���� = Position(std::rand() % 100 - 50.f, std::rand() % 50 - 25.f);
		{
			const ��λ���� ����(��λ����::��);
			��λ::���λ���� ����;
			��λ::Find���λ����(����, ����);
			refSpace.����λ(ref�ӿ�.m_spPlayer, NickName(), { pos����.x, pos����.z + 6 }, ����, ����);
		}
		{
			const ��λ���� ����(��λ����::��ɫ̹��);
			��λ::���λ���� ����;
			��λ::Find���λ����(����, ����);

			refSpace.����λ(ref�ӿ�.m_spPlayer, NickName(), { pos����.x + 6, pos����.z }, ����, ����);
		}
		{
			const ��λ���� ����(��λ����::���̳�);
			��λ::���λ���� ����;
			��λ::Find���λ����(����, ����);

			SpEntity sp���̳� = refSpace.����λ(ref�ӿ�.m_spPlayer, NickName(), pos����, ����, ����);
			Send�����ӿ�(*sp���̳�);
		}
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
	Say("������ֻ��һ̨���̳������̳����Խ��콨���������п��Բ������λ", SayChannel::ϵͳ);
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
	OnDestroy();
	LOG(INFO) << "ϣ����Space:" << msg.idSapce;
	auto wp = Space::GetSpace(msg.idSapce);
	CHECK_WP_RET_VOID(wp);
	auto wp�ӿ� = EnterSpace(wp);

	if (m_funCancel����ͼ)
		m_funCancel����ͼ();

	Co������������ͼ(wp�ӿ�).RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg�뿪Space& msg)
{
	LOG(INFO) << "ϣ���뿪Space:";// << msg.idSapce;
	OnDestroy();
}

std::unordered_map<����ID, ��������> g_map�������� =
{
	{ѵ��ս,{"all_tiles_tilecache.bin",		"sceneս��",	���˾���::Coѵ��ս}},
	{����ս,{"����ս.bin",					"scene����ս",	���˾���::Co����ս}},
	{����������ͼ,{"all_tiles_tilecache.bin","sceneս��",	{}}},
};

bool Get��������(const ����ID id, ��������& refOut)
{
	const auto itFind = g_map��������.find(id);
	if (itFind == g_map��������.end())
	{
		assert(false);
		return false;
	}

	refOut = itFind->second;
	return true;
}

void PlayerGateSession_Game::OnRecv(const Msg�����˾��鸱��& msg)
{
	�������� ����;
	{
		const auto ok = Get��������(msg.id, ����);
		CHECK_RET_VOID(ok);
	}

	m_spSpace���˾��鸱�� = std::make_shared<Space, const ��������&>(����);
	auto wp�ӿ� = EnterSpace(m_spSpace���˾��鸱��);
	CHECK_WP_RET_VOID(wp�ӿ�);
	����.funCo����(*m_spSpace���˾��鸱��, *wp�ӿ�.lock(), m_funCancel����ͼ, *this).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.pos;
	const auto pos = msg.pos;
	if (m_wpSpace.expired())
	{
		Sayϵͳ("��û����ͼ");
		return;
	}
	bool b�Ѳ�������(false);
	ForEachSelected([this, msg, &b�Ѳ�������](Entity& ref)
		{
			if (!ref.m_sp��)
				return;

			if (�콨��Component::���ڽ���(ref))
			{
				PlayerComponent::��������(ref, "BUZZ", "���ڽ��죬�����ƶ�");
				return;
			}

			if (ref.m_sp�ɼ�)
				ref.m_sp�ɼ�->m_TaskCancel.TryCancel();

			if (ref.m_spAttack)
				ref.m_spAttack->TryCancel();

			��Component::Cancel���а�����·��Э��(ref);
			if (msg.b���������Զ�����)
				ref.m_sp��->WalkToPos(msg.pos);
			else
				ref.m_sp��->WalkToPos�ֶ�����(msg.pos);

			if (b�Ѳ�������)
				return;

			b�Ѳ������� = true;
			if (ref.m_spAttack)
			{
				switch (ref.m_����)
				{
				case ��:��������(msg.b���������Զ����� ? "����/������������" : "����/��������������"); break;//Standing by. ������
				case ��ս��:��������("tfbYes03"); break;//Checked up and good to go. �����ϣ�׼������
				case ���̳�:��������(msg.b���������Զ����� ? "����/��Ů���ɰ���" : "����/����Ů���ɰ���"); break;
				case ��ɫ̹��:��������("����/ttayes01"); break;
				default:break;
				}
			}
			//else if (ref.m_spBuilding) 
			//{
			//	switch (ref.m_spBuilding->m_����)
			//	{
			//	case ����:��������("tcsWht00"); break;
			//	case ����:��������("tclWht00"); break;
			//	default:
			//		break;
			//	}
			//}
		});
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
			//assert(false);
			continue;
		}
		auto& spEntity = itFind->second;
		auto& refMap = sp->m_mapPlayer[NickName()].m_mapWpEntity;
		if (refMap.end() == std::find_if(refMap.begin(), refMap.end(), [&spEntity](const auto& kv)
			{
				auto& wp = kv.second;
				assert(!wp.expired());
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

void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//CoAddBuilding(msg.����, msg.pos).RunNew();
	ForEachSelected([this, msg](Entity& ref)
		{
			if (�콨��Component::���ڽ���(ref))
			{
				PlayerComponent::��������(ref, "BUZZ", "���ڽ��죬���ܽ���");
				return;
			}

			if (ref.m_spAttack)
			{
				if (ref.m_spAttack->m_cancelAttack)
				{
					LOG(INFO) << "���ڹ������ܽ���";
					return;
				}
				if (ref.m_spAttack->m_TaskCancel.cancel)
				{
					LOG(INFO) << "�������򹥻�Ŀ�겻�ܽ���";
					return;
				}
			}
			if (!ref.m_sp�콨��)
			{
				PlayerComponent::��������(ref, "BUZZ", "�첻�����ֽ���");
				return;
			}

			��Component::Cancel���а�����·��Э��(ref); //TryCancel();
			ref.m_sp�콨��->Co�콨��(msg.pos, msg.����).RunNew();
		});
}

WpEntity PlayerGateSession_Game::EnterSpace(WpSpace wpSpace)
{
	assert(m_wpSpace.expired());
	assert(!wpSpace.expired());
	m_wpSpace = wpSpace;
	auto spSpace = m_wpSpace.lock();

	Send<Msg��Space>({ .idSapce = 1 });
	{
		auto mapOld = spSpace->m_map������PlayerEntity[NickName()];
		for (auto [id, wp] : mapOld)
		{
			if (wp.expired())
				continue;

			auto sp = wp.lock();
			PlayerComponent::AddComponent(*sp, *this);
			spSpace->m_mapPlayer[NickName()].m_mapWpEntity.insert({ sp->Id ,sp });
		}
		mapOld.clear();
	}
	for (const auto& [id, spEntity] : spSpace->m_mapEntity)//���е�ͼ�ϵ�ʵ�巢���Լ�
	{
		LOG(INFO) << spEntity->NickName() << ",��������," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����&>(
		{ 0.0 }, *spSpace, �ӿ�, { "�ӿ�","smoke", "" });
	spSpace->m_mapPlayer[NickName()].m_mapWpEntity[spEntityViewPort->Id] = (spEntityViewPort);
	PlayerComponent::AddComponent(*spEntityViewPort, *this);
	{
		const auto [k, ok] = spSpace->m_map�ӿ�.insert({ spEntityViewPort->Id ,spEntityViewPort });
		CHECK_RET_DEFAULT(ok);
	}
	spSpace->AddEntity(spEntityViewPort, 100);
	spEntityViewPort->BroadcastEnter();

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
	return spEntityViewPort;
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	MsgSay msg������ = msg;
	auto strGbk = StrConv::Utf8ToGbk(msg������.content);
	LOG(INFO) << "�յ�����:" << strGbk;
	msg������.content = StrConv::GbkToUtf8(NickName() + " ˵:" + strGbk);
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
		assert(false);
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
	case MsgId::Gateת��:
		LOG(ERROR) << "������ת��";
		assert(false);
		break;
	default:
		LOG(ERROR) << "û����msgId:" << idMsg;
		assert(false);
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

	if (m_spSpace���˾��鸱��)
		m_spSpace���˾��鸱��->Update();
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
		const auto& refEntity = wp.lock();
		if (!refEntity->m_spBuilding)continue;
		if (!refEntity->m_spBuilding->�����())continue;

		switch (refEntity->m_spBuilding->m_����)
		{
		case ��:result += 5; break;
		case ����:result += 2; break;
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
	//for (const auto [k, wp] : m_mapWpEntity)
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

		//if (spEntity->m_spBuilding)
		//	continue;//���ɿ�ѡ������λ

		if (EntitySystem::Is�ӿ�(*spEntity))
			continue;

		if (!spEntity->m_spPlayer)
			continue;

		if (&spEntity->m_spPlayer->m_refSession != this)//�����Լ��ĵ�λ
			continue;

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
		//switch (refEntity.m_spAttack->m_����)
		//{
		//case ��:��������("TMaPss00"); break;// Say������ʾ("������!"); break;//Standing by. ������
		//case ��ս��:��������("tfbPss00"); break;//Say������ʾ("׼���ж�!"); break;//Checked up and good to go. �����ϣ�׼������
		//case ���̳�:��������("TSCPss00"); break;//Commander.
		//default:break;
		//}
		��������(refEntity.m_����.strѡ����Ч);
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
		��������(refEntity.m_����.strѡ����Ч);
	}
	else if (refEntity.m_sp��Դ) {
		switch (refEntity.m_sp��Դ->m_����)
		{
		case �����:
		case ȼ����:
		default:
			��������("��Ч/BUTTON");
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
	for (const auto [id, sp] : m_refGameSvrSession.m_mapPlayerGateSession)
	{
		if (!sp->m_spSpace���˾��鸱��)
			continue;

		msgResponce.vec����ս���е����.push_back(
			{
				StrConv::GbkToUtf8(sp->NickName()),
				StrConv::GbkToUtf8(sp->m_spSpace���˾��鸱��->m_����.strSceneName)
			});
	}
	Send(msgResponce);
}

void PlayerGateSession_Game::OnRecv(const Msg��������Ҹ���ս��& msg)
{
	const auto strGbk = StrConv::Utf8ToGbk(msg.nickName�������);
	auto iterFind = std::find_if(m_refGameSvrSession.m_mapPlayerGateSession.begin(), m_refGameSvrSession.m_mapPlayerGateSession.end(),
		[&strGbk](const auto& pair)->bool
		{
			return pair.second->NickName() == strGbk;
		});
	CHECK_RET_VOID(iterFind != m_refGameSvrSession.m_mapPlayerGateSession.end());
	auto& refSp = iterFind->second->m_spSpace���˾��鸱��;
	CHECK_RET_VOID(refSp);
	EnterSpace(refSp);
}