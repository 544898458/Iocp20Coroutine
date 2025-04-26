#pragma once
#include<memory>
struct krx;
class SslTlsSvr final
{
public:
	SslTlsSvr();
	~SslTlsSvr();
	static void InitAll();
	void Init();
	int 处理前端发来的密文(const void* buf, const int len);
	bool 握手OK();
	template<int len>
	int 读出已解密的明文(char(&bufOut)[len]);
	template<int len>
	int 获取准备发往前端的密文(char(&bufOut)[len]);
	int 把要发给前端的明文交给Ssl处理(const void* buf, const int len);
	void do_handshake();
private:
	krx* m_pServer;
	std::mutex m_mutex;
};
