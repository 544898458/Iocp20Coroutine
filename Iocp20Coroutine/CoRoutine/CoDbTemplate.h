#include "pch.h"
#include "CoDb.h"
#include <deque>
#include <sstream>
#include <fstream>
#include <iostream>



template<class T>
void CoDb<T>::Init(const HANDLE hIocp)
{
	m_hIocp = hIocp;
	m_OverlappedNotify.OnComplete = &Iocp::Overlapped::OnCompleteNotifySend;
	m_OverlappedNotify.pOverlapped = &m_OverlappedWork;
	m_OverlappedNotify.coTask.m_desc = "NotifyDb";

	m_OverlappedWork.OnComplete = &Iocp::Overlapped::OnCompleteSend;
	m_OverlappedWork.coTask.m_desc = "DbWork";
	m_OverlappedWork.coTask = CoDbDbThreadProcess(m_OverlappedWork);
	m_OverlappedWork.coTask.Run();
}

template<class T>
void CoDb<T>::LoadFromDbThread(const std::string nickName, SpCoAwaiterT& spCoAwait)
{
	std::ostringstream oss;
	oss << typeid(T).name() << "_" << nickName << ".bin";
	const auto& strFileName = oss.str();
	// ���ļ�
	std::ifstream in(strFileName, std::ios::binary);
	// ����ļ��Ƿ�ɹ���
	T objT;
	if (in)
	{
		// ��ȡ�ļ���С
		in.seekg(0, std::ios::end);
		std::streamsize size = in.tellg();
		in.seekg(0, std::ios::beg);

		// ��ȡ�ļ����ݵ�vector
		std::vector<char> buffer(size);
		if (in.read(buffer.data(), size)) {
			// �ɹ���ȡ���ݣ�buffer�а����ļ�����
			std::cout << "�ļ���С: " << size << " �ֽ�" << std::endl;
		}
		else {
			std::cerr << "��ȡ�ļ�ʧ��" << std::endl;
		}
		msgpack::object_handle oh = msgpack::unpack(buffer.data(), buffer.size());//û�ж�Խ�磬Ҫ��try
		msgpack::object obj = oh.get();
		objT = obj.as<T>();
		in.close();
		LOG(INFO) << "�Ѷ���" << strFileName;
	}
	else
	{
		LOG(WARNING) << "�޷����ļ�" << strFileName;
		//dequeLocal.pop_front();
	}

	//ģ��дӲ�̺ܿ�
	std::this_thread::sleep_for(std::chrono::seconds(1));
	{
		spCoAwait->SetResult(objT);
		std::lock_guard lock(m_mutexDequeResult);
		m_dequeResult.push_back(spCoAwait);
	}
}

template<class T>
void CoDb<T>::SaveInDbThread(const T& ref, SpCoAwaiterT& spCoAwait)
{
	std::ostringstream oss;
	oss << typeid(T).name() << "_" << ref.nickName << ".bin";
	const auto& strFileName = oss.str();

	// ���ļ�
	std::ofstream out(strFileName, std::ios::binary);

	// ����ļ��Ƿ�ɹ���
	if (!out)
	{
		LOG(ERROR) << "�޷����ļ�" << strFileName;
		//dequeLocal.pop_front();
		return;
	}

	MsgPack::SendMsgpack(ref, [&out](const void* buf, int len) { out.write((const char*)buf, len); });

	// �ر��ļ�
	out.close();
	LOG(INFO) << "��д��" << strFileName;
	//ģ��дӲ�̺ܿ�
	std::this_thread::sleep_for(std::chrono::seconds(5));
	{
		spCoAwait->SetResult(ref);
		std::lock_guard lock(m_mutexDequeResult);
		m_dequeResult.push_back(spCoAwait);
	}
};

template<class T>
CoAwaiter<T>& CoDb<T>::Save(const T& ref, FunCancel& cancel)
{
	return DoDb([this, &ref](SpCoAwaiterT& sp) {this->SaveInDbThread(ref, sp); }, cancel);
}

template<class T>
CoAwaiter<T>& CoDb<T>::DoDb(DbFun funDb, FunCancel& cancel)
{
	std::lock_guard lock(m_mutexDequeSave);

	const auto sn = CoAwaiterBool::GenSn();

	auto sp = std::make_shared<CoAwaiter<T>, const long&, FunCancel&>(sn, cancel);
	this->m_dequeSave.push_back({ funDb, sp });
	if (Iocp::Overlapped::SendState_Sending != this->m_OverlappedNotify.atomicSendState.load())
	{
		PostQueuedCompletionStatus(m_hIocp, 0, (ULONG_PTR)this, &m_OverlappedNotify.overlapped);
	}

	return *sp;
}

template<class T>
CoAwaiter<T>& CoDb<T>::Load(const std::string nickName, FunCancel& cancel)
{
	return DoDb([this, nickName](SpCoAwaiterT& sp) {this->LoadFromDbThread(nickName, sp); }, cancel);
}
template<class T>
CoTask<Iocp::Overlapped::YieldReturn> CoDb<T>::CoDbDbThreadProcess(Iocp::Overlapped&)
{
	{
		while (true)
		{
			DbThreadProcess();
			co_yield Iocp::Overlapped::SendStop;
		}

		co_return Iocp::Overlapped::OK;
	}
}
template<class T>
void CoDb<T>::DbThreadProcess()
{
	std::deque<std::tuple<DbFun, SpCoAwaiterT>> dequeLocal;

	{
		std::lock_guard lock(m_mutexDequeSave);
		while (!m_dequeSave.empty())
		{
			auto& [fun, spCoAwait] = this->m_dequeSave.front();
			dequeLocal.push_back({ fun, spCoAwait });
			this->m_dequeSave.pop_front();
		}
	}

	while (!dequeLocal.empty())
	{
		auto& [fun, spCoAwait] = dequeLocal.front();
		//SaveInDbThread(ref, std::forward<CoAwaiterBool&&>(coAwait));
		fun(spCoAwait);
		dequeLocal.pop_front();
	}
}

template<class T>
inline void CoDb<T>::Process()
{
	{
		std::lock_guard lock(m_mutexDequeResult);
		while (!m_dequeResult.empty())
		{
			auto& spCoAwait = m_dequeResult.front();
			spCoAwait->Run();
			this->m_dequeResult.pop_front();
		}
	}
}
