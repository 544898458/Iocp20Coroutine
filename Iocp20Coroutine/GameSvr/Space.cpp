#include "pch.h"
#include "Space.h"
#include "Entity.h"
#include "��λ���/PlayerComponent.h"
#include "GameSvrSession.h"
#include "��λ���/��ԴComponent.h"
#include "��λ���/PlayerComponent.h"
#include "��λ���/AoiComponent.h"
#include "��λ���/AttackComponent.h"
#include "��λ���/PlayerNickNameComponent.h"
#include "EntitySystem.h"
#include "��λ���/DefenceComponent.h"
#include "��λ���/��Component.h"
#include "��λ���/�ɼ�Component.h"
#include "��λ���/�콨��Component.h"
#include "��λ���/����λComponent.h"
#include "��λ���/BuildingComponent.h"
#include "../IocpNetwork/StrConv.h"

#include <fstream>

Space::Space(const ��������& ref) :m_����(ref)
{
	std::shared_ptr<CrowdToolState> CreateCrowdToolState(const std::string & stfѰ·�ļ�);
	m_spCrowdToolState = CreateCrowdToolState(ref.strѰ·�ļ���);
	m_spCrowdToolState���� = CreateCrowdToolState("�������ϰ�.bin");
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
		if (!spEntity->m_spPlayerNickName)
			continue;//ֻ����ҵ�λ

		if (EntitySystem::Is�ӿ�(*spEntity))
			continue;
		//if (spEntity->m_spBuilding && !spEntity->m_sp��Դ)
		spEntity->Save(file);
	}
	file.close();// �ر��ļ�
	LOG(INFO) << "��д��" << strFileName;
}

void Space::Load(const uint8_t idSpace)
{
	const auto strFileName = GetFileName(idSpace);
	std::ifstream file(strFileName, std::ios::binary);
	// ��ȡ�ļ���С
	file.seekg(0, std::ios::end);
	std::streamsize sizeFile = file.tellg();
	file.seekg(0, std::ios::beg);
	while (true)
	{
		std::streamsize size�Ѷ�ȡ = file.tellg();
		if (size�Ѷ�ȡ >= sizeFile)
			break;

		uint16_t u16Size(0);
		file.read((char*)&u16Size, sizeof(u16Size));
		char buf[1024] = { 0 };
		CHECK_GE_VOID(sizeof(buf), u16Size);
		file.read(buf, u16Size);
		Entity::Load(*this, buf, u16Size);
	}

	file.close();// �ر��ļ�
}

WpEntity Space::GetEntity(const int64_t id)
{
	auto itFind = m_mapEntity.find(id);
	if (itFind == m_mapEntity.end())
	{
		LOG(INFO) << "ѡ�е�ʵ�岻����:" << id;
		//_ASSERT(false);
		return {};
	}
	CHECK_DEFAULT(itFind->second);
	return itFind->second->weak_from_this();
}

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);
void Space::On�������(const std::string& refStrNickName����)
{
	m_b���� = true;
	for (const auto& [refStrNickName, _] : m_mapPlayer)
	{
		if (refStrNickName���� == refStrNickName)
			continue;

		if (GetPlayerGateSession(refStrNickName).expired())
			continue;

		m_b���� = false;
		return;
	}

	LOG(INFO) << m_����.strSceneName << ",����";
}

void Space::Update()
{
	if (m_b����)
		return;

	EraseEntity(false);

	void CrowToolUpdate(Space & ref, CrowdToolState & refCrowdToolState);
	CrowToolUpdate(*this, *m_spCrowdToolState);
	CrowToolUpdate(*this, *m_spCrowdToolState����);
}

std::unordered_map<uint8_t, SpSpace> g_mapSpace;
std::unordered_map<std::string, SpSpace> g_mapSpace����;
const std::unordered_map<std::string, SpSpace>& Space::����ս��()
{
	return g_mapSpace����;
}

WpSpace Space::GetSpace����(const std::string& refStrPlayerNickName)
{
	auto itFind = g_mapSpace����.find(refStrPlayerNickName);
	if (g_mapSpace����.end() != itFind)
		return itFind->second;

	return {};
}

std::tuple<bool, WpSpace> Space::GetSpace����(const std::string& refStrPlayerNickName, const ��������& ����)
{
	auto itFind = g_mapSpace����.find(refStrPlayerNickName);
	if (g_mapSpace����.end() != itFind)
		return { false,itFind->second };

	auto pair = g_mapSpace����.insert({ refStrPlayerNickName, std::make_shared<Space, const ��������&>(����) });
	_ASSERT(pair.second);
	return { true,pair.first->second };
}
bool Space::DeleteSpace����(const std::string& refStrPlayerNickName)
{
	auto wp = GetSpace����(refStrPlayerNickName);
	if (wp.expired())
		return false;

	wp.lock()->OnDestory();
	auto sizeErase = g_mapSpace����.erase(refStrPlayerNickName);
	_ASSERT(1 == sizeErase);
	return true;
}
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
	for (auto [_, sp] : g_mapSpace����)
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
Space::SpacePlayer& Space::GetSpacePlayer(const std::string strPlayerNickName)
{
	return m_mapPlayer[strPlayerNickName];
}
Space::SpacePlayer& Space::GetSpacePlayer(const Entity& ref)
{
	if (ref.m_spPlayerNickName)
		return ref.m_refSpace.GetSpacePlayer(ref.m_spPlayerNickName->m_strNickName);

	LOG(ERROR) << ref.Id << ",�������";
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

		if (spEntity->m_spPlayerNickName)
		{
			m_mapPlayer[spEntity->m_spPlayerNickName->m_strNickName].Erase(spEntity->Id);
		}

		LOG(INFO) << "ɾ�����ڶ���," << spEntity->ͷ��Name() << ",Id=" << spEntity->Id << ",ɾ��ǰʣ��" << m_mapEntity.size();
		spEntity->OnDestroy();
		iter = m_mapEntity.erase(iter);
	}
}

CrowdToolState& Space::GetCrowdToolState(const ��λ���� ����)
{
	switch (����)
	{
	case �ɻ�:
	case �ɳ�:
	case ����:
		return *m_spCrowdToolState����;
	default:
		return *m_spCrowdToolState;
	}

}

int Space::Get���ﵥλ��(const ��λ���� ����)const
{
	return Get��λ��([����](const Entity& refEntity)
		{
			if (refEntity.m_spPlayerNickName)//��������Ƶĵ�λ
				return false;
			if (EntitySystem::Is��Դ(refEntity.m_����))
				return false;
			if (���� > ��λ����_Invalid_0 && ���� != refEntity.m_����)
				return false;

			return true;
		});
}

int Space::Get��Դ��λ��(const ��λ���� ����)
{
	return Get��λ��([����](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_sp��Դ)
				return false;

			return ���� == refEntity.m_sp��Դ->m_����;
		});
}

int Space::Get��λ��(const ��λ���� arg����)const
{
	return Get��λ��([arg����](const Entity& refEntity)
		{
			return refEntity.m_���� == arg����;
		});
}

int Space::Get��ҵ�λ��(const std::string& strPlayerNickName, const ��λ���� ����)
{
	return Get��λ��([&strPlayerNickName, ����](const Entity& refEntity)
		{
			if (nullptr == refEntity.m_spPlayerNickName)
				return false;

			if (strPlayerNickName != refEntity.m_spPlayerNickName->m_strNickName)
				return false;

			if (EntitySystem::Is�ӿ�(refEntity))
				return false;

			if (EntitySystem::Is���(refEntity))
				return false;

			if (���� != ��λ����::��λ����_Invalid_0 && ���� != refEntity.m_����)
				return false;

			if (refEntity.m_spBuilding && !refEntity.m_spBuilding->�����())
				return false;

			return true;
		});
}
int Space::Get��λ��(const std::function<bool(const Entity&)>& fun�Ƿ�ͳ�ƴ˵�λ)const
{
	int i32��λ��(0);
	for (const auto [k, spEntity] : m_mapEntity)
	{
		if (spEntity->IsDead())
			continue
			;
		if (fun�Ƿ�ͳ�ƴ˵�λ && !fun�Ƿ�ͳ�ƴ˵�λ(*spEntity))
			continue;

		++i32��λ��;
	}

	return i32��λ��;
}

void Space::AddEntity(SpEntity spNewEntity, const int32_t i32��Ұ��Χ)
{
	if (EntitySystem::Is�ӿ�(spNewEntity->m_����))
	{
		LOG(INFO) << m_����.strSceneName << ",����";
		m_b���� = false;
	}
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
	if (m_funCancel����)
	{
		m_funCancel����();
		m_funCancel���� = nullptr;
	}

	�������ȫ�˳�();
	EraseEntity(true);
}

Space::SpacePlayer::SpacePlayer() :m_map����״̬{ {��ս��,{}}, {ǹ��,{}} }
{

}

bool Space::SpacePlayer::�ѽ���(const ��λ���� ����) const
{
	auto iterFind = m_map����״̬.find(����);
	if (iterFind == m_map����״̬.end())
		return true;//�������

	return iterFind->second.b�ѽ���;
}

bool Space::SpacePlayer::�������(const ��λ���� ����, Entity& refEntity�ڴ˽����н���)
{
	auto iterFind = m_map����״̬.find(����);
	CHECK_RET_FALSE(iterFind != m_map����״̬.end());
	auto& ref����״̬ = iterFind->second;

	CHECK_RET_FALSE(!ref����״̬.b�ѽ���);
	CHECK_WP_RET_FALSE(ref����״̬.wpEntity�ڴ˽����н���);
	auto& ref�������� = *ref����״̬.wpEntity�ڴ˽����н���.lock();

	CHECK_RET_FALSE(&ref�������� == &refEntity�ڴ˽����н���);

	ref����״̬.wpEntity�ڴ˽����н���.reset();
	ref����״̬.b�ѽ��� = true;

	return true;
}

bool Space::SpacePlayer::��ʼ������λ(const ��λ���� ����, Entity& refEntity�ڴ˽����н���)
{
	auto iterFind = m_map����״̬.find(����);
	CHECK_RET_FALSE(iterFind != m_map����״̬.end());
	auto& ref����״̬ = iterFind->second;

	if (ref����״̬.b�ѽ���)
	{
		PlayerGateSession_Game::��������Buzz(EntitySystem::GetNickName(refEntity�ڴ˽����н���), "�����ظ�����");
		return false;
	}

	if (!ref����״̬.wpEntity�ڴ˽����н���.expired())
	{
		PlayerGateSession_Game::��������Buzz(EntitySystem::GetNickName(refEntity�ڴ˽����н���), "���ڽ���");
		return false;
	}

	ref����״̬.wpEntity�ڴ˽����н��� = refEntity�ڴ˽����н���.shared_from_this();

	return true;
}

/// <summary>
/// �ܷ������ڽ������жϣ����������жϣ�����ֻ������
/// </summary>
/// <param name="����"></param>
/// <param name="����"></param>
/// <param name="refEntity�ڴ˽����н���"></param>
/// <returns></returns>
bool Space::SpacePlayer::��ʼ������λ����(const ��λ���� ��λ, const ��λ�������� ����, Entity& refEntity�ڴ˽����н���)
{
	auto& ref��λ���Եȼ� = m_map��λ��������״̬[��λ][����];
	if (!ref��λ���Եȼ�.wpEntity�ڴ˽���������.expired())
	{
		PlayerGateSession_Game::��������Buzz(EntitySystem::GetNickName(refEntity�ڴ˽����н���), "��������");
		return false;
	}

	ref��λ���Եȼ�.wpEntity�ڴ˽��������� = refEntity�ڴ˽����н���.shared_from_this();

	return true;
}

bool Space::SpacePlayer::������λ�������(const ��λ���� ��λ, const ��λ�������� ����, Entity& refEntity�ڴ˽���������)
{
	auto& ref��λ���Եȼ� = m_map��λ��������״̬[��λ][����];
	CHECK_WP_RET_FALSE(ref��λ���Եȼ�.wpEntity�ڴ˽���������);
	auto& ref���� = *ref��λ���Եȼ�.wpEntity�ڴ˽���������.lock();

	CHECK_RET_FALSE(&ref���� == &refEntity�ڴ˽���������);

	ref��λ���Եȼ�.wpEntity�ڴ˽���������.reset();
	++ref��λ���Եȼ�.u16�ȼ�;

	return true;
}

uint16_t Space::SpacePlayer::��λ���Եȼ�(const ��λ���� ��λ, const ��λ�������� ����)const
{
	auto iterFind��λ = m_map��λ��������״̬.find(��λ);
	CHECK_RET_DEFAULT(iterFind��λ == m_map��λ��������״̬.end());
	auto iterFind���� = iterFind��λ->second.find(����);
    CHECK_RET_DEFAULT(iterFind���� == iterFind��λ->second.end());

	return iterFind����->second.u16�ȼ�;
}

void Space::SpacePlayer::OnDestroy(const bool bɾ��������е�λ, Space& refSpace, const std::string& refStrNickName)
{
	auto mapLocal = m_mapWpEntity;//������ForEach��ɾ������
	for (auto [_, wp] : mapLocal)
	{
		//_ASSERT(!wp.expired());
		if (wp.expired())
		{
			LOG(ERROR) << "ɾ�˵�λ����������ûɾ";
			continue;
		}
		auto& ref = *wp.lock();
		if (bɾ��������е�λ || EntitySystem::Is�ӿ�(ref))//ɾ����λ
		{
			if (ref.m_refSpace.GetEntity(ref.Id).expired())
			{
				LOG(INFO) << "�����ǵر���ı�" << ref.ͷ��Name();
				continue;
			}

			if (EntitySystem::Is�ӿ�(ref))
				ref.Broadcast<MsgSay>({ .content = StrConv::GbkToUtf8(refStrNickName + " �뿪��") });

			LOG(INFO) << "m_mapEntity.size=" << ref.m_refSpace.m_mapEntity.size();
			ref.OnDestroy();

			auto countErase = m_mapWpEntity.erase(ref.Id);
			_ASSERT(1 == countErase);

			countErase = ref.m_refSpace.m_mapEntity.erase(ref.Id);
			_ASSERT(1 == countErase);
		}
		else//��ɾ��ֻɾ��Session����
		{
			ref.m_spPlayer.reset();
			//refSpace.m_map������PlayerEntity[refStrNickName].insert({ sp->Id,sp });
		}
	}

	//m_mapWpEntity.clear();
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

std::weak_ptr<PlayerGateSession_Game> GetPlayerGateSession(const std::string& refStrNickName);

WpEntity Space::����λ(Entity& ref�ӿ�, const std::string& refStrNickName, const ��λ���� ����, const Position& refPos, bool b�����ӿ�)
{
	auto wp = ����λ(ref�ӿ�.m_spPlayer, refStrNickName, refPos, ����);
	CHECK_WP_RET_DEFAULT(wp);
	if (b�����ӿ�)
	{
		auto wpSession = GetPlayerGateSession(refStrNickName);
		CHECK_WP_RET_DEFAULT(wpSession);

		wpSession.lock()->Send�����ӿ�(*wp.lock());
	}
	return wp;
}

WpEntity Space::����λ(std::shared_ptr<PlayerComponent>& refSpPlayer���ܿ�, const std::string& strNickName, const Position& pos, const ��λ���� ����)
{
	��λ::���λ���� �;
	��λ::��λ���� ��λ;
	��λ::�������� ����;
	��λ::ս������ ս��;
	CHECK_RET_DEFAULT(��λ::Find���λ����(����, �));
	CHECK_RET_DEFAULT(��λ::Find��λ����(����, ��λ));
	CHECK_RET_DEFAULT(��λ::Find��������(����, ����));
	CHECK_RET_DEFAULT(��λ::Findս������(����, ս��));
	SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����&>(
		pos, *this, std::forward<const ��λ����&&>(����), ��λ);
	PlayerComponent::AddComponent(*spNewEntity, refSpPlayer���ܿ�, strNickName);
	switch (����)
	{
	case ���:
		break;
	case �׳�:
		DefenceComponent::AddComponent(*spNewEntity, ����.u16��ʼHp);
		break;
	default:
		DefenceComponent::AddComponent(*spNewEntity, ����.u16��ʼHp);
		AttackComponent::AddComponent(*spNewEntity);
		break;
	}
	switch (����)
	{
	case ���:
		break;
	default:
		��Component::AddComponent(*spNewEntity);
		break;
	}

	m_mapPlayer[strNickName].m_mapWpEntity[spNewEntity->Id] = spNewEntity;//�Լ����Ƶĵ�λ
	AddEntity(spNewEntity);//ȫ��ͼ��λ
	spNewEntity->m_�ٶ�ÿ֡�ƶ����� = ս��.fÿ֡�ƶ�����;
	PlayerComponent::��������(*spNewEntity, �.str�볡����); //SCV, good to go, sir. SCV���Կ�����
	switch (����)
	{
	case ���̳�:
	case ����:
		�ɼ�Component::AddComponent(*spNewEntity);
		�콨��Component::AddComponent(*spNewEntity);
		break;
	case �׳�:
		����λComponent::AddComponent(*spNewEntity);
		break;
	default:break;
	}

	spNewEntity->BroadcastEnter();
	PlayerComponent::Send��Դ(*spNewEntity);

	return spNewEntity;
}

bool CrowdTool����ֱ��(CrowdToolState& refCrowTool, const Position& pos��ʼ, const Position& posĿ��);
bool Space::CrowdTool����ֱ��(const Position& pos��ʼ, const Position& posĿ��)
{
	return ::CrowdTool����ֱ��(*m_spCrowdToolState, pos��ʼ, posĿ��);
}
bool Space::�ɷ��ý���(const Position& refPos, float f��߳�)
{
	const Position pos���� = { refPos.x - f��߳�, refPos.z - f��߳� };
	const Position pos���� = { refPos.x + f��߳�, refPos.z + f��߳� };
	const Position pos���� = { refPos.x - f��߳�, refPos.z + f��߳� };
	const Position pos���� = { refPos.x + f��߳�, refPos.z - f��߳� };
	if (!CrowdTool��վ��(pos����))return false;
	if (!CrowdTool��վ��(pos����))return false;
	if (!CrowdTool��վ��(pos����))return false;
	if (!CrowdTool��վ��(pos����))return false;

	//����ȫ��ͼ���н����ж��ص�
	for (const auto& kv : m_mapEntity)
	{
		auto& refEntity = *kv.second;
		const auto& refPosOld = refEntity.Pos();
		bool CrowdTool�жϵ�λ�ص�(const Position & refPosOld, const Position & refPosNew, const float f��߳�);
		if (CrowdTool�жϵ�λ�ص�(refPos, refPosOld, f��߳�))
			return false;
	}


	return	CrowdTool����ֱ��(pos����, pos����)
		&& CrowdTool����ֱ��(pos����, pos����)
		&& CrowdTool����ֱ��(pos����, pos����)
		&& CrowdTool����ֱ��(pos����, pos����)
		&& CrowdTool����ֱ��(pos����, pos����)
		&& CrowdTool����ֱ��(pos����, pos����)
		;
}