#pragma once
#include "SpEntity.h"
class Space;

namespace MyEvent
{
	struct AddEntity
	{
		WpEntity wpEntity;
	};
	struct MoveEntity
	{
		WpEntity wpEntity;
	};
	struct ��ʼ�ɼ������
	{
	};
	struct ��������˻ػ���
	{

	};
	bool SameSpace(const WpEntity& refWpEntity, const Space& refSpace);;
}

