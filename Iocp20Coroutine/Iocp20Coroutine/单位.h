#pragma once
#include "MyMsgQueue.h"
#include "BuildingComponent.h"
namespace ��λ
{
	struct ��λ����
	{
		std::string strName;
		std::string strPrefabName;
	};
	struct ��������
	{
		uint32_t u32���ľ����;
		uint32_t u32����ȼ����;
	};
	struct ������λ����
	{
		��λ���� ����;
		�������� ����;
		BuildingComponent::Fun��� fun���;
		/// <summary>
		/// ��̬�赲��԰�İ뾶�������εİ�߳���
		/// </summary>
		float f��߳�;
	};

	struct ���λ����
	{
		��λ���� ����;
		�������� ����;
		float f�������;
	};

	struct ��Դ��λ����
	{
		��λ���� ����;
	};

	bool Find������λ����(const ������λ���� ����, ������λ����& refOut);
	bool Find���λ����(const ���λ���� ����, ���λ����& refOut);
	bool Find��Դ��λ����(const ��Դ���� ����, ��Դ��λ����& refOut);
};

