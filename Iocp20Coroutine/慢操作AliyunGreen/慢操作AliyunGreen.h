#pragma once
#include "../CoRoutine/CoDb.h"
class ������AliyunGreen
{
public:
	/// <summary>
	/// �������߳�Э������ã�ʵ��ֻ������Ž����У�Ȼ��ʲôҲ����
	/// </summary>
	/// <param name="ref"></param>
	/// <param name="cancel"></param>
	/// <returns></returns>
	CoAwaiterBool& CoAliyunGreen(const std::string& strContent, FunCancel& cancel);
	������<bool> m_������;

private:
};


