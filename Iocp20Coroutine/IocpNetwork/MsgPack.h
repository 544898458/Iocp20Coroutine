#pragma once
namespace MsgPack
{
	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="T_Msg">����Ϣ��MsgPack���л�</typeparam>
	/// <typeparam name="FUN"></typeparam>
	/// <param name="ref"></param>
	/// <param name="refFun"></param>
	template<class T_Msg,typename FUN>
	static void SendMsgpack(const T_Msg& ref, const FUN &refFun)
	{
		//��ͨ�ṹ���л�����ѹ���޼���
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
		//LOG(INFO) << typeid(T_Msg).name();
	}
};

