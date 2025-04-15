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
		//LOG(INFO) << "调用m_cancel";
		cancel();
		cancel = nullptr;
		LOG_IF(ERROR, !co.Finished()) << "";
		_ASSERT(co.Finished());
	}
	else
	{
		//LOG(INFO) << "m_cancel是空的，没有要取消的协程";
		if (!co.Finished())
		{
			LOG(ERROR) << "协程没结束，却提前清空了m_cancel";
			_ASSERT(false);
		}
	}

	LOG_IF(ERROR, !co.Finished()) << "";
	_ASSERT(co.Finished());
}
