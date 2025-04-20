#include "pch.h"
#include "�콨��Component.h"
#include "��λ.h"
#include "AiCo.h"
#include "Entity.h"
#include "EntitySystem.h"
#include "PlayerComponent.h"
#include "AttackComponent.h"
#include "PlayerGateSession_Game.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "����λComponent.h"
#include "�ر�Component.h"
#include "DefenceComponent.h"
#include "BuildingComponent.h"
#include "�泲Component.h"
#include "������λComponent.h"
#include "������λ����Component.h"
#include "̦��Component.h"
#include "̦������Component.h"
#include "̦��Component.h"
#include "��Component.h"
#include "AoiComponent.h"
#include "BuffComponent.h"
#include "��̦���ͳ�����ѪComponent.h"

void �콨��Component::AddComponent(Entity& refEntity)
{
	refEntity.m_sp�콨�� = std::make_shared<�콨��Component, Entity&>(refEntity);
}

bool �콨��Component::���ڽ���(const Entity& refEntity)
{
	if (!refEntity.m_sp�콨��)
		return false;

	return refEntity.m_sp�콨��->m_cancel�콨��.operator bool();
}

�콨��Component::�콨��Component(Entity& refEntity) :m_refEntity(refEntity)
{
	switch (refEntity.m_����)
	{
	case ���̳�:
		m_set�������� = { ����,��,��Ӫ,�ر�,��̨,����,�س��� };
		break;
	case ����:
		m_set�������� = { �泲,��Ӫ,����,��̬Դ,��̨,̫�� };
		break;
	default:
		break;
	}
}

�콨��Component::~�콨��Component()
{
	_ASSERT(!m_cancel�콨��.operator bool());
}

bool �콨��Component::����(const ��λ���� ����)const
{
	return m_set��������.end() != m_set��������.find(����);
}

void �콨��Component::�콨��(const Position pos, const ��λ���� ����)
{
	Co�콨��(pos, ����).RunNew();
}

CoTaskBool �콨��Component::Co�콨��(const Position pos, const ��λ���� ����)
{
	if (!����(����))
	{
		co_return false;
	}

	//���ߵ�Ŀ���
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		_ASSERT(false);
		co_return true;
	}
	if (!m_refEntity.m_refSpace.�ɷ��ý���(pos, ����.f��߳�))
	{
		//��������("TSCErr00", "���赲���޷�����");//��Err00�� I can't build it, something's in the way. ��û�����⽨���ж�������
		PlayerComponent::��������(m_refEntity, "����/�޷������ｨ��ɰ���", "���赲���޷�����");
		co_return true;
	}

	//ֻ�г��ཨ����λ��������̦���ϣ����ұ�������̦���ϣ��泲���⣩
	{
		bool b�˴���̦�� = �˴���̦����(pos);
		if (��λ::Is��(����))
		{
			if (�泲 != ���� && !b�˴���̦��)
			{
				PlayerGateSession_Game::��������Buzz(m_refEntity, "������̦���ĵط�����");
				co_return false;
			}
		}
		else
		{
			if (b�˴���̦��)
			{
				PlayerGateSession_Game::��������Buzz(m_refEntity, "����û��̦���ĵط�����");
				co_return false;
			}
		}
	}

	PlayerComponent::��������(m_refEntity, "����/����Ů���ɰ���");

	EntitySystem::BroadcastEntity����(m_refEntity, "�������");

	CHECK_CO_RET_FALSE(m_refEntity.m_sp��);
	if (co_await m_refEntity.m_sp��->WalkToPos(pos, m_cancel�콨��, ����.f��߳� + 1))
		co_return true;

	//Ȼ��ʼ��Ǯ����
	//CHECK_CO_RET_FALSE(m_refEntity.m_spPlayer);
	auto wpEntity���� = AddBuilding(����, pos);
	if (wpEntity����.expired())
		co_return false;

	auto& ref���� = *wpEntity����.lock();
	CHECK_CO_RET_FALSE(ref����.m_spBuilding);

	if (���̳� == m_refEntity.m_����)
	{
		EntitySystem::BroadcastEntity����(m_refEntity, "���ڽ���");

		if (m_refEntity.m_���� == ���̳�)
			m_refEntity.BroadcastChangeSkeleAnim("2", true);

		if (co_await Co�������(wpEntity����, m_cancel�콨��))
			co_return true;

		EntitySystem::BroadcastEntity����(m_refEntity, "");
	}
	else
	{
		using namespace std;
		m_refEntity.CoDelayDelete(1ms).RunNew();
		ref����.m_spBuilding->StartCo�������();
	}

	co_return false;
}

CoTaskBool �콨��Component::Co�������(WpEntity wpEntity����, FunCancel& cancel)
{
	PlayerComponent::��������(m_refEntity, "��Ч/��׮�������");

	KeepCancel kc(cancel);
	std::weak_ptr<BuildingComponent> wpBuilding(wpEntity����.lock()->m_spBuilding);

	while (!wpBuilding.expired() && BuildingComponent::MAX����ٷֱ� > wpBuilding.lock()->m_n������Ȱٷֱ�)
	{
		if (co_await CoTimer::WaitNextUpdate(cancel))
			co_return true;
		if (wpBuilding.expired())
			co_return true;

		BuildingComponent& refBuilding = *wpBuilding.lock();

		if (refBuilding.m_n������Ȱٷֱ� > 0 && 0 == refBuilding.m_n������Ȱٷֱ� % 20)
			PlayerComponent::��������(m_refEntity, "EDrRep00");//Repair

		++refBuilding.m_n������Ȱٷֱ�;

		std::ostringstream oss;
		if (BuildingComponent::MAX����ٷֱ� <= refBuilding.m_n������Ȱٷֱ�)
			oss << "�������";
		else
			oss << "���ڽ���:" << refBuilding.m_n������Ȱٷֱ� << "%";

		_ASSERT(!wpEntity����.expired());
		EntitySystem::BroadcastEntity����(*wpEntity����.lock(), oss.str());
		PlayerComponent::Send��Դ(m_refEntity);
	}

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_����)
		{
		case ���̳�:PlayerComponent::��������(m_refEntity, "����/�������Ů���ɰ���"); break;
		default:break;
		}
	}
	co_return 0;
}

void �콨��Component::TryCancel()
{
	if (m_cancel�콨��)
	{
		//LOG(INFO) << "����m_cancel";
		m_cancel�콨��();
	}
	else
	{

	}
}


WpEntity �콨��Component::AddBuilding(const ��λ���� ����, const Position pos)
{
	��λ::������λ���� ����;
	��λ::��λ���� ��λ;
	��λ::�������� ����;
	CHECK_RET_DEFAULT(��λ::Find������λ����(����, ����));
	CHECK_RET_DEFAULT(��λ::Find��λ����(����, ��λ));
	CHECK_RET_DEFAULT(��λ::Find��������(����, ����));

	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!m_refEntity.m_refSpace.�ɷ��ý���(pos, ����.f��߳�))
	{
		//��������("TSCErr00", "���赲���޷�����");//��Err00�� I can't build it, something's in the way. ��û�����⽨���ж�������
		PlayerComponent::��������(m_refEntity, "����/�޷������ｨ��ɰ���", "���赲");
		return{};
	}
	if (����.����.u16����ȼ���� > Space::GetSpacePlayer(m_refEntity).m_u32ȼ����)
	{
		//std::ostringstream oss;
		PlayerComponent::��������(m_refEntity, "����/ȼ������ɰ���", "ȼ�������޷�����");// << ����.����.u16����ȼ����;//(low error beep) Insufficient Vespene Gas.������ 
		//Sayϵͳ(oss.str());
		return{};
	}
	Space::GetSpacePlayer(m_refEntity).m_u32ȼ���� -= ����.����.u16����ȼ����;
	//auto iterNew = m_vecFunCancel.insert(m_vecFunCancel.end(), std::make_shared<FunCancel>());//���ܴ�������ݿ��ܵ������ú�ָ��ʧЧ
	//auto [stop, responce] = co_await AiCo::ChangeMoney(*this, ����.����.u16���ľ����, false, **iterNew);
	//LOG(INFO) << "Э��RPC����,error=" << responce.error << ",finalMoney=" << responce.finalMoney;
	//if (stop)
	//{
	//	m_u32ȼ���� += ����.����.u16����ȼ����;//����ȼ����
	//	co_return{};
	//}
	//if (0 != responce.error)
	if (����.����.u16���ľ���� > Space::GetSpacePlayer(m_refEntity).m_u32�����)
	{
		//LOG(WARNING) << "��Ǯʧ��,error=" << responce.error;
		//m_u32ȼ���� += ����.����.u16����ȼ����;//����ȼ����
		PlayerComponent::��������(m_refEntity, "����/�������ɰ���", "�������");

		return{};
	}
	Space::GetSpacePlayer(m_refEntity).m_u32����� -= ����.����.u16���ľ����;

	PlayerComponent::Send��Դ(m_refEntity);

	//�ӽ���
	//CHECK_CO_RET_0(!m_wpSpace.expired());
	//auto spSpace = m_wpSpace.lock();
	auto wpNew = ��������(m_refEntity.m_refSpace, pos, ����, m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity));
	PlayerComponent::Send��Դ(m_refEntity);
	return wpNew;
}

bool �콨��Component::�˴���̦����(const Position pos)
{
	const auto [id����, _, __] = AoiComponent::����(pos);
	auto mapWp = m_refEntity.m_refSpace.m_map�ܿ�����һ��[id����];
	for (const auto& [id, wp] : mapWp)
	{
		CHECK_WP_CONTINUE(wp);
		const auto& refEntity = *wp.lock();
		if (refEntity.m_up̦�� && refEntity.m_up̦��->�ڰ뾶��(pos))
			return true;
	}
	return false;
}

WpEntity �콨��Component::��������(Space& refSpace, const Position& pos, const ��λ���� ����, SpPlayerComponent spPlayer, const std::string& strPlayerNickName)
{
	��λ::������λ���� ����;
	��λ::��λ���� ��λ;
	��λ::�������� ����;
	CHECK_RET_DEFAULT(��λ::Find������λ����(����, ����));
	CHECK_RET_DEFAULT(��λ::Find��λ����(����, ��λ));
	CHECK_RET_DEFAULT(��λ::Find��������(����, ����));

	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����& >(
		pos, refSpace, std::forward<const ��λ����&&>(����), ��λ);
	//spNewEntity->AddComponentAttack();
	���ݽ�������AddComponent(refSpace, ����, *spNewEntity, spPlayer, strPlayerNickName);

	spNewEntity->BroadcastEnter();
	return spNewEntity;
}

void �콨��Component::���ݽ�������AddComponent(Space& refSpace, const ��λ���� ����, Entity& refNewEntity, SpPlayerComponent spPlayer, const std::string& strPlayerNickName)
{
	��λ::������λ���� ����;
	��λ::��λ���� ��λ;
	��λ::�������� ����;
	CHECK_RET_VOID(��λ::Find������λ����(����, ����));
	CHECK_RET_VOID(��λ::Find��λ����(����, ��λ));
	CHECK_RET_VOID(��λ::Find��������(����, ����));

	if (spPlayer)
		PlayerComponent::AddComponent(refNewEntity, spPlayer, spPlayer->m_refSession.NickName());

	BuildingComponent::AddComponent(refNewEntity, ����.f��߳�);

	switch (����)
	{
	case ��Ӫ:
		������λComponent::AddComponent(refNewEntity);
		������λ����Component::AddComponent(refNewEntity);
		break;
	case ��Ӫ:
		������λComponent::AddComponent(refNewEntity);
		����λComponent::AddComponent(refNewEntity);
		������λ����Component::AddComponent(refNewEntity);
		break;
	case ����:
		����λComponent::AddComponent(refNewEntity);
		break;
	case ����:
	case �س���:
		������λ����Component::AddComponent(refNewEntity);
		����λComponent::AddComponent(refNewEntity);
		break;
	case ����:
		������λ����Component::AddComponent(refNewEntity);
		break;
	case �ر�:
		�ر�Component::AddComponent(refNewEntity);
		break;
	case ��:
		break;
	case ��̨:
		AttackComponent::AddComponent(refNewEntity);
		break;
	case �泲:
		�泲Component::AddComponent(refNewEntity);
		̦������Component::AddComponent(refNewEntity);
		break;
	case ̫��:
		̦������Component::AddComponent(refNewEntity);
		break;
	case ̦��:
		̦��Component::AddComponent(refNewEntity);
		break;
	default:
		break;
	}
	if (̦�� != ����)
	{
		DefenceComponent::AddComponent(refNewEntity, ����.u16��ʼHp);
		BuffComponent::AddComponent(refNewEntity);
	}
	if (��λ::Is��(����) && EntitySystem::Is����(����))
	{
        ��̦���ͳ�����ѪComponent::AddComponent(refNewEntity);
	}

	refSpace.m_mapPlayer[strPlayerNickName].m_mapWpEntity[refNewEntity.Id] = refNewEntity.shared_from_this();//�Լ����Ƶĵ�λ
	const int i32��Ұ��Χ = ̦�� == ���� ? ̦��Component::MAX�뾶 : 0;
	refSpace.AddEntity(refNewEntity.shared_from_this(), i32��Ұ��Χ);

	if (�泲 == ���� || ̫�� == ����)
	{
		auto wp̦�� = �콨��Component::��������(refSpace, refNewEntity.Pos(), ̦��, spPlayer, strPlayerNickName);
		CHECK_WP_RET_VOID(wp̦��);
		auto& ref̦�� = *wp̦��.lock();

		CHECK_RET_VOID(ref̦��.m_up̦��->m_wp���Ž���.expired());
		ref̦��.m_up̦��->m_wp���Ž��� = refNewEntity.shared_from_this();

		CHECK_RET_VOID(refNewEntity.m_up̦������->m_wp̦��.expired());
		refNewEntity.m_up̦������->m_wp̦�� = wp̦��;
	}
}
