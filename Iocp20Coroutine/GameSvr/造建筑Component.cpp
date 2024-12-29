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

void �콨��Component::AddComponent(Entity& refEntity, const ���λ���� ����)
{
	refEntity.m_sp�콨�� = std::make_shared<�콨��Component, Entity&, const ���λ����>(refEntity, std::forward<const ���λ����&&>(����));
}

bool �콨��Component::���ڽ���(Entity& refEntity)
{
	if (!refEntity.m_sp�콨��)
		return false;

	return refEntity.m_sp�콨��->m_cancel�콨��.operator bool();
}

�콨��Component::�콨��Component(Entity& refEntity, const ���λ���� ����) :m_refEntity(refEntity)
{
	switch (����)
	{
	case ���̳�:
		m_set��������.insert(����);
		m_set��������.insert(��);
		m_set��������.insert(����);
		m_set��������.insert(�ر�);
		break;
	default:
		break;
	}
}

�콨��Component::~�콨��Component()
{
	assert(!m_cancel�콨��.operator bool());
}

CoTaskBool �콨��Component::Co�콨��(const Position pos, const ������λ���� ����)
{
	if (m_set��������.end() == m_set��������.find(����))
	{
		co_return false;
	}

	PlayerComponent::��������(m_refEntity, "����/����Ů���ɰ���");

	//���ߵ�Ŀ���
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		assert(false);
		co_return true;
	}
	if (!m_refEntity.m_spAttack)
	{
		assert(false);
		co_return true;
	}
	if (co_await AiCo::WalkToPos(m_refEntity, pos, m_cancel�콨��, ����.f��߳� + m_refEntity.m_spAttack->m_ս������.f��������))
		co_return true;

	//Ȼ��ʼ��Ǯ����
	CHECK_CO_RET_FALSE(m_refEntity.m_spPlayer);
	auto spEntity���� = co_await CoAddBuilding(����, pos);
	if (!spEntity����)
		co_return false;

	if (co_await Co�������(spEntity����, m_cancel�콨��))
		co_return true;

	co_return false;
}

CoTaskBool �콨��Component::Co�������(WpEntity wpEntity����, FunCancel& cancel)
{
	PlayerComponent::��������(m_refEntity, "TSCTra00");

	KeepCancel kc(cancel);
	std::weak_ptr<BuildingComponent> wpBuilding(wpEntity����.lock()->m_spBuilding);

	while (!wpBuilding.expired() && MAX����ٷֱ� > wpBuilding.lock()->m_n������Ȱٷֱ�)
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
		if (MAX����ٷֱ� <= refBuilding.m_n������Ȱٷֱ�)
			oss << "�������";
		else
			oss << "���ڽ���:" << refBuilding.m_n������Ȱٷֱ� << "%";

		assert(!wpEntity����.expired());
		EntitySystem::BroadcastEntity����(*wpEntity����.lock(), oss.str());
		PlayerComponent::Send��Դ(m_refEntity);
	}

	if (m_refEntity.m_spAttack)
	{
		switch (m_refEntity.m_spAttack->m_����)
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


CoTask<SpEntity> �콨��Component::CoAddBuilding(const ������λ���� ����, const Position pos)
{
	��λ::������λ���� ����;
	if (!��λ::Find������λ����(����, ����))
	{
		co_return{};
	}
	//Position pos = { 35,float(std::rand() % 60) - 30 };
	if (!m_refEntity.m_refSpace.�ɷ��ý���(pos, ����.f��߳�))
	{
		//��������("TSCErr00", "���赲���޷�����");//��Err00�� I can't build it, something's in the way. ��û�����⽨���ж�������
		PlayerComponent::��������(m_refEntity, "����/�޷������ｨ��ɰ���", "���赲���޷�����");
		co_return{};
	}
	if (����.����.u16����ȼ���� > Space::GetSpacePlayer(m_refEntity).m_u32ȼ����)
	{
		//std::ostringstream oss;
		PlayerComponent::��������(m_refEntity, "����/ȼ������ɰ���", "ȼ�����㣬�޷�����");// << ����.����.u16����ȼ����;//(low error beep) Insufficient Vespene Gas.������ 
		//Sayϵͳ(oss.str());
		co_return{};
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

		co_return{};
	}
	Space::GetSpacePlayer(m_refEntity).m_u32�����-= ����.����.u16���ľ����;

	PlayerComponent::Send��Դ(m_refEntity);

	//�ӽ���
	//CHECK_CO_RET_0(!m_wpSpace.expired());
	//auto spSpace = m_wpSpace.lock();
	auto spNewEntity = std::make_shared<Entity, const Position&, Space&, const ��λ::��λ����& >(
		pos, m_refEntity.m_refSpace, ����.����);
	//spNewEntity->AddComponentAttack();
	PlayerComponent::AddComponent(*spNewEntity, m_refEntity.m_spPlayer, EntitySystem::GetNickName(m_refEntity));
	BuildingComponent::AddComponent(*spNewEntity, ����, ����.f��߳�);
	switch (����)
	{
	case ����:
	case ����:
		����λComponent::AddComponent(*spNewEntity, ����);
		break;
	case �ر�:
		�ر�Component::AddComponet(*spNewEntity);
		break;
	case ��:break;
	}
	DefenceComponent::AddComponent(*spNewEntity, ����.����.u16��ʼHp);
	//spNewEntity->m_spBuilding->m_fun����λ = ����.fun���;
	Space::GetSpacePlayer(m_refEntity).m_mapWpEntity[spNewEntity->Id] = spNewEntity;//�Լ����Ƶĵ�λ
	//spSpace->m_mapEntity.insert({ (int64_t)spNewEntity.get() ,spNewEntity });//ȫ��ͼ��λ
	m_refEntity.m_refSpace.AddEntity(spNewEntity);

	spNewEntity->BroadcastEnter();
	PlayerComponent::Send��Դ(m_refEntity);
	co_return spNewEntity;
}