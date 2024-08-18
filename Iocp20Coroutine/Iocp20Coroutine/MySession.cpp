#include "StdAfx.h"
#include <glog/logging.h>

//#include "IocpNetwork/ListenSocketCompletionKey.cpp"
#include "../IocpNetwork/SessionSocketCompletionKeyTemplate.h"
#include "../websocketfiles-master/src/ws_endpoint.cpp"
#include "../IocpNetwork/WebSocketSessionTemplate.h"
#include "../IocpNetwork/SessionsTemplate.h"
#include "MySession.h"

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

//template<MySession>
//std::set<Iocp::SessionSocketCompletionKey<MySession>*> g_setSession;
//template<MySession> std::mutex g_setSessionMutex;
template Iocp::SessionSocketCompletionKey<WebSocketSession<MySession> >;
template class WebSocketSession<MySession>;
template void WebSocketSession<MySession>::OnInit<MyServer>(Iocp::SessionSocketCompletionKey<WebSocketSession<MySession> >& refSession, MyServer& server);
template class WebSocketEndpoint<MySession, Iocp::SessionSocketCompletionKey<WebSocketSession<MySession> > >;

template<class T>
void MySession::Send(const T& ref)
{
	WebSocketPacket wspacket;
	// set FIN and opcode
	wspacket.set_fin(1);
	wspacket.set_opcode(0x02);// packet.get_opcode());

	std::stringstream buffer;
	msgpack::pack(buffer, ref);
	buffer.seekg(0);

	// deserialize the buffer into msgpack::object instance.
	std::string str(buffer.str());
	wspacket.set_payload(str.data(), str.size());
	ByteBuffer output;
	// pack a websocket data frame
	wspacket.pack_dataframe(output);
	//send to client
	//this->to_wire(output.bytes(), output.length());
	this->m_pWsSession->Send(output.bytes(), output.length());
}

/// <summary>
/// 网络线程，多线程
/// </summary>
/// <param name="buf"></param>
/// <param name="len"></param>
void MySession::OnRecvWsPack(const void* buf, const int len)
{
	msgpack::object_handle oh = msgpack::unpack((const char*)buf, len);//没判断越界，要加try
	msgpack::object obj = oh.get();
	const auto msgId = (MsgId)obj.via.array.ptr[0].via.i64;//没判断越界，要加try
	LOG(INFO) << obj;
	//auto pSessionSocketCompeletionKey = static_cast<Iocp::SessionSocketCompletionKey<WebSocketSession<MySession>>*>(this->nt_work_data_);
	auto pSessionSocketCompeletionKey = this->m_pWsSession->m_pSession;
	switch (msgId)
	{
	case MsgId::Login:m_MsgQueue.PushMsg<MsgLogin>(*this,obj);break;
	case MsgId::Move:m_MsgQueue.PushMsg<MsgMove>(*this, obj);break;
	case MsgId::Say:m_MsgQueue.PushMsg<MsgSay >(*this, obj); break;
	case MsgId::SelectRoles:m_MsgQueue.PushMsg<MsgSelectRoles>(*this, obj);break;
	case MsgId::AddRole:m_MsgQueue.PushMsg<MsgAddRole>(*this, obj); break;
	default:
		LOG(ERROR) << "没处理msgId:" << msgId;
		assert(false);
		break;
	}
}

void MySession::OnInit(WebSocketSession<MySession>& refWsSession, MyServer& server)
{
	server.m_Sessions.AddSession(refWsSession.m_pSession, [this, &refWsSession, &server]()
		{
			m_pServer = &server;
			m_pWsSession = &refWsSession;
		});
}

void MySession::OnDestroy()
{
	for (auto sp : m_vecSpEntity)
	{
		m_pServer->m_space.setEntity.erase(sp);

		sp->OnDestroy();
	}
	m_vecSpEntity.clear();

	/*m_pServer->m_Sessions.DeleteSession(this->m_pWsSession->m_pSession, [this]()
		{
		});*/

	m_pServer = nullptr;//不用加锁
}

void MySession::Erase(SpEntity spEntity)
{
	if (!m_vecSpEntity.contains(spEntity))
	{
		LOG(WARNING) << "ERR";
		return;
	}
	
	m_vecSpEntity.erase(spEntity);
}

template void MySession::Send(const MsgAddRoleRet&);
template void MySession::Send(const MsgNotifyPos&);
template void MySession::Send(const MsgChangeSkeleAnim&);
template void MySession::Send(const MsgSay&);
template void MySession::Send(const MsgDelRoleRet&);



//主线程，单线程
void MySession::Process()
{
	const MsgId msgId = this->m_MsgQueue.PopMsg();
	switch (msgId)
	{
	case MsgId::Invalid_0://没有消息可处理
		return;
	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &MySession::OnRecv); break;
	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, &MySession::OnRecv); break;
	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &MySession::OnRecv); break;
	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, &MySession::OnRecv); break;
	case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, &MySession::OnRecv); break;
	default:
		LOG(ERROR) << "msgId:" << msgId;
		assert(false);
		break;
	}
}
template<> std::deque<MsgLogin>& MySession::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgMove>& MySession::GetQueue() { return m_queueMove; }
template<> std::deque<MsgSay>& MySession::GetQueue() { return m_queueSay; }
template<> std::deque<MsgSelectRoles>& MySession::GetQueue() { return m_queueSelectRoles; }
template<> std::deque<MsgAddRole>& MySession::GetQueue() { return m_queueAddRole; }

void MySession::OnRecv(const MsgAddRole& msg)
{
	//前一个没结束就会内存泄漏
	m_coRpc = CoAddRole();//启动协程，先去WorldSvr扣钱后加一个新单位
	m_coRpc.Run();
}



CoTask<int> MySession::CoAddRole()
{
	void SendToWorldSvr(const MsgConsumeMoney & msg);
	MsgConsumeMoneyResponce responce = co_await CoRpc<MsgConsumeMoneyResponce>::Send<MsgConsumeMoney>({ .consumeMoney = 3 }, SendToWorldSvr);//以同步编程的方式，向另一个服务器发送请求并等待返回
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ 30,30 }, m_pServer->m_space, "altman-blue");
	spNewEntity->AddComponentPlayer(this);
	m_vecSpEntity.insert(spNewEntity);//自己控制的单位
	m_pServer->m_space.setEntity.insert(spNewEntity);//全地图单位

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void MySession::OnRecv(const MsgLogin& msg)
{
	auto utf8Name = msg.name;
	auto gbkName = StrConv::Utf8ToGbk(msg.name);
	//printf("准备广播%s",utf8Name.c_str());
	/*for (auto p : g_set)
	{
		const auto strBroadcast = "[" + utf8Name + "]进来了";
		MsgLoginRet ret = { 223,GbkToUtf8(strBroadcast.c_str()) };
		p->Send(ret);
	}*/
	//const auto strBroadcast = "[" + utf8Name + "]进来了";
	if (msg.name.empty())
	{
		Send(MsgSay(StrConv::GbkToUtf8("请输入名字")));
		return;
	}
	if (!m_vecSpEntity.empty())
	{
		Send(MsgSay(StrConv::GbkToUtf8("不能重复登录")));
		return;
	}

	m_nickName = gbkName;

	//for (const auto pENtity : refThis.m_pSession->m_pServer->m_space.setEntity)
	//{
	//	if (pENtity == &refThis.m_pSession->m_entity)
	//		continue;

	//	if (pENtity->m_nickName == utf8Name)
	//	{
	//		LOG(WARNING) << "重复登录" << utf8Name;
	//		//主动断线还没做
	//		return;
	//	}
	//}


	for (const auto& spEntity : m_pServer->m_space.setEntity)//别人发给自己
	{
		Send(MsgAddRoleRet((uint64_t)spEntity.get(), StrConv::GbkToUtf8(spEntity->NickName()), spEntity->m_strPrefabName));
		Send(MsgNotifyPos(*spEntity));
	}
}

void MySession::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto pServer = m_pServer;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : m_vecSelectedEntity)
	{
		Entity* pEntity = (Entity*)id;
		const auto& refVecSpEntity = m_vecSpEntity;
		if (refVecSpEntity.end() == std::find_if(refVecSpEntity.begin(), refVecSpEntity.end(), [pEntity](const auto& sp) {return sp.get() == pEntity; }))
		{
			LOG(ERROR) << id << "不是自己的单位，不能移动";
			continue;
		}

		pEntity->WalkToPos(Position(targetX, targetZ), pServer);

	}
}

void MySession::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "收到聊天:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}

void MySession::OnRecv(const MsgSelectRoles& msg)
{
	LOG(INFO) << "收到选择:" << msg.ids.size();
	m_vecSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(m_vecSelectedEntity), [](const double& id) {return uint64_t(id); });
}