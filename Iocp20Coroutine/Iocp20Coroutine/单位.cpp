#include "pch.h"
#include "��λ.h"
#include <unordered_map>
namespace ��λ
{
	std::unordered_map<������λ����, ������λ����> g_map������λ���� =
	{
		{����, {{"����","house_type06",1},&BuildingComponent::Co�칤�̳�, 2.0f}},
		{����, {{"����","house_type19",2},&BuildingComponent::Co���, 2.0f}},
	};

	std::unordered_map<���λ����, ���λ����> g_map���λ���� =
	{
		{���̳�,{{"���̳�"	, "altman-yellow"	, 3}, 8	}},
		{��,	{{"��"		, "altman-blue"		, 5}, 15}},
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
}