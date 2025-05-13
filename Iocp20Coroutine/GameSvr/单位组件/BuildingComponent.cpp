#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "��ʱ�赲Component.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "��λ.h"

void BuildingComponent::AddComponent(Entity& refThis, float f��߳�)
{
	refThis.AddComponentOnDestroy(&Entity::m_upBuilding, refThis);
	if (̦�� != refThis.m_����)
		��ʱ�赲Component::AddComponent(refThis, f��߳�);
}

void BuildingComponent::StartCo�������()
{
	Co�������(m_cancel����).RunNew();
}

void BuildingComponent::ֱ�����()
{
	m_n������Ȱٷֱ� = MAX����ٷֱ�;
}

void BuildingComponent::OnEntityDestroy(const bool bDestroy)
{
	m_cancel����.TryCancel();
}

BuildingComponent::BuildingComponent(Entity& refEntity) :m_refEntity(refEntity), m_cancel����("m_cancel����")
{
	//if (!m_coAddMoney.Finished())
	//{
	//	LOG(INFO) << "ǰһ���콨��Э�̻�û����";
	//	return;
	//}
	//Co�������(m_cancel����).RunNew();
}

/// <summary>
/// ����Ҫռ�ù��̳��Ľ��죨����/��� �� ����/���� �Ľ������춼���Զ��ģ�
/// </summary>
/// <param name="cancel"></param>
/// <returns></returns>
CoTaskBool BuildingComponent::Co�������(FunCancel& cancel)
{
	KeepCancel kc(cancel);

	while (MAX����ٷֱ� > this->m_n������Ȱٷֱ�)
	{
		if (co_await CoTimer::WaitNextUpdate(cancel))
			co_return 0;

		++this->m_n������Ȱٷֱ�;

		std::ostringstream oss;
		if (MAX����ٷֱ� <= this->m_n������Ȱٷֱ�)
			oss << "�������";
		else
			oss << "���ڱ���:" << this->m_n������Ȱٷֱ� << "%";

		this->m_refEntity.Broadcast<MsgEntity����>({ .idEntity = this->m_refEntity.Id, .str���� = StrConv::GbkToUtf8(oss.str()) });
	}

	co_return 0;
}

bool BuildingComponent::�����() const
{
	return m_n������Ȱٷֱ� >= MAX����ٷֱ�;

}

float BuildingComponent::������߳�(const Entity& refEntity)
{
	��λ::������λ���� ����;
	if (!refEntity.m_upBuilding)
		return 0;

	if (��λ::Find������λ����(refEntity.m_����, ����))
		return ����.f��߳�;

	_ASSERT(false);
	return 0;
}