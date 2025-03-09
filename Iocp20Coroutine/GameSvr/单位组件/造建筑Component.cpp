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
#include "������Component.h"

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
		m_set��������.insert(����);
		m_set��������.insert(��);
		m_set��������.insert(����);
		m_set��������.insert(�ر�);
		m_set��������.insert(������);
		m_set��������.insert(����);
		m_set��������.insert(�ع���);
		break;
	case ����:
		m_set��������.insert(������);
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
	if (!m_refEntity.m_spAttack)
	{
		_ASSERT(false);
		co_return true;
	}

	PlayerComponent::��������(m_refEntity, "����/����Ů���ɰ���");

	if (co_await AiCo::WalkToPos(m_refEntity, pos, m_cancel�콨��, ����.f��߳� + m_refEntity.m_spAttack->m_ս������.f��������))
		co_return true;

	//Ȼ��ʼ��Ǯ����
	//CHECK_CO_RET_FALSE(m_refEntity.m_spPlayer);
	auto wpEntity���� = AddBuilding(����, pos);
	if (wpEntity����.expired())
		co_return false;

	auto &ref���� = *wpEntity����.lock();
	CHECK_CO_RET_FALSE(ref����.m_spBuilding);

	if (���̳� == m_refEntity.m_����)
	{
		if (co_await Co�������(wpEntity����, m_cancel�콨��))
			co_return true;
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
	PlayerComponent::��������(m_refEntity, "TSCTra00");

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
	if (!��λ::Find������λ����(����, ����))
	{
		return{};
	}
	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!m_refEntity.m_refSpace.�ɷ��ý���(pos, ����.f��߳�))
	{
		//��������("TSCErr00", "���赲���޷�����");//��Err00�� I can't build it, something's in the way. ��û�����⽨���ж�������
		PlayerComponent::��������(m_refEntity, "����/�޷������ｨ��ɰ���", "���赲���޷�����");
		return{};
	}
	if (����.����.u16����ȼ���� > Space::GetSpacePlayer(m_refEntity).m_u32ȼ����)
	{
		//std::ostringstream oss;
		PlayerComponent::��������(m_refEntity, "����/ȼ������ɰ���", "ȼ�����㣬�޷�����");// << ����.����.u16����ȼ����;//(low error beep) Insufficient Vespene Gas.������ 
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
		PlayerComponent::��������(m_refEntity, "����/�������ɰ���", "��������޷�����");//Sayϵͳ("��������" + ����.����.u16���ľ����);

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

WpEntity �콨��Component::��������(Space& refSpace, const Position& pos, const ��λ���� ����, SpPlayerComponent spPlayer, const std::string& strPlayerNickName)
{
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		return{};
	}
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����& >(
		pos, refSpace, std::forward<const ��λ����&&>(����), ����.����);
	//spNewEntity->AddComponentAttack();
	���ݽ�������AddComponent(refSpace, ����, *spNewEntity, spPlayer, strPlayerNickName, ����);


	spNewEntity->BroadcastEnter();
	return spNewEntity;
}

void �콨��Component::���ݽ�������AddComponent(Space& refSpace, const ��λ���� ����, Entity& refNewEntity, SpPlayerComponent spPlayer, const std::string& strPlayerNickName, const ��λ::������λ����& ����)
{
	if (spPlayer)
		PlayerComponent::AddComponent(refNewEntity, spPlayer, spPlayer->m_refSession.NickName());

	BuildingComponent::AddComponent(refNewEntity, ����.f��߳�);

	switch (����)
	{
	case ����:
	case ����:
	case ����:
	case �ع���:
		����λComponent::AddComponent(refNewEntity, ����);
		break;
	case �ر�:
		�ر�Component::AddComponet(refNewEntity);
		break;
	case ��:
		break;
	case ������:
	{
		using namespace std;
		//						���� ���� �˺� ����
		��λ::ս������ ս�� = { 20,	20,		4,	0,	"", 0ms,"pao002",400ms,"��Ч/PhoFir00","��Ч/explo1","Take 001", 400ms };
		AttackComponent::AddComponent(refNewEntity, ����, ս��);
	}
	break;
	case ������:
		������Component::AddComponet(refNewEntity);
	break;
	}
	DefenceComponent::AddComponent(refNewEntity, ����.����.u16��ʼHp);
	refSpace.m_mapPlayer[strPlayerNickName].m_mapWpEntity[refNewEntity.Id] = refNewEntity.shared_from_this();//�Լ����Ƶĵ�λ
	refSpace.AddEntity(refNewEntity.shared_from_this());
}
