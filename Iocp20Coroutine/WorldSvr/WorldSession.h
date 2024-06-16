#pragma once
#include"../IocpNetwork/SessionSocketCompeletionKey.h"
#include "../LogStrategy/StrategyLog.h"
#include "../IocpNetwork/WebSocketSession.h"
#include <msgpack.hpp>
class WorldServer;
class WorldSession
{
public:
	using CompeletionKeySession = Iocp::SessionSocketCompeletionKey < WorldSession >;
	int OnRecv(CompeletionKeySession&, const void* buf, int len);
	void OnDestroy();
	void OnInit(CompeletionKeySession& refSession, WorldServer&);
	template<class T>
	void Send(const T& ref) 
	{
		//普通结构序列化，无压缩无加密
		CHECK_PTR(m_pSession);
		std::stringstream buffer;
		msgpack::pack(buffer, ref);
		buffer.seekg(0);

		// deserialize the buffer into msgpack::object instance.
		std::string str(buffer.str());
		//wspacket.set_payload(str.data(), str.size());
		//ByteBuffer output;
		// pack a websocket data frame
		//wspacket.pack_dataframe(output);
		//send to client
		//this->to_wire(output.bytes(), output.length());
		const uint16_t usSize = str.size();
		this->m_pSession->Send(&usSize, sizeof(usSize));
		this->m_pSession->Send(str.data(), str.size());
		LOG(INFO) << typeid(T).name();
	}
	CompeletionKeySession* m_pSession = nullptr;
};

