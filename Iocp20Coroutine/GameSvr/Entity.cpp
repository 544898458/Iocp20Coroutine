#include "pch.h"
#include "Entity.h"
#include "GameSvrSession.h"
#include "Space.h"
#include "../CoRoutine/CoTimer.h"
#include "../CoRoutine/CoEvent.h"
#include "GameSvr.h"
#include "AiCo.h"
#include <cmath>
#include "��λ���/PlayerComponent.h"
#include "AiCo.h"
#include "../IocpNetwork/StrConv.h"
#include "��λ���/MonsterComponent.h"
#include "��λ���/AttackComponent.h"
#include "��λ���/BuildingComponent.h"
#include "PlayerGateSession_Game.h"
#include "��λ���/�ɼ�Component.h"
#include "��λ���/����λComponent.h"
#include "��λ���/DefenceComponent.h"
#include "��λ���/�ر�Component.h"
#include "��λ���/��Component.h"
#include "��λ���/�콨��Component.h"
#include "��λ���/AoiComponent.h"
#include "��λ���/����Ŀ��Component.h"
#include "��λ���/�泲Component.h"
#include "��λ���/������λComponent.h"
#include "��λ���/������λ����Component.h"
#include "EntitySystem.h"
#include "��λ���/PlayerNickNameComponent.h"
#include "../IocpNetwork/MsgPack.h"
#include <fstream>

using namespace std;

Entity::Entity(const Position& pos, Space& space, const ��λ���� ����, const ��λ::��λ����& ref����) :
	Id((uint64_t)this), m_����(ref����), m_refSpace(space), m_Pos(pos),
	m_����(����)
{
}

bool Entity::DistanceLessEqual(const Entity& refEntity, float fDistance)
{
	return this->Pos().DistanceLessEqual(refEntity.Pos(), fDistance);
}

float Entity::DistancePow2(const Entity& refEntity)const
{
	return this->Pos().DistancePow2(refEntity.Pos());
}
float Entity::Distance(const Entity& refEntity)const
{
	return std::sqrtf(DistancePow2(refEntity));
}

bool Entity::IsDead() const
{
	if (!m_spDefence)
		return false;
	return m_spDefence->IsDead();
}

Entity::~Entity()
{
	LOG(INFO) << "~Entity()," << Id;
}

struct SaveEntity
{
	��λ���� m_����;
	��λ::��λ���� m_����;
	Position m_Pos;
	std::string m_strNickName;
	MSGPACK_DEFINE(m_����, m_����, m_Pos, m_strNickName);
};

void Entity::Save(std::ofstream& refOf)
{
	SaveEntity save = { m_����,m_����, m_Pos, EntitySystem::GetNickName(*this) };
	MsgPack::SendMsgpack(save, [&refOf](const void* buf, int len) { refOf.write((const char*)buf, len); }, true);
}

bool Entity::Load(Space& refSpace, char(&buf)[1024], const uint16_t u16Size)
{
	SaveEntity load;
	CHECK_FALSE(MsgPack::RecvMsgpack(load, buf, u16Size));

	switch (load.m_����)
	{
	case ���:
		return false;
	default:
		break;
	}
	
	WpEntity wpNew;
	if (EntitySystem::Is����(load.m_����))
	{
		SpEntity spNewEntity = std::make_shared<Entity, const Position&, Space&, ��λ����, const ��λ::��λ����&>(
			load.m_Pos, refSpace, std::forward<��λ����&&>(load.m_����), load.m_����);
		PlayerComponent::AddComponent(*spNewEntity, {}, load.m_strNickName);
		�콨��Component::���ݽ�������AddComponent(refSpace, load.m_����, *spNewEntity, {}, load.m_strNickName);

		CHECK_NOTNULL_RET_FALSE(spNewEntity->m_spBuilding);
		wpNew = spNewEntity;
	}
	else if(EntitySystem::Is���λ(load.m_����)) {
		std::shared_ptr<PlayerComponent> spNull;
		��λ::���λ���� ���λ����;
		CHECK_FALSE(��λ::Find���λ����(load.m_����, ���λ����));

		wpNew = refSpace.����λ(spNull, load.m_strNickName, load.m_Pos, load.m_����);
	}
	else 
	{
		_ASSERT(false);
		return false;
	}
	
	CHECK_WP_RET_FALSE(wpNew);
	wpNew.lock()->OnLoad();
	return true;
}

//���̵߳��߳�ִ��
void Entity::Update()
{
	if (m_spAttack)
		m_spAttack->Update();

	if (m_sp�ر�)
		m_sp�ر�->Update();
}

bool Entity::IsEnemy(const Entity& refEntity)
{
	if (!m_spPlayerNickName && !refEntity.m_spPlayerNickName)
		return false;//���ǹ�

	if (!m_spPlayerNickName || !refEntity.m_spPlayerNickName)
		return true;//��һ���ǹ�

	//������ҵ�λ
	return m_spPlayerNickName->m_strNickName != refEntity.m_spPlayerNickName->m_strNickName;
}

void Entity::SetPos(const Position& refNewPos)
{
	if (m_Pos == refNewPos)
		return;

	if (m_sp��ʱ�赲)
		m_sp��ʱ�赲.reset();//�ƶ��ˣ��϶�Ҫɾ���赲

	if (m_upAoi)
	{
		m_upAoi->OnBeforeChangePos(refNewPos);
	}
	m_Pos = refNewPos;
	if (m_upAoi)
	{
		m_upAoi->OnAfterChangePos();
	}
}

float Entity::��������() const
{
	if (m_spAttack)
		return m_spAttack->m_ս������.f��������;

	return 0;
}
float Entity::�������() const
{
	if (m_spAttack)
		return m_spAttack->m_ս������.f�������;

	return 0;
}

void Entity::OnLoad() 
{
	if(m_spBuilding)
		m_spBuilding->ֱ�����();

	if (m_sp�泲)
		m_sp�泲->OnLoad();
}

void Entity::OnDestroy()
{
	BroadcastLeave();

	if (EntitySystem::Is�ӿ�(*this))
	{
		const auto sizeCount = m_refSpace.m_map�ӿ�.erase(Id);
		LOG_IF(ERROR, 1 != sizeCount) << "";
		_ASSERT(1 == sizeCount);
	}
	//Ӧ����proxy��ͬ���������ģ��������
	if (m_spAttack)
		m_spAttack->TryCancel(true);

	if (m_sp����λ)
		m_sp����λ->TryCancel(*this);

	if (m_sp�ɼ�)
		m_sp�ɼ�->m_TaskCancel.TryCancel();

	if (m_sp��)
		m_sp��->TryCancel();

	if (m_sp�ر�)
		m_sp�ر�->OnDestroy();

	if (m_sp��ʱ�赲)
		m_sp��ʱ�赲.reset();

	if (m_sp�콨��)
		m_sp�콨��->TryCancel();

	if (m_spBuilding)
		m_spBuilding->TryCancel();

	if (m_sp�泲)
		m_sp�泲->TryCancel();

	if (m_up����Ŀ��)
		m_up����Ŀ��->TryCancel();

	if (m_cancelDelete)
	{
		LOG(INFO) << "ȡ��ɾ��Э��";
		m_cancelDelete();
	}
	else
	{
		LOG(INFO) << "û��ɾ��Э��";
	}

	if (m_upAoi)
		m_upAoi->OnDestory();
}

void Entity::BroadcastLeave()
{
	LOG(INFO) << ͷ��Name() << "����Entity::BroadcastLeave," << Id;
	Broadcast(MsgDelRoleRet(Id));
}

const std::string& Entity::ͷ��Name()const
{
	if (m_spPlayerNickName)
		return m_spPlayerNickName->m_strNickName;

	if (m_sp��Դ)
	{
		static const std::string str("��Դ");
		return str;
	}

	if (��Ч == m_����)
	{
		static const std::string str("");
		return str;
	}

	{
		static const std::string str("����");
		return str;
	}
}


void Entity::BroadcastEnter()
{
	//LOG(INFO) << NickName() << "����Entity::BroadcastEnter," << Id;
	Broadcast(MsgAddRoleRet(*this));//�Լ��㲥������
	BroadcastNotifyPos();
	CoEvent<MyEvent::AddEntity>::OnRecvEvent({ weak_from_this() });
}

void Entity::BroadcastNotifyPos()
{
	Broadcast(MsgNotifyPos(*this));
}

void Entity::BroadcastChangeSkeleAnim(const std::string& refAniClipName, bool loop)
{
	Broadcast(MsgChangeSkeleAnim(*this, StrConv::GbkToUtf8(refAniClipName), loop));//������������
}

template<class T>
void Entity::Broadcast(const T& msg)
{
	if (!m_upAoi)
		return;

	auto wp = m_wpOwner;
	if (wp.expired())
		wp = weak_from_this();

	CHECK_WP_RET_VOID(wp);

	auto& ref�Լ���Owner = *wp.lock();
	for (auto [k, wp�ܿ�����] : ref�Լ���Owner.m_upAoi->m_map�ܿ����ҵ�)
	{
		CHECK_WP_CONTINUE(wp�ܿ�����);
		auto& refEntity = *wp�ܿ�����.lock();
		if (!EntitySystem::Is�ӿ�(refEntity))
			continue;

		if (refEntity.m_spPlayer)
			refEntity.m_spPlayer->m_refSession.Send(msg);
	}

	//m_refSpace.Broadcast(msg);
}

void Entity::DelayDelete(const uint32_t u32����)
{
	CoDelayDelete(std::chrono::milliseconds(u32����)).RunNew();
}
CoTaskBool Entity::CoDelayDelete(const std::chrono::system_clock::duration& dura)
{
	LOG(INFO) << "��ʼ��ʱɾ���Լ���Э��";
	//_ASSERT(!m_cancelDelete);//���ɲ���
	if (m_cancelDelete)
		co_return false;

	if (m_sp�ر�)
		m_sp�ر�->OnBeforeDelayDelete();

	using namespace std;
	if (co_await CoTimer::Wait(dura, m_cancelDelete))//�������������̴߳�ѭ����ÿ��ѭ������һ��
	{
		LOG(INFO) << "WaitDeleteЭ��ȡ����";
		co_return true;
	}

	m_bNeedDelete = true;
	co_return false;
}


WpEntity Entity::Get�����Entity֧�ֵر��еĵ�λ(const FindType bFindEnemy, std::function<bool(const Entity&)> fun��������)
{
	WpEntity wp = m_wpOwner;
	if (!m_refSpace.GetEntity(Id).expired())
	{
		wp = weak_from_this();
	}
	if (wp.expired())
		return {};

	return wp.lock()->Get�����Entity(bFindEnemy, fun��������);
}

WpEntity Entity::Get�����Entity(const FindType bFindEnemy, const ��λ���� Ŀ������)
{
	return Get�����Entity(Entity::����, [Ŀ������](const Entity& ref) {return ref.m_���� == Ŀ������; });
}

WpEntity Entity::Get�����Entity(const FindType findType)
{
	return Get�����Entity(findType, [](const Entity& ) {return true; });
}

WpEntity Entity::Get�����Entity(const FindType bFindEnemy, std::function<bool(const Entity&)> fun��������)
{
	if (!m_upAoi)
		return{};

	std::vector<std::pair<int64_t, WpEntity>> vecEnemy;
	std::copy_if(m_upAoi->m_map���ܿ�����.begin(), m_upAoi->m_map���ܿ�����.end(), std::back_inserter(vecEnemy),
		[bFindEnemy, this, &fun��������](const auto& pair)
		{
			auto& wp = pair.second;
			CHECK_FALSE(!wp.expired());
			Entity& ref = *wp.lock();
			if (ref.m_cancelDelete)
				return false;

			const auto bEnemy = ref.IsEnemy(*this);
			if (bFindEnemy == �з� && !bEnemy)
				return false;
			if (bFindEnemy == �ѷ� && bEnemy)
				return false;

			if (fun�������� && !fun��������(ref))
				return false;

			return &ref != this && !ref.IsDead();
		});

	if (vecEnemy.empty())
	{
		return {};
	}

	auto iterMin = std::min_element(vecEnemy.begin(), vecEnemy.end(), [this](const auto& pair1, const auto& pair2)
		{
			auto& sp1 = pair1.second;
			auto& sp2 = pair2.second;
			return this->DistancePow2(*sp1.lock()) < this->DistancePow2(*sp2.lock());
		});
	return iterMin->second.lock()->weak_from_this();
}
template void Entity::Broadcast(const MsgAddRoleRet& msg);
template void Entity::Broadcast(const MsgDelRoleRet& msg);
template void Entity::Broadcast(const MsgEntity����& msg);
template void Entity::Broadcast(const Msg��������& msg);
template void Entity::Broadcast(const Msg������Ч& msg);
template void Entity::Broadcast(const MsgSay& msg);