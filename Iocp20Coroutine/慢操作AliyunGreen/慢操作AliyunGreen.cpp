#include "pch.h"
#include "������AliyunGreen.h"
#include "../CoRoutine/CoDbTemplate.h"
#include "../AliyunGreen/AliyunGreen.h"

CoAwaiterBool& ������AliyunGreen::CoAliyunGreen(const std::string& strContent, FunCancel& cancel)
{
	return m_������.DoDb([this, strContent]() 
		{
			const auto ok = AliyunGreen::Check(strContent);
			LOG(INFO) << strContent << ",�̻����," << ok;
			return ok;
		}, cancel);
}