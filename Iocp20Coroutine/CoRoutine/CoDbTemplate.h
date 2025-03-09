#include "CoDb.h"
#include <deque>
#include <sstream>
#include <fstream>
#include <iostream>
#include "../IocpNetwork/MsgPack.h"



template<class T>
void ������<T>::Init(const HANDLE hIocp)
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
T CoDb<T>::LoadFromDbThread(const std::string nickName)
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

	return objT;
}

template<class T>
T CoDb<T>::SaveInDbThread(const T& ref, const std::string & strNickName)
{
	std::ostringstream oss;
	oss << typeid(T).name() << "_" << strNickName << ".bin";
	const auto& strFileName = oss.str();

	// ���ļ�
	std::ofstream out(strFileName, std::ios::binary);

	// ����ļ��Ƿ�ɹ���
	if (!out)
	{
		LOG(ERROR) << "�޷����ļ�" << strFileName;
		//dequeLocal.pop_front();
		return ref;
	}

	MsgPack::SendMsgpack(ref, [&out](const void* buf, int len) { out.write((const char*)buf, len); }, false);

	// �ر��ļ�
	out.close();
	LOG(INFO) << "��д��" << strFileName;
	//ģ��дӲ�̺ܿ�
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	return ref;
};

template<class T>
CoAwaiter<T>& CoDb<T>::CoSave(const T& ref, const std::string& strNickName, FunCancel& cancel)
{
	return m_������.DoDb([this, &ref, strNickName]() {return this->SaveInDbThread(ref, strNickName); }, cancel);
}

template<class T>
CoAwaiter<T>& ������<T>::DoDb(DbFun funDb, FunCancel& cancel)
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
	return m_������.DoDb([this, nickName]() { return this->LoadFromDbThread(nickName); }, cancel);
}

template<class T>
CoTask<Iocp::Overlapped::YieldReturn> ������<T>::CoDbDbThreadProcess(Iocp::Overlapped&)
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
void ������<T>::DbThreadProcess()
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
		//ģ���Ӳ�̺ܿ�
		spCoAwait->SetResult(fun());
		m_dequeResult.push_back(spCoAwait);
		dequeLocal.pop_front();
	}
}

template<class T>
inline void ������<T>::Process()
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
