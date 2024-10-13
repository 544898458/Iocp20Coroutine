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
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "BuildingComponent.h"

/// <summary>
/// GameSvr通过GateSvr透传给游戏客户端
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
			MsgGate转发 msg(buf, len, m_idPlayerGateSession, m_snSend);
			MsgPack::SendMsgpack(msg, [this](const void* buf转发, int len转发)
				{
					this->m_refSession.SendToGate(buf转发, len转发);
				});
		}, false);
}


void PlayerGateSession_Game::OnDestroy()
{
	for (auto sp : m_vecSpEntity)
	{
		sp->m_refSpace.m_mapEntity.erase((int64_t)sp.get());
		LOG(INFO) << "m_mapEntity.size=" << sp->m_refSpace.m_mapEntity.size();
		sp->OnDestroy();
	}

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

void PlayerGateSession_Game::Say(const std::string& str)
{
	Send(MsgSay(StrConv::GbkToUtf8(str)));
}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	CHECK_NOTNULL_VOID(m_pCurSpace);
	for (const auto id : m_vecSelectedEntity)
	{
		auto itFind = m_pCurSpace->m_mapEntity.find(id);
		if (itFind == m_pCurSpace->m_mapEntity.end())
		{
			LOG(INFO) << "选中的实体不存在:" << id;
			//assert(false);
			continue;
		}
		auto& spEntity = itFind->second;
		if (m_vecSpEntity.end() == std::find_if(m_vecSpEntity.begin(), m_vecSpEntity.end(), [&spEntity](const auto& sp) {return sp == spEntity; }))
		{
			LOG(ERROR) << id << "不是自己的单位，不能操作";
			continue;
		}

		if (spEntity->m_spBuilding)
			spEntity->m_spBuilding->造兵(*this, *spEntity);

	}

	//CoAddRole().RunNew();
}

template<class T> void SendToWorldSvr(const T& msg, const uint64_t idGateSession);

void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//if (!m_coRpc.Finished())
	//{
	//	LOG(WARNING) << "m_coRpc前一个协程还没结束";
	//	return;
	//}
	auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddBuilding(msg.类型));
	if (iterNew == m_vecCoRpc.end())
	{
		LOG(ERROR) << "err";
		return;
	}
	iterNew->Run();
}

CoTask<int> PlayerGateSession_Game::CoAddBuilding(const 建筑类型 类型)
{
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//不能存对象，扩容可能导致引用和指针失效
	auto tuple = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 1 },
		[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//以同步编程的方式，向另一个服务器发送请求并等待返回
	const MsgChangeMoneyResponce& responce = std::get<1>(tuple);
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	CHECK_NOTNULL_CO_RET_0(m_pCurSpace);
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >({ 0,float(std::rand() % 50) }, *m_pCurSpace, "house_type19", "兵厂");
	if (0 != responce.error)
	{
		LOG(WARNING) << "扣钱失败,error=" << responce.error;
		co_return 0;
	}
	//spNewEntity->AddComponentAttack();
	spNewEntity->AddComponentPlayer(*this);
	spNewEntity->AddComponentBuilding(*this);
	m_vecSpEntity.insert(spNewEntity);//自己控制的单位
	m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void PlayerGateSession_Game::EnterSpace(Space& refSpace, const std::string &strNickName)
{
	m_pCurSpace = &refSpace;
	m_strNickName = strNickName;
	for (const auto& [id, spEntity] : refSpace.m_mapEntity)//所有地图上的实体发给自己
	{
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
	AiCo::单人剧情(m_Space单人剧情, m_funCancel单人剧情, *this).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	CHECK_NOTNULL_VOID(m_pCurSpace);
	auto& refSpace = *m_pCurSpace;
	//refThis.m_pSession->m_entity.WalkToPos(targetX, targetZ, pServer);
	for (const auto id : m_vecSelectedEntity)
	{
		auto itFind = refSpace.m_mapEntity.find(id);
		if (itFind == refSpace.m_mapEntity.end())
		{
			LOG(INFO) << "选中的实体不存在:" << id;
			//assert(false);
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

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	auto utf8Content = StrConv::Utf8ToGbk(msg.content);
	LOG(INFO) << "收到聊天:" << utf8Content;
	SendToWorldSvr<MsgSay>(msg, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	LOG(INFO) << "收到选择:" << msg.ids.size();
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
	//	if (MsgId::Invalid_0 == msgId)//没有消息可处理
	//		break;

	//	switch (msgId)
	//	{
	//	case MsgId::Login:this->m_MsgQueue.OnRecv(this->m_queueLogin, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::Move:this->m_MsgQueue.OnRecv(this->m_queueMove, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::Say:this->m_MsgQueue.OnRecv(this->m_queueSay, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::SelectRoles:this->m_MsgQueue.OnRecv(this->m_queueSelectRoles, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::AddRole:this->m_MsgQueue.OnRecv(this->m_queueAddRole, *this, &PlayerGateSession::OnRecv); break;
	//	case MsgId::AddBuilding:this->m_MsgQueue.OnRecv(this->m_queueAddBuilding, *this, &PlayerGateSession::OnRecv); break;
	//		//case MsgId::Gate转发:this->m_MsgQueue.OnRecv(this->m_queueGate转发, *this, &OnRecv); break;
	//	default:
	//		LOG(ERROR) << "msgId:" << msgId;
	//		assert(false);
	//		break;
	//	}
	//}
	m_Space单人剧情.Update();
}
