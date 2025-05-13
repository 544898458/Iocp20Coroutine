#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "GameSvrSession.h"
#include "../IocpNetwork/StrConv.h"
#include "枚举/BuffId.h"
#include <fstream>
#include "单位组件/PlayerComponent.h"
#include "单位组件/资源Component.h"
#include "单位组件/PlayerComponent.h"
#include "单位组件/AoiComponent.h"
#include "单位组件/AttackComponent.h"
#include "单位组件/PlayerNickNameComponent.h"
#include "单位组件/DefenceComponent.h"
#include "单位组件/走Component.h"
#include "单位组件/采集Component.h"
#include "单位组件/造建筑Component.h"
#include "单位组件/造活动单位Component.h"
#include "单位组件/BuildingComponent.h"
#include "单位组件/医疗兵Component.h"
#include "单位组件/BuffComponent.h"
#include "单位组件/可进活动单位Component.h"


Space::Space(const 副本配置& ref) :m_配置(ref), m_funCancel剧情("m_funCancel剧情")
{
	std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string & stf寻路文件);
	m_spCrowdToolState = CreateCrowdToolState(ref.str寻路文件名);
	m_spCrowdToolState空中 = CreateCrowdToolState("空中无障碍.bin");
}

Space::~Space()
{
}

std::string GetFileName(const uint16_t idSpace)
{
	std::ostringstream oss;
	oss << idSpace << ".space";
	return oss.str();
}

void Space::Save(const uint8_t idSpace)
{
	const auto strFileName = GetFileName(idSpace);
	std::ofstream file(strFileName, std::ios::binary);
	for (auto [id, spEntity] : m_mapEntity)
	{
		if (!spEntity->m_upPlayerNickName)
			continue;//只存玩家单位

		if (EntitySystem::Is视口(*spEntity) || 苔蔓 == spEntity->m_类型)
			continue;
		//if (spEntity->m_spBuilding && !spEntity->m_sp资源)
		spEntity->Save(file);
	}
	file.close();// 关闭文件
	LOG(INFO) << "已写入" << strFileName;
}

void Space::Load(const uint8_t idSpace)
{
	const auto strFileName = GetFileName(idSpace);
	std::ifstream file(strFileName, std::ios::binary);
	// 获取文件大小
	file.seekg(0, std::ios::end);
	std::streamsize sizeFile = file.tellg();
	file.seekg(0, std::ios::beg);
	while (true)
	{
		std::streamsize size已读取 = file.tellg();
		if (size已读取 >= sizeFile)
			break;

		uint16_t u16Size(0);
		file.read((char*)&u16Size, sizeof(u16Size));
		char buf[1024] = { 0 };
		CHECK_GE_VOID(sizeof(buf), u16Size);
		file.read(buf, u16Size);
		Entity::Load(*this, buf, u16Size);
	}

	file.close();// 关闭文件
}

WpEntity Space::GetEntity(const int64_t id)
{
	auto itFind = m_mapEntity.find(id);
	if (itFind == m_mapEntity.end())
	{
		LOG(INFO) << "找不到,GetEntity:" << id;
		//_ASSERT(false);
		return {};
	}
	CHECK_DEFAULT(itFind->second);
	return itFind->second->weak_from_this();
}

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);
void Space::On玩家离线(const std::string& refStrNickName离线)
{
	m_b休眠 = true;
	for (const auto& [refStrNickName, _] : m_mapPlayer)
	{
		if (refStrNickName离线 == refStrNickName)
			continue;

		if (GetPlayerGateSession(refStrNickName).expired())
			continue;

		m_b休眠 = false;
		return;
	}

	LOG(INFO) << m_配置.strSceneName << ",休眠";
}

void Space::Update()
{
	if (m_b休眠)
		return;

	EraseEntity(false);

	void CrowToolUpdate(Space & ref, CrowdToolState & refCrowdToolState);
	CrowToolUpdate(*this, *m_spCrowdToolState);
	CrowToolUpdate(*this, *m_spCrowdToolState空中);
}

std::unordered_map<uint8_t, SpSpace> g_mapSpace;
std::unordered_map<std::string, SpSpace> g_mapSpace单人;
const std::unordered_map<std::string, SpSpace>& Space::个人战局()
{
	return g_mapSpace单人;
}

WpSpace Space::GetSpace单人(const std::string& refStrPlayerNickName)
{
	auto itFind = g_mapSpace单人.find(refStrPlayerNickName);
	if (g_mapSpace单人.end() != itFind)
		return itFind->second;

	return {};
}

std::tuple<bool, WpSpace> Space::GetSpace单人(const std::string& refStrPlayerNickName, const 副本配置& 配置)
{
	auto itFind = g_mapSpace单人.find(refStrPlayerNickName);
	if (g_mapSpace单人.end() != itFind)
		return { false,itFind->second };

	auto pair = g_mapSpace单人.insert({ refStrPlayerNickName, std::make_shared<Space, const 副本配置&>(配置) });
	_ASSERT(pair.second);
	return { true,pair.first->second };
}
bool Space::DeleteSpace单人(const std::string& refStrPlayerNickName)
{
	auto wp = GetSpace单人(refStrPlayerNickName);
	if (wp.expired())
		return false;

	wp.lock()->OnDestory();
	auto sizeErase = g_mapSpace单人.erase(refStrPlayerNickName);
	_ASSERT(1 == sizeErase);
	return true;
}
WpSpace Space::AddSpace(const uint8_t idSpace)
{
	auto wpOld = GetSpace(idSpace);
	if (!wpOld.expired())
		return wpOld;

	副本配置 配置;
	{
		bool Get副本配置(const 副本ID id, 副本配置 & refOut);
		const auto ok = Get副本配置(多人联机地图, 配置);
		CHECK_RET_DEFAULT(ok);
	}
	auto [iterNew, bOk] = g_mapSpace.insert({ idSpace,std::make_shared<Space,const 副本配置&>(配置) });
	_ASSERT(bOk);
	iterNew->second->Load(idSpace);
	return iterNew->second;
}

WpSpace Space::GetSpace(const uint8_t idSpace)
{
	auto iterFind = g_mapSpace.find(idSpace);
	if (g_mapSpace.end() == iterFind)
		return {};

	return iterFind->second;
}

void Space::StaticOnAppExit()
{
	for (auto [id, sp] : g_mapSpace)
	{
		sp->Save(id);
	}
}

void Space::StaticUpdate()
{
	for (auto [_, sp] : g_mapSpace)
	{
		sp->Update();
	}
	for (auto [_, sp] : g_mapSpace单人)
	{
		sp->Update();
	}
}

bool Space::CrowdTool可站立(const Position& refPos)
{
	if (!m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return true;
	}
	bool CrowdTool可站立(CrowdToolState & refCrowTool, const Position & refPos);
	return CrowdTool可站立(*m_spCrowdToolState, refPos);
}

bool Space::CrowdToolFindNerestPos(Position& refPos)
{
	if (!m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return false;
	}
	bool CrowdToolFindNerestPos(CrowdToolState & refCrowTool, Position & refPos);
	return CrowdToolFindNerestPos(*m_spCrowdToolState, refPos);
}
Space::SpacePlayer& Space::GetSpacePlayer(const std::string strPlayerNickName)
{
	return m_mapPlayer[strPlayerNickName];
}
Space::SpacePlayer& Space::GetSpacePlayer(const Entity& ref)
{
	if (ref.m_upPlayerNickName)
		return ref.m_refSpace.GetSpacePlayer(ref.m_upPlayerNickName->m_strNickName);

	LOG(ERROR) << ref.Id << ",不是玩家";
	_ASSERT(false);
	static Space::SpacePlayer s_err;
	return s_err;
}

void Space::EraseEntity(const bool bForceEraseAll)
{
	for (auto iter = m_mapEntity.begin(); iter != m_mapEntity.end(); )
	{
		auto& spEntity = iter->second;
		spEntity->Update();

		if (!bForceEraseAll && !spEntity->NeedDelete())
		{
			++iter;
			continue;
		}

		if (spEntity->m_upPlayerNickName)
		{
			const auto sizeErase = m_mapPlayer[spEntity->m_upPlayerNickName->m_strNickName].Erase(spEntity->Id);
			CHECK_NOT_RETURN(1 == sizeErase);
		}
		else
		{
#if true//_DEBUG
			for (auto& [refStrNickName, refSpacePlayer] : m_mapPlayer)
			{
				if (refSpacePlayer.m_mapWpEntity.contains(spEntity->Id))
				{
					LOG(ERROR) << "没有m_upPlayerNickName组件却在m_mapWpEntity中," << spEntity->头顶Name() << ",Id=" << spEntity->Id << ",删除前剩余" << m_mapEntity.size();
					_ASSERT(false);
				}
			}
#endif
		}

		//LOG(INFO) << "删除过期对象," << spEntity->头顶Name() << ",Id=" << spEntity->Id << ",删除前剩余" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}

CrowdToolState& Space::GetCrowdToolState(const 单位类型 类型)
{
	switch (类型)
	{
	case 飞机:
	case 飞虫:
	case 房虫:
		return *m_spCrowdToolState空中;
	default:
		return *m_spCrowdToolState;
	}

}

int Space::Get怪物单位数(const 单位类型 类型)const
{
	return Get单位数([类型](const Entity& refEntity)
		{
			if (refEntity.m_upPlayerNickName)//属于玩家制的单位
				return false;
			if (EntitySystem::Is资源(refEntity.m_类型))
				return false;
			if (类型 > 单位类型_Invalid_0 && 类型 != refEntity.m_类型)
				return false;

			return true;
		});
}

int Space::Get资源单位数(const 单位类型 类型)
{
	return Get单位数([类型](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_up资源)
				return false;

			return 类型 == refEntity.m_up资源->m_类型;
		});
}

int Space::Get单位数(const 单位类型 arg类型)const
{
	return Get单位数([arg类型](const Entity& refEntity)
		{
			return refEntity.m_类型 == arg类型;
		});
}

int Space::Get玩家单位数(const std::string& strPlayerNickName, const 单位类型 类型)
{
	return Get单位数([&strPlayerNickName, 类型](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_upPlayerNickName)
				return false;

			if (strPlayerNickName != refEntity.m_upPlayerNickName->m_strNickName)
				return false;

			if (EntitySystem::Is视口(refEntity))
				return false;

			if (EntitySystem::Is光刺(refEntity))
				return false;

			if (类型 != 单位类型::单位类型_Invalid_0 && 类型 != refEntity.m_类型)
				return false;

			if (refEntity.m_upBuilding && !refEntity.m_upBuilding->已造好())
				return false;

			return true;
		});
}
int Space::Get单位数(const std::function<bool(const Entity&)>& fun是否统计此单位)const
{
	int i32单位数(0);
	for (const auto [k, spEntity] : m_mapEntity)
	{
		if (spEntity->IsDead())
			continue
			;
		if (fun是否统计此单位 && !fun是否统计此单位(*spEntity))
			continue;

		++i32单位数;
	}

	return i32单位数;
}

void Space::AddEntity(SpEntity spNewEntity, const int32_t i32视野范围)
{
	if (EntitySystem::Is视口(spNewEntity->m_类型))
	{
		LOG(INFO) << m_配置.strSceneName << ",醒了";
		m_b休眠 = false;
	}
	m_mapEntity.insert({ spNewEntity->Id ,spNewEntity });//全地图单位
	AoiComponent::Add(*this, *spNewEntity, i32视野范围);
}

void Space::所有玩家全退出()
{
	auto map = m_map视口;
	for (auto [id, wp] : map)//迭代过程会删除	m_map视口	项
	{
		CHECK_WP_CONTINUE(wp);
		auto& ref视口 = *wp.lock();
		CHECK_RET_VOID(EntitySystem::Is视口(ref视口));
		CHECK_RET_VOID(ref视口.m_upPlayer);
		ref视口.m_upPlayer->m_refSession.OnDestroy();//旁观的人退出
	}
}

void Space::OnDestory()
{
	m_funCancel剧情.TryCancel();
	
	所有玩家全退出();
	EraseEntity(true);
}

Space::SpacePlayer::SpacePlayer() :m_map单位解锁{ {近战兵,{}}, {枪虫,{}} }
{

}

bool Space::SpacePlayer::已解锁(const 单位类型 类型) const
{
	auto iterFind = m_map单位解锁.find(类型);
	if (iterFind == m_map单位解锁.end())
		return true;//无需解锁

	return iterFind->second.b已解锁;
}

bool Space::SpacePlayer::解锁完成(const 单位类型 类型, Entity& refEntity在此建筑中解锁)
{
	auto iterFind = m_map单位解锁.find(类型);
	CHECK_RET_FALSE(iterFind != m_map单位解锁.end());
	auto& ref解锁状态 = iterFind->second;

	CHECK_RET_FALSE(!ref解锁状态.b已解锁);
	CHECK_WP_RET_FALSE(ref解锁状态.wpEntity在此建筑中解锁);
	auto& ref解锁建筑 = *ref解锁状态.wpEntity在此建筑中解锁.lock();

	CHECK_RET_FALSE(&ref解锁建筑 == &refEntity在此建筑中解锁);

	ref解锁状态.wpEntity在此建筑中解锁.reset();
	ref解锁状态.b已解锁 = true;

	return true;
}

bool Space::SpacePlayer::开始解锁单位(const 单位类型 单位, Entity& refEntity在此建筑中解锁)
{
	auto iterFind = m_map单位解锁.find(单位);
	CHECK_RET_FALSE(iterFind != m_map单位解锁.end());
	auto& ref解锁状态 = iterFind->second;

	if (ref解锁状态.b已解锁)
	{
		PlayerGateSession_Game::播放声音Buzz(refEntity在此建筑中解锁, "无需重复解锁");
		return false;
	}

	if (!ref解锁状态.wpEntity在此建筑中解锁.expired())
	{
		PlayerGateSession_Game::播放声音Buzz(refEntity在此建筑中解锁, "正在解锁");
		return false;
	}

	单位::消耗资源 消耗;
	if (!单位::Find单位解锁配置(单位, 消耗))
	{
		PlayerGateSession_Game::播放声音Buzz(refEntity在此建筑中解锁, "无需解锁");
		return false;
	}

	if (消耗.u16消耗燃气矿 > m_u32燃气矿)
	{
		PlayerComponent::播放声音(refEntity在此建筑中解锁, "语音/燃气矿不足正经版", "燃气矿不足，不能解锁");//(low error beep) Insufficient Vespene Gas.气矿不足 
		return false;
	}
	m_u32燃气矿 -= 消耗.u16消耗燃气矿;

	if (消耗.u16消耗晶体矿 > m_u32晶体矿)
	{
		//refGateSession.m_u32燃气矿 += 配置.制造.u16消耗燃气矿;
		PlayerComponent::播放声音(refEntity在此建筑中解锁, "语音/晶体矿不足正经版", "晶体矿不足，不能解锁");
		return false;
	}
	m_u32晶体矿 -= 消耗.u16消耗晶体矿;

	PlayerComponent::Send资源(refEntity在此建筑中解锁);

	ref解锁状态.wpEntity在此建筑中解锁 = refEntity在此建筑中解锁.shared_from_this();

	return true;
}

/// <summary>
/// 能否升级在建筑里判断，不在这里判断，这里只管升级
/// </summary>
/// <param name="类型"></param>
/// <param name="属性"></param>
/// <param name="refEntity在此建筑中解锁"></param>
/// <returns></returns>
bool Space::SpacePlayer::开始升级单位属性(const 单位类型 单位, const 属性类型 属性, Entity& refEntity在此建筑中升级)
{
	auto& ref单位属性等级 = m_map单位属性等级[单位][属性];
	if (!ref单位属性等级.wpEntity在此建筑中升级.expired())
	{
		PlayerGateSession_Game::播放声音Buzz(refEntity在此建筑中升级, "正在升级");
		return false;
	}

	单位::单位属性等级配置详情 等级详情;
	if (!单位::Find单位属性等级配置(单位, 属性, ref单位属性等级.u16等级 + 1, 等级详情))
	{
		PlayerGateSession_Game::播放声音Buzz(refEntity在此建筑中升级, "已满级");
		return false;
	}

	if (等级详情.消耗.u16消耗燃气矿 > m_u32燃气矿)
	{
		PlayerComponent::播放声音(refEntity在此建筑中升级, "语音/燃气矿不足正经版", "燃气矿不足，不能升级");//(low error beep) Insufficient Vespene Gas.气矿不足 
		return false;
	}
	m_u32燃气矿 -= 等级详情.消耗.u16消耗燃气矿;

	if (等级详情.消耗.u16消耗晶体矿 > m_u32晶体矿)
	{
		//refGateSession.m_u32燃气矿 += 配置.制造.u16消耗燃气矿;
		PlayerComponent::播放声音(refEntity在此建筑中升级, "语音/晶体矿不足正经版", "晶体矿不足，不能升级");
		return false;
	}
	m_u32晶体矿 -= 等级详情.消耗.u16消耗晶体矿;

	PlayerComponent::Send资源(refEntity在此建筑中升级);

	ref单位属性等级.wpEntity在此建筑中升级 = refEntity在此建筑中升级.shared_from_this();

	return true;
}

bool Space::SpacePlayer::升级单位属性完成(const 单位类型 单位, const 属性类型 属性, Entity& refEntity在此建筑中升级)
{
	auto& ref单位属性等级 = m_map单位属性等级[单位][属性];
	CHECK_WP_RET_FALSE(ref单位属性等级.wpEntity在此建筑中升级);
	auto& ref建筑 = *ref单位属性等级.wpEntity在此建筑中升级.lock();

	CHECK_RET_FALSE(&ref建筑 == &refEntity在此建筑中升级);

	ref单位属性等级.wpEntity在此建筑中升级.reset();
	++ref单位属性等级.u16等级;

	return true;
}

uint16_t Space::SpacePlayer::单位属性等级(const 单位类型 单位, const 属性类型 属性)const
{
	auto iterFind单位 = m_map单位属性等级.find(单位);
	if (iterFind单位 == m_map单位属性等级.end())
		return 0;

	auto iterFind属性 = iterFind单位->second.find(属性);
	if (iterFind属性 == iterFind单位->second.end())
		return 0;

	return iterFind属性->second.u16等级;
}

void Space::SpacePlayer::OnDestroy(const bool b删除玩家所有单位, Space& refSpace, const std::string& refStrNickName)
{
	auto mapLocal = m_mapWpEntity;//不能在ForEach内删除容器
	bool b已删除视口 = false;
	for (auto [_, wp] : mapLocal)
	{
		//_ASSERT(!wp.expired());
		if (wp.expired())
		{
			LOG(ERROR) << "删了单位，但是这里没删";
			continue;
		}
		auto& ref = *wp.lock();
		const bool b视口 = EntitySystem::Is视口(ref);
		if (b删除玩家所有单位 || b视口)//删除单位
		{
			if (ref.m_refSpace.GetEntity(ref.Id).expired())
			{
				LOG(INFO) << "可能是地堡里的兵" << ref.头顶Name();
				continue;
			}

			if (b视口)
			{
				ref.Broadcast<MsgSay>({ .content = StrConv::GbkToUtf8(refStrNickName + " 离开了") });
				CHECK_NOT_RETURN(!b已删除视口);
				b已删除视口 = true;
			}
			LOG(INFO) << "m_mapEntity.size=" << ref.m_refSpace.m_mapEntity.size();
			ref.OnDestroy();

			auto countErase = m_mapWpEntity.erase(ref.Id);
			CHECK_NOT_RETURN(1 == countErase);

			countErase = ref.m_refSpace.m_mapEntity.erase(ref.Id);
			CHECK_NOT_RETURN(1 == countErase);
		}
		else//不删，只删除Session引用
		{
			ref.m_upPlayer.reset();
			//refSpace.m_map已离线PlayerEntity[refStrNickName].insert({ sp->Id,sp });
		}
	}
	CHECK_NOT_RETURN(b已删除视口);
	//m_mapWpEntity.clear();
}

size_t Space::SpacePlayer::Erase(uint64_t u64Id)
{
	CHECK_RET_DEFAULT(m_mapWpEntity.contains(u64Id));
	return m_mapWpEntity.erase(u64Id);
}

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);

WpEntity Space::造活动单位(Entity& ref视口, const std::string& refStrNickName, const 单位类型 类型, const Position& refPos, bool b设置视口)
{
	auto wp = 造活动单位(std::forward<UpPlayerComponent>(ref视口.m_upPlayer), refStrNickName, refPos, 类型);
	CHECK_WP_RET_DEFAULT(wp);
	if (b设置视口)
	{
		auto wpSession = GetPlayerGateSession(refStrNickName);
		CHECK_WP_RET_DEFAULT(wpSession);

		wpSession.lock()->Send设置视口(*wp.lock());
	}
	return wp;
}

WpEntity Space::造活动单位(UpPlayerComponent&& refSpPlayer可能空, const std::string& strNickName, const Position& pos, const 单位类型 类型)
{
	单位::活动单位配置 活动;
	单位::单位配置 单位;
	单位::制造配置 制造;
	单位::战斗配置 战斗;
	CHECK_RET_DEFAULT(单位::Find活动单位配置(类型, 活动));
	CHECK_RET_DEFAULT(单位::Find单位配置(类型, 单位));
	CHECK_RET_DEFAULT(单位::Find制造配置(类型, 制造));

	SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, const 单位类型, const 单位::单位配置&>(
		pos, *this, std::forward<const 单位类型&&>(类型), 单位);
	PlayerComponent::AddComponent(*spNewEntity, std::forward<UpPlayerComponent>(refSpPlayer可能空), strNickName);

	if (光刺 != 类型)
		DefenceComponent::AddComponent(*spNewEntity, 制造.u16初始Hp);

	if (EntitySystem::Is活动单位建筑怪(类型))
	{
		BuffComponent::AddComponent(*spNewEntity);
	}

	switch (类型)
	{
	case 医疗兵:
		医疗兵Component::AddComponent(*spNewEntity);
		break;
	default:
		AttackComponent::AddComponent(*spNewEntity);
		break;
	}

	switch (类型)
	{
	case 光刺:
		break;
	default:
		走Component::AddComponent(*spNewEntity);
		break;
	}

	if (!strNickName.empty())
	{
		auto [pair, ok] = m_mapPlayer[strNickName].m_mapWpEntity.insert({ spNewEntity->Id, spNewEntity });//自己控制的单位
		CHECK_NOT_RETURN(ok);
	}
	AddEntity(spNewEntity);//全地图单位

	PlayerComponent::播放声音(*spNewEntity, 活动.str入场语音); //SCV, good to go, sir. SCV可以开工了
	switch (类型)
	{
	case 工程车:
	case 工虫:
		采集Component::AddComponent(*spNewEntity);
		造建筑Component::AddComponent(*spNewEntity);
		break;
	case 幼虫:
		造活动单位Component::AddComponent(*spNewEntity);
		break;
	case 房虫:
		可进活动单位Component::AddComponent(*spNewEntity);
		break;
	default:break;
	}


	if (单位::Is虫(类型) && EntitySystem::Is活动单位(类型))
	{
		spNewEntity->m_upBuff->定时改数值(虫活动单位持续加血, spNewEntity->Id);
	}

	spNewEntity->BroadcastEnter();
	PlayerComponent::Send资源(*spNewEntity);

	return spNewEntity;
}

bool CrowdTool可走直线(CrowdToolState& refCrowTool, const Position& pos起始, const Position& pos目标);
bool Space::CrowdTool可走直线(const Position& pos起始, const Position& pos目标)
{
	return ::CrowdTool可走直线(*m_spCrowdToolState, pos起始, pos目标);
}

int CrowdToolGetAgentIdx(CrowdToolState& refCrowTool, const uint64_t idEntity);
int Space::CrowdToolGetAgentIdx(const Entity& refEntity)
{
	return ::CrowdToolGetAgentIdx(GetCrowdToolState(refEntity.m_类型), refEntity.Id);
}
bool Space::可放置建筑(const Position& refPos, float f半边长)
{
	const Position pos左下 = { refPos.x - f半边长, refPos.z - f半边长 };
	const Position pos右上 = { refPos.x + f半边长, refPos.z + f半边长 };
	const Position pos左上 = { refPos.x - f半边长, refPos.z + f半边长 };
	const Position pos右下 = { refPos.x + f半边长, refPos.z - f半边长 };
	if (!CrowdTool可站立(pos左上))return false;
	if (!CrowdTool可站立(pos左下))return false;
	if (!CrowdTool可站立(pos右上))return false;
	if (!CrowdTool可站立(pos右下))return false;

	//遍历全地图所有建筑判断重叠
	for (const auto& kv : m_mapEntity)
	{
		auto& refEntity = *kv.second;
		if (!EntitySystem::Is建筑(refEntity))
			continue;

		const auto& refPosOld = refEntity.Pos();
		bool CrowdTool判断单位重叠(const Position & refPosOld, const Position & refPosNew, const float f半边长);
		if (CrowdTool判断单位重叠(refPos, refPosOld, f半边长))
			return false;
	}


	return	CrowdTool可走直线(pos左下, pos右上)
		&& CrowdTool可走直线(pos左上, pos右下)
		&& CrowdTool可走直线(pos左上, pos左下)
		&& CrowdTool可走直线(pos右上, pos右下)
		&& CrowdTool可走直线(pos右上, pos右下)
		&& CrowdTool可走直线(pos左上, pos左下)
		;
}