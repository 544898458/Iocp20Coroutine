#pragma once
#include "MyMsgQueue.h"
#include "BuildingComponent.h"
namespace ��λ
{
	struct ��λ����
	{
		std::string strName;
		std::string strPrefabName;
		uint32_t u32����Ǯ;
	};

	struct ������λ����
	{
		��λ���� ����;
		BuildingComponent::Fun��� fun���;
	};

	struct ���λ����
	{
		��λ���� ����;
	};

	bool Find������λ����(const ������λ���� ����, ������λ����& refOut);
	bool Find���λ����(const ���λ���� ����, ���λ����& refOut);
};

