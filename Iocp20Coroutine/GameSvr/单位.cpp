#include "pch.h"
#include "��λ.h"
#include <unordered_map>
namespace ��λ
{

	std::unordered_map<������λ����, ������λ����> g_map������λ���� =
	{
		//								����ȼ��HP
		{����, {{"����","house_type06"},{20, 0,200}, 2.0f}},
		{����, {{"����","house_type19"},{15,11,100},2.0f} },
		{��, {{"��","house_type21"},{10, 8,50}, 1.0f} },
		{�ر�, {{"�ر�","house_type17"},{15,15,500}, 2.0f} },
	};

	std::unordered_map<���λ����, ���λ����> g_map���λ���� =
	{
		//										����ȼ��HP	����	����	
		{���̳�,{{"���̳�"	, "altman-yellow"	},{ 3,0,20	}, 8 , 4} },
		{��,	{{"��"		, "altman-blue"		},{ 5,6,15	}, 20, 9} },
		{��ս��,{{"��ս��"	, "axe-yellow"		},{ 8,8,18	}, 15, 4} },
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
}