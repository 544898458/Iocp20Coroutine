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
	void OnEntityDestroy(const bool bDestroy);

	void 加属性(BuffId idBuff表);
	float 属性(属性类型 属性类型)const;
	void 定时改数值(const BuffId id, const uint64_t idAttacker);
	void 删Buff(BuffId id);
	bool 已有Buff(BuffId id)const;
private:
	void On属性变化(属性类型 属性类型);
	CoTaskBool Co定时改数值(const 属性类型 属性, std::chrono::system_clock::duration dura间隔, int16_t i16变化, FunCancel& funCancel, const uint64_t idAttacker);
	Entity& m_refEntity;
	std::map<BuffId, FunCancel> m_mapFunCancel;
	struct 属性数值 final
	{
		属性数值(float f变化) :m_funCancel("属性数值"), 变化(f变化){

		}
		~属性数值()
		{
			m_funCancel.TryCancel();
		}
		const float 变化;
		FunCancel安全 m_funCancel;
	};
	std::map<属性类型, std::map<BuffId, 属性数值>> m_map属性数值;
};

