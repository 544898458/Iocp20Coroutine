#include "StdAfx.h"
#include <glog/logging.h>

//#include "IocpNetwork/ListenSocketCompletionKey.cpp"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
//#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "GameSvrSession.h"

//#include <iostream>
#include <cassert>

#include <codecvt>
//#include "MsgQueue.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "MyServer.h"
#include "AiCo.h"
#include "Entity.h"
#include "../CoRoutine/CoRpc.h"
#include "../IocpNetwork/StrConv.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "AttackComponent.h"
#include "../IocpNetwork/MsgPack.h"
//template<MySession>
//std::set<Iocp::SessionSocketCompletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;
template Iocp::SessionSocketCompletionKey<GameSvrSession>;
//template class WebSocketSession<GameSvrSession>;
//template void WebSocketSession<GameSvrSession>::OnInit<GameSvr>(Iocp::SessionSocketCompletionKey<WebSocketSession<GameSvrSession> >& refSession, GameSvr& server);
//template class WebSocketEndpoint<GameSvrSession, Iocp::SessionSocketCompletionKey<WebSocketSession<GameSvrSession> > >;

template<class T>
void GameSvrSession::Send(const T& ref)
{
	MsgPack::SendMsgpack(ref, [this](const void* buf, int len) 
		{
			MsgGateת�� msg(buf, len, 0);
			MsgPack::SendMsgpack(msg, [this](const void* buf, int len)
				{
					this->m_pWsSession->Send(buf, len);
				});
			
		});
	//if (!m_bLoginOk)
	//{
	//	LOG(WARNING) << "��û��¼����������������,T:" << typeid(T).name();
	//	return;
	//}
	//WebSocketPacket wspacket;
	//// set FIN and opcode
	//wspacket.set_fin(1);
	//wspacket.set_opcode(0x02);// packet.get_opcode());

	//std::stringstream buffer;
	//msgpack::pack(buffer, ref);
	//buffer.seekg(0);

	//// deserialize the buffer into msgpack::object instance.
	//std::string str(buffer.str());
	//wspacket.set_payload(str.data(), str.size());
	//ByteBuffer output;
	//// pack a websocket data frame
	//wspacket.pack_dataframe(output);
	////send to client
	////this->to_wire(output.bytes(), output.length());
	//this->m_pWsSession->Send(output.bytes(), output.length());
}

/// <summary>
/// �����̣߳����߳�
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
int GameSvrSession::OnRecv(WebSocketGameSession&, const void* buf, const int len)
{
	const auto&& [bufPack, lenPack] = Iocp::OnRecv2(buf, len);
	if (lenPack > 0 && nullptr != bufPack)
	{
		OnRecvPack(bufPack, lenPack);
	}

	return lenPack;
}

void GameSvrSession::OnRecvPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	LOG(INFO) << obj;
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msgId)
	{
	case MsgId::Login:m_MsgQueue.PushMsg<MsgLogin>(*this, obj); break;
	case MsgId::Move:m_MsgQueue.PushMsg<MsgMove>(*this, obj); break;
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay >(*this, obj); break;
	case MsgId::SelectRoles:m_MsgQueue.PushMsg<MsgSelectRoles>(*this, obj); break;
	case MsgId::AddRole:m_MsgQueue.PushMsg<MsgAddRole>(*this, obj); break;
	case MsgId::AddBuilding:m_MsgQueue.PushMsg<MsgAddBuilding>(*this, obj); break;
	case MsgId::Gateת��:m_MsgQueue.PushMsg<MsgGateת��>(*this, obj); break;
	default:
		LOG(ERROR) << "û����msgId:" << msgId;
		assert(false);
		break;
	}
}

void GameSvrSession::OnInit(WebSocketGameSession& refWsSession, GameSvr& server)
{
	server.m_Sessions.AddSession(&refWsSession, [this, &refWsSession, &server]()
		{
			m_pServer = &server;
			m_pWsSession = &refWsSession;
		});
}

void GameSvrSession::OnDestroy()
{
	for (auto sp : m_vecSpEntity)
	{
		m_pServer->m_space.m_mapEntity.erase((int64_t)sp.get());

		sp->OnDestroy();
	}
	LOG(INFO) << "m_mapEntity.size=" << m_pServer->m_space.m_mapEntity.size();
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

	m_pServer = nullptr;//���ü���
}

void GameSvrSession::Erase(SpEntity spEntity)
{
	if (!m_vecSpEntity.contains(spEntity))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_vecSpEntity.erase(spEntity);
}

//���̣߳����߳�
void GameSvrSession::Process()
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
	{
		const auto oldSize = m_vecCoRpc.size();
		std::erase_if(m_vecCoRpc, [](CoTask<int>& refCo)->bool {return refCo.Finished(); });
		const auto newSize = m_vecCoRpc.size();
		if (oldSize != newSize)
		{
			LOG(INFO) << "oldSize:" << oldSize << ",newSize:" << newSize;
		}
	}
	const MsgId msgId = this->m_MsgQueue.PopMsg();
	switch (msgId)
	{
	case MsgId::Invalid_0://û����Ϣ�ɴ���
		return;
	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &GameSvrSession::OnRecv); break;
	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, &GameSvrSession::OnRecv); break;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &GameSvrSession::OnRecv); break;
	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, &GameSvrSession::OnRecv); break;
	case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, &GameSvrSession::OnRecv); break;
	case MsgId::AddBuilding:this->m_MsgQueue.OnRecv(this->m_queueAddBuilding, *this, &GameSvrSession::OnRecv); break;
	case MsgId::Gateת��:this->m_MsgQueue.OnRecv(this->m_queueGateת��, *this, &GameSvrSession::OnRecv); break;
	default:
		LOG(ERROR) << "msgId:" << msgId;
		assert(false);
		break;
	}
}
template<> std::deque<MsgLogin>& GameSvrSession::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgMove>& GameSvrSession::GetQueue() { return m_queueMove; }
template<> std::deque<MsgSay>& GameSvrSession::GetQueue() { return m_queueSay; }
template<> std::deque<MsgSelectRoles>& GameSvrSession::GetQueue() { return m_queueSelectRoles; }
template<> std::deque<MsgAddRole>& GameSvrSession::GetQueue() { return m_queueAddRole; }
template<> std::deque<MsgAddBuilding>& GameSvrSession::GetQueue() { return m_queueAddBuilding; }
template<> std::deque<MsgGateת��>& GameSvrSession::GetQueue() { return m_queueGateת��; }

void GameSvrSession::OnRecv(const MsgAddRole& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpcǰһ��Э�̻�û����";
	//	return;
	//}
	auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddRole());
	if (iterNew == m_vecCoRpc.end())
	{
		LOG(ERROR) << "err";
		return;
	}
	iterNew->Run();
}

void SendToWorldSvr(const MsgChangeMoney& msg);
void GameSvrSession::OnRecv(const MsgAddBuilding& msg)
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

CoTask<int> GameSvrSession::CoAddBuilding()
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//���ܴ�������ݿ��ܵ������ú�ָ��ʧЧ
	auto tuple = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 0 }, SendToWorldSvr, **iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	const MsgChangeMoneyResponce& responce = std::get<1>(tuple);
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ 0,float(std::rand() % 50) }, m_pServer->m_space, "house_type19");
	if (0 != responce.error)
	{
		LOG(WARNING) << "��Ǯʧ��,error=" << responce.error;
		co_return 0;
	}
	//spNewEntity->AddComponentAttack();
	spNewEntity->AddComponentPlayer(this);
	spNewEntity->AddComponentBuilding();
	m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	m_pServer->m_space.m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}



CoTask<int> GameSvrSession::CoAddRole()
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());
	const auto [stop, responce]= co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 3 }, SendToWorldSvr, **iterNew);//��ͬ����̵ķ�ʽ������һ���������������󲢵ȴ�����
	LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ float(std::rand()%30),30 }, m_pServer->m_space, "altman-blue");
	if (stop)
	{
		LOG(WARNING) << "��Ǯʧ��";
		co_return 0;
	}
	spNewEntity->AddComponentPlayer(this);
	spNewEntity->AddComponentAttack();
	m_vecSpEntity.insert(spNewEntity);//�Լ����Ƶĵ�λ
	m_pServer->m_space.m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void GameSvrSession::OnRecv(const MsgLogin& msg)
{
	auto utf8Name = msg.name;
	auto gbkName = StrConv::Utf8ToGbk(msg.name);
	//printf("׼���㲥%s",utf8Name.c_str());
	/*for (auto p : g_set)
	{
		const auto strBroadcast = "[" + utf8Name + "]������";
		MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
		p->Send(ret);
	}*/
	//const auto strBroadcast = "[" + utf8Name + "]������";
	if (msg.name.empty())
	{
		Send(MsgSay(StrConv::GbkToUtf8("����������")));
		return;
	}
	if (!m_vecSpEntity.empty())
	{
		Send(MsgSay(StrConv::GbkToUtf8("�����ظ���¼")));
		return;
	}

	m_nickName = gbkName;
	m_bLoginOk = true;
	//for (const auto pENtity : refThis.m_pSession->m_pServer->m_space.setEntity)
	//{
	//	if (pENtity == &refThis.m_pSession->m_entity)
	//		continue;

	//	if (pENtity->m_nickName == utf8Name)
	//	{
	//		LOG(WARNING) << "�ظ���¼" << utf8Name;
	//		//�������߻�û��
	//		return;
	//	}
	//}


	for (const auto& [id,spEntity]: m_pServer->m_space.m_mapEntity)//���˷����Լ�
	{
		Send(MsgAddRoleRet((uint64_t)spEntity.get(), StrConv::GbkToUtf8(spEntity->NickName()), spEntity->m_strPrefabName));
		Send(MsgNotifyPos(*spEntity));
	}
}

void GameSvrSession::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "�յ��������:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = m_pServer;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : m_vecSelectedEntity)
	{
		auto itFind = m_pServer->m_space.m_mapEntity.find(id);
		if (itFind == m_pServer->m_space.m_mapEntity.end())
		{
			LOG(ERROR) << "ERR";
			assert(false);
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

void GameSvrSession::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "�յ�����:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}

void GameSvrSession::OnRecv(const MsgSelectRoles& msg)
{
	LOG(INFO) << "�յ�ѡ��:" << msg.ids.size();
	m_vecSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(m_vecSelectedEntity), [](const double& id) {return uint64_t(id); });
}
template<class T_Msg>
void GameSvrSession::RecvMsg(msgpack::object &obj)
{
	const auto msg = obj.as<T_Msg>();
	OnRecv(msg);
}
void GameSvrSession::OnRecv(const MsgGateת�� & msg)
{
	if (msg.vecByte.empty())
	{
		LOG(ERROR) << "ERR";
		assert(false);
		return;
	}
	
	msgpack::object_handle oh = msgpack::unpack((const char*)&msg.vecByte[0], msg.vecByte.size());//û�ж�Խ�磬Ҫ��try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	LOG(INFO) << obj;
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	//auto pSessionSocketCompeletionKey = this->m_pWsSession;
	switch (msgId)
	{
	case MsgId::Login:RecvMsg<MsgLogin>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::Gateת��:
		LOG(ERROR) << "������ת��" ;
		assert(false);
		break;
	default:
		LOG(ERROR) << "û����msgId:" << msgId;
		assert(false);
		break;
	}
}

template void GameSvrSession::Send(const MsgAddRoleRet&);
template void GameSvrSession::Send(const MsgNotifyPos&);
template void GameSvrSession::Send(const MsgChangeSkeleAnim&);
template void GameSvrSession::Send(const MsgSay&);
template void GameSvrSession::Send(const MsgDelRoleRet&);
template void GameSvrSession::Send(const MsgNotifyMoney&);