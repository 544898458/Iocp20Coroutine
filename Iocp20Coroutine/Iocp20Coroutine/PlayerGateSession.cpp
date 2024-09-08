#include "pch.h"
#include "PlayerGateSession.h"
#include "Space.h"
#include "Entity.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "AiCo.h"
#include "AttackComponent.h"
#include "MyServer.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"


template<class T>
void PlayerGateSession::Send(const T& ref)
{
	++m_snSend;
	ref.msg.SetSn(m_snSend);

	std::stringstream buffer;
	msgpack::pack(buffer, ref);
	buffer.seekg(0);

	std::string str(buffer.str());
	CHECK_GE_VOID(UINT16_MAX, str.size());
	MsgGate转发 msg(str.data(), (int)str.size(), m_idPlayerGateSession);
	MsgPack::SendMsgpack(msg, [this](const void* buf, int len)
		{
			this->m_refSession.SendToGate(buf, len);
		});
}


void PlayerGateSession::OnDestroy(Space& refSpace)
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

void PlayerGateSession::Erase(SpEntity& spEntity)
{
	if (!m_vecSpEntity.contains(spEntity))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_vecSpEntity.erase(spEntity);
}

template<> std::deque<MsgLogin>& PlayerGateSession::GetQueue() { return m_queueLogin; }
template<> std::deque<MsgMove>& PlayerGateSession::GetQueue() { return m_queueMove; }
template<> std::deque<MsgSay>& PlayerGateSession::GetQueue() { return m_queueSay; }
template<> std::deque<MsgSelectRoles>& PlayerGateSession::GetQueue() { return m_queueSelectRoles; }
template<> std::deque<MsgAddRole>& PlayerGateSession::GetQueue() { return m_queueAddRole; }
template<> std::deque<MsgAddBuilding>& PlayerGateSession::GetQueue() { return m_queueAddBuilding; }


void PlayerGateSession::OnRecv(const MsgAddRole& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpc前一个协程还没结束";
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
void PlayerGateSession::OnRecv(const MsgAddBuilding& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpc前一个协程还没结束";
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

CoTask<int> PlayerGateSession::CoAddBuilding()
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//不能存对象，扩容可能导致引用和指针失效
	auto tuple = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 0 }, SendToWorldSvr, **iterNew);//以同步编程的方式，向另一个服务器发送请求并等待返回
	const MsgChangeMoneyResponce& responce = std::get<1>(tuple);
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ 0,float(std::rand() % 50) }, m_refSession.m_pServer->m_space, "house_type19");
	if (0 != responce.error)
	{
		LOG(WARNING) << "扣钱失败,error=" << responce.error;
		co_return 0;
	}
	//spNewEntity->AddComponentAttack();
	spNewEntity->AddComponentPlayer(*this);
	spNewEntity->AddComponentBuilding(*this);
	m_vecSpEntity.insert(spNewEntity);//自己控制的单位
	m_refSession.m_pServer->m_space.m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

	spNewEntity->BroadcastEnter();
	co_return 0;
}

CoTask<int> PlayerGateSession::CoAddRole()
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());
	const auto [stop, responce] = co_await AiCo::ChangeMoney(*this, -3, **iterNew);//以同步编程的方式，向另一个服务器发送请求并等待返回
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string& >({ float(std::rand() % 30),30 }, m_refSession.m_pServer->m_space, "altman-blue");
	if (stop)
	{
		LOG(WARNING) << "扣钱失败";
		co_return 0;
	}
	spNewEntity->AddComponentPlayer(*this);
	spNewEntity->AddComponentAttack();
	m_vecSpEntity.insert(spNewEntity);//自己控制的单位
	m_refSession.m_pServer->m_space.m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void PlayerGateSession::OnRecv(const MsgLogin& msg)
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
	m_bLoginOk = true;
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


	for (const auto& [id, spEntity] : m_refSession.m_pServer->m_space.m_mapEntity)//别人发给自己
	{
		Send(MsgAddRoleRet((uint64_t)spEntity.get(), StrConv::GbkToUtf8(spEntity->NickName()), spEntity->m_strPrefabName));
		Send(MsgNotifyPos(*spEntity));
	}
}

void PlayerGateSession::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	auto& refSpace = m_refSession.m_pServer->m_space;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : m_vecSelectedEntity)
	{
		auto itFind = refSpace.m_mapEntity.find(id);
		if (itFind == refSpace.m_mapEntity.end())
		{
			LOG(ERROR) << "ERR";
			assert(false);
			continue;
		}
		auto& spEntity = itFind->second;
		if (m_vecSpEntity.end() == std::find_if(m_vecSpEntity.begin(), m_vecSpEntity.end(), [&spEntity](const auto& sp) {return sp == spEntity; }))
		{
			LOG(ERROR) << id << "不是自己的单位，不能移动";
			continue;
		}

		if (spEntity->m_spAttack)
			spEntity->m_spAttack->WalkToPos(*spEntity.get(), Position(targetX, targetZ));

	}
}

void PlayerGateSession::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "收到聊天:" << utf8Content;
	void SendToWorldSvr(const MsgSay & msg);
	SendToWorldSvr(msg);
}

void PlayerGateSession::OnRecv(const MsgSelectRoles& msg)
{
	LOG(INFO) << "收到选择:" << msg.ids.size();
	m_vecSelectedEntity.clear();
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(m_vecSelectedEntity), [](const double& id) {return uint64_t(id); });
}

template void PlayerGateSession::Send(const MsgAddRoleRet&);
template void PlayerGateSession::Send(const MsgNotifyPos&);
template void PlayerGateSession::Send(const MsgChangeSkeleAnim&);
template void PlayerGateSession::Send(const MsgSay&);
template void PlayerGateSession::Send(const MsgDelRoleRet&);
template void PlayerGateSession::Send(const MsgNotifyMoney&);


template<class T_Msg>
void PlayerGateSession::RecvMsg(const msgpack::object& obj)
{
	const auto msg = obj.as<T_Msg>();
	OnRecv(msg);
}

void PlayerGateSession::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::Login:RecvMsg<MsgLogin>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::Gate转发:
		LOG(ERROR) << "不能再转发";
		assert(false);
		break;
	default:
		LOG(ERROR) << "没处理msgId:" << idMsg;
		assert(false);
		break;
	}
}

void PlayerGateSession::Process()
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

	while (true)
	{
		const MsgId msgId = this->m_MsgQueue.PopMsg();
		if (MsgId::Invalid_0 == msgId)//没有消息可处理
			break;

		switch (msgId)
		{
		case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &PlayerGateSession::OnRecv); break;
		case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, &PlayerGateSession::OnRecv); break;
		case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &PlayerGateSession::OnRecv); break;
		case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, &PlayerGateSession::OnRecv); break;
		case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, &PlayerGateSession::OnRecv); break;
		case MsgId::AddBuilding:this->m_MsgQueue.OnRecv(this->m_queueAddBuilding, *this, &PlayerGateSession::OnRecv); break;
			//case MsgId::Gate转发:this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &OnRecv); break;
		default:
			LOG(ERROR) << "msgId:" << msgId;
			assert(false);
			break;
		}
	}
}
