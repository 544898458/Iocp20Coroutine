#include "pch.h"
#include "慢操作AliyunGreen.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "../AliyunGreen/AliyunGreen.h"

CoAwaiterBool& 慢操作AliyunGreen::CoAliyunGreen(const std::string& strContent, FunCancel& cancel)
{
	return m_慢操作.DoDb([this, strContent]() 
		{
			const auto ok = AliyunGreen::Check(strContent);
			LOG(INFO) << strContent << ",绿化结果," << ok;
			return ok;
		}, cancel);
}

CoAwaiterBool& 慢操作AliyunGreen::Co记录战局结束(const uint32_t uSvrId, const std::string& nickName, int 战局类型, bool is赢, FunCancel& cancel)
{
	return m_慢操作.DoDb([this, nickName, 战局类型, is赢, uSvrId]() 
		{
			const auto ok = AliyunGreen::记录战局结束(uSvrId, nickName, 战局类型, is赢);
			LOG(INFO) << uSvrId << "," << nickName << "," << 战局类型 << "," << is赢 << ",记录战局结束," << ok;
			return true;
		}, cancel);
}

CoAwaiterBool& 慢操作AliyunGreen::Co记录击杀(const uint32_t uSvrId, const int 战局类型, const std::string& nickName攻击, const 单位类型 单位类型攻击, const std::string& nickName阵亡, const 单位类型 单位类型阵亡, FunCancel& cancel)
{
	return m_慢操作.DoDb([this, 战局类型, nickName攻击, 单位类型攻击, nickName阵亡, 单位类型阵亡, uSvrId]() 
		{
			const auto ok = AliyunGreen::记录击杀( uSvrId, 战局类型, nickName攻击, 单位类型攻击, nickName阵亡, 单位类型阵亡);
			LOG(INFO) << 战局类型 << nickName攻击 << "," << 单位类型攻击 << "," << nickName阵亡 << "," << 单位类型阵亡 << ",记录击杀," << ok;
			return ok;
		}, cancel);
}

