#pragma once
#include <map>
class Entity;
enum 属性类型;

/// <summary>
/// 记录生命、能量等属性
/// </summary>
class 数值Component
{
public:
	数值Component(Entity& refEntity);
	static 数值Component& AddComponent(Entity& refEntity);
	static int Get(const Entity& refEntity, const 属性类型 属性);
	static int Set(Entity& refEntity, const 属性类型 属性, const int 数值);
	static int 改变(Entity& refEntity, const 属性类型 属性, const int 变化);
	int Get(const 属性类型 属性)const;
	int Set(const 属性类型 属性,const int 数值);
	int 最大属性(const 属性类型& 属性);
	int 改变(const 属性类型 属性, const int 变化);
	int 确保属性在范围内(const 属性类型 属性);
private:
	Entity& m_refEntity;
	std::map<属性类型, int> m_map属性数值;
};

