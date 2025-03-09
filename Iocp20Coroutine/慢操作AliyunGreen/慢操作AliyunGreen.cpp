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