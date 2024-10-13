#include "pch.h"
#include "CoTask.h"

void CoTaskCancel::TryCancel()
{
	if (cancel)
	{
		//LOG(INFO) << "����m_cancel";
		cancel();
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

	assert(co.Finished());
}
