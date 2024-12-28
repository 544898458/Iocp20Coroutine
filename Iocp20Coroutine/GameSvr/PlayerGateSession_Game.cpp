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
					this->m_refSession.SendToGate(bufת��, lenת��);
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
	for (auto [_, wp] : m_mapWpEntity)
	{
		//assert(!wp.expired());
		if (wp.expired())
		{
			LOG(ERROR) << "ɾ�˵�λ����������ûɾ";
			continue;
		}
		auto sp = wp.lock();
		if (m_spSpace���˾��鸱�� || EntitySystem::Is�ӿ�(*sp))
		{
			if (sp->m_refSpace.GetEntity(sp->Id).expired())
			{
				LOG(INFO) << "�����ǵر���ı�" << sp->NickName();
				continue;
			}
			LOG(INFO) << "m_mapEntity.size=" << sp->m_refSpace.m_mapEntity.size();
			sp->OnDestroy();
			auto countErase = sp->m_refSpace.m_mapEntity.erase(sp->Id);
			assert(1 == countErase);
		}
		else
		{
			CHECK_RET_VOID(!m_wpSpace.expired());
			sp->m_spPlayer.reset();
			m_wpSpace.lock()->m_map������PlayerEntity[NickName()].insert({ sp->Id,sp });
		}
	}

	m_mapWpEntity.clear();

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
	m_spSpace���˾��鸱��.reset();

	if (b�뿪)
		Send<Msg�뿪Space>({});
}

void PlayerGateSession_Game::Erase(uint64_t u64Id)
{
	if (!m_mapWpEntity.contains(u64Id))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_mapWpEntity.erase(u64Id);
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

CoTaskBool PlayerGateSession_Game::Co������������ͼ()
{
	auto pos���� = Position(std::rand() % 100 - 50, std::rand() % 50 - 25);
	{
		const ���λ���� ����(���λ����::��);
		��λ::���λ���� ����;
		��λ::Find���λ����(����, ����);
		����λComponent::����λ(*this, { pos����.x, pos����.z + 6 }, ����, ����);
	}
	{
		const ���λ���� ����(���λ����::��ɫ̹��);
		��λ::���λ���� ����;
		��λ::Find���λ����(����, ����);

		����λComponent::����λ(*this, { pos����.x + 6, pos����.z }, ����, ����);
	}
	{
		const ���λ���� ����(���λ����::���̳�);
		��λ::���λ���� ����;
		��λ::Find���λ����(����, ����);

		SpEntity sp���̳� = ����λComponent::����λ(*this, pos����, ����, ����);
		Send�����ӿ�(*sp���̳�);
	}
	auto [stop, msgResponce] = co_await AiCo::ChangeMoney(*this, 0, true, m_funCancel����ͼ);
	if (stop)
		co_return true;

	const uint16_t u16��ʼ�����(100);
	if (msgResponce.finalMoney < u16��ʼ�����)
	{
		if (std::get<0>(co_await AiCo::ChangeMoney(*this, u16��ʼ�����, true, m_funCancel����ͼ)))
			co_return true;
	}

	using namespace std;
	const auto seconds��Ϣ��� = 10s;
	Say("����ÿ����Ҷ��������ɹ�ͬ����ĳ������ֲ���һЩ��Դ�������Ĺ֣���Դ�������ٶȺ���", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("������ֻ��һ̨���̳������̳����Խ��콨���������п��Բ������λ", SayChannel::ϵͳ);
	if (co_await CoTimer::Wait(seconds��Ϣ���, m_funCancel����ͼ)) co_return false;
	Say("һ�����˳��˳�������ߣ����ڴ˳����е����е�λ������ʧ", SayChannel::ϵͳ);
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
	EnterSpace(Space::GetSpace(msg.idSapce));

	if (m_funCancel����ͼ)
		m_funCancel����ͼ();

	Co������������ͼ().RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg�뿪Space& msg)
{
	LOG(INFO) << "ϣ���뿪Space:";// << msg.idSapce;
	OnDestroy();
}

typedef CoTask<int>(*funCo��������)(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
struct ��������
{
	std::string strѰ·�ļ���;
	funCo�������� funCo����;
};

std::unordered_map<���˾��鸱��ID, ��������> g_map�������� =
{
	{ѵ��ս,{"all_tiles_tilecache.bin",���˾���::Coѵ��ս}},
	{����ս,{"����ս.bin",���˾���::Co����ս}},
};

void PlayerGateSession_Game::OnRecv(const Msg�����˾��鸱��& msg)
{
	const auto itFind = g_map��������.find(msg.id);
	if (itFind == g_map��������.end())
	{
		assert(false);
		return;
	}
	const auto& ref���� = itFind->second;
	m_spSpace���˾��鸱�� = std::make_shared<Space, const std::string&>(ref����.strѰ·�ļ���);
	EnterSpace(m_spSpace���˾��鸱��);

	ref����.funCo����(*m_spSpace���˾��鸱��, m_funCancel����ͼ, *this).RunNew();
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
				switch (ref.m_spAttack->m_����)
				{
				case ��:��������("TMaYes00"); break;//Standing by. ������
				case ��ս��:��������("tfbYes03"); break;//Checked up and good to go. �����ϣ�׼������
				case ���̳�:��������("����/��Ů���ɰ���"); break;
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

void PlayerGateSession_Game::ForEachSelected(std::function<void(Entity& ref)> fun)
{
	CHECK_VOID(!m_wpSpace.expired());
	auto sp = m_wpSpace.lock();
	for (const auto id : m_listSelectedEntity)
	{
		auto itFind = sp->m_mapEntity.find(id);
		if (itFind == sp->m_mapEntity.end())
		{
			LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
			//assert(false);
			continue;
		}
		auto& spEntity = itFind->second;
		if (m_mapWpEntity.end() == std::find_if(m_mapWpEntity.begin(), m_mapWpEntity.end(), [&spEntity](const auto& kv)
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

CoTask<SpEntity> PlayerGateSession_Game::CoAddBuilding(const ������λ���� ����, const Position pos)
{
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		co_return{};
	}
	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!�ɷ��ý���(pos, ����.f��߳�))
	{
		//��������("TSCErr00", "���赲���޷�����");//��Err00�� I can't build it, something's in the way. ��û�����⽨���ж�������
		��������("����/�޷������ｨ��ɰ���", "���赲���޷�����");
		co_return{};
	}
	if (����.����.u16����ȼ���� > m_u32ȼ����)
	{
		//std::ostringstream oss;
		��������("����/ȼ������ɰ���", "ȼ�����㣬�޷�����");// << ����.����.u16����ȼ����;//(low error beep) Insufficient Vespene Gas.������ 
		//Sayϵͳ(oss.str());
		co_return{};
	}
	m_u32ȼ���� -= ����.����.u16����ȼ����;
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//���ܴ�������ݿ��ܵ������ú�ָ��ʧЧ
	//auto [stop, responce] = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = ����.����.u16���ľ���� },
	//	[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	auto [stop, responce] = co_await AiCo::ChangeMoney(*this, ����.����.u16���ľ����, false, **iterNew);
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	if (stop)
	{
		m_u32ȼ���� += ����.����.u16����ȼ����;//����ȼ����
		co_return{};
	}
	if (0 != responce.error)
	{
		//LOG(WARNING) << "��Ǯʧ��,error=" << responce.error;
		m_u32ȼ���� += ����.����.u16����ȼ����;//����ȼ����
		��������("����/�������ɰ���", "��������޷�����");//Sayϵͳ("��������" + ����.����.u16���ľ����);

		co_return{};
	}

	Send��Դ();

	//�ӽ���
	CHECK_CO_RET_0(!m_wpSpace.expired());
	auto spSpace = m_wpSpace.lock();
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const ��λ::��λ����& >(
		pos, *spSpace, ����.����);
	//spNewEntity->AddComponentAttack();
	PlayerComponent::AddComponent(*spNewEntity, *this);
	BuildingComponent::AddComponent(*spNewEntity, *this, ����, ����.f��߳�);
	switch (����)
	{
	case ����:
	case ����:
		����λComponent::AddComponent(*spNewEntity, *this, ����);
		break;
	case �ر�:
		�ر�Component::AddComponet(*spNewEntity, *this);
		break;
	case ��:break;
	}
	DefenceComponent::AddComponent(*spNewEntity, ����.����.u16��ʼHp);
	//spNewEntity->m_spBuilding->m_fun����λ = ����.fun���;
	m_mapWpEntity[spNewEntity->Id] = spNewEntity;//�Լ����Ƶĵ�λ
	//spSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ
	spSpace->AddEntity(spNewEntity);

	spNewEntity->BroadcastEnter();
	Send��Դ();
	co_return spNewEntity;
}

void PlayerGateSession_Game::EnterSpace(WpSpace wpSpace)
{
	assert(m_wpSpace.expired());
	assert(!wpSpace.expired());
	m_wpSpace = wpSpace;
	auto sp = m_wpSpace.lock();

	Send<Msg��Space>({ .idSapce = 1 });
	{
		auto mapOld = sp->m_map������PlayerEntity[NickName()];
		for (auto [id, wp] : mapOld)
		{
			if (wp.expired())
				continue;

			auto sp = wp.lock();
			PlayerComponent::AddComponent(*sp, *this);
			m_mapWpEntity.insert({ sp->Id ,sp });
		}
		mapOld.clear();
	}
	for (const auto& [id, spEntity] : sp->m_mapEntity)//���е�ͼ�ϵ�ʵ�巢���Լ�
	{
		LOG(INFO) << spEntity->NickName() << ",��������," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const ��λ::��λ����&>(
		{ 0.0 }, *sp, { "�ӿ�","smoke", "" });
	m_mapWpEntity[spEntityViewPort->Id] = (spEntityViewPort);
	PlayerComponent::AddComponent(*spEntityViewPort, *this);
	sp->AddEntity(spEntityViewPort, 100);
	spEntityViewPort->BroadcastEnter();

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
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
	CHECK_VOID(!m_wpSpace.expired());
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
	m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_strNickName(strNickName)
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
	Send<Msg��Դ>({ .ȼ���� = m_u32ȼ����,.���λ = ���λ������������е�(),.���λ���� = ���λ����() });
}

uint16_t PlayerGateSession_Game::���λ����() const
{
	uint16_t result = 0;
	for (const auto& [_, wp] : m_mapWpEntity)
	{
		assert(!wp.expired());
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
	uint16_t ��������еĵ�λ = 0;
	for (const auto& [_, wp] : m_mapWpEntity)
	{
		assert(!wp.expired());
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

bool PlayerGateSession_Game::�ɷ��ý���(const Position& refPos, float f��߳�)
{

	CHECK_FALSE(!m_wpSpace.expired());
	auto spSpace = m_wpSpace.lock();

	if (!spSpace->CrowdTool��վ��({ refPos.x - f��߳� ,refPos.z + f��߳� }))return false;
	if (!spSpace->CrowdTool��վ��({ refPos.x - f��߳� ,refPos.z - f��߳� }))return false;
	if (!spSpace->CrowdTool��վ��({ refPos.x + f��߳� ,refPos.z + f��߳� }))return false;
	if (!spSpace->CrowdTool��վ��({ refPos.x + f��߳� ,refPos.z - f��߳� }))return false;

	//����ȫ��ͼ���н����ж��ص�
	CHECK_RET_FALSE(!m_wpSpace.expired());
	for (const auto& kv : m_wpSpace.lock()->m_mapEntity)
	{
		auto& refEntity = *kv.second;
		const auto& refPosOld = refEntity.Pos();
		bool CrowdTool�жϵ�λ�ص�(const Position & refPosOld, const Position & refPosNew, const float f��߳�);
		if (CrowdTool�жϵ�λ�ص�(refPos, refPosOld, f��߳�))
			return false;
	}

	return true;
}

void PlayerGateSession_Game::OnRecv(const Msg��ѡ& msg)
{
	const Position pos����(std::min(msg.pos��ʼ.x, msg.pos����.x), std::min(msg.pos��ʼ.z, msg.pos����.z));
	const Position pos����(std::max(msg.pos��ʼ.x, msg.pos����.x), std::max(msg.pos��ʼ.z, msg.pos����.z));
	const Rect rect = { pos����,pos���� };
	std::vector<uint64_t> vec;
	for (const auto [k, wpEntity] : m_mapWpEntity)
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
	m_listSelectedEntity.clear();
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

		m_listSelectedEntity.push_back(spEntity->Id);
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
	msgResponse.ids.insert(msgResponse.ids.end(), m_listSelectedEntity.begin(), m_listSelectedEntity.end());
	Send(msgResponse);
}
