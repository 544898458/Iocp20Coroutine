#include "pch.h"
#include "PlayerGateSession_Game.h"
#include "Space.h"
#include "Entity.h"
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"
#include "../CoRoutine/CoRpc.h"
#include "../CoRoutine/CoEvent.h"
#include "AiCo.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/DefenceComponent.h"
#include "单位组件/采集Component.h"
#include "GameSvr.h"
#include "../IocpNetwork/MsgQueueMsgPackTemplate.h"
#include "单位组件/BuildingComponent.h"
#include <unordered_map>
#include "单位.h"
#include "单人剧情.h"
#include "多人战局.h"
#include <sstream>
#include "单位组件/造活动单位Component.h"
#include "单位组件/造建筑Component.h"
#include "单位组件/地堡Component.h"
#include "单位组件/走Component.h"
#include "单位组件/PlayerComponent.h"
#include "单位组件/资源Component.h"
#include "../CoRoutine/CoTimer.h"
#include "EntitySystem.h"
#include "单位组件/PlayerNickNameComponent.h"
#include "MyEvent.h"
#include "../读配置文件/Try读Ini本地机器专用.h"
std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);


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
					this->m_refGameSvrSession.SendToGate(buf转发, len转发);
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
template void PlayerGateSession_Game::Send(const Msg弹丸特效&);


void PlayerGateSession_Game::OnDestroy()
{
	离开Space(false);
}

void PlayerGateSession_Game::离开Space(const bool b主动退)
{
	if (!m_wpSpace.expired())
	{
		auto wp我的单人 = Space::GetSpace单人(NickName());
		m_wpSpace.lock()->m_mapPlayer[NickName()].OnDestroy(b主动退 && !wp我的单人.expired(), *m_wpSpace.lock(), NickName());
	}

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
	{
		m_funCancel进地图();
		m_funCancel进地图 = nullptr;
	}

	const bool b离开 = !m_wpSpace.expired();
	m_wpSpace.reset();
	//if (m_spSpace单人剧情副本)
	//{
	//	m_spSpace单人剧情副本->OnDestory();
	//	m_spSpace单人剧情副本.reset();
	//}
	if (b主动退)
	{
		Space::DeleteSpace单人(NickName());
	}
	//if (m_spSpace多人战局)
	//{
	//	m_spSpace多人战局->OnDestory();
	//	m_spSpace多人战局.reset();
	//}
	if (b离开)
		Send<Msg离开Space>({});
}

void PlayerGateSession_Game::Say(const std::string& str, const SayChannel channel)
{
	Send<MsgSay>({ .content = StrConv::GbkToUtf8(str),.channel = channel });
}

void PlayerGateSession_Game::Say系统(const std::string& refStrNickName, const std::string& str)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->Say系统(str);

}

void PlayerGateSession_Game::Say任务提示(const std::string& refStrNickName, const std::string& str)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->Say(str, 任务提示);

}

void PlayerGateSession_Game::Say系统(const std::string& str)
{
	Say(str, SayChannel::系统);
}

//void PlayerGateSession_Game::Say语音提示(const std::string& str)
//{
//	Say(str, SayChannel::聊天);
//}

void PlayerGateSession_Game::OnRecv(const MsgAddRole& msg)
{
	bool bOK(false);
	ForEachSelected([this, &msg, &bOK](Entity& ref)
		{
			if (!ref.m_sp造活动单位)return;
			if (!ref.m_sp造活动单位->可造(msg.类型))return;
			ref.m_sp造活动单位->造兵(*this, msg.类型);
			bOK = true;
		});

	if (bOK)
		return;

	//自动找兵厂去造
	CHECK_WP_RET_VOID(m_wpSpace);
	Space& refSpace = *m_wpSpace.lock();
	std::vector<WpEntity> vecWp可造;
	for (auto [_, wp] : refSpace.m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntiy = *wp.lock();
		if (!refEntiy.m_spBuilding)continue;
		if (!refEntiy.m_spBuilding->已造好())continue;
		if (!refEntiy.m_sp造活动单位)continue;
		if (!refEntiy.m_sp造活动单位->可造(msg.类型))continue;
		vecWp可造.push_back(wp);
	}

	//待造队列最短的在前面
	std::sort(vecWp可造.begin(), vecWp可造.end(), [](const WpEntity& wp左, const WpEntity& wp右)->bool
		{
			CHECK_WP_RET_FALSE(wp左);
			CHECK_WP_RET_FALSE(wp右);
			CHECK_NOTNULL_RET_FALSE(wp左.lock()->m_sp造活动单位);
			CHECK_NOTNULL_RET_FALSE(wp右.lock()->m_sp造活动单位);
			return wp左.lock()->m_sp造活动单位->等待造Count() < wp右.lock()->m_sp造活动单位->等待造Count();
		});
	for (auto wp : vecWp可造)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntiy = *wp.lock();
		refEntiy.m_sp造活动单位->造兵(*this, msg.类型);
		return;
	}

	{
		std::string str提示("请先造一个");
		const auto _ = msg.类型;
		switch (_)
		{
		case 工程车:str提示 += "基地"; break;
		case 飞机:str提示 += "机场"; break;
		case 三色坦克:str提示 += "重工厂"; break;
		default:str提示 += "兵厂"; break;
		}
		播放声音Buzz(str提示);
	}
}

void PlayerGateSession_Game::OnRecv(const Msg采集& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
		{
			CHECK_VOID(!m_wpSpace.expired());
			auto wpEntity = m_wpSpace.lock()->GetEntity((int64_t)msg.id目标资源);
			CHECK_WP_RET_VOID(wpEntity);
			auto& ref资源 = *wpEntity.lock();
			if (基地 == ref.m_类型 || 孵化场 == ref.m_类型)
			{
				CHECK_RET_VOID(ref.m_sp造活动单位);
				ref.m_sp造活动单位->m_pos集结点 = ref资源.Pos();
				Say系统("已设定采集资源");
				return;
			}

			if (!ref.m_sp采集)
			{
				播放声音Buzz("此单位无法采集资源");
				return;
			}

			ref.m_sp采集->采集(wpEntity);
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
	CHECK_VOID(!m_wpSpace.expired());
	auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.id目标地堡);
	CHECK_RET_VOID(!wpTarget.expired());
	auto& refTarget地堡 = *wpTarget.lock();
	if (!refTarget地堡.m_sp地堡)
	{
		播放声音Buzz("目标不是地堡");
		return;
	}
	if (EntitySystem::GetNickName(refTarget地堡) != NickName())
	{
		//播放声音Buzz("不能进别人的地堡");
		MsgMove msg = { .pos = refTarget地堡.Pos(),.b遇到敌人自动攻击 = true };
		OnRecv(msg);

		return;
	}

	std::list<std::function<void()>> listFun;
	ForEachSelected([this, &msg, &listFun, &wpTarget](Entity& ref)
		{
			if (!ref.m_spAttack || !EntitySystem::Is可进地堡(ref.m_类型))
			{
				//Say系统("此单位不可进入地堡");
				播放声音Buzz("此单位不可进入地堡");
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

CoTaskBool PlayerGateSession_Game::Co进多人联机地图(WpEntity wp视口)
{
	{
		CHECK_WP_CO_RET_FALSE(m_wpSpace);
		CHECK_WP_CO_RET_FALSE(wp视口);
		auto& refSpace = *m_wpSpace.lock();
		auto& ref视口 = *wp视口.lock();
		auto pos出生 = Position(std::rand() % 100 - 50.f, std::rand() % 100 - 50.f);
		refSpace.造活动单位(ref视口, NickName(), 单位类型::工程车, pos出生, true);
		refSpace.造活动单位(ref视口, NickName(), 单位类型::兵, { pos出生.x, pos出生.z + 6 });
		refSpace.造活动单位(ref视口, NickName(), 单位类型::三色坦克, { pos出生.x + 6, pos出生.z });
		refSpace.造活动单位(ref视口, NickName(), 单位类型::工蜂, { pos出生.x + 6, pos出生.z + 6 });
		refSpace.造活动单位(ref视口, NickName(), 单位类型::近战兵, { pos出生.x - 6, pos出生.z });
		refSpace.造活动单位(ref视口, NickName(), 单位类型::飞机, { pos出生.x - 6, pos出生.z - 6 });

		//auto [stop, msgResponce] = co_await AiCo::ChangeMoney(*this, 0, true, m_funCancel进地图);
		//if (stop)
		//	co_return true;

		const uint16_t u16初始晶体矿(1000);
		auto& refSpacePlayer = refSpace.GetSpacePlayer(ref视口);
		if (refSpacePlayer.m_u32晶体矿 < u16初始晶体矿)
		{
			refSpacePlayer.m_u32晶体矿 += u16初始晶体矿;
			refSpacePlayer.m_u32燃气矿 += u16初始晶体矿;
			PlayerComponent::Send资源(ref视口);
		}
		//if (msgResponce.finalMoney < u16初始晶体矿)
		//{
			//if (std::get<0>(co_await AiCo::ChangeMoney(*this, u16初始晶体矿, true, m_funCancel进地图)))
			//	co_return true;
		//}
	}
	using namespace std;
	const auto seconds消息间隔 = 10s;
	Say("这是每个玩家都可以自由共同进入的场景，分布有一些资源和少量的怪，资源的再生速度很慢", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("您开局只有一辆工程车，工程车可以建造建筑，建筑中可以产出活动单位", SayChannel::系统);
	if (co_await CoTimer::Wait(seconds消息间隔, m_funCancel进地图)) co_return false;
	Say("一旦您退出此场景或断线后，您在此场景中的所有单位可能会因为缺少指挥而遭到攻击", SayChannel::系统);
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
	离开Space(true);
	LOG(INFO) << "希望进Space:" << msg.idSapce;
	auto wp = Space::GetSpace(msg.idSapce);
	CHECK_WP_RET_VOID(wp);
	EnterSpace(wp);

	if (m_funCancel进地图)
	{
		m_funCancel进地图();
		m_funCancel进地图 = nullptr;
	}

	Co进多人联机地图(m_wp视口).RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg离开Space& msg)
{
	LOG(INFO) << "希望离开Space:";// << msg.idSapce;
	离开Space(true);
}

std::unordered_map<副本ID, 副本配置> g_map副本配置 =
{
	{训练战,{"all_tiles_tilecache.bin",		"scene战斗",	单人剧情::Co训练战,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse.mp3"}},
	{防守战,{"防守战.bin",					"scene防守战",	单人剧情::Co防守战,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse_2.mp3"}},
	{攻坚战,{"攻坚战.bin",					"scene攻坚战",	单人剧情::Co攻坚战,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse.mp3"}},
	{多人联机地图,{"all_tiles_tilecache.bin","scene战斗",	{},	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse.mp3"}},
	{四方对战,{"四方对战.bin",				"scene四方对战",多人战局::Co四方对战,	"https://www.rtsgame.online/music/Suno_Edge_of_Collapse_2.mp3"}},
};

bool Get副本配置(const 副本ID id, 副本配置& refOut)
{
	const auto itFind = g_map副本配置.find(id);
	if (itFind == g_map副本配置.end())
	{
		LOG(ERROR) << "";
		_ASSERT(false);
		return false;
	}

	refOut = itFind->second;
	return true;
}

void PlayerGateSession_Game::OnRecv(const Msg进单人剧情副本& msg)
{
	CHECK_VOID(msg.id == 训练战 || msg.id == 防守战 || msg.id == 攻坚战);
	副本配置 配置;
	{
		const auto ok = Get副本配置(msg.id, 配置);
		CHECK_RET_VOID(ok);
	}

	//m_spSpace单人剧情副本 = std::make_shared<Space, const 副本配置&>(配置);
	auto wpOld = Space::GetSpace单人(NickName());
	if (!wpOld.expired() && wpOld.lock()->m_配置.strSceneName != 配置.strSceneName)
	{
		Space::DeleteSpace单人(NickName());
	}
	auto [b新, wpSpace] = Space::GetSpace单人(NickName(), 配置);
	//m_wpSpace单人剧情副本 = wpSpace;
	CHECK_WP_RET_VOID(wpSpace);
	EnterSpace(wpSpace);
	CHECK_WP_RET_VOID(m_wp视口);
	if (b新)
		配置.funCo剧情(*wpSpace.lock(), wpSpace.lock()->m_funCancel剧情, NickName()).RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg创建多人战局& msg)
{
	CHECK_VOID(msg.id == 四方对战);
	副本配置 配置;
	{
		const auto ok = Get副本配置(msg.id, 配置);
		CHECK_RET_VOID(ok);
	}

	//m_spSpace多人战局 = std::make_shared<Space, const 副本配置&>(配置);
	auto wpOld = Space::GetSpace单人(NickName());
	if (!wpOld.expired() && wpOld.lock()->m_配置.strSceneName != 配置.strSceneName)
	{
		Space::DeleteSpace单人(NickName());
	}
	auto [b新, wpSpace] = Space::GetSpace单人(NickName(), 配置);
	//m_wpSpace单人剧情副本 = wpSpace;
	CHECK_WP_RET_VOID(wpSpace);
	m_wp视口 = EnterSpace(wpSpace);
	CHECK_WP_RET_VOID(m_wp视口);
	配置.funCo剧情(*wpSpace.lock(), wpSpace.lock()->m_funCancel剧情, NickName()).RunNew();
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.pos;
	const auto pos = msg.pos;
	const auto b保持队形 = !pos.DistanceLessEqual(m_pos上次点击走路目标, 5);
	m_pos上次点击走路目标 = pos;
	if (m_wpSpace.expired())
	{
		Say系统("还没进地图");
		return;
	}
	auto& refSpace = *m_wpSpace.lock();

	std::vector<WpEntity> vecWp;
	bool b已播放声音(false);
	Position pos中心点 = { 0 };
	ForEachSelected([this, msg, &b已播放声音, &vecWp, &pos中心点](Entity& ref)
		{
			if (!ref.m_sp走)
				return;

			if (造建筑Component::正在建造(ref))
			{
				PlayerComponent::播放声音Buzz(ref, "正在建造，不能移动");
				return;
			}

			if (ref.m_sp采集)
				ref.m_sp采集->m_TaskCancel.TryCancel();

			if (ref.m_spAttack)
				ref.m_spAttack->TryCancel();

			vecWp.push_back(ref.weak_from_this());

			pos中心点 += ref.Pos();
		});

	if (vecWp.empty())
	{
		LOG(WARNING) << "vecWp";
		return;
	}
	pos中心点.x /= vecWp.size();
	pos中心点.z /= vecWp.size();
	for (auto& wp : vecWp)
	{
		auto& ref = *wp.lock();
		auto pos偏离 = ref.Pos() - pos中心点;

		if (ref.m_spAttack)
		{
			const auto f距离中心点Max = b保持队形 ? ref.m_spAttack->m_战斗配置.f警戒距离 : ref.m_spAttack->m_战斗配置.f攻击距离 / 5;

			if (std::abs(pos偏离.x) > f距离中心点Max)
				pos偏离.x = pos偏离.x / std::abs(pos偏离.x) * f距离中心点Max;

			if (std::abs(pos偏离.z) > f距离中心点Max)
				pos偏离.z = pos偏离.z / std::abs(pos偏离.z) * f距离中心点Max;
		}

		auto pos目标 = msg.pos + pos偏离;

		if (!refSpace.CrowdTool可站立(pos目标))
		{
			LOG(INFO) << pos目标 << "不可站立，找附近的可站立点";
			if (!refSpace.CrowdToolFindNerestPos(pos目标))
			{
				LOG(WARNING) << pos目标 << "附近没有可站立的点";
				continue;
			}
			LOG(INFO) << "找到附近的点:" << pos目标;
		}

		走Component::Cancel所有包含走路的协程(ref);
		if (ref.m_spDefence)
			ref.m_spDefence->m_map对我伤害.clear();

		if (msg.b遇到敌人自动攻击)
			ref.m_sp走->WalkToPos(pos目标);
		else
			ref.m_sp走->WalkToPos手动控制(pos目标);

		if (b已播放声音)
			continue;

		if (ref.m_spAttack)
		{
			switch (ref.m_类型)
			{
			case 兵:播放声音(msg.b遇到敌人自动攻击 ? "语音/是男声正经版" : "语音/明白男声正经版", ""); break;//Standing by. 待命中
			case 近战兵:播放声音("tfbYes03", ""); break;//Checked up and good to go. 检查完毕，准备动身
			case 工程车:播放声音(msg.b遇到敌人自动攻击 ? "语音/是女声可爱版" : "语音/明白女声可爱版", ""); break;
			case 三色坦克:播放声音("语音/ttayes01", ""); break;
			case 飞机:播放声音(msg.b遇到敌人自动攻击 ? "音效/飞机声" : "语音/飞机明白", ""); break;
			default:
				continue;
				break;
			}

			b已播放声音 = true;
		}
	}
}

void PlayerGateSession_Game::播放声音(const std::string& refStrNickName, const std::string& refStr声音, const std::string& str文本)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->播放声音(refStr声音, str文本);
}

void PlayerGateSession_Game::播放声音Buzz(const std::string& str文本)
{
	播放声音("BUZZ", str文本);
}
void PlayerGateSession_Game::播放声音(const std::string& refStr声音, const std::string& str文本)
{
	Send<Msg播放声音>({ .str声音 = StrConv::GbkToUtf8(refStr声音), .str文本 = StrConv::GbkToUtf8(str文本) });
}

void PlayerGateSession_Game::Send设置视口(const Entity& refEntity)
{
	Send<Msg设置视口>({ .pos视口 = refEntity.Pos() });
}

void PlayerGateSession_Game::删除选中(const uint64_t id)
{
	auto temp = m_vecSelectedEntity;
	temp.erase(std::remove(temp.begin(), temp.end(), id), temp.end());
	选中单位(temp);
}

void PlayerGateSession_Game::ForEachSelected(std::function<void(Entity& ref)> fun)
{
	CHECK_VOID(!m_wpSpace.expired());
	auto sp = m_wpSpace.lock();
	auto temp = m_vecSelectedEntity;//防止循环中修改容器
	for (const auto id : temp)
	{
		auto itFind = sp->m_mapEntity.find(id);
		if (itFind == sp->m_mapEntity.end())
		{
			LOG(INFO) << "选中的实体不存在:" << id;
			//_ASSERT(false);
			continue;
		}
		auto& spEntity = itFind->second;
		auto& refMap = sp->m_mapPlayer[NickName()].m_mapWpEntity;
		if (refMap.end() == std::find_if(refMap.begin(), refMap.end(), [&spEntity](const auto& kv)
			{
				auto& wp = kv.second;
				CHECK_WP_RET_FALSE(wp);
				//_ASSERT(!wp.expired());
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


bool 能造(const Entity& refEntiy, const 单位类型 造活动单位类型)
{
	if (refEntiy.m_类型 != 工程车 && refEntiy.m_类型 != 工蜂) return false;
	if (造建筑Component::正在建造(refEntiy))return false;

	CHECK_FALSE(refEntiy.m_spAttack);
	if (refEntiy.m_spAttack->m_cancelAttack)return false;
	if (refEntiy.m_spAttack->m_TaskCancel.cancel)return false;

	CHECK_FALSE(refEntiy.m_sp造建筑);
	if (造活动单位类型 > 单位类型_Invalid_0 && !refEntiy.m_sp造建筑->可造(造活动单位类型))return false;

	return true;
}
void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//CoAddBuilding(msg.类型, msg.pos).RunNew();
	bool b操作OK(false);
	ForEachSelected([this, msg, &b操作OK](Entity& refEntiy)
		{
			if (!能造(refEntiy, msg.类型))
				return;

			走Component::Cancel所有包含走路的协程(refEntiy); //TryCancel();
			refEntiy.m_sp造建筑->Co造建筑(msg.pos, msg.类型).RunNew();
			b操作OK = true;
		});

	if (b操作OK)
		return;

	//自动找一个合适的工程车去造
	auto vecWp = Get空闲工程车(msg.类型, true);
	if (vecWp.empty())
	{
		if (孵化场 == msg.类型)
			播放声音Buzz("没找到空闲的工蜂");
		else
			播放声音Buzz("没找到空闲的工程车");

		return;
	}

	std::sort(vecWp.begin(), vecWp.end(), [&msg](WpEntity& wp左, WpEntity& wp右) {
		CHECK_WP_RET_FALSE(wp左);
		CHECK_WP_RET_FALSE(wp右);
		auto& ref左 = *wp左.lock();
		auto& ref右 = *wp右.lock();
		const auto b左_在采集 = 采集Component::正在采集(ref左);
		const auto b右_在采集 = 采集Component::正在采集(ref右);
		if (b左_在采集 && !b右_在采集)
			return false;

		if (!b左_在采集 && b右_在采集)
			return true;

		return msg.pos.DistancePow2(ref左.Pos()) < msg.pos.DistancePow2(ref右.Pos());
		});

	CHECK_WP_RET_VOID(*vecWp.begin());
	auto& wp第一个 = *vecWp.begin();
	auto& ref第一个 = *wp第一个.lock();
	走Component::Cancel所有包含走路的协程(ref第一个);
	ref第一个.m_sp造建筑->Co造建筑(msg.pos, msg.类型).RunNew();

}

std::vector<WpEntity> PlayerGateSession_Game::Get空闲工程车(const 单位类型 造活动单位类型, bool b包括采集中的工程车)
{
	CHECK_WP_RET_DEFAULT(m_wpSpace);
	Space& refSpace = *m_wpSpace.lock();
	std::vector<WpEntity> vecWp;
	for (auto [_, wp] : refSpace.m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntity = *wp.lock();
		if (!能造(refEntity, 造活动单位类型))
			continue;

		if (!b包括采集中的工程车 && 采集Component::正在采集(refEntity))
			continue;

		vecWp.push_back(wp);
	}
	return vecWp;
}

WpEntity PlayerGateSession_Game::EnterSpace(WpSpace wpSpace)
{
	CHECK_RET_DEFAULT(m_wpSpace.expired());
	CHECK_WP_RET_DEFAULT(wpSpace);
	m_wpSpace = wpSpace;
	auto& refSpace = *m_wpSpace.lock();

	Send<Msg进Space>({ .idSapce = 1 });
	{
		auto& sencePlayer = refSpace.m_mapPlayer[NickName()];
		for (auto [id, wp] : sencePlayer.m_mapWpEntity)
		{
			if (wp.expired())
				continue;

			auto sp = wp.lock();
			PlayerComponent::AddComponent(*sp, *this);
			//refSpace.m_mapPlayer[NickName()].m_mapWpEntity.insert({ sp->Id ,sp });
		}
		//mapOld.clear();


	}
	for (const auto& [id, spEntity] : refSpace.m_mapEntity)//所有地图上的实体发给自己
	{
		LOG(INFO) << spEntity->头顶Name() << ",发给单人," << spEntity->Id;
		Send(MsgAddRoleRet(*spEntity));
		Send(MsgNotifyPos(*spEntity));
		if (spEntity->m_spBuilding && !spEntity->m_spBuilding->已造好())
			Send<MsgEntity描述>({ .idEntity = spEntity->Id, .str描述 = StrConv::GbkToUtf8(std::format("建造进度{0}%",spEntity->m_spBuilding->m_n建造进度百分比)) });
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置&>(
		{ 0.0 }, refSpace, 视口, { "视口","smoke", "" });
	refSpace.m_mapPlayer[NickName()].m_mapWpEntity[spEntityViewPort->Id] = (spEntityViewPort);
	PlayerComponent::AddComponent(*spEntityViewPort, *this);
	{
		const auto [k, ok] = refSpace.m_map视口.insert({ spEntityViewPort->Id ,spEntityViewPort });
		CHECK_RET_DEFAULT(ok);
	}
	refSpace.AddEntity(spEntityViewPort, 500);
	spEntityViewPort->BroadcastEnter();
	m_wp视口 = spEntityViewPort;
	CoEvent<MyEvent::玩家进入Space>::OnRecvEvent({ this->weak_from_this(), spEntityViewPort, wpSpace });
	spEntityViewPort->Broadcast<MsgSay>({ .content = StrConv::GbkToUtf8(NickName() + " 进来了") });

	auto& playerSpace = refSpace.GetSpacePlayer(NickName());
	if (!playerSpace.m_msg上次发给前端的剧情对话.str对话内容.empty())
	{
		Send(playerSpace.m_msg上次发给前端的剧情对话);
		playerSpace.m_msg上次发给前端的剧情对话.str对话内容.clear();
	}
	{
		auto strHttps = refSpace.m_配置.strHttps音乐;
		Try读Ini本地机器专用(strHttps, "PlayerGateSession_Game", refSpace.m_配置.strSceneName + "_音乐");
		Send<Msg播放网络音乐>({ .strHttpsMp3 = strHttps });
	}
	return spEntityViewPort;
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{
	MsgSay msg加名字 = msg;
	auto strGbk = StrConv::Utf8ToGbk(msg加名字.content);
	LOG(INFO) << "收到聊天:" << strGbk << ",channel:" << msg.channel;
	msg加名字.content = StrConv::GbkToUtf8(NickName() + " 说:\n" + strGbk);
	msg加名字.channel = 聊天;
	SendToWorldSvr<MsgSay>(msg加名字, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	if (m_wpSpace.expired()) {
		LOG(ERROR) << "m_wpSpace";
		return;
	}
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
		_ASSERT(false);
		return;
	}
}

PlayerGateSession_Game::PlayerGateSession_Game(GameSvrSession& ref, uint64_t idPlayerGateSession, const std::string& strNickName) :
	m_refGameSvrSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_strNickName(strNickName)
{

}

void PlayerGateSession_Game::RecvMsg(const MsgId idMsg, const msgpack::object& obj)
{
	switch (idMsg)
	{
	case MsgId::进Space:RecvMsg<Msg进Space>(obj); break;
	case MsgId::离开Space:RecvMsg<Msg离开Space>(obj); break;
	case MsgId::进单人剧情副本:RecvMsg<Msg进单人剧情副本>(obj); break;
	case MsgId::创建多人战局:RecvMsg<Msg创建多人战局>(obj); break;
	case MsgId::Move:RecvMsg<MsgMove>(obj); break;
	case MsgId::Say:RecvMsg<MsgSay >(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::采集:RecvMsg<Msg采集>(obj); break;
	case MsgId::进地堡:RecvMsg<Msg进地堡>(obj); break;
	case MsgId::出地堡:RecvMsg<Msg出地堡>(obj); break;
	case MsgId::框选:RecvMsg<Msg框选>(obj); break;
	case MsgId::玩家个人战局列表:RecvMsg<Msg玩家个人战局列表>(obj); break;
	case MsgId::进其他玩家个人战局:RecvMsg<Msg进其他玩家个人战局>(obj); break;
	case MsgId::玩家多人战局列表:RecvMsg<Msg玩家多人战局列表>(obj); break;
	case MsgId::进其他玩家多人战局:RecvMsg<Msg进其他玩家多人战局>(obj); break;
	case MsgId::切换空闲工程车:RecvMsg<Msg切换空闲工程车>(obj); break;
	case MsgId::剧情对话已看完:RecvMsg<Msg剧情对话已看完>(obj); break;
	case MsgId::建筑产出活动单位的集结点:RecvMsg<Msg建筑产出活动单位的集结点>(obj); break;
	case MsgId::Gate转发:
		LOG(ERROR) << "不能再转发";
		_ASSERT(false);
		break;
	default:
		LOG(ERROR) << "没处理msgId:" << idMsg;
		_ASSERT(false);
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

	//if (m_spSpace单人剧情副本)
		//m_spSpace单人剧情副本->Update();

	//if (m_spSpace多人战局)
		//m_spSpace多人战局->Update();
}


void PlayerGateSession_Game::Send资源()
{
	CHECK_WP_RET_VOID(m_wpSpace);
	auto& ref = m_wpSpace.lock()->m_mapPlayer[NickName()];
	Send<Msg资源>({ .晶体矿 = ref.m_u32晶体矿,
					.燃气矿 = ref.m_u32燃气矿,
					.活动单位 = 活动单位包括制造队列中的(),
					.活动单位上限 = 活动单位上限() });
}

uint16_t PlayerGateSession_Game::活动单位上限() const
{
	if (m_wpSpace.expired())
		return 0;

	uint16_t result = 0;
	for (const auto& [_, wp] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		const auto& refEntity = *wp.lock();
		if (!refEntity.m_spBuilding)continue;
		if (!refEntity.m_spBuilding->已造好())continue;

		switch (refEntity.m_类型)
		{
		case 民房:result += 8; break;
		case 基地:result += 6; break;
		default:break;
		}
	}
	return result;
}

uint16_t PlayerGateSession_Game::活动单位包括制造队列中的() const
{
	if (m_wpSpace.expired())
	{
		return 0;
	}

	uint16_t 制造队列中的单位 = 0;
	for (const auto& [_, wp] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
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


void PlayerGateSession_Game::OnRecv(const Msg框选& msg)
{
	if (m_wpSpace.expired())
		return;

	const Position pos左上(std::min(msg.pos起始.x, msg.pos结束.x), std::min(msg.pos起始.z, msg.pos结束.z));
	const Position pos右下(std::max(msg.pos起始.x, msg.pos结束.x), std::max(msg.pos起始.z, msg.pos结束.z));
	const Rect rect = { pos左上,pos右下 };
	std::vector<uint64_t> vec;
	for (const auto [k, wpEntity] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
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
	m_vecSelectedEntity.clear();
	bool b已发送选中音效(false);

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

		if (spEntity->m_spBuilding && vecId.size() > 1)
			continue;//建筑单位目前只能单选

		if (EntitySystem::Is视口(*spEntity))
			continue;

		if (!spEntity->m_spPlayer)
			continue;

		if (&spEntity->m_spPlayer->m_refSession != this)//不是自己的单位
		{
			if (vecId.size() == 1)//单选一个敌方单位，就是走过去打
			{
				MsgMove msg = { .pos = spEntity->Pos(),.b遇到敌人自动攻击 = true };
				OnRecv(msg);
				return;
			}
			continue;
		}

		m_vecSelectedEntity.push_back(spEntity->Id);
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
		播放声音(refEntity.m_配置.str选中音效, "");
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
		播放声音(refEntity.m_配置.str选中音效, "");
	}
	else if (refEntity.m_sp资源) {
		switch (refEntity.m_sp资源->m_类型)
		{
		case 晶体矿:
		case 燃气矿:
		default:
			播放声音("音效/BUTTON", "");
			break;
		}
	}
}

void PlayerGateSession_Game::Send选中单位Responce()
{
	MsgSelectRoles msgResponse;
	msgResponse.ids.insert(msgResponse.ids.end(), m_vecSelectedEntity.begin(), m_vecSelectedEntity.end());
	Send(msgResponse);
}

void PlayerGateSession_Game::OnRecv(const Msg玩家个人战局列表& msg)
{
	Msg玩家个人战局列表Responce msgResponce;
	//for (const auto [id, sp] : m_refGameSvrSession.m_mapPlayerGateSession)
	for (auto [strNickName, sp] : Space::个人战局())
	{
		//if (!sp->m_spSpace单人剧情副本)
		//	continue;

		if (sp->m_配置.strSceneName == "scene四方对战")
			continue;

		msgResponce.vec个人战局中的玩家.push_back(
			{
				StrConv::GbkToUtf8(strNickName),
				StrConv::GbkToUtf8(sp->m_配置.strSceneName)
			});
	}
	Send(msgResponce);
}
void PlayerGateSession_Game::OnRecv(const Msg玩家多人战局列表& msg)
{
	Msg玩家多人战局列表Responce msgResponce;
	//for (const auto [id, sp] : m_refGameSvrSession.m_mapPlayerGateSession)
	for (auto [strNickName, sp] : Space::个人战局())
	{
		//if (!sp->m_spSpace多人战局)
			//continue;
		if (sp->m_配置.strSceneName != "scene四方对战")
			continue;

		msgResponce.vec多人战局中的Host玩家.push_back(
			{
				StrConv::GbkToUtf8(strNickName),
				StrConv::GbkToUtf8(sp->m_配置.strSceneName)//sp->m_spSpace多人战局->m_配置.strSceneName)
			});
	}
	Send(msgResponce);
}

void PlayerGateSession_Game::OnRecv(const Msg进其他玩家个人战局& msg)
{
	const auto strGbk = StrConv::Utf8ToGbk(msg.nickName其他玩家);
	//auto iterFind = std::find_if(m_refGameSvrSession.m_mapPlayerGateSession.begin(), m_refGameSvrSession.m_mapPlayerGateSession.end(),
	//	[&strGbk](const auto& pair)->bool
	//	{
	//		return pair.second->NickName() == strGbk;
	//	});
	auto wpSpace = Space::GetSpace单人(strGbk);
	CHECK_WP_RET_VOID(wpSpace);
	EnterSpace(wpSpace);
}

void PlayerGateSession_Game::OnRecv(const Msg进其他玩家多人战局& msg)
{
	const auto strGbk = StrConv::Utf8ToGbk(msg.nickName其他玩家);
	//auto iterFind = std::find_if(m_refGameSvrSession.m_mapPlayerGateSession.begin(), m_refGameSvrSession.m_mapPlayerGateSession.end(),
	//	[&strGbk](const auto& pair)->bool
	//	{
	//		return pair.second->NickName() == strGbk;
	//	});
	//CHECK_RET_VOID(iterFind != m_refGameSvrSession.m_mapPlayerGateSession.end());
	//auto& refSp = iterFind->second->m_spSpace多人战局;
	auto wpSpace = Space::GetSpace单人(strGbk);
	CHECK_WP_RET_VOID(wpSpace);
	EnterSpace(wpSpace);
}

void PlayerGateSession_Game::OnRecv(const Msg切换空闲工程车& msg)
{
	auto vecWp = Get空闲工程车(单位类型_Invalid_0, false);
	if (vecWp.empty())
	{
		播放声音Buzz("没有空闲的工程车");
		return;
	}
	m_idx切换工程车 = m_idx切换工程车 % vecWp.size();
	auto& wp = vecWp[m_idx切换工程车];
	CHECK_WP_RET_VOID(wp);
	Entity& refEntity = *wp.lock();
	选中单位({ refEntity.Id });
	Send设置视口(refEntity);
	++m_idx切换工程车;
}

void PlayerGateSession_Game::OnRecv(const Msg剧情对话已看完& msg)
{
	CoEvent<MyEvent::已阅读剧情对话>::OnRecvEvent({ .wpPlayerGateSession = weak_from_this() });
}
void PlayerGateSession_Game::剧情对话已看完()
{
	Send<Msg剧情对话已看完>({});
}

void PlayerGateSession_Game::OnRecv(const Msg建筑产出活动单位的集结点& msg)
{
	CHECK_WP_RET_VOID(m_wpSpace);
	auto& refSpace = *m_wpSpace.lock();
	auto& refSpacePlayer = refSpace.GetSpacePlayer(NickName());

	CHECK_RET_VOID(!m_vecSelectedEntity.empty());
	auto iterFind = refSpacePlayer.m_mapWpEntity.find(*m_vecSelectedEntity.begin());
	CHECK_RET_VOID(refSpacePlayer.m_mapWpEntity.end() != iterFind);

	auto& wp我方建筑 = iterFind->second;
	CHECK_WP_RET_VOID(wp我方建筑);

	auto& refEntity我方建筑 = *wp我方建筑.lock();
	CHECK_RET_VOID(EntitySystem::Is建筑(refEntity我方建筑.m_类型));
	CHECK_RET_VOID(refEntity我方建筑.m_sp造活动单位);

	refEntity我方建筑.m_sp造活动单位->m_pos集结点 = msg.pos;
	Say系统("修改已生效。此建筑新产出的活动单位将会自动走向此集结点。");
}