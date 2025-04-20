#pragma once
#include "../../CoRoutine/CoTask.h"
#include <map>
class Entity;
class PlayerGateSession_Game;
class Space;
enum 属性类型;
enum BuffId;
class BuffComponent
{
public:
	BuffComponent(Entity& ref);
	static BuffComponent& AddComponent(Entity& refEntity);
	void OnDestroy();

	void 加属性(BuffId idBuff表, 属性类型 属性类型, float f变化, std::chrono::system_clock::duration dura删除);
	float 属性(属性类型 属性类型)const;
	void 定时改数值(const BuffId id);
private:
	void On属性变化(属性类型 属性类型);
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
	std::map<属性类型, std::map<BuffId, 属性数值>> m_map属性数值;
};

