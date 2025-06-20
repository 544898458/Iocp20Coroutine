#include "pch.h"
#include "SpaceSystem.h"
#include "Entity.h"
#include "Space.h"
#include "��λ���/�ɽ����λComponent.h"
#include "��λ���/AttackComponent.h"
#include "��λ���/DefenceComponent.h"
#include "��λ���/��Component.h"

std::vector<SpEntity> SpaceSystem::AddMonster(Space& refSpace, const ��λ���� ����, const Position& refPos, const int count, ��Ŀ���߹�ȥComponent::Fun��������Ŀ�� fun��������Ŀ��)
{
	std::vector<SpEntity> vecRet;
	��λ::������ ������;
	��λ::��λ���� ��λ����;
	��λ::ս������ ս������;
	CHECK_RET_DEFAULT(��λ::Find������(����, ������));
	CHECK_RET_DEFAULT(��λ::Find��λ����(����, ��λ����));
	CHECK_RET_DEFAULT(��λ::Findս������(����, ս������));
	for (int i = 0; i < count; ++i)
	{
		SpEntity spEntityMonster = std::make_shared<Entity, const Position&, Space&, const ��λ����, const ��λ::��λ����&>(
			refPos, refSpace, std::forward<const ��λ����&&>(����), ��λ����);

		if (���� == ����)
			�ɽ����λComponent::AddComponent(*spEntityMonster);
		else
		{
			AttackComponent::AddComponent(*spEntityMonster);
			CHECK_RET_DEFAULT(spEntityMonster->m_up��Ŀ���߹�ȥ);
			spEntityMonster->m_up��Ŀ���߹�ȥ->m_fun��������˴� = fun��������Ŀ��;
		}


		DefenceComponent::AddComponent(*spEntityMonster, ������.u16��ʼHp);
		��Component::AddComponent(*spEntityMonster);
		//AddComponent(*spEntityMonster);


		refSpace.AddEntity(spEntityMonster);
		//LOG(INFO) << "SpawnMonster:" << refSpace.m_mapEntity.size();
		spEntityMonster->BroadcastEnter();
		vecRet.emplace_back(spEntityMonster);
		//auto [pair, ok] = refSpace.m_mapPlayer[""].m_mapWpEntity.insert({ spEntityMonster->Id, spEntityMonster });//��������ǳ�Ϊ""�����
		//CHECK_NOT_RETURN(ok);
	}
	return vecRet;
}