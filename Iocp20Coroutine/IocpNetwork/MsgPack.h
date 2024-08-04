#pragma once
namespace MsgPack
{
	template<class T,typename FUN>
	static void SendMsgpack(const T& ref, const FUN &refFun)
	{
		//普通结构序列化，无压缩无加密
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
		CHECK_GE_VOID(UINT16_MAX,str.size());
		const uint16_t usSize = (uint16_t)str.size();
		refFun(&usSize, sizeof(usSize));
		refFun(str.data(), usSize);
		LOG(INFO) << typeid(T).name();
	}
};

