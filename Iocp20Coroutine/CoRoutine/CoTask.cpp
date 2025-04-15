#include "pch.h"
#include "CoTask.h"

CoTaskCancel::~CoTaskCancel()
{
	_ASSERT(co.Finished());
}

void CoTaskCancel::TryCancel()
{
	if (cancel)
	{
		//LOG(INFO) << "����m_cancel";
		cancel();
		cancel = nullptr;
		LOG_IF(ERROR, !co.Finished()) << "";
		_ASSERT(co.Finished());
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!co.Finished())
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			_ASSERT(false);
		}
	}

	LOG_IF(ERROR, !co.Finished()) << "";
	_ASSERT(co.Finished());
}
