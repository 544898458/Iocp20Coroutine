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
		uint16_t u16���ľ����;
		uint16_t u16����ȼ����;
		uint16_t u16��ʼHp;
	};
	struct ������λ����
	{
		��λ���� ����;
		�������� ����;
		//BuildingComponent::Fun��� fun���;
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

