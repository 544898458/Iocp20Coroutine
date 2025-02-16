#pragma once
#include<memory>
struct krx;
class SslTlsSvr final
{
public:
	SslTlsSvr();
	~SslTlsSvr();
	static void InitAll();
	void Init(const bool bServer);
	int ����ǰ�˷���������(const void* buf, const int len);
	bool ����OK() const;
	template<int len>
	int �����ѽ��ܵ�����(char(&bufOut)[len]);
	template<int len>
	int ��ȡ׼������ǰ�˵�����(char(&bufOut)[len]);
	int ��Ҫ����ǰ�˵����Ľ���Ssl����(const void* buf, const int len);
	void do_handshake();
private:
	krx* m_pServer;
};
