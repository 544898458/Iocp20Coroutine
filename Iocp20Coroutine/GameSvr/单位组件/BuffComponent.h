#pragma once
#include "../../CoRoutine/CoTask.h"
#include <map>
class Entity;
class PlayerGateSession_Game;
class Space;
enum 单位属性类型;
class BuffComponent
{
public:
	BuffComponent(Entity& ref);
	static BuffComponent& AddComponent(Entity& refEntity);
	void 加属性(uint32_t u32Buff表Id, 单位属性类型 属性类型, float f变化, std::chrono::system_clock::duration dura删除);
	float 属性(单位属性类型 属性类型)const;
	void 定时回血();
	void OnDestroy();
private:
	void On属性变化(单位属性类型 属性类型);
	CoTaskBool Co定时改数值(std::chrono::system_clock::duration dura间隔, int16_t i16变化);
	Entity& m_refEntity;
	FunCancel m_funCancel;
	struct 属性数值 final
	{
		~属性数值()
		{
			if (funCancel)
			{
				funCancel();
				funCancel = nullptr;
			}
		}
		float 变化;
		FunCancel funCancel;
	};
	std::map<单位属性类型, std::map<uint32_t, 属性数值>> m_map属性数值;
};

