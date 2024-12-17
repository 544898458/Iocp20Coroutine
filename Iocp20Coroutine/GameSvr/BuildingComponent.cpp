#include "pch.h"
#include "BuildingComponent.h"
#include "Entity.h"
#include "PlayerComponent.h"
#include "PlayerGateSession_Game.h"
#include "��ʱ�赲Component.h"
#include "../CoRoutine/CoTimer.h"
#include "../IocpNetwork/StrConv.h"
#include "��λ.h"

void BuildingComponent::AddComponent(Entity& refThis, PlayerGateSession_Game& refSession, const ������λ���� ����, float f��߳�)
{
	refThis.m_spBuilding = std::make_shared<BuildingComponent, PlayerGateSession_Game&, const ������λ����&, Entity&>(refSession, ����, refThis);
	��ʱ�赲Component::AddComponent(refThis, f��߳�);
}

BuildingComponent::BuildingComponent(PlayerGateSession_Game& refSession, const ������λ����& ����, Entity& refEntity) :m_����(����), m_refEntity(refEntity)
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
			oss << "���ڽ���:" << this->m_n������Ȱٷֱ� << "%";

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
	if (!refEntity.m_spBuilding)
		return 0;

	if (��λ::Find������λ����(refEntity.m_spBuilding->m_����, ����))
		return ����.f��߳�;

	assert(false);
	return 0;
}