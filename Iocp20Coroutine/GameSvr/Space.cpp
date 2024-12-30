#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "GameSvrSession.h"
#include "��ԴComponent.h"
#include "PlayerComponent.h"
#include "AoiComponent.h"
#include "AttackComponent.h"
#include "PlayerNickNameComponent.h"
#include "EntitySystem.h"
#include "DefenceComponent.h"
#include "��Component.h"
#include "�ɼ�Component.h"
#include "�콨��Component.h"

Space::Space(const ��������& ref) :m_����(ref)
{
	std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string & stfѰ·�ļ�);
	m_spCrowdToolState = CreateCrowdToolState(ref.strѰ·�ļ���);
}

Space::~Space()
{
}

WpEntity Space::GetEntity(const int64_t id)
{
	auto itFind = m_mapEntity.find(id);
	if (itFind == m_mapEntity.end())
	{
		LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
		//assert(false);
		return {};
	}
	CHECK_DEFAULT(itFind->second);
	return itFind->second->weak_from_this();
}

WpEntity Space::Get�����Entity֧�ֵر��еĵ�λ(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun��������)
{
	WpEntity wp = refEntity.m_wpOwner;
	if (!GetEntity(refEntity.Id).expired())
	{
		wp = refEntity.weak_from_this();
	}
	if (wp.expired())
		return {};

	return Get�����Entity(*wp.lock(), bFindEnemy, fun��������);
}

WpEntity Space::Get�����Entity(Entity& refEntity, const bool bFindEnemy, std::function<bool(const Entity&)> fun��������)
{
	if (!refEntity.m_upAoi)
		return{};

	std::vector<std::pair<int64_t, WpEntity>> vecEnemy;
	std::copy_if(refEntity.m_upAoi->m_map���ܿ�����.begin(), refEntity.m_upAoi->m_map���ܿ�����.end(), std::back_inserter(vecEnemy),
		[bFindEnemy, &refEntity, &fun��������](const auto& pair)
		{
			auto& wp = pair.second;
			CHECK_FALSE(!wp.expired());
			Entity& ref = *wp.lock();
			const auto bEnemy = ref.IsEnemy(refEntity);
			if (bEnemy != bFindEnemy)
				return false;

			if (fun�������� && !fun��������(ref))
				return false;

			return &ref != &refEntity && !ref.IsDead();
		});

	if (vecEnemy.empty())
	{
		return {};
	}

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [&refEntity](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return refEntity.DistancePow2(*sp1.lock()) < refEntity.DistancePow2(*sp2.lock());
		});
	return iterMin->second.lock()->weak_from_this();
}

void Space::Update()
{
	EraseEntity(false);

	void CrowToolUpdate(Space & ref);
	CrowToolUpdate(*this);
}

std::unordered_map<uint8_t, SpSpace> g_mapSpace;
WpSpace Space::AddSpace(const uint8_t idSpace)
{
	auto wpOld = GetSpace(idSpace);
	if (!wpOld.expired())
		return wpOld;

	�������� ����;
	{
		bool Get��������(const ����ID id, �������� & refOut);
		const auto ok = Get��������(����������ͼ, ����);
		CHECK_RET_DEFAULT(ok);
	}
	auto [iterNew, bOk] = g_mapSpace.insert({ idSpace,std::make_shared<Space,const ��������&>(����) });
	assert(bOk);
	return iterNew->second;
}

WpSpace Space::GetSpace(const uint8_t idSpace)
{
	auto iterFind = g_mapSpace.find(idSpace);
	if (g_mapSpace.end() == iterFind)
		return {};

	return iterFind->second;
}

void Space::StaticUpdate()
{
	for (auto [id, sp] : g_mapSpace)
	{
		sp->Update();
	}
}

bool Space::CrowdTool��վ��(const Position& refPos)
{
	if (!m_spCrowdToolState)
	{
		LOG(ERROR) << "m_spCrowdToolState";
		return true;
	}
	bool CrowdTool��վ��(CrowdToolState & refCrowTool, const Position & refPos);
	return CrowdTool��վ��(*m_spCrowdToolState, refPos);
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

Space::SpacePlayer& Space::GetSpacePlayer(const Entity& ref)
{
	if (ref.m_spPlayerNickName)
		return ref.m_refSpace.m_mapPlayer[ref.m_spPlayerNickName->m_strNickName];

	LOG(ERROR) << ref.Id << ",�������";
	assert(false);
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

		if (spEntity->m_spPlayerNickName)
		{
			m_mapPlayer[spEntity->m_spPlayerNickName->m_strNickName].Erase(spEntity->Id);
		}

		LOG(INFO) << "ɾ�����ڶ���," << spEntity->NickName() << ",Id=" << spEntity->Id << ",ɾ��ǰʣ��" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}

int Space::Get���ﵥλ��()
{
	return Get��λ��([](const Entity& refEntity)
		{
			if (refEntity.IsDead())
				return false;

			if (nullptr == refEntity.m_spMonster)
				return false;

			if (refEntity.m_spPlayerNickName)//��������Ƶĵ�λ
				return false;

			return true;
		});
}

int Space::Get��Դ��λ��(const ��Դ���� ����)
{
	return Get��λ��([����](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_sp��Դ)
				return false;

			return ���� == refEntity.m_sp��Դ->m_����;
		});
}

int Space::Get��ҵ�λ��(const PlayerGateSession_Game& ref)
{
	return Get��λ��([&ref](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_spPlayerNickName)
				return false;

			if (ref.NickName() != refEntity.m_spPlayerNickName->m_strNickName)
				return false;

			if (!refEntity.m_spDefence)
				return false;//�ӿ�

			return true;
		});
}
int Space::Get��λ��(const std::function<bool(const Entity&)>& fun�Ƿ�ͳ�ƴ˵�λ)
{
	int i32��λ��(0);
	for (const auto [k, spEntity] : m_mapEntity)
	{
		if (fun�Ƿ�ͳ�ƴ˵�λ && !fun�Ƿ�ͳ�ƴ˵�λ(*spEntity))
			continue;

		++i32��λ��;
	}

	return i32��λ��;
}

void Space::AddEntity(SpEntity& spNewEntity, const int32_t i32��Ұ��Χ)
{
	m_mapEntity.insert({ spNewEntity->Id ,spNewEntity });//ȫ��ͼ��λ
	AoiComponent::Add(*this, *spNewEntity, i32��Ұ��Χ);
}

void Space::�������ȫ�˳�()
{
	auto map = m_map�ӿ�;
	for (auto [id, wp] : map)//�������̻�ɾ��	m_map�ӿ�	��
	{
		CHECK_WP_CONTINUE(wp);
		auto& ref�ӿ� = *wp.lock();
		CHECK_RET_VOID(EntitySystem::Is�ӿ�(ref�ӿ�));
		CHECK_RET_VOID(ref�ӿ�.m_spPlayer);
		ref�ӿ�.m_spPlayer->m_refSession.OnDestroy();//�Թ۵����˳�
	}
}

void Space::OnDestory()
{
	�������ȫ�˳�();
	EraseEntity(true);
}

Space::SpacePlayer::SpacePlayer()
{
}

void Space::SpacePlayer::OnDestroy(const bool b���˸���, Space& refSpace, const std::string& refStrNickName)
{
	for (auto [_, wp] : m_mapWpEntity)
	{
		//assert(!wp.expired());
		if (wp.expired())
		{
			LOG(ERROR) << "ɾ�˵�λ����������ûɾ";
			continue;
		}
		auto sp = wp.lock();
		if (b���˸��� || EntitySystem::Is�ӿ�(*sp))
		{
			if (sp->m_refSpace.GetEntity(sp->Id).expired())
			{
				LOG(INFO) << "�����ǵر���ı�" << sp->NickName();
				continue;
			}
			LOG(INFO) << "m_mapEntity.size=" << sp->m_refSpace.m_mapEntity.size();
			sp->OnDestroy();
			auto countErase = sp->m_refSpace.m_mapEntity.erase(sp->Id);
			assert(1 == countErase);
		}
		else
		{
			sp->m_spPlayer.reset();
			refSpace.m_map������PlayerEntity[refStrNickName].insert({ sp->Id,sp });
		}
	}

	m_mapWpEntity.clear();
}

inline void Space::SpacePlayer::Erase(uint64_t u64Id)
{
	if (!m_mapWpEntity.contains(u64Id))
	{
		LOG(WARNING) << "ERR";
		return;
	}

	m_mapWpEntity.erase(u64Id);
}


SpEntity Space::����λ(std::shared_ptr<PlayerComponent> &refSpPlayer���ܿ�, const std::string &strNickName, const Position& pos, const ��λ::���λ����& ����, const ���λ���� ����)
{
	SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, const ��λ::��λ����&>(
		pos, *this, ����.����);
	PlayerComponent::AddComponent(*spNewEntity, refSpPlayer���ܿ�, strNickName);
	AttackComponent::AddComponent(*spNewEntity, ����, ����.ս��);
	DefenceComponent::AddComponent(*spNewEntity, ����.����.u16��ʼHp);
	��Component::AddComponent(*spNewEntity);
	m_mapPlayer[strNickName].m_mapWpEntity[spNewEntity->Id] = spNewEntity;//�Լ����Ƶĵ�λ
	AddEntity(spNewEntity);//ȫ��ͼ��λ
	spNewEntity->m_�ٶ�ÿ֡�ƶ����� = ����.ս��.fÿ֡�ƶ�����;
	switch (����)
	{
	case ���̳�:
		�ɼ�Component::AddComponent(*spNewEntity);
		�콨��Component::AddComponent(*spNewEntity, ����);
		PlayerComponent::��������(*spNewEntity, "����/���̳�׼������Ů���ɰ���"); //refGateSession.Say������ʾ("���̳����Կ�����!");//SCV, good to go, sir. SCV���Կ�����
		break;
	case ��:PlayerComponent::��������(*spNewEntity, "����/׼��ս������������"); break;//refGateSession.Say������ʾ("��˵����Ҫ���ҵĹ�ͷ��");//You want a piece of me, boy?��Ҫ�ҵ�һ������С�ӣ�
	case ��ս��:PlayerComponent::��������(*spNewEntity, "tfbRdy00"); break;//refGateSession.Say������ʾ("��˵����Ҫ�ҵĸ�ͷ��");//You want a piece of me, boy?��Ҫ�ҵ�һ������С�ӣ�
	case ��ɫ̹��:PlayerComponent::��������(*spNewEntity, "����/ttardy00"); break;
	default:break;
	}

	spNewEntity->BroadcastEnter();
	PlayerComponent::Send��Դ(*spNewEntity);

	return spNewEntity;
}


bool Space::�ɷ��ý���(const Position& refPos, float f��߳�)
{
	if (!CrowdTool��վ��({ refPos.x - f��߳� ,refPos.z + f��߳� }))return false;
	if (!CrowdTool��վ��({ refPos.x - f��߳� ,refPos.z - f��߳� }))return false;
	if (!CrowdTool��վ��({ refPos.x + f��߳� ,refPos.z + f��߳� }))return false;
	if (!CrowdTool��վ��({ refPos.x + f��߳� ,refPos.z - f��߳� }))return false;

	//����ȫ��ͼ���н����ж��ص�
	for (const auto& kv : m_mapEntity)
	{
		auto& refEntity = *kv.second;
		const auto& refPosOld = refEntity.Pos();
		bool CrowdTool�жϵ�λ�ص�(const Position & refPosOld, const Position & refPosNew, const float f��߳�);
		if (CrowdTool�жϵ�λ�ص�(refPos, refPosOld, f��߳�))
			return false;
	}

	return true;
}