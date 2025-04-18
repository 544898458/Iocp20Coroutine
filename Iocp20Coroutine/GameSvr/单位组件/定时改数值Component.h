#pragma once
#include "../../CoRoutine/CoTask.h"
#include <map>
class Entity;
class PlayerGateSession_Game;
class Space;
enum 单位属性类型;
class 定时改数值Component
{
public:
	定时改数值Component(Entity& ref);
	static 定时改数值Component& AddComponent(Entity& refEntity);
	void 加属性(单位属性类型 属性类型, int16_t 变化, std::chrono::system_clock::duration dura删除);
	int16_t 属性(单位属性类型 属性类型)const;
	void 定时回血();
	void OnDestroy();
private:
	CoTaskBool Co定时改数值(std::chrono::system_clock::duration dura间隔, int16_t i16变化);
	Entity& m_refEntity;
	FunCancel m_funCancel;
	struct 属性数值
	{
		int16_t 变化;
		FunCancel funCancel;
	};
	std::map<单位属性类型, std::map<uint32_t, 属性数值>> m_map属性数值;
	uint32_t m_sn属性数值 = 0;
};

