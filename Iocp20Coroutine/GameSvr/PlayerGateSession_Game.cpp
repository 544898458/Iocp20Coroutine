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
#include "造活动单位Component.h"
#include "地堡Component.h"
#include "走Component.h"
#include "PlayerComponent.h"

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

void PlayerGateSession_Game::Say(const std::string& str, const SayChannel channel)
{
	Send< MsgSay>({ .content = StrConv::GbkToUtf8(str),.channel = channel });
}
void PlayerGateSession_Game::Say系统(const std::string& str)
{
	Say(str, SayChannel::系统);
}

void PlayerGateSession_Game::Say语音提示(const std::string& str)
{
	Say(str, SayChannel::语音提示);
}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			if (!ref.m_sp造活动单位)
			{
				Say系统("造不了");
				return;
			}

			ref.m_sp造活动单位->造兵(*this, ref, msg.类型);
		});
}

void PlayerGateSession_Game::OnRecv(const Msg采集& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			CHECK_VOID(!m_wpSpace.expired());
			auto wpEntity = m_wpSpace.lock()->GetEntity((int64_t)msg.id目标资源);
			CHECK_RET_VOID(!wpEntity.expired());

			if (!ref.m_sp采集)
			{
				Say系统("此单位无法采集资源");
				return;
			}

			ref.m_sp采集->采集(*this, wpEntity);
		});
}

void PlayerGateSession_Game::OnRecv(const Msg出地堡& msg)
{
	CHECK_VOID(!m_wpSpace.expired());
	auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.id地堡);
	//msg.vecId地堡内单位.size();
	CHECK_RET_VOID(!wpTarget.expired());
	auto spTarget = wpTarget.lock();
	if (!spTarget->m_sp地堡)
	{
		Say系统("目标不是地堡");
		return;
	}
	spTarget->m_sp地堡->全都出地堡();
}

void PlayerGateSession_Game::OnRecv(const Msg进地堡& msg)
{
	std::list<std::function<void()>> listFun;
	ForEachSelected([this, &msg, &listFun](Entity& ref)
		{
			CHECK_VOID(!m_wpSpace.expired());
			auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.id目标地堡);
			CHECK_RET_VOID(!wpTarget.expired());
			auto spTarget = wpTarget.lock();
			if (!spTarget->m_sp地堡)
			{
				Say系统("目标不是地堡");
				return;
			}

			if (!ref.m_spAttack)
			{
				Say系统("此单位不可进入地堡");
				return;
			}

			listFun.emplace_back([&ref, &wpTarget, this]()
				{
					if (ref.m_sp走)
						ref.m_sp走->走进地堡(wpTarget);
				});
		});

	for (auto& fun : listFun)
	{
		fun();
	}
}

void PlayerGateSession_Game::OnRecv(const Msg进Space& msg)
{
	LOG(INFO) << "希望进Space:" << msg.idSapce;
	EnterSpace(Space::GetSpace(msg.idSapce), this->NickName());
}

void PlayerGateSession_Game::OnRecv(const Msg进单人剧情副本& msg)
{
	m_spSpace单人剧情副本 = std::make_shared<Space>();
	EnterSpace(m_spSpace单人剧情副本, this->NickName());
	单人剧情::Co(*m_spSpace单人剧情副本, m_funCancel单人剧情, *this).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	if (m_wpSpace.expired())
	{
		Say系统("还没进地图");
		return;
	}
	ForEachSelected([this, targetX, targetZ](Entity& ref)
		{
			if (!ref.m_sp走)
				return;

			if (ref.m_sp采集)
				ref.m_sp采集->m_TaskCancel.TryCancel();

			if (ref.m_spAttack)
				ref.m_spAttack->TryCancel();

			ref.m_sp走->TryCancel();
			ref.m_sp走->WalkToPos手动控制(Position(targetX, targetZ));
			Say语音提示("走走走!");//Go! Go! Go!

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
	CoAddBuilding(msg.类型, msg.pos).RunNew();
}

CoTask<int> PlayerGateSession_Game::CoAddBuilding(const 建筑单位类型 类型, const Position pos)
{
	单位::建筑单位配置 配置;
	if (!单位::Find建筑单位配置(类型, 配置))
	{
		co_return 0;
	}
	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!可放置建筑(pos, 配置.f半边长))
	{
		Say系统("此处不可放置");//（Err00） I can't build it, something's in the way. 我没法在这建，有东西挡道
		co_return 0;
	}
	if (配置.建造.u16消耗燃气矿 > m_u32燃气矿)
	{
		std::ostringstream oss;
		oss << "燃气矿不足" << 配置.建造.u16消耗燃气矿;//(low error beep) Insufficient Vespene Gas.气矿不足 
		Say系统(oss.str());
		co_return 0;
	}
	m_u32燃气矿 -= 配置.建造.u16消耗燃气矿;
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//不能存对象，扩容可能导致引用和指针失效
	auto [stop, responce] = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 配置.建造.u16消耗晶体矿 },
		[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//以同步编程的方式，向另一个服务器发送请求并等待返回
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	if (stop)
	{
		m_u32燃气矿 += 配置.建造.u16消耗燃气矿;//返还燃气矿
		co_return 0;
	}
	if (0 != responce.error)
	{
		//LOG(WARNING) << "扣钱失败,error=" << responce.error;
		m_u32燃气矿 += 配置.建造.u16消耗燃气矿;//返还燃气矿
		Say系统("晶体矿矿不足" + 配置.建造.u16消耗晶体矿);
		co_return 0;
	}

	//加建筑
	CHECK_CO_RET_0(!m_wpSpace.expired());
	auto spSpace = m_wpSpace.lock();
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >(
		pos, *spSpace, 配置.配置.strPrefabName, 配置.配置.strName);
	//spNewEntity->AddComponentAttack();
	PlayerComponent::AddComponent(*spNewEntity, *this);
	BuildingComponent::AddComponent(*spNewEntity, *this, 类型, 配置.f半边长);
	switch (类型)
	{
	case 基地:
	case 兵厂:
		造活动单位Component::AddComponet(*spNewEntity, *this, 类型);
		break;
	case 地堡:
		地堡Component::AddComponet(*spNewEntity, *this);
		break;
	case 民房:break;
	}
	DefenceComponent::AddComponent(*spNewEntity, 配置.建造.u16初始Hp);
	//spNewEntity->m_spBuilding->m_fun造活动单位 = 配置.fun造兵;
	m_setSpEntity.insert(spNewEntity);//自己控制的单位
	spSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位

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

	Send<Msg进Space>({ .idSapce = 1 });

	for (const auto& [id, spEntity] : sp->m_mapEntity)//所有地图上的实体发给自己
	{
		LOG(INFO) << spEntity->NickName() << ",发给单人," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const std::string&, const std::string& >({ 0.0 }, *sp, "smoke", "视口");
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
	LOG(INFO) << "收到聊天:" << utf8Content;
	SendToWorldSvr<MsgSay>(msg, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	CHECK_VOID(!m_wpSpace.expired());
	LOG(INFO) << "收到选择:" << msg.ids.size();
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
			switch (spEntity->m_spAttack->m_类型)
			{
			case 兵:Say语音提示("待命中!"); break;//Standing by. 待命中
			case 近战兵:Say语音提示("准备行动!"); break;//Checked up and good to go. 检查完毕，准备动身
			case 工程车:Say语音提示("老大!"); break;//Commander.
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
		LOG(ERROR) << typeid(T_Msg).name() << ",反序列化失败," << error.what();
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
	case MsgId::进Space:RecvMsg<Msg进Space>(obj); break;
	case MsgId::进单人剧情副本:RecvMsg<Msg进单人剧情副本>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::采集:RecvMsg<Msg采集>(obj); break;
	case MsgId::进地堡:RecvMsg<Msg进地堡>(obj); break;
	case MsgId::出地堡:RecvMsg<Msg出地堡>(obj); break;
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

	if (m_spSpace单人剧情副本)
		m_spSpace单人剧情副本->Update();
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
		if (refEntity->m_sp造活动单位)
		{
			制造队列中的单位 += (uint16_t)refEntity->m_sp造活动单位->等待造Count();//m_i等待造兵数;

		}
		else
		{
			++制造队列中的单位;
		}

	}

	return 制造队列中的单位;

}

bool PlayerGateSession_Game::可放置建筑(const Position& refPos, float f半边长)
{

	CHECK_FALSE(!m_wpSpace.expired());
	auto spSpace = m_wpSpace.lock();

	if (!spSpace->CrowdTool可站立({ refPos.x - f半边长 ,refPos.z + f半边长 }))return false;
	if (!spSpace->CrowdTool可站立({ refPos.x - f半边长 ,refPos.z - f半边长 }))return false;
	if (!spSpace->CrowdTool可站立({ refPos.x + f半边长 ,refPos.z + f半边长 }))return false;
	if (!spSpace->CrowdTool可站立({ refPos.x + f半边长 ,refPos.z - f半边长 }))return false;

	//遍历全地图所有建筑判断重叠
	CHECK_RET_FALSE(!m_wpSpace.expired());
	for (const auto& kv : m_wpSpace.lock()->m_mapEntity)
	{
		const auto& refPosOld = kv.second->m_Pos;
		bool CrowdTool判断单位重叠(const Position & refPosOld, const Position & refPosNew, const float f半边长);
		if (CrowdTool判断单位重叠(refPos, refPosOld, f半边长))
			return false;
	}

	return true;
}