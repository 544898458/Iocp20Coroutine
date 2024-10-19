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
#include "采集Component.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "BuildingComponent.h"
#include <unordered_map>
#include "单位.h"
#include "单人剧情.h"
#include <sstream>
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

	if (m_funCancel单人剧情)
		m_funCancel单人剧情();
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

void PlayerGateSession_Game::Say(const std::string& str)
{
	Send(MsgSay(StrConv::GbkToUtf8(str)));
}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	ForEachSelected([this](Entity& ref)
		{
			if (ref.m_spBuilding)
				ref.m_spBuilding->造兵(*this, ref);
		});
}

void PlayerGateSession_Game::OnRecv(const Msg采集& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			if (!ref.m_sp采集)
				return;

			CHECK_NOTNULL_VOID(m_pCurSpace);
			auto wpEntity = m_pCurSpace->GetEntity((int64_t)msg.id目标资源);
			if (!wpEntity.expired())
				ref.m_sp采集->采集(*this, ref, wpEntity);
		});
}


void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	CHECK_NOTNULL_VOID(m_pCurSpace);
	ForEachSelected([this, targetX, targetZ](Entity& ref)
		{
			if (ref.m_spAttack)
			{
				if (ref.m_sp采集)
					ref.m_sp采集->m_TaskCancel.TryCancel();

				ref.m_spAttack->TryCancel();
				ref.m_spAttack->WalkToPos手动控制(Position(targetX, targetZ));
			}
		});
}

void PlayerGateSession_Game::ForEachSelected(std::function<void(Entity& ref)> fun)
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
		if (m_setSpEntity.end() == std::find_if(m_setSpEntity.begin(), m_setSpEntity.end(), [&spEntity](const auto& sp) {return sp == spEntity; }))
		{
			LOG(ERROR) << id << "不是自己的单位，不能操作";
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
	//	LOG(WARNING) << "m_coRpc前一个协程还没结束";
	//	return;
	//}
	/*auto iterNew = m_vecCoRpc.insert(m_vecCoRpc.end(), CoAddBuilding(msg.类型));
	if (iterNew == m_vecCoRpc.end())
	{
		LOG(ERROR) << "err";
		return;
	}
	iterNew->Run();*/
	CoAddBuilding(msg.类型).RunNew();
}

CoTask<int> PlayerGateSession_Game::CoAddBuilding(const 建筑单位类型 类型)
{
	单位::建筑单位配置 配置;
	if (!单位::Find建筑单位配置(类型, 配置))
	{
		co_return 0;
	}
	if (配置.消耗.u32消耗燃气矿 > m_u32燃气矿)
	{
		std::ostringstream oss;
		oss << "燃气矿不足" << 配置.消耗.u32消耗燃气矿;
		Say(oss.str());
		co_return 0;
	}
	m_u32燃气矿 -= 配置.消耗.u32消耗燃气矿;
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//不能存对象，扩容可能导致引用和指针失效
	auto [stop, responce] = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 配置.消耗.u32消耗晶体矿 },
		[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//以同步编程的方式，向另一个服务器发送请求并等待返回
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	if (stop)
	{
		m_u32燃气矿 += 配置.消耗.u32消耗燃气矿;//返还燃气矿
		co_return 0;
	}
	if (0 != responce.error)
	{
		//LOG(WARNING) << "扣钱失败,error=" << responce.error;
		m_u32燃气矿 += 配置.消耗.u32消耗燃气矿;//返还燃气矿
		Say("晶体矿矿不足" + 配置.消耗.u32消耗晶体矿);
		co_return 0;
	}

	//加建筑
	CHECK_NOTNULL_CO_RET_0(m_pCurSpace);
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >(
		{ 35,float(std::rand() % 20) }, *m_pCurSpace, 配置.配置.strPrefabName, 配置.配置.strName);
	//spNewEntity->AddComponentAttack();
	spNewEntity->AddComponentPlayer(*this);
	BuildingComponent::AddComponent(*spNewEntity, *this, 类型, 配置.f半边长);
	DefenceComponent::AddComponent(*spNewEntity);
	spNewEntity->m_spBuilding->m_fun造活动单位 = 配置.fun造兵;
	m_setSpEntity.insert(spNewEntity);//自己控制的单位
	m_pCurSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

	spNewEntity->BroadcastEnter();
	co_return 0;
}

void PlayerGateSession_Game::EnterSpace(Space& refSpace, const std::string& strNickName)
{
	m_pCurSpace = &refSpace;
	m_strNickName = strNickName;
	for (const auto& [id, spEntity] : refSpace.m_mapEntity)//所有地图上的实体发给自己
	{
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
	单人剧情::Co(m_Space单人剧情, m_funCancel单人剧情, *this).RunNew();
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
	try
	{
		const auto msg = obj.as<T_Msg>();
		OnRecv(msg);
	}
	catch (const msgpack::type_error& error)
	{
		LOG(ERROR) << typeid(T_Msg).name() << ",反序列化失败," << error.what();
		assert(false);
		return;
	}
}
extern Space g_Space无限刷怪;
PlayerGateSession_Game::PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession) :
	m_refSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_Space单人剧情(g_Space无限刷怪)
{

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
	case MsgId::采集:RecvMsg<Msg采集>(obj); break;
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


void PlayerGateSession_Game::Send资源()
{
	Send<Msg资源>({ .燃气矿 = m_u32燃气矿,.活动单位 = 活动单位包括制造队列中的(),.活动单位上限 = 活动单位上限() });
}

uint16_t PlayerGateSession_Game::活动单位上限() const
{
	uint16_t result = 0;
	for (const auto& refEntity : m_setSpEntity)
	{
		if (!refEntity->m_spBuilding)
			continue;

		switch (refEntity->m_spBuilding->m_类型)
		{
		case 民房:result += 5; break;
		case 基地:result += 2; break;
		default:break;
		}
	}
	return result;
}

uint16_t PlayerGateSession_Game::活动单位包括制造队列中的() const
{
	uint16_t 制造队列中的单位 = 0;
	for (const auto& refEntity : m_setSpEntity)
	{
		if (refEntity->m_spBuilding)
		{
			制造队列中的单位 += refEntity->m_spBuilding->m_i等待造兵数;
			
		}
		else
		{
			++制造队列中的单位;
		}
		
	}

	return 制造队列中的单位;

}