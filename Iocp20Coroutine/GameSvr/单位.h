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
		float f��߳� = 0;
	};
	struct ս������
	{
		float f�������;
		float f��������;
		float f�˺�;
		float fÿ֡�ƶ�����;
		std::string strǰҡ����;
		std::chrono::system_clock::duration dura��ʼ���Ź�������;
		std::string str��������;
		std::chrono::system_clock::duration dura��ʼ�˺�;
		std::string str������Ч;
		
	};
	struct ���λ����
	{
		��λ���� ����;
		�������� ����;
		ս������ ս��;
	};
	struct ������
	{
		��λ���� ����;
		ս������ ս��;
		uint16_t u16��ʼHp;
	};
	
	struct ��Դ��λ����
	{
		��λ���� ����;
	};

	bool Find������λ����(const ������λ���� ����, ������λ����& refOut);
	bool Find���λ����(const ���λ���� ����, ���λ����& refOut);
	bool Find��Դ��λ����(const ��Դ���� ����, ��Դ��λ����& refOut);
	bool Find������(const ���λ���� ����, ������& refOut);
};

