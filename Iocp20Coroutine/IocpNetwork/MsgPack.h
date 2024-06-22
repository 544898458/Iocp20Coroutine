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
		const uint16_t usSize = str.size();
		refFun(&usSize, sizeof(usSize));
		refFun(str.data(), str.size());
		LOG(INFO) << typeid(T).name();
	}
};

