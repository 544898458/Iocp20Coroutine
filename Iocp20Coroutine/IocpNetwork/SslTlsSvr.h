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
	int ����ǰ�˷���������(const void* buf, const int len);
	bool ����OK();
	template<int len>
	int �����ѽ��ܵ�����(char(&bufOut)[len]);
	template<int len>
	int ��ȡ׼������ǰ�˵�����(char(&bufOut)[len]);
	int ��Ҫ����ǰ�˵����Ľ���Ssl����(const void* buf, const int len);
	void do_handshake();
private:
	krx* m_pServer;
	std::mutex m_mutex;
};
