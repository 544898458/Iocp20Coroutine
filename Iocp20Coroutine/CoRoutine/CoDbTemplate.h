#include "pch.h"
#include "CoDb.h"
#include <deque>
#include <sstream>
#include <fstream>
#include <iostream>

template<class T>
CoAwaiterBool& CoDb<T>::Save(const T& ref, FunCancel& cancel)
{
	const auto sn = CoAwaiterBool::GenSn();
	this->m_dequeSave.push_back({ ref,CoAwaiterBool(sn, cancel) });
	return std::get<1>(this->m_dequeSave.back());
}

template<class T>
void CoDb<T>::DbThreadProcess()
{
	while (!this->m_dequeSave.empty())
	{
		auto&& [ref,coAwait] = this->m_dequeSave.front();
		std::ostringstream oss;
		oss << typeid(T).name() << "_" << ref.id << ".bin";
		const auto& strFileName = oss.str();

		// 打开文件
		std::ofstream out(strFileName, std::ios::binary);

		// 检查文件是否成功打开
		if (!out) 
		{
			LOG(ERROR) << "无法打开文件" << strFileName;
			this->m_dequeSave.pop_front();
			continue;
		}

		// 写入数据
		out.write(reinterpret_cast<char*>(&ref), sizeof(ref));

		// 关闭文件
		out.close();

		//模拟写硬盘很卡
		std::this_thread::sleep_for(std::chrono::seconds(1));
		m_dequeResult.push_back( std::forward<CoAwaiterBool&&>(coAwait));
		this->m_dequeSave.pop_front();
	}
}

template<class T>
inline void CoDb<T>::Process()
{
}
