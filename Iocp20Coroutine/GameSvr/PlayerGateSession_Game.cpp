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
#include "造建筑Component.h"
#include "地堡Component.h"
#include "走Component.h"
#include "PlayerComponent.h"
#include "资源Component.h"
#include "../CoRoutine/CoTimer.h"
#include "EntitySystem.h"

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


template void PlayerGateSession_Game::Send(const MsgAddRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyPos&);
template void PlayerGateSession_Game::Send(const MsgChangeSkeleAnim&);
template void PlayerGateSession_Game::Send(const MsgSay&);
template void PlayerGateSession_Game::Send(const MsgDelRoleRet&);
template void PlayerGateSession_Game::Send(const MsgNotifyMoney&);
template void PlayerGateSession_Game::Send(const Msg显示界面&);
template void PlayerGateSession_Game::Send(const MsgEntity描述&);


void PlayerGateSession_Game::OnDestroy()
{
	for (auto [_, wp] : m_mapWpEntity)
	{
		//assert(!wp.expired());
		if (wp.expired())
		{
			LOG(ERROR) << "删了单位，但是这里没删";
			continue;
		}
		auto sp = wp.lock();
		if (m_spSpace单人剧情副本 || EntitySystem::Is视口(*sp))
		{
			if (sp->m_refSpace.GetEntity(sp->Id).expired())
			{
				LOG(INFO) << "可能是地堡里的兵" << sp->NickName();
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
			m_wpSpace.lock()->m_map已离线PlayerEntity[NickName()].insert({ sp->Id,sp });
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

	if (m_funCancel进地图)
		m_funCancel进地图();

	const bool b离开 = !m_wpSpace.expired();
	m_wpSpace.reset();
	m_spSpace单人剧情副本.reset();

	if (b离开)
		Send<Msg离开Space>({});
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
				PlayerComponent::播放声音(ref, "BUZZ", "造不了");
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

			listFun.emplace_back([&ref, wpTarget, this]()
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

CoTaskBool PlayerGateSession_Game::Co进多人联机地图()
{
	auto pos出生 = Position(std::rand() % 100 - 50, std::rand() % 50 - 25);
	{
		const 活动单位类型 类型(活动单位类型::兵);
		单位::活动单位配置 配置;
		单位::Find活动单位配置(类型, 配置);
		造活动单位Component::造活动单位(*this, { pos出生.x, pos出生.z + 6 }, 配置, 类型);
	}
	{
		const 活动单位类型 类型(活动单位类型::三色坦克);
		单位::活动单位配置 配置;
		单位::Find活动单位配置(类型, 配置);

		造活动单位Component::造活动单位(*this, { pos出生.x + 6, pos出生.z }, 配置, 类型);
	}
	{
		const 活动单位类型 类型(活动单位类型::工程车);
		单位::活动单位配置 配置;
		单位::Find活动单位配置(类型, 配置);

		SpEntity sp工程车 = 造活动单位Component::造活动单位(*this, pos出生, 配置, 类型);
		Send设置视口(*sp工程车);
	}
	auto [stop, msgResponce] = co_await AiCo::ChangeMoney(*this, 0, true, m_funCancel进地图);
	if (stop)
		co_return true;

	const uint16_t u16初始晶体矿(100);
	if (msgResponce.finalMoney < u16初始晶体矿)
	{
		if (std::get<0>(co_await AiCo::ChangeMoney(*this, u16初始晶体矿, true, m_funCancel进地图)))
			co_return true;
	}

	using namespace std;
	const auto seconds消息间隔 = 10s;
	Say("这是每个玩家都可以自由共同进入的场景，分布有一些资源和少量的怪，资源的再生速度很慢", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("您开局只有一台工程车，工程车可以建造建筑，建筑中可以产出活动单位", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("一旦您退出此场景或断线，您在此场景中的所有单位都会消失", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("其他玩家是您的敌人，他们可能会向您进攻！", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("如果局势对您不利，您可以退出此场景再次进入，就有机会东山再起！", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("请施展您的指挥艺术吧，加油！", SayChannel::系统);

	co_return false;
}
void PlayerGateSession_Game::OnRecv(const Msg进Space& msg)
{
	OnDestroy();
	LOG(INFO) << "希望进Space:" << msg.idSapce;
	EnterSpace(Space::GetSpace(msg.idSapce));

	if (m_funCancel进地图)
		m_funCancel进地图();

	Co进多人联机地图().RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg离开Space& msg)
{
	LOG(INFO) << "希望离开Space:";// << msg.idSapce;
	OnDestroy();
}

typedef CoTask<int>(*funCo副本剧情)(Space& refSpace, FunCancel& funCancel, PlayerGateSession_Game& refGateSession);
struct 副本配置
{
	std::string str寻路文件名;
	funCo副本剧情 funCo剧情;
};

std::unordered_map<单人剧情副本ID, 副本配置> g_map副本配置 =
{
	{训练战,{"all_tiles_tilecache.bin",单人剧情::Co训练战}},
	{防守战,{"防守战.bin",单人剧情::Co防守战}},
};

void PlayerGateSession_Game::OnRecv(const Msg进单人剧情副本& msg)
{
	const auto itFind = g_map副本配置.find(msg.id);
	if (itFind == g_map副本配置.end())
	{
		assert(false);
		return;
	}
	const auto& ref配置 = itFind->second;
	m_spSpace单人剧情副本 = std::make_shared<Space, const std::string&>(ref配置.str寻路文件名);
	EnterSpace(m_spSpace单人剧情副本);

	ref配置.funCo剧情(*m_spSpace单人剧情副本, m_funCancel进地图, *this).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.pos;
	const auto pos = msg.pos;
	if (m_wpSpace.expired())
	{
		Say系统("还没进地图");
		return;
	}
	bool b已播放声音(false);
	ForEachSelected([this, msg, &b已播放声音](Entity& ref)
		{
			if (!ref.m_sp走)
				return;

			if (造建筑Component::正在建造(ref))
			{
				PlayerComponent::播放声音(ref, "BUZZ", "正在建造，不能移动");
				return;
			}

			if (ref.m_sp采集)
				ref.m_sp采集->m_TaskCancel.TryCancel();

			if (ref.m_spAttack)
				ref.m_spAttack->TryCancel();

			走Component::Cancel所有包含走路的协程(ref);
			if (msg.b遇到敌人自动攻击)
				ref.m_sp走->WalkToPos(msg.pos);
			else
				ref.m_sp走->WalkToPos手动控制(msg.pos);

			if (b已播放声音)
				return;

			b已播放声音 = true;
			if (ref.m_spAttack)
			{
				switch (ref.m_spAttack->m_类型)
				{
				case 兵:播放声音("TMaYes00"); break;//Standing by. 待命中
				case 近战兵:播放声音("tfbYes03"); break;//Checked up and good to go. 检查完毕，准备动身
				case 工程车:播放声音("语音/是女声可爱版"); break;
				case 三色坦克:播放声音("语音/ttayes01"); break;
				default:break;
				}
			}
			//else if (ref.m_spBuilding) 
			//{
			//	switch (ref.m_spBuilding->m_类型)
			//	{
			//	case 基地:播放声音("tcsWht00"); break;
			//	case 兵厂:播放声音("tclWht00"); break;
			//	default:
			//		break;
			//	}
			//}
		});
}

void PlayerGateSession_Game::播放声音(const std::string& refStr声音, const std::string& str文本)
{
	Send<Msg播放声音>({ .str声音 = StrConv::GbkToUtf8(refStr声音), .str文本 = StrConv::GbkToUtf8(str文本) });
}

void PlayerGateSession_Game::Send设置视口(const Entity& refEntity)
{
	Send<Msg设置视口>({ .pos视口 = refEntity.Pos() });
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
		if (m_mapWpEntity.end() == std::find_if(m_mapWpEntity.begin(), m_mapWpEntity.end(), [&spEntity](const auto& kv)
			{
				auto& wp = kv.second;
				assert(!wp.expired());
				auto sp = wp.lock();
				return sp == spEntity;
			}))
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
	//CoAddBuilding(msg.类型, msg.pos).RunNew();
	ForEachSelected([this, msg](Entity& ref)
		{
			if (造建筑Component::正在建造(ref))
			{
				PlayerComponent::播放声音(ref, "BUZZ", "正在建造，不能建造");
				return;
			}

			if (ref.m_spAttack)
			{
				if (ref.m_spAttack->m_cancelAttack)
				{
					LOG(INFO) << "正在攻击不能建造";
					return;
				}
				if (ref.m_spAttack->m_TaskCancel.cancel)
				{
					LOG(INFO) << "正在走向攻击目标不能建造";
					return;
				}
			}
			if (!ref.m_sp造建筑)
			{
				PlayerComponent::播放声音(ref, "BUZZ", "造不了这种建筑");
				return;
			}

			走Component::Cancel所有包含走路的协程(ref); //TryCancel();
			ref.m_sp造建筑->Co造建筑(msg.pos, msg.类型).RunNew();
		});
}

CoTask<SpEntity> PlayerGateSession_Game::CoAddBuilding(const 建筑单位类型 类型, const Position pos)
{
	单位::建筑单位配置 配置;
	if (!单位::Find建筑单位配置(类型, 配置))
	{
		co_return{};
	}
	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!可放置建筑(pos, 配置.f半边长))
	{
		//播放声音("TSCErr00", "有阻挡，无法建造");//（Err00） I can't build it, something's in the way. 我没法在这建，有东西挡道
		播放声音("语音/无法在这里建造可爱版", "有阻挡，无法建造");
		co_return{};
	}
	if (配置.建造.u16消耗燃气矿 > m_u32燃气矿)
	{
		//std::ostringstream oss;
		播放声音("语音/燃气矿不足可爱版", "燃气矿不足，无法建造");// << 配置.建造.u16消耗燃气矿;//(low error beep) Insufficient Vespene Gas.气矿不足 
		//Say系统(oss.str());
		co_return{};
	}
	m_u32燃气矿 -= 配置.建造.u16消耗燃气矿;
	auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//不能存对象，扩容可能导致引用和指针失效
	//auto [stop, responce] = co_await CoRpc<MsgChangeMoneyResponce>::Send<MsgChangeMoney>({ .changeMoney = 配置.建造.u16消耗晶体矿 },
	//	[this](const MsgChangeMoney& ref) {SendToWorldSvr<MsgChangeMoney>(ref, m_idPlayerGateSession); }, **iterNew);//以同步编程的方式，向另一个服务器发送请求并等待返回
	auto [stop, responce] = co_await AiCo::ChangeMoney(*this, 配置.建造.u16消耗晶体矿, false, **iterNew);
	LOG(INFO) << "协程RPC返回,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	if (stop)
	{
		m_u32燃气矿 += 配置.建造.u16消耗燃气矿;//返还燃气矿
		co_return{};
	}
	if (0 != responce.error)
	{
		//LOG(WARNING) << "扣钱失败,error=" << responce.error;
		m_u32燃气矿 += 配置.建造.u16消耗燃气矿;//返还燃气矿
		播放声音("语音/晶体矿不足可爱版", "晶体矿不足无法建造");//Say系统("晶体矿矿不足" + 配置.建造.u16消耗晶体矿);

		co_return{};
	}

	Send资源();

	//加建筑
	CHECK_CO_RET_0(!m_wpSpace.expired());
	auto spSpace = m_wpSpace.lock();
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const 单位::单位配置& >(
		pos, *spSpace, 配置.配置);
	//spNewEntity->AddComponentAttack();
	PlayerComponent::AddComponent(*spNewEntity, *this);
	BuildingComponent::AddComponent(*spNewEntity, *this, 类型, 配置.f半边长);
	switch (类型)
	{
	case 基地:
	case 兵厂:
		造活动单位Component::AddComponent(*spNewEntity, *this, 类型);
		break;
	case 地堡:
		地堡Component::AddComponet(*spNewEntity, *this);
		break;
	case 民房:break;
	}
	DefenceComponent::AddComponent(*spNewEntity, 配置.建造.u16初始Hp);
	//spNewEntity->m_spBuilding->m_fun造活动单位 = 配置.fun造兵;
	m_mapWpEntity[spNewEntity->Id] = spNewEntity;//自己控制的单位
	//spSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//全地图单位
	spSpace->AddEntity(spNewEntity);

	spNewEntity->BroadcastEnter();
	Send资源();
	co_return spNewEntity;
}

void PlayerGateSession_Game::EnterSpace(WpSpace wpSpace)
{
	assert(m_wpSpace.expired());
	assert(!wpSpace.expired());
	m_wpSpace = wpSpace;
	auto sp = m_wpSpace.lock();

	Send<Msg进Space>({ .idSapce = 1 });
	{
		auto mapOld = sp->m_map已离线PlayerEntity[NickName()];
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
	for (const auto& [id, spEntity] : sp->m_mapEntity)//所有地图上的实体发给自己
	{
		LOG(INFO) << spEntity->NickName() << ",发给单人," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const 单位::单位配置&>(
		{ 0.0 }, *sp, { "视口","smoke", "" });
	m_mapWpEntity[spEntityViewPort->Id] = (spEntityViewPort);
	PlayerComponent::AddComponent(*spEntityViewPort, *this);
	sp->AddEntity(spEntityViewPort, 100);
	spEntityViewPort->BroadcastEnter();

	CoEvent<PlayerGateSession_Game*>::OnRecvEvent(false, this);
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	MsgSay msg加名字 = msg;
	auto strGbk = StrConv::Utf8ToGbk(msg加名字.content);
	LOG(INFO) << "收到聊天:" << strGbk;
	msg加名字.content = StrConv::GbkToUtf8(NickName() + " 说:" + strGbk);
	SendToWorldSvr<MsgSay>(msg加名字, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	CHECK_VOID(!m_wpSpace.expired());
	LOG(INFO) << "收到选择:" << msg.ids.size();
	std::vector<uint64_t> listSelectedEntity;
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(listSelectedEntity), [](const double& id) {return uint64_t(id); });
	选中单位(listSelectedEntity);
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
		LOG(ERROR) << typeid(T_Msg).name() << ",反序列化失败," << error.what();
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
	case MsgId::进Space:RecvMsg<Msg进Space>(obj); break;
	case MsgId::离开Space:RecvMsg<Msg离开Space>(obj); break;
	case MsgId::进单人剧情副本:RecvMsg<Msg进单人剧情副本>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::采集:RecvMsg<Msg采集>(obj); break;
	case MsgId::进地堡:RecvMsg<Msg进地堡>(obj); break;
	case MsgId::出地堡:RecvMsg<Msg出地堡>(obj); break;
	case MsgId::框选:RecvMsg<Msg框选>(obj); break;
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
	for (const auto& [_, wp] : m_mapWpEntity)
	{
		assert(!wp.expired());
		const auto& refEntity = wp.lock();
		if (!refEntity->m_spBuilding)continue;
		if (!refEntity->m_spBuilding->已造好())continue;

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
	for (const auto& [_, wp] : m_mapWpEntity)
	{
		assert(!wp.expired());
		const auto& refEntity = *wp.lock();
		if (EntitySystem::Is视口(refEntity))
		{
			continue;//不可攻击的自己的单位，是视口
		}
		if (refEntity.m_sp造活动单位)
		{
			制造队列中的单位 += (uint16_t)refEntity.m_sp造活动单位->等待造Count();//m_i等待造兵数;

		}

		if (refEntity.m_spBuilding)
			continue;//民房

		++制造队列中的单位;
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
		auto& refEntity = *kv.second;
		const auto& refPosOld = refEntity.Pos();
		bool CrowdTool判断单位重叠(const Position & refPosOld, const Position & refPosNew, const float f半边长);
		if (CrowdTool判断单位重叠(refPos, refPosOld, f半边长))
			return false;
	}

	return true;
}

void PlayerGateSession_Game::OnRecv(const Msg框选& msg)
{
	const Position pos左上(std::min(msg.pos起始.x, msg.pos结束.x), std::min(msg.pos起始.z, msg.pos结束.z));
	const Position pos右下(std::max(msg.pos起始.x, msg.pos结束.x), std::max(msg.pos起始.z, msg.pos结束.z));
	const Rect rect = { pos左上,pos右下 };
	std::vector<uint64_t> vec;
	for (const auto [k, wpEntity] : m_mapWpEntity)
	{
		if (wpEntity.expired())
		{
			LOG(ERROR) << "";
			continue;
		}

		auto& refEntity = *wpEntity.lock();
		if (rect.包含此点(refEntity.Pos()))
			vec.push_back(refEntity.Id);
	}
	选中单位(vec);
}

void PlayerGateSession_Game::选中单位(const std::vector<uint64_t>& vecId)
{
	m_listSelectedEntity.clear();
	bool b已发送选中音效(false);
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
			LOG(WARNING) << "可能选中了已进地堡的兵";
			continue;
		}
		auto spEntity = wpEntity.lock();
		if (!spEntity->m_wpOwner.expired())
			continue;//地堡内

		//if (spEntity->m_spBuilding)
		//	continue;//不可框选建筑单位

		if (EntitySystem::Is视口(*spEntity))
			continue;

		if (!spEntity->m_spPlayer)
			continue;

		if (&spEntity->m_spPlayer->m_refSession != this)//不是自己的单位
			continue;

		m_listSelectedEntity.push_back(spEntity->Id);
		if (!b已发送选中音效)
		{
			b已发送选中音效 = true;
			Send选中音效(*spEntity);
		}
	}
	Send选中单位Responce();
}

void PlayerGateSession_Game::Send选中音效(const Entity& refEntity)
{
	if (refEntity.m_spAttack)
	{
		//switch (refEntity.m_spAttack->m_类型)
		//{
		//case 兵:播放声音("TMaPss00"); break;// Say语音提示("待命中!"); break;//Standing by. 待命中
		//case 近战兵:播放声音("tfbPss00"); break;//Say语音提示("准备行动!"); break;//Checked up and good to go. 检查完毕，准备动身
		//case 工程车:播放声音("TSCPss00"); break;//Commander.
		//default:break;
		//}
		播放声音(refEntity.m_配置.str选中音效);
	}
	else if (refEntity.m_spBuilding)
	{
		//switch (refEntity.m_spBuilding->m_类型)
		//{
		//case 基地:播放声音("tcsWht00"); break;
		//case 兵厂:播放声音("tacWht00"); break;
		//case 民房:播放声音("tclWht00"); break;
		//default:
		//	break;
		//}
		播放声音(refEntity.m_配置.str选中音效);
	}
	else if (refEntity.m_sp资源) {
		switch (refEntity.m_sp资源->m_类型)
		{
		case 晶体矿:
		case 燃气矿:
		default:
			播放声音("音效/BUTTON");
			break;
		}
	}
}

void PlayerGateSession_Game::Send选中单位Responce()
{
	MsgSelectRoles msgResponse;
	msgResponse.ids.insert(msgResponse.ids.end(), m_listSelectedEntity.begin(), m_listSelectedEntity.end());
	Send(msgResponse);
}
