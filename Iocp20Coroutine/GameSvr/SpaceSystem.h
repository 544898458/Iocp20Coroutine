#pragma once
#include "SpEntity.h"
#include "��λ���/��Ŀ���߹�ȥComponent.h"

class Space;
struct Position;
enum ��λ����;
namespace SpaceSystem
{
	std::vector<SpEntity> AddMonster(Space& refSpace, const ��λ���� ����, const Position& refPos, const int count = 1, ��Ŀ���߹�ȥComponent::Fun��������Ŀ�� fun��������Ŀ�� = ��Ŀ���߹�ȥComponent::�����й�);
};

