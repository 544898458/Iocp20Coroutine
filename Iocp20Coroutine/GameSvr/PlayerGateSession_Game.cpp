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
#include "�ر�Component.h"
#include "��Component.h"
#include "PlayerComponent.h"

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


void PlayerGateSession_Game::OnDestroy()
{
	for (auto sp : m_setSpEntity)
	{
		sp->m_refSpace.m_mapEntity.erase((int64_t)sp.get());
		LOG(INFO) << "m_mapEntity.size=" << sp->m_refSpace.m_mapEntity.size();
		sp->OnDestroy();
	}

	m_setSpEntity.clear();

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

	if (m_funCancel���˾���)
		m_funCancel���˾���();
}

void PlayerGateSession_Game::Erase(SpEntity& spEntity)
{
	if (!m_setSpEntity.contains(spEntity))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_setSpEntity.erase(spEntity);
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
				Sayϵͳ("�첻��");
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

			listFun.emplace_back([&ref, &wpTarget, this]()
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

void PlayerGateSession_Game::OnRecv(const Msg��Space& msg)
{
	LOG(INFO) << "ϣ����Space:" << msg.idSapce;
	EnterSpace(Space::GetSpace(msg.idSapce), this->NickName());
}

void PlayerGateSession_Game::OnRecv(const Msg�����˾��鸱��& msg)
{
	m_spSpace���˾��鸱�� = std::make_shared<Space>();
	EnterSpace(m_spSpace���˾��鸱��, this->NickName());
	���˾���::Co(*m_spSpace���˾��鸱��, m_funCancel���˾���, *this).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	if (m_wpSpace.expired())
	{
		Sayϵͳ("��û����ͼ");
		return;
	}
	ForEachSelected([this, targetX, targetZ](Entity& ref)
		{
			if (!ref.m_sp��)
				return;

			if (ref.m_sp�ɼ�)
				ref.m_sp�ɼ�->m_TaskCancel.TryCancel();

			if (ref.m_spAttack)
				ref.m_spAttack->TryCancel();

			ref.m_sp��->TryCancel();
			ref.m_sp��->WalkToPos�ֶ�����(Position(targetX, targetZ));
			Say������ʾ("������!");//Go! Go! Go!

		});
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
		if (m_setSpEntity.end() == std::find_if(m_setSpEntity.begin(), m_setSpEntity.end(), [&spEntity](const auto& sp) {return sp == spEntity; }))
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
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpcǰһ��Э�̻�û����";
	//	return;
	//}
	/*auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddBuilding(msg.����));
	if (iterNew == m_vecCoRpc.end())
	{
		LOG(ERROR) << "err";
		return;
	}
	iterNew->Run();*/
	CoAddBuilding(msg.����, msg.pos).RunNew();
}

CoTask<int> PlayerGateSession_Game::CoAddBuilding(const ������λ���� ����, const Position pos)
{
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		co_return 0;
	}
	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!�ɷ��ý���(pos, ����.f��߳�))
	{
		Sayϵͳ("�˴����ɷ���");//��Err00�� I can't build it, something's in the way. ��û�����⽨���ж�������
		co_return 0;
	}
	if (����.����.u16����ȼ���� > m_u32ȼ����)
	{
		std::ostringstream oss;
		oss << "ȼ������" << ����.����.u16����ȼ����;//(low error beep) Insufficient Vespene Gas.������ 
		Sayϵͳ(oss.str());
		co_return 0;
	}
	m_u32ȼ���� -= ����.����.u16����ȼ����;
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//���ܴ�������ݿ��ܵ������ú�ָ��ʧЧ
	auto [stop, responce] = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = ����.����.u16���ľ���� },
		[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	if (stop)
	{
		m_u32ȼ���� += ����.����.u16����ȼ����;//����ȼ����
		co_return 0;
	}
	if (0 != responce.error)
	{
		//LOG(WARNING) << "��Ǯʧ��,error=" << responce.error;
		m_u32ȼ���� += ����.����.u16����ȼ����;//����ȼ����
		Sayϵͳ("��������" + ����.����.u16���ľ����);
		co_return 0;
	}

	//�ӽ���
	CHECK_CO_RET_0(!m_wpSpace.expired());
	auto spSpace = m_wpSpace.lock();
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >(
		pos, *spSpace, ����.����.strPrefabName, ����.����.strName);
	//spNewEntity->AddComponentAttack();
	PlayerComponent::AddComponent(*spNewEntity, *this);
	BuildingComponent::AddComponent(*spNewEntity, *this, ����, ����.f��߳�);
	switch (����)
	{
	case ����:
	case ����:
		����λComponent::AddComponet(*spNewEntity, *this, ����);
		break;
	case �ر�:
		�ر�Component::AddComponet(*spNewEntity, *this);
		break;
	case ��:break;
	}
	DefenceComponent::AddComponent(*spNewEntity, ����.����.u16��ʼHp);
	//spNewEntity->m_spBuilding->m_fun����λ = ����.fun���;
	m_setSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	spSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void PlayerGateSession_Game::EnterSpace(WpSpace wpSpace, const std::string& strNickName)
{
	assert(m_wpSpace.expired());
	assert(!wpSpace.expired());
	m_wpSpace = wpSpace;
	auto sp = m_wpSpace.lock();
	m_strNickName = strNickName;

	Send<Msg��Space>({ .idSapce = 1 });

	for (const auto& [id, spEntity] : sp->m_mapEntity)//���е�ͼ�ϵ�ʵ�巢���Լ�
	{
		LOG(INFO) << spEntity->NickName() << ",��������," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >({ 0.0 }, *sp, "smoke", "�ӿ�");
	sp->m_mapEntity.insert({ spEntityViewPort->Id, spEntityViewPort });
	m_setSpEntity.insert(spEntityViewPort);
	//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
	PlayerComponent::AddComponent(*spEntityViewPort, *this);
	spEntityViewPort->BroadcastEnter();

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "�յ�����:" << utf8Content;
	SendToWorldSvr<MsgSay>(msg, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	CHECK_VOID(!m_wpSpace.expired());
	LOG(INFO) << "�յ�ѡ��:" << msg.ids.size();
	m_listSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(m_listSelectedEntity), [](const double& id) {return uint64_t(id); });
	for (const auto id : m_listSelectedEntity)
	{
		auto wpEntity = m_wpSpace.lock()->GetEntity(id);
		if (wpEntity.expired())
			continue;

		auto spEntity = wpEntity.lock();
		if (spEntity->m_spAttack)
		{
			switch (spEntity->m_spAttack->m_����)
			{
			case ��:Say������ʾ("������!"); break;//Standing by. ������
			case ��ս��:Say������ʾ("׼���ж�!"); break;//Checked up and good to go. �����ϣ�׼������
			case ���̳�:Say������ʾ("�ϴ�!"); break;//Commander.
			default:break;
			}
		}
	}
}

template void PlayerGateSession_Game::Send(const MsgAddRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyPos&);
template void PlayerGateSession_Game::Send(const MsgChangeSkeleAnim&);
template void PlayerGateSession_Game::Send(const MsgSay&);
template void PlayerGateSession_Game::Send(const MsgDelRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyMoney&);


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

PlayerGateSession_Game::PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession) :
	m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession)
{

}

void PlayerGateSession_Game::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::��Space:RecvMsg<Msg��Space>(obj); break;
	case MsgId::�����˾��鸱��:RecvMsg<Msg�����˾��鸱��>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::�ɼ�:RecvMsg<Msg�ɼ�>(obj); break;
	case MsgId::���ر�:RecvMsg<Msg���ر�>(obj); break;
	case MsgId::���ر�:RecvMsg<Msg���ر�>(obj); break;
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
	for (const auto& refEntity : m_setSpEntity)
	{
		if (!refEntity->m_spBuilding)
			continue;

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
	for (const auto& refEntity : m_setSpEntity)
	{
		if (refEntity->m_sp����λ)
		{
			��������еĵ�λ += (uint16_t)refEntity->m_sp����λ->�ȴ���Count();//m_i�ȴ������;

		}
		else
		{
			++��������еĵ�λ;
		}

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
		const auto& refPosOld = kv.second->m_Pos;
		bool CrowdTool�жϵ�λ�ص�(const Position & refPosOld, const Position & refPosNew, const float f��߳�);
		if (CrowdTool�жϵ�λ�ص�(refPos, refPosOld, f��߳�))
			return false;
	}

	return true;
}