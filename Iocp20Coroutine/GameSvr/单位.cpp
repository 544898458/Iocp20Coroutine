#include "pch.h"
#include "��λ.h"
#include <unordered_map>
namespace ��λ
{

	std::unordered_map<������λ����, ������λ����> g_map������λ���� =
	{
		//								ѡ����Ч		��  ȼ HP	��߳�
		{����, {{"����","����/����",	"tcsWht00"},	{50, 0,200}, 5.0f}},
		{����, {{"����","����/����",	"tacWht00"},	{25, 9,100}, 4.0f} },
		{��, {{"��","����/��",	"tclWht00"},	{20, 3,50},  3.f} },
		{�ر�, {{"�ر�","����/�ر�",	"��Ч/BUTTON"},	{30,11,500}, 3.0f} },
	};
	using namespace std;
	std::unordered_map<���λ����, ���λ����> g_map���λ���� =
	{
		//											ѡ����Ч			  �� ȼ HP ���� ���� �˺� ���� ǰҡ���� dura��ʼ���Ź������� �������� dura��ʼ�˺� ������Ч		  ������Ч	��������
		{���̳�,{{"���̳�"	, "altman-yellow",		"����/��Ů���ɰ���"},{ 3, 0,20},{8 , 4,  1,	0.4f,		"",		0s,				  "attack",		500ms,	 "TSCMin00",	 "TSCDth00","died"}}},
		{��,	{{"��"		, "���λ/����",		"����/������������"},{ 5, 2,15},{20, 9,  3,	0.5f,		"",		0s,				  "attack",		900ms,	 "��Ч/TTaFir00","TMaDth00","die01"}}},
		{��ս��,{{"��ս��"	, "axe-yellow",			"tfbPss00"},		 { 8, 3,18},{15, 4,  5,	0.5f,		"",		0s,				  "attack",		600ms,	 "Tfrshoot",	 "TMaDth00","died"}}},
		{��ɫ̹��,{{"��ɫ̹��","���λ/��ɫ̹��",	"����/ttapss00"},	 {20,10,60},{35,25,	30,	0.1f,	"idle",		5s,			 "attack_loop",		300ms,	 "��Ч/TTaFi200","��Ч/TTaDth00","die02"}}},
	};
	std::unordered_map<���λ����, ������> g_map������ = {
		//								 ���� ���� ��  ���� ǰҡ���� dura��ʼ���Ź������� �������� dura��ʼ�˺� ������Ч	������Ч �������� Ѫ��
		{��,	{{"��"	, "altman-red"	}, {20,8,	5, 0.2f,	"",			0s,			  "attack",	300ms,		"TMaFir00","TMaDth00","died"}, 50} },
		{��ս��,{{"����", "���λ/����"},{15,3,	8, 0.4f,	"",			0s,			  "attack",	400ms, "��Ч/��צ����","TMaDth00","died"}, 65} },
	};

	std::unordered_map<��Դ����, ��Դ��λ����> g_map��Դ��λ���� =
	{
		{�����,{{"�����"	, "tree_large"	}}},
		{ȼ����,{{"ȼ����"	, "house_type03"}}},
	};

	template<class K, class V>
	bool Find����(std::unordered_map<K, V>& refMap, const K ����, V& refOut)
	{
		const auto& iterFind = refMap.find(����);
		if (iterFind == refMap.end())
		{
			assert(false);
			return false;
		}

		refOut = iterFind->second;
		return true;
	}
	bool Find������λ����(const ������λ���� ����, ������λ����& refOut)
	{
		return Find����(g_map������λ����, ����, refOut);
	}

	bool Find���λ����(const ���λ���� ����, ���λ����& refOut)
	{
		return Find����(g_map���λ����, ����, refOut);
	}
	bool Find��Դ��λ����(const ��Դ���� ����, ��Դ��λ����& refOut)
	{
		return Find����(g_map��Դ��λ����, ����, refOut);
	}
	bool Find������(const ���λ���� ����, ������& refOut)
	{
		return Find����(g_map������, ����, refOut);
	}
}