#include "pch.h"
#include "��λ.h"
#include <unordered_map>
namespace ��λ
{

	std::unordered_map<������λ����, ������λ����> g_map������λ���� =
	{
		//								����ȼ��HP
		{����, {{"����","����/����"},{50, 0,200}, 5.0f}},
		{����, {{"����","house_type19"},{25, 9,100},2.0f} },
		{��, {{"��","house_type21"},{20, 3,50}, 1.0f} },
		{�ر�, {{"�ر�","house_type17"},{30,11,500}, 2.0f} },
	};

	std::unordered_map<���λ����, ���λ����> g_map���λ���� =
	{
		//										����ȼ��HP	  ���� ����	
		{���̳�,{{"���̳�"	, "altman-yellow"	},{ 3,0,20	}, {8 , 4, 1}} },
		{��,	{{"��"		, "���λ/����"	},{ 5,2,15	}, {20, 9, 3}} },
		{��ս��,{{"��ս��"	, "axe-yellow"		},{ 8,3,18	}, {15, 4, 5}} },
	};
	std::unordered_map<���λ����, ������> g_map������ = {
		//									���� ���� ��	Ѫ��
		{��,	{{"��"		, "altman-red"	},{20,8,5}, 50} },
		{��ս��,{{"��ͷ��"	, "axe-red"		},{15,3,8}, 65} },
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