#include "pch.h"
#include "CoTask.h"

void CoTaskCancel::TryCancel()
{
	if (cancel)
	{
		//LOG(INFO) << "����m_cancel";
		cancel();
		LOG_IF(ERROR, !co.Finished()) << "";
		assert(co.Finished());
	}
	else
	{
		//LOG(INFO) << "m_cancel�ǿյģ�û��Ҫȡ����Э��";
		if (!co.Finished())
		{
			LOG(ERROR) << "Э��û������ȴ��ǰ�����m_cancel";
			assert(false);
		}
	}

	LOG_IF(ERROR, !co.Finished()) << "";
	assert(co.Finished());
}
