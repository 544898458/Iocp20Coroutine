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
#include "单位组件/可进活动单位Component.h"
#include "单位组件/走Component.h"
#include "单位组件/PlayerComponent.h"
#include "单位组件/资源Component.h"
#include "单位组件/AoiComponent.h"
#include "单位组件/虫巢Component.h"
#include "单位组件/解锁单位Component.h"
#include "单位组件/升级单位属性Component.h"
#include "单位组件/找目标走过去Component.h"
#include "单位组件/太岁Component.h"
#include "单位组件/跟随Component.h"
#include "../CoRoutine/CoTimer.h"
#include "EntitySystem.h"
#include "单位组件/PlayerNickNameComponent.h"
#include "MyEvent.h"
#include "../读配置文件/Try读Ini本地机器专用.h"
#include "枚举/属性类型.h"
#include "混战.h"

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
template void PlayerGateSession_Game::Send(const Msg苔蔓半径&);
template void PlayerGateSession_Game::Send(const MsgNotify属性&);


void PlayerGateSession_Game::OnDestroy()
{
	离开Space(false);

	LOG(INFO) << "m_refGameSvrSession.m_mapPlayerGateSession.size:" << m_refGameSvrSession.m_mapPlayerGateSession.size();
	if (1 >= m_refGameSvrSession.m_mapPlayerGateSession.size())
	{
		LOG(INFO) << "没有玩家了，保存Space全局混战数据(卡一下也没关系，反正没人了）";
		Space::StaticSave();
	}
}

void PlayerGateSession_Game::离开Space(const bool b主动退)
{
	if (!m_wpSpace.expired())
	{
		auto wp我的单人 = Space::GetSpace单人(NickName());
		m_wpSpace.lock()->m_mapPlayer[NickName()].OnDestroy(b主动退 && !wp我的单人.expired(), *m_wpSpace.lock(), NickName());
		//if (!b主动退)
		m_wpSpace.lock()->没人就休眠(NickName());
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

	m_funCancel进地图.TryCancel();

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
		if (!ref.m_up造活动单位)return;
		if (!ref.m_up造活动单位->可造(msg.类型))return;
		ref.m_up造活动单位->造(*this, msg.类型);
		bOK = true;
	});

	if (bOK)
		return;

	//自动找兵营去造
	CHECK_WP_RET_VOID(m_wpSpace);
	Space& refSpace = *m_wpSpace.lock();
	std::vector<WpEntity> vecWp可造;
	for (auto [_, wp] : refSpace.m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntiy = *wp.lock();
		if (refEntiy.m_upBuilding && !refEntiy.m_upBuilding->已造好())continue;
		if (!refEntiy.m_up造活动单位)continue;
		if (!refEntiy.m_up造活动单位->可造(msg.类型))continue;
		vecWp可造.push_back(wp);
	}

	//待造队列最短的在前面
	std::sort(vecWp可造.begin(), vecWp可造.end(), [](const WpEntity& wp左, const WpEntity& wp右)->bool
	{
		CHECK_WP_RET_FALSE(wp左);
		CHECK_WP_RET_FALSE(wp右);
		CHECK_NOTNULL_RET_FALSE(wp左.lock()->m_up造活动单位);
		CHECK_NOTNULL_RET_FALSE(wp右.lock()->m_up造活动单位);
		return wp左.lock()->m_up造活动单位->等待造Count() < wp右.lock()->m_up造活动单位->等待造Count();
	});
	for (auto wp : vecWp可造)
	{
		CHECK_WP_CONTINUE(wp);
		Entity& refEntiy = *wp.lock();
		refEntiy.m_up造活动单位->造(*this, msg.类型);
		return;
	}

	if(单位::Is虫(msg.类型))
	{
		播放声音Buzz("缺少 " "幼虫");
	}
	else
	{
		单位::制造配置 制造;
		CHECK_RET_VOID(单位::Find制造配置(msg.类型, 制造));
		单位::单位配置 前置单位配置;
		CHECK_RET_VOID(单位::Find单位配置(制造.前置单位, 前置单位配置));
		播放声音Buzz("缺少 " + 前置单位配置.strName);
	}
}

void PlayerGateSession_Game::OnRecv(const Msg采集& msg)
{
	ForEachSelected([this, &msg](Entity& ref)
	{
		CHECK_RET_VOID(!m_wpSpace.expired());
		auto wpEntity = m_wpSpace.lock()->GetEntity((int64_t)msg.id目标资源);
		CHECK_WP_RET_VOID(wpEntity);
		auto& ref资源 = *wpEntity.lock();
		if (基地 == ref.m_类型)
		{
			CHECK_RET_VOID(ref.m_up造活动单位);
			ref.m_up造活动单位->m_mapPos集结点[工程车] = ref资源.Pos();
			Say系统("已设定新工程车采集资源");
			return;
		}
		else if (虫巢 == ref.m_类型)
		{
			CHECK_RET_VOID(ref.m_up虫巢);
			ref.m_up虫巢->Set集结点(ref资源.Pos(), 单位类型::工虫);
			Say系统("已设定新工虫采集资源");
			return;
		}

		if (!ref.m_up采集)
		{
			播放声音Buzz("此单位无法采集资源");
			return;
		}

		ref.m_up采集->采集(wpEntity, true);
	});
}

void PlayerGateSession_Game::OnRecv(const Msg出地堡& msg)
{
	CHECK_RET_VOID(!m_wpSpace.expired());
	auto wpTarget = m_wpSpace.lock()->GetEntity((int64_t)msg.id地堡);
	//msg.vecId地堡内单位.size();
	CHECK_RET_VOID(!wpTarget.expired());
	auto spTarget = wpTarget.lock();
	if (!spTarget->m_up可进活动单位)
	{
		Say系统("目标不是地堡");
		return;
	}
	spTarget->m_up可进活动单位->全都出去();
}

void PlayerGateSession_Game::OnRecv(const Msg进地堡& msg)
{
	OnRecv进((uint64_t)msg.id目标地堡);
}

void PlayerGateSession_Game::OnRecv进(const uint64_t id单位容器)
{
	CHECK_RET_VOID(!m_wpSpace.expired());
	auto wpTarget = m_wpSpace.lock()->GetEntity(id单位容器);
	CHECK_RET_VOID(!wpTarget.expired());
	auto& refTarget地堡或房虫 = *wpTarget.lock();
	if (!refTarget地堡或房虫.m_up可进活动单位)
	{
		播放声音Buzz("不可进入");
		return;
	}

	if (EntitySystem::GetNickName(refTarget地堡或房虫) != NickName())
	{
		//播放声音Buzz("不能进别人的地堡");
		MsgMove msg = { .pos = refTarget地堡或房虫.Pos(),.b遇到敌人自动攻击 = true };
		OnRecv(msg);

		return;
	}

	std::list<std::function<void()>> listFun;
	ForEachSelected([this, &listFun, &wpTarget, &refTarget地堡或房虫](Entity& ref)
	{
		if (!EntitySystem::Is可进地堡或房虫(ref.m_类型))
		{
			//Say系统("此单位不可进入地堡");
			if (地堡 == refTarget地堡或房虫.m_类型)
				播放声音Buzz("此单位不可进入地堡");
			else
				播放声音Buzz("此单位不可进入房虫");
			return;
		}

		listFun.emplace_back([&ref, wpTarget, this]()
		{
			if (ref.m_up走)
				ref.m_up走->走进(wpTarget);
		});
	});

	for (auto& fun : listFun)
	{
		fun();
	}
}


void PlayerGateSession_Game::OnRecv(const Msg进房虫& msg)
{
	OnRecv进((uint64_t)msg.id目标房虫);
}

void PlayerGateSession_Game::OnRecv(const Msg进Space& msg)
{
	离开Space(true);
	LOG(INFO) << "希望进Space:" << msg.战局;
	auto wp = Space::GetSpace(msg.战局);
	CHECK_WP_RET_VOID(wp);
	EnterSpace(wp);

	m_funCancel进地图.TryCancel();
	CHECK_WP_RET_VOID(m_wpSpace);
	CHECK_WP_RET_VOID(m_wp视口);
    auto &refSpace = *m_wpSpace.lock();
            
	混战::Co进多人联机混战(refSpace, *m_wp视口.lock(), msg.战局 == 多玩家混战 ? 90 : 130, msg.b已看完激励视频广告, m_funCancel进地图).RunNew();
}

void PlayerGateSession_Game::OnRecv(const Msg离开Space& msg)
{
	LOG(INFO) << "希望离开Space:";// << msg.idSapce;
	离开Space(true);
}

单位::战局配置::funCo副本剧情 Get战局Co(const 战局类型 类型)
{
	switch (类型)
	{
	case 新手训练_单位介绍_人:
		return 单人剧情::Co新手训练_单位介绍_人;
	case 新手训练_单位介绍_虫:
		return 单人剧情::Co新手训练_单位介绍_虫;
	case 新手训练_战斗_人:
		return 单人剧情::Co新手训练_战斗_人;
	case 新手训练_战斗_虫:
		return 单人剧情::Co新手训练_战斗_虫;
	case 新手训练_反空降战_人:
		return 单人剧情::Co新手训练_反空降战_人;
	case 新手训练_空降战_虫:
		return 单人剧情::Co新手训练_空降战_虫;
	case 防守战_人:
		return 单人剧情::Co防守战;
	case 攻坚战_人:
		return 单人剧情::Co攻坚战;
	case 攻坚战_虫:
		return 单人剧情::Co攻坚战_虫;
	case 防守战_虫:
		return 单人剧情::Co防守战_虫;
	case 中央防守_人:
		return 单人剧情::Co中央防守_人;
	case 四方对战:
		return 多人战局::Co四方对战;
	case 一打一:
		return 多人战局::Co一打一;
	default:
		LOG(ERROR) << "未知战局类型:" << 类型;
		_ASSERT(false);
		return {};
	}
}

void Run战局Co(const 战局类型 类型, Space& refSpace, FunCancel& funCancel, const std::string strNickName)
{
	auto fun = Get战局Co(类型);
	CHECK_RET_VOID(fun);
	fun(refSpace, funCancel, strNickName).RunNew();
}


void PlayerGateSession_Game::OnRecv(const Msg进单人剧情副本& msg)
{
	单位::战局配置 配置;
	{
		const auto ok = 单位::Find战局配置(msg.类型, 配置);
		CHECK_RET_VOID(ok);
		CHECK_RET_VOID(!配置.Is多人战局());
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
		Run战局Co(msg.类型, *wpSpace.lock(), wpSpace.lock()->m_funCancel剧情, NickName());
}

void PlayerGateSession_Game::OnRecv(const Msg创建多人战局& msg)
{
	单位::战局配置 配置;
	{
		const auto ok = 单位::Find战局配置(msg.类型, 配置);
		CHECK_RET_VOID(ok);

		CHECK_RET_VOID(配置.Is多人战局());
	}

	//m_spSpace多人战局 = std::make_shared<Space, const 副本配置&>(配置);
	auto wpOld = Space::GetSpace单人(NickName());
	if (!wpOld.expired() && wpOld.lock()->m_配置.strSceneName != 配置.strSceneName)
	{
		Space::DeleteSpace单人(NickName());
	}
	const auto [b新, wpSpace] = Space::GetSpace单人(NickName(), 配置);
	//m_wpSpace单人剧情副本 = wpSpace;
	CHECK_WP_RET_VOID(wpSpace);
	m_wp视口 = EnterSpace(wpSpace);
	CHECK_WP_RET_VOID(m_wp视口);

	if (b新)
		Run战局Co(msg.类型, *wpSpace.lock(), wpSpace.lock()->m_funCancel剧情, NickName());
}

void PlayerGateSession_Game::OnRecv(const MsgMove& msg)
{
	//LOG(INFO) << "收到点击坐标:" << msg.pos;
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
		if (!ref.m_up走)
			return;

		if (造建筑Component::正在建造(ref))
		{
			PlayerComponent::播放声音Buzz(ref, "正在建造，不能移动");
			return;
		}

		vecWp.push_back(ref.weak_from_this());

		pos中心点 += ref.Pos();
	});

	if (vecWp.empty())
	{
		//LOG(WARNING) << "vecWp";
		return;
	}
	pos中心点.x /= vecWp.size();
	pos中心点.z /= vecWp.size();
	for (auto& wp : vecWp)
	{
		auto& ref = *wp.lock();
		auto pos偏离 = ref.Pos() - pos中心点;

		if (ref.m_up找目标走过去)
		{
			const auto f距离中心点Max = b保持队形 ? ref.m_up找目标走过去->m_战斗配置.f警戒距离 : ref.m_up找目标走过去->m_战斗配置.f攻击距离 / 5;

			if (std::abs(pos偏离.x) > f距离中心点Max)
				pos偏离.x = pos偏离.x / std::abs(pos偏离.x) * f距离中心点Max;

			if (std::abs(pos偏离.z) > f距离中心点Max)
				pos偏离.z = pos偏离.z / std::abs(pos偏离.z) * f距离中心点Max;
		}

		auto pos目标 = msg.pos + pos偏离;

		if (!refSpace.CrowdTool可站立(pos目标))
		{
			//LOG(INFO) << pos目标 << "不可站立，找附近的可站立点";
			if (!refSpace.CrowdToolFindNerestPos(pos目标))
			{
				//LOG(WARNING) << pos目标 << "附近没有可站立的点";
				continue;
			}
			//LOG(INFO) << "找到附近的点:" << pos目标;
		}

		std::shared_ptr<RecastNavigationCrowd> sp;
		if (!ref.m_up走->m_wpRecastNavigationCrowd.expired())
			sp = ref.m_up走->m_wpRecastNavigationCrowd.lock();

		走Component::Cancel所有包含走路的协程(ref, true);
		if (ref.m_upDefence)
			ref.m_upDefence->m_map对我伤害.clear();

		if (ref.m_up找目标走过去)
		{
			ref.m_up找目标走过去->m_fun空闲走向此处 = [pos目标](const Position&)->Position {return pos目标; };
			ref.m_up找目标走过去->m_b原地坚守 = false;
		}
		if (msg.b遇到敌人自动攻击)
			ref.m_up走->WalkToPos(pos目标);
		else
			ref.m_up走->WalkToPos手动控制(pos目标);

		if (b已播放声音)
			continue;

		单位::活动单位配置 配置;
		if (单位::Find活动单位配置(ref.m_类型, 配置))
		{

			const std::string str显示文字 = msg.b遇到敌人自动攻击 ? "" : "强行走向目标位置";
			const std::string str声音 = msg.b遇到敌人自动攻击 ? 配置.str普通走语音 : 配置.str强行走语音;
			//switch (ref.m_类型)
			//{
			//case 枪兵:播放声音(msg.b遇到敌人自动攻击 ? "语音/是男声正经版" : "语音/明白男声正经版", str显示文字); break;//Standing by. 待命中
			//case 近战兵:播放声音("tfbYes03", str显示文字); break;//Checked up and good to go. 检查完毕，准备动身
			//case 工程车:播放声音(msg.b遇到敌人自动攻击 ? "语音/是女声可爱版" : "语音/明白女声可爱版", str显示文字); break;
			//case 三色坦克:播放声音(msg.b遇到敌人自动攻击 ? "音效/坦克行进声" : "语音/坦克明白", str显示文字); break;
			//case 飞机:播放声音(msg.b遇到敌人自动攻击 ? "音效/飞机声" : "语音/飞机明白", str显示文字); break;
			//default:
			//	continue;
			//	break;
			//}
			if (!str声音.empty())
			{
				播放声音(str声音, str显示文字);
				continue;
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

void PlayerGateSession_Game::播放声音Buzz(const std::string& refStrNickName, const std::string& str文本)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->播放声音Buzz(str文本);
}

void PlayerGateSession_Game::播放声音Buzz(const Entity& refEntity, const std::string& str文本)
{
	auto wp = GetPlayerGateSession(EntitySystem::GetNickName(refEntity));
	if (!wp.expired())
		wp.lock()->播放声音Buzz(str文本);
}

void PlayerGateSession_Game::播放声音(const std::string& refStr声音, const std::string& str文本)
{
	if (refStr声音.empty())
		return;

	auto& refTime = g_map上次播放音效[refStr声音];
	using namespace std;
	const auto now = chrono::steady_clock::now();
	if (now < refTime)
	{
		//LOG(INFO) << "播放同一个音效太频繁,跳过," << refStr声音;
		return;
	}
	refTime = now + 1s;
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
	CHECK_RET_VOID(!m_wpSpace.expired());
	auto sp = m_wpSpace.lock();
	auto temp = m_vecSelectedEntity;//防止循环中修改容器
	for (const auto id : temp)
	{
		auto itFind = sp->m_mapEntity.find(id);
		if (itFind == sp->m_mapEntity.end())
		{
			//LOG(INFO) << "选中的实体不存在:" << id;
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

template<class T> void SendToWorldSvr转发(const T& msg, const uint64_t idGateSession);


bool 能造(const Entity& refEntiy, const 单位类型 造活动单位类型)
{
	if (refEntiy.m_类型 != 工程车 && refEntiy.m_类型 != 工虫) return false;
	if (造建筑Component::正在建造(refEntiy))return false;

	CHECK_FALSE(refEntiy.m_upAttack);
	if (refEntiy.m_upAttack->m_cancelAttack)return false;
	//if (refEntiy.m_upAttack->m_TaskCancel.cancel)return false;

	CHECK_FALSE(refEntiy.m_up造建筑);
	if (造活动单位类型 > 单位类型_Invalid_0 && !refEntiy.m_up造建筑->可造(造活动单位类型))return false;

	return true;
}
void PlayerGateSession_Game::OnRecv(const MsgAddBuilding& msg)
{
	//CoAddBuilding(msg.类型, msg.pos).RunNew();
	bool b操作OK(false);
	ForEachSelected([this, msg, &b操作OK](Entity& refEntiy)
	{
		if (b操作OK)//只要造一个，因为只有一个pos
			return;

		if (!能造(refEntiy, msg.类型))
			return;

		走Component::Cancel所有包含走路的协程(refEntiy, true); //TryCancel();
		refEntiy.m_up造建筑->造建筑(msg.pos, msg.类型);
		b操作OK = true;
	});

	if (b操作OK)
		return;

	//自动找一个合适的工程车去造
	auto vecWp = Get空闲工程车(msg.类型, true);
	if (vecWp.empty())
	{
		switch (msg.类型)
		{
		case 虫巢:
		case 虫营:
		case 飞塔:
		case 拟态源:
		case 太岁:
			播放声音Buzz("没找到空闲的工虫");
			break;
		default:
			播放声音Buzz("没找到空闲的工程车");
			break;
		}

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
	走Component::Cancel所有包含走路的协程(ref第一个, true);
	ref第一个.m_up造建筑->造建筑(msg.pos, msg.类型);

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
void PlayerGateSession_Game::Send已解锁单位()
{
	CHECK_WP_RET_VOID(m_wpSpace);
	auto& refSpace = *m_wpSpace.lock();
	auto& spacePlayer = refSpace.m_mapPlayer[NickName()];
	Msg已解锁单位 msg已解锁;
	for (const auto& [类型, 状态] : spacePlayer.m_map单位解锁)
	{
		msg已解锁.map解锁状态.insert({ 类型, 状态.b已解锁 });
	}

	Send(msg已解锁);
}

void PlayerGateSession_Game::Send单位属性()
{
	CHECK_WP_RET_VOID(m_wpSpace);
	auto& refSpace = *m_wpSpace.lock();
	auto& spacePlayer = refSpace.m_mapPlayer[NickName()];
	Msg单位属性等级 msg属性等级;
	for (const auto& [单位, map属性] : spacePlayer.m_map单位属性等级)
	{
		for (const auto& [属性, 等级] : map属性)
		{
			msg属性等级.map单位属性等级[单位].insert({ 属性 , 等级.u16等级 });
		}
	}

	Send(msg属性等级);

}

WpEntity PlayerGateSession_Game::EnterSpace(WpSpace wpSpace)
{
	CHECK_RET_DEFAULT(m_wpSpace.expired());
	CHECK_WP_RET_DEFAULT(wpSpace);
	m_wpSpace = wpSpace;
	auto& refSpace = *m_wpSpace.lock();

	Send<Msg进Space>({ .战局 = refSpace.m_配置.战局 });
	Send已解锁单位();
	Send单位属性();
	{
		auto& spacePlayer = refSpace.m_mapPlayer[NickName()];
		for (auto [id, wp] : spacePlayer.m_mapWpEntity)
		{
			if (wp.expired())
				continue;

			auto sp = wp.lock();
			PlayerComponent::AddComponent(*sp, *this);
		}
	}


	for (const auto& [id, spEntity] : refSpace.m_mapEntity)//所有地图上的实体发给自己
	{
		auto& refEntity其它 = *spEntity;
		//LOG(INFO) << refEntity其它.头顶Name() << ",发给单人," << refEntity其它.Id;
		Send(MsgAddRoleRet(refEntity其它));
		Send(MsgNotifyPos(refEntity其它));
		Send(MsgNotify属性(refEntity其它, g_list所有属性));

		if (refEntity其它.m_up苔蔓)
			Send(Msg苔蔓半径(refEntity其它));

		if (refEntity其它.m_upBuilding && !refEntity其它.m_upBuilding->已造好())
			Send<MsgEntity描述>({ .idEntity = refEntity其它.Id, .str描述 = StrConv::GbkToUtf8(std::format("建造进度{0}",refEntity其它.m_upBuilding->m_n建造进度帧)) });
	}

	SpEntity spEntityViewPort = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置&>(
		{ 0.0 }, refSpace, 视口, { "视口", 无, "smoke", false, "" });
	auto [pair, ok] = refSpace.m_mapPlayer[NickName()].m_mapWpEntity.insert({ spEntityViewPort->Id, spEntityViewPort });
	CHECK_NOT_RETURN(ok);
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
	播放音乐(refSpace.m_配置.strHttps音乐, std::format("战局音乐_{0}", (int)refSpace.m_配置.战局));
	return spEntityViewPort;
}

void PlayerGateSession_Game::播放音乐(const std::string& strHttps默认, const std::string& strKey)
{
	auto strHttps = strHttps默认;
	Try读Ini本地机器专用(strHttps, "PlayerGateSession_Game", strKey);
	if (strHttps.empty())
		return;

	Send<Msg播放网络音乐>({ .strHttpsMp3 = StrConv::GbkToUtf8(strHttps) });
}

void PlayerGateSession_Game::播放音乐(const std::string& refStrNickName, const std::string& strHttps默认, const std::string& strKey)
{
	auto wp = GetPlayerGateSession(refStrNickName);
	if (!wp.expired())
		wp.lock()->播放音乐(strHttps默认, strKey);
}

void PlayerGateSession_Game::OnLoginOk()
{
	播放音乐("", "登录场景_音乐");
}

void PlayerGateSession_Game::OnRecv(const MsgSay& msg)
{

	const auto now = std::chrono::steady_clock::now();
	if (now - m_tp上次说话 < std::chrono::seconds(5))
	{
		Say("请稍后再说", 聊天);
		return;
	}

	m_tp上次说话 = now;

	MsgSay msg加名字 = msg;
	auto strGbk = StrConv::Utf8ToGbk(msg加名字.content);
	//LOG(INFO) << "收到聊天:" << strGbk << ",channel:" << msg.channel;
	msg加名字.content = StrConv::GbkToUtf8(NickName() + " 说:\n" + strGbk);
	msg加名字.channel = 聊天;
	SendToWorldSvr转发<MsgSay>(msg加名字, m_idPlayerGateSession);
}

void PlayerGateSession_Game::OnRecv(const MsgSelectRoles& msg)
{
	if (m_wpSpace.expired()) {
		LOG(ERROR) << "m_wpSpace";
		return;
	}
	LOG(INFO) << "收到选择:" << msg.ids.size();
	std::vector<uint64_t> listSelectedEntity;
	std::transform(msg.ids.begin(), msg.ids.end(), std::back_inserter(listSelectedEntity), [](const auto& id) {return uint64_t(id); });
	选中单位(listSelectedEntity, msg.b追加);
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
	m_refGameSvrSession(ref), m_idPlayerGateSession(idPlayerGateSession), m_strNickName(strNickName), m_funCancel进地图("m_funCancel进地图")
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
	case MsgId::Say:RecvMsg<MsgSay>(obj); break;
	case MsgId::SelectRoles:RecvMsg<MsgSelectRoles>(obj); break;
	case MsgId::AddRole:RecvMsg<MsgAddRole>(obj); break;
	case MsgId::AddBuilding:RecvMsg<MsgAddBuilding>(obj); break;
	case MsgId::采集:RecvMsg<Msg采集>(obj); break;
	case MsgId::进地堡:RecvMsg<Msg进地堡>(obj); break;
	case MsgId::出地堡:RecvMsg<Msg出地堡>(obj); break;
	case MsgId::进房虫:RecvMsg<Msg进房虫>(obj); break;
	case MsgId::框选:RecvMsg<Msg框选>(obj); break;
	case MsgId::玩家个人战局列表:RecvMsg<Msg玩家个人战局列表>(obj); break;
	case MsgId::进其他玩家个人战局:RecvMsg<Msg进其他玩家个人战局>(obj); break;
	case MsgId::玩家多人战局列表:RecvMsg<Msg玩家多人战局列表>(obj); break;
	case MsgId::进其他玩家多人战局:RecvMsg<Msg进其他玩家多人战局>(obj); break;
	case MsgId::切换空闲工程车:RecvMsg<Msg切换空闲工程车>(obj); break;
	case MsgId::剧情对话已看完:RecvMsg<Msg剧情对话已看完>(obj); break;
	case MsgId::建筑产出活动单位的集结点:RecvMsg<Msg建筑产出活动单位的集结点>(obj); break;
	case MsgId::原地坚守:RecvMsg<Msg原地坚守>(obj); break;
	case MsgId::解锁单位:RecvMsg<Msg解锁单位>(obj); break;
	case MsgId::升级单位属性:RecvMsg<Msg升级单位属性>(obj); break;
	case MsgId::太岁分裂:RecvMsg<Msg太岁分裂>(obj); break;
	case MsgId::跟随:RecvMsg<Msg跟随>(obj); break;
	case MsgId::取消跟随:RecvMsg<Msg取消跟随>(obj); break;
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
					.活动单位上限 = 活动单位上限(),
					.建筑单位 = 建筑单位(),
					.建筑单位上限 = 建筑单位上限 });
}

uint16_t PlayerGateSession_Game::活动单位上限(bool&& refBool已达单玩家活动单位上限) const
{
	refBool已达单玩家活动单位上限 = false;

	if (m_wpSpace.expired())
		return 0;

	uint16_t result = 0;
	for (const auto& [_, wp] : m_wpSpace.lock()->m_mapPlayer[NickName()].m_mapWpEntity)
	{
		CHECK_WP_CONTINUE(wp);
		const auto& refEntity = *wp.lock();
		if (refEntity.m_upBuilding && !refEntity.m_upBuilding->已造好())
			continue;

		switch (refEntity.m_类型)
		{
		case 民房:result += 8; break;
		case 房虫:result += 6; break;
		case 基地:result += 6; break;
		case 虫巢:result += 5; break;
		default:break;
		}
	}

	uint32_t u32单玩家活动单位上限 = 100;
	Try读Ini本地机器专用(u32单玩家活动单位上限, "PlayerGateSession_Game", "单玩家活动单位上限 ");
	if (u32单玩家活动单位上限 < result)
	{
		refBool已达单玩家活动单位上限 = true;
		result = u32单玩家活动单位上限;
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
		if (refEntity.m_up造活动单位)
		{
			制造队列中的单位 += (uint16_t)refEntity.m_up造活动单位->等待造Count();//m_i等待造兵数;

		}

		if (!EntitySystem::Is活动单位(refEntity))
			continue;

		if (refEntity.m_类型 == 幼虫 || refEntity.m_类型 == 房虫)
			continue;

		++制造队列中的单位;
	}

	return 制造队列中的单位;
}

uint16_t PlayerGateSession_Game::建筑单位() const
{
	if (m_wpSpace.expired())
	{
		LOG(ERROR) << "m_wpSpace";
		return 0;
	}

	return m_wpSpace.lock()->m_mapPlayer[NickName()].建筑单位数量();
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

void PlayerGateSession_Game::选中单位(std::vector<uint64_t> vecId, bool b追加)
{
	if (m_wpSpace.expired())
	{
		LOG(ERROR) << "没进场景，不能选中单位";
		return;
	}
	auto& refSpace = *m_wpSpace.lock();
	if (vecId.empty())
		m_id上次单选选中 = 0;

	//如果重复选择同一单位，就是把附近同类型的单位全部选中
	if (1 == vecId.size())
	{
		const auto id这次单选选中 = vecId[0];

		auto& refMap我的所有单位 = refSpace.GetSpacePlayer(NickName()).m_mapWpEntity;
		auto itefFind = refMap我的所有单位.find(id这次单选选中);
		if (refMap我的所有单位.end() == itefFind)
		{
			LOG(WARNING) << "选中的单位不属于我," << id这次单选选中;
			return;
		}

		CHECK_WP_RET_VOID(itefFind->second);
		auto& refEntity重复选中的1个单位 = *itefFind->second.lock();

		if (EntitySystem::Is建筑(refEntity重复选中的1个单位.m_类型))
		{
			b追加 = false;//单选建筑
		}

		if (id这次单选选中 == m_id上次单选选中)
		{
			CHECK_RET_VOID(refEntity重复选中的1个单位.m_upAoi);
			for (const auto [id, wp] : refEntity重复选中的1个单位.m_upAoi->m_map我能看到的)
			{
				if (id这次单选选中 == id)
					continue;

				CHECK_WP_CONTINUE(wp);
				auto& refEntity附近 = *wp.lock();
				if (EntitySystem::Is建筑(refEntity附近.m_类型)) {
					continue;
				}

				if (refEntity附近.m_类型 == refEntity重复选中的1个单位.m_类型)
					vecId.emplace_back(id);
			}
		}

		m_id上次单选选中 = id这次单选选中;
	}

	if (!b追加)
		m_vecSelectedEntity.clear();

	std::vector<uint64_t> vecId并集;//每次都是添加选择
	if (!vecId.empty())
	{
		std::sort(m_vecSelectedEntity.begin(), m_vecSelectedEntity.end());
		std::sort(vecId.begin(), vecId.end());
		std::set_union(m_vecSelectedEntity.begin(), m_vecSelectedEntity.end(), vecId.begin(), vecId.end(), std::back_inserter(vecId并集));
	}
	m_vecSelectedEntity.clear();
	bool b已发送选中音效(false);

	for (const auto id : vecId并集)
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

		if (spEntity->m_upBuilding && vecId并集.size() > 1)
			continue;//建筑单位目前只能单选

		if (EntitySystem::Is视口(*spEntity))
			continue;

		if (光刺 == spEntity->m_类型)
			continue;

		if (EntitySystem::GetNickName(*spEntity) != NickName())//不是自己的单位
		{
			if (vecId并集.size() == 1)//单选一个敌方单位，就是走过去打
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
	if (!refEntity.m_配置.str选中音效.empty())
	{
		播放声音(refEntity.m_配置.str选中音效, "");
	}
}

void PlayerGateSession_Game::Send选中单位Responce()
{
	MsgSelectRoles msgResponse;
	std::transform(m_vecSelectedEntity.begin(), m_vecSelectedEntity.end(), std::back_inserter(msgResponse.ids), [](const auto& id)->double { return (double)id; });
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

		if (sp->m_配置.Is多人战局())
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
		if (!sp->m_配置.Is多人战局())
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

	uint32_t u间隔秒(10);
	Try读Ini本地机器专用(u间隔秒, "PlayerGateSession_Game", "切换闲工间隔秒");
	if (std::chrono::steady_clock::now() - m_tp上次切换闲工 > std::chrono::seconds(u间隔秒))
	{
		m_idx切换工程车 = std::numeric_limits<uint16_t>::max();
	}

	m_tp上次切换闲工 = std::chrono::steady_clock::now();
	if (m_idx切换工程车 >= vecWp.size())//选中所有空闲工程车或工虫
	{
		m_idx切换工程车 = 0;
		std::vector<uint64_t> vecId;
		std::transform(vecWp.begin(), vecWp.end(), std::back_inserter(vecId), [](const WpEntity& wp) -> uint64_t
		{
			CHECK_WP_RET_DEFAULT(wp);
			return wp.lock()->Id;
		});
		CHECK_RET_VOID(!vecId.empty());

		m_id上次单选选中 = 0;//清除上次单选的单位，防止这次自动选中所有同类型单位
		选中单位(vecId);

		auto wp = *vecWp.begin();
		CHECK_WP_RET_VOID(wp);
		Send设置视口(*wp.lock());

	}
	else//选中工程车并且切换到下一个
	{
		m_idx切换工程车 = m_idx切换工程车 % vecWp.size();
		auto& wp = vecWp[m_idx切换工程车];
		CHECK_WP_RET_VOID(wp);
		Entity& refEntity = *wp.lock();
		选中单位({ refEntity.Id });
		Send设置视口(refEntity);

		++m_idx切换工程车;//下次切换到下一个
	}
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
	if (refEntity我方建筑.m_up虫巢)
	{
		refEntity我方建筑.m_up虫巢->Set集结点(msg.pos, msg.单位);
		Say系统("修改已生效。已设置所有幼虫蜕变后的单位会自动走向此集结点。");
		return;
	}

	CHECK_RET_VOID(refEntity我方建筑.m_up造活动单位);
	refEntity我方建筑.m_up造活动单位->m_mapPos集结点[msg.单位] = msg.pos;
	Say系统("修改已生效。此建筑新产出的活动单位将会自动走向此集结点。");
}

void PlayerGateSession_Game::OnRecv(const Msg原地坚守& msg)
{
	ForEachSelected([](Entity& ref)
	{
		if (!ref.m_up找目标走过去)
			return;

		走Component::Cancel所有包含走路的协程(ref, true);
		ref.m_up找目标走过去->m_b原地坚守 = true;
	});
}

void PlayerGateSession_Game::OnRecv(const Msg解锁单位& msg)
{
	ForEachSelected([&msg](Entity& ref)
	{
		if (ref.m_up解锁单位)
			ref.m_up解锁单位->解锁单位(msg.类型);
	});
}
void PlayerGateSession_Game::OnRecv(const Msg升级单位属性& msg)
{
	ForEachSelected([&msg](Entity& ref)
	{
		if (ref.m_up升级单位属性)
			ref.m_up升级单位属性->升级(msg.单位, msg.属性);
	});
}
void PlayerGateSession_Game::OnRecv(const Msg太岁分裂& msg)
{
	ForEachSelected([&msg](Entity& ref)
	{
		if (ref.m_up太岁)
			ref.m_up太岁->分裂(msg.pos放置);
	});
}

void PlayerGateSession_Game::OnRecv(const Msg跟随& msg)
{
	auto wpEntity目标 = m_wpSpace.lock()->GetEntity(msg.idEntity目标);
	CHECK_WP_RET_VOID(wpEntity目标);
	auto& refEntity目标 = *wpEntity目标.lock();
	ForEachSelected([this, &wpEntity目标](Entity& ref)
	{
		CHECK_RET_VOID(!m_wpSpace.expired());
		if (!ref.m_up跟随)
		{
			播放声音Buzz("此单位无法跟随");
			return;
		}
		ref.m_up跟随->跟随(wpEntity目标);
	});
}

void PlayerGateSession_Game::OnRecv(const Msg取消跟随& msg)
{
	ForEachSelected([this](Entity& ref)
	{
		if (ref.m_up跟随)
		{
			ref.m_up跟随->取消跟随();
		}
	});
}