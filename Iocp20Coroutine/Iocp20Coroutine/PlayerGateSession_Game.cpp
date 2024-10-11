#include "pch.h"
#include "PlayerGateSession_Game.h"
#include "Space.h"
#include "Entity.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"

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
		},false);
}


void PlayerGateSession_Game::OnDestroy(Space& refSpace)
{
	for (auto sp : m_vecSpEntity)
	{
		refSpace.m_mapEntity.erase((int64_t)sp.get());

		sp->OnDestroy();
	}
	LOG(INFO) << "m_mapEntity.size=" << refSpace.m_mapEntity.size();
	m_vecSpEntity.clear();

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


}

void PlayerGateSession_Game::Erase(SpEntity& spEntity)
{
	if (!m_vecSpEntity.contains(spEntity))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_vecSpEntity.erase(spEntity);
}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpcǰһ��Э�̻�û����";
	//	return;
	//}
	//auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddRole());
	//if (iterNew == m_vecCoRpc.end())
	//{
	//	LOG(ERROR) << "err";
	//	return;
	//}
	//iterNew->Run();
	CoAddRole().RunNew();
}

template<class T> void SendToWorldSvr(const T& msg, const uint64_t idGateSession);

void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpcǰһ��Э�̻�û����";
	//	return;
	//}
	auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddBuilding());
	if (iterNew == m_vecCoRpc.end())
	{
		LOG(ERROR) << "err";
		return;
	}
	iterNew->Run();
}

CoTask<int> PlayerGateSession_Game::CoAddBuilding()
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//���ܴ�������ݿ��ܵ������ú�ָ��ʧЧ
	auto tuple = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 1 }, 
		[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref,m_idPlayerGateSession); }, ** iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	const MsgChangeMoneyResponce& responce = std::get<1>(tuple);
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ 0,float(std::rand() % 50) }, m_refSession.m_pServer->m_space, "house_type19");
	if (0 != responce.error)
	{
		LOG(WARNING) << "��Ǯʧ��,error=" << responce.error;
		co_return 0;
	}
	//spNewEntity->AddComponentAttack();
	spNewEntity->AddComponentPlayer(*this);
	spNewEntity->AddComponentBuilding(*this);
	m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	m_refSession.m_pServer->m_space.m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}

CoTask<int> PlayerGateSession_Game::CoAddRole()
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());
	const auto [stop, responce] = co_await AiCo::ChangeMoney(*this, 3, false, **iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ float(std::rand() % 30),30 }, m_refSession.m_pServer->m_space, "altman-blue");
	if (stop)
	{
		LOG(WARNING) << "��Ǯʧ��";
		co_return 0;
	}
	spNewEntity->AddComponentPlayer(*this);
	spNewEntity->AddComponentAttack();
	m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	m_refSession.m_pServer->m_space.m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void PlayerGateSession_Game::Init()
{
	for (const auto& [id, spEntity] : m_refSession.m_pServer->m_space.m_mapEntity)//���е�ͼ�ϵ�ʵ�巢���Լ�
	{
		Send(MsgAddRoleRet((uint64_t)spEntity.get(), StrConv::GbkToUtf8(spEntity->NickName()), spEntity->m_strPrefabName));
		Send(MsgNotifyPos(*spEntity));
	}
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto& refSpace = m_refSession.m_pServer->m_space;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : m_vecSelectedEntity)
	{
		auto itFind = refSpace.m_mapEntity.find(id);
		if (itFind == refSpace.m_mapEntity.end())
		{
			LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
			//assert(false);
			continue;
		}
		auto& spEntity = itFind->second;
		if (m_vecSpEntity.end() == std::find_if(m_vecSpEntity.begin(), m_vecSpEntity.end(), [&spEntity](const auto& sp) {return sp == spEntity; }))
		{
			LOG(ERROR) << id << "�����Լ��ĵ�λ�������ƶ�";
			continue;
		}

		if (spEntity->m_spAttack)
			spEntity->m_spAttack->WalkToPos(*spEntity.get(), Position(targetX, targetZ));

	}
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "�յ�����:" << utf8Content;
	SendToWorldSvr<MsgSay>(msg,m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	LOG(INFO) << "�յ�ѡ��:" << msg.ids.size();
	m_vecSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(m_vecSelectedEntity), [](const double& id) {return uint64_t(id); });
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
	const auto msg = obj.as<T_Msg>();
	OnRecv(msg);
}

void PlayerGateSession_Game::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
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
	//{
	//	const auto oldSize = m_vecCoRpc.size();
	//	std::erase_if(m_vecCoRpc, [](CoTask<int>& refCo)->bool {return refCo.Finished(); });
	//	const auto newSize = m_vecCoRpc.size();
	//	if (oldSize != newSize)
	//	{
	//		LOG(INFO) << "oldSize:" << oldSize << ",newSize:" << newSize;
	//	}
	//}

	//while (true)
	//{
	//	const MsgId msgId = this->m_MsgQueue.PopMsg();
	//	if (MsgId::Invalid_0 == msgId)//û����Ϣ�ɴ���
	//		break;

	//	switch (msgId)
	//	{
	//	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::AddBuilding:this->m_MsgQueue.OnRecv(this->m_queueAddBuilding, *this, &PlayerGateSession::OnRecv); break;
	//		//case MsgId::Gateת��:this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &OnRecv); break;
	//	default:
	//		LOG(ERROR) << "msgId:" << msgId;
	//		assert(false);
	//		break;
	//	}
	//}
}
