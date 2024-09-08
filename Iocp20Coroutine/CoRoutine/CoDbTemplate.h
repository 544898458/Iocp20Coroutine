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

		// ���ļ�
		std::ofstream out(strFileName, std::ios::binary);

		// ����ļ��Ƿ�ɹ���
		if (!out) 
		{
			LOG(ERROR) << "�޷����ļ�" << strFileName;
			this->m_dequeSave.pop_front();
			continue;
		}

		// д������
		out.write(reinterpret_cast<char*>(&ref), sizeof(ref));

		// �ر��ļ�
		out.close();

		//ģ��дӲ�̺ܿ�
		std::this_thread::sleep_for(std::chrono::seconds(1));
		m_dequeResult.push_back( std::forward<CoAwaiterBool&&>(coAwait));
		this->m_dequeSave.pop_front();
	}
}

template<class T>
inline void CoDb<T>::Process()
{
}
