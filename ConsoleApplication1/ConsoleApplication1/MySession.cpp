#include "MySession.h"
#include "Server.cpp"
#include "ListenSocketCompeletionKey.cpp"
#include "SessionSocketCompeletionKey.cpp"

#include <msgpack.hpp>
#include <iostream>
#include <cassert>

#include <ws_endpoint.h>
#include <codecvt>


template class Iocp::Server<MySession>;
template class Iocp::ListenSocketCompeletionKey<MySession>;
template class Iocp::SessionSocketCompeletionKey<MySession>;

struct MsgLogin {
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(name, pwd);
};

struct MsgLoginRet
{
	int id;
	std::string nickName;
	MSGPACK_DEFINE(id, nickName);
};

void net_write_cb(char* buf, int64_t size, void* wd)
{
	static_cast<Iocp::SessionSocketCompeletionKey<MySession>*>(wd)->Send(buf, size);
}
class MyWebSocketEndpoint :public WebSocketEndpoint
{
public:
	MyWebSocketEndpoint(nt_write_cb write_cb, void* work_data) :WebSocketEndpoint(write_cb, work_data)
	{
	}
	//std::string StringToUTF8(const std::string& gbkData)
	//{
	//	const char* GBK_LOCALE_NAME = "CHS";  //GBK在windows下的locale name(.936, CHS ), linux下的locale名可能是"zh_CN.GBK"

	//	std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>>
	//		conv(new std::codecvt<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
	//	std::wstring wString = conv.from_bytes(gbkData);    // string => wstring

	//	std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
	//	std::string utf8str = convert.to_bytes(wString);     // wstring => utf-8

	//	return utf8str;
	//}
	string GbkToUtf8(const char* src_str)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
		string strTemp = str;
		if (wstr) delete[] wstr;
		if (str) delete[] str;
		return strTemp;
	}

	virtual int32_t user_defined_process(WebSocketPacket& packet, ByteBuffer& frame_payload)override
	{
		switch (packet.get_opcode())
		{
		case WebSocketPacket::WSOpcode_Continue:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Continue opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Text:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Text opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Binary:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Binary opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Close:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Close opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			return 0;
			break;
		case WebSocketPacket::WSOpcode_Ping:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Ping opcode." << std::endl;
			user_defined_process(packet, frame_payload);
			break;
		case WebSocketPacket::WSOpcode_Pong:
			// add your process code here
			//std::cout << "WebSocketEndpoint - recv a Pong opcode." << std::endl;
			//user_defined_process(packet, frame_payload);
			break;
		default:
			std::cout << "WebSocketEndpoint - recv an unknown opcode." << std::endl;
			break;
		}

		msgpack::object_handle oh = msgpack::unpack(frame_payload.bytes(), frame_payload.length());
		msgpack::object obj = oh.get();
		std::cout << obj << std::endl;
		const auto msgLogin = obj.as<MsgLogin>();

		WebSocketPacket wspacket;
		// set FIN and opcode
		wspacket.set_fin(1);
		wspacket.set_opcode(0x02);// packet.get_opcode());
		// set payload data
		MsgLoginRet ret = { 223,GbkToUtf8("大侠") };
		std::stringstream buffer;
		msgpack::pack(buffer, ret);
		buffer.seekg(0);

		// deserialize the buffer into msgpack::object instance.
		std::string str(buffer.str());
		wspacket.set_payload(str.data(), str.size());
		ByteBuffer output;
		// pack a websocket data frame
		wspacket.pack_dataframe(output);
		// send to client
		to_wire(output.bytes(), output.length());

		return 0;
	}
};
void MySession::OnInit(Iocp::SessionSocketCompeletionKey<MySession>& refSession)
{

}
int MySession::OnRecv(Iocp::SessionSocketCompeletionKey<MySession>& refSession, const char buf[], int len)
{
	if (ws == nullptr)
	{
		ws = new MyWebSocketEndpoint(net_write_cb, &refSession);
	}

	ws->from_wire(buf, len);
	return len;
}

void MySession::OnDestroy() 
{

}