#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"

class Entity;
const float fExponent = 2.0f;
struct Position
{
	float x;
	float z;
	MSGPACK_DEFINE(x, z);
	bool operator==(const Position& refRight)const
	{
		return x == refRight.x && z == refRight.z;
	}

	bool DistanceLessEqual(const Position& refPos, float fDistance)const
	{
		return this->DistancePow2(refPos) <= std::pow(fDistance, fExponent);
	}

	float DistancePow2(const Position& refPos)const
	{
		return std::pow(x - refPos.x, fExponent) + std::pow(z - refPos.z, fExponent);
	}
};
template <class _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const Position& _ref)
{
	return _Ostr << "(" << _ref.x << "," << _ref.z << ")";
}
enum MsgId
{
	Invalid_0,
	Login,
	Move,
	AddRoleRet,
	NotifyPos,
	ChangeSkeleAnim,
	Say,
	SelectRoles,
	AddRole,
	DelRoleRet,
	ChangeMoney,
	ChangeMoneyResponce,
	AddBuilding,
	NotifyeMoney,
	Gateת��,
	GateAddSession,
	GateAddSessionResponce,
	GateDeleteSession,
	GateDeleteSessionResponce,
	�ɼ�,
	��Դ,
	���ر�,
	���ر�,
	��Space,
	�����˾��鸱��,
	��ʾ����,
};
MSGPACK_ADD_ENUM(MsgId);

enum ������λ����
{
	����,//ָ������(Command Center),�����칤�̳�()
	����,//��Ӫ(Barracks)���������
	��,//����վ(Supply Depot)
	�ر�,//����; �ر�(Bunker),���Խ���
};
MSGPACK_ADD_ENUM(������λ����);

enum ��Դ����
{
	�����,//Minerals
	ȼ����,//Vespene Gas
};
MSGPACK_ADD_ENUM(��Դ����);

enum ���λ����
{
	���̳�,//�ռ乤�̳�Space Construction Vehicle�����Բɿ󣬲�����Ҳ���Լ򵥹���
	��,//½ս��ԱMarine��ֻ�ܹ��������ܲɿ�
	��ս��,//��������Firebat
};
MSGPACK_ADD_ENUM(���λ����);

struct MsgHead
{
	//void SetSn(uint32_t snParam) const
	//{
	//	const_cast<Msg*>(this)->sn = snParam;
	//}
	static MsgHead GetMsgId(msgpack::object obj)
	{
		return obj.via.array.ptr[0].as<MsgHead>();
		//return obj.as<Msg>();
		//return (MsgId)obj.via.array.ptr[0].via.array.ptr[0].via.i64;//û�ж�Խ�磬Ҫ��try
	}
	MsgId id;
	mutable uint32_t sn;//�����������α�����ݣ�������α��������㷨��������
	mutable uint32_t rpcSnId;

	MSGPACK_DEFINE(id, sn, rpcSnId);
};

struct MsgLogin
{
	MsgHead msg;
	std::string name;
	std::string pwd;
	MSGPACK_DEFINE(msg, name, pwd);
};

struct MsgLoginResponce
{
	MsgHead msg = { .id = MsgId::Login };
	enum Error
	{
		OK,
		Busy,
		PwdErr,
		NameErr,
	};
	Error  result = OK;

	MSGPACK_DEFINE(msg, result);
};
MSGPACK_ADD_ENUM(MsgLoginResponce::Error);

struct MsgAddRole
{
	MsgHead msg;
	���λ���� ����;
	MSGPACK_DEFINE(msg, ����);
};

struct MsgAddBuilding
{
	MsgHead msg;
	������λ���� ����;
	Position pos;
	MSGPACK_DEFINE(msg, ����, pos);
};

struct MsgChangeMoney
{
	MsgHead msg{ .id = ChangeMoney };
	bool addMoney;
	int32_t changeMoney;
	MSGPACK_DEFINE(msg, addMoney, changeMoney);
};

struct MsgChangeMoneyResponce
{
	MsgHead msg{ .id = ChangeMoneyResponce };
	/// <summary>
	/// 1��һ��DBЭ�̻�û����
	/// </summary>
	int error = 0;
	uint32_t consumeMoney;
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, error, consumeMoney, finalMoney);
};

struct MsgNotifyMoney
{
	MsgHead msg{ .id = NotifyeMoney };
	int32_t finalMoney;
	MSGPACK_DEFINE(msg, finalMoney);
};

struct MsgMove
{
	MsgHead msg;
	float x;
	float z;
	MSGPACK_DEFINE(msg, x, z);
};

enum SayChannel
{
	ϵͳ,
	������ʾ,
};
MSGPACK_ADD_ENUM(SayChannel);

struct MsgSay
{
	MsgHead msg{ .id = MsgId::Say };
	std::string content;

	SayChannel channel = ϵͳ;
	MSGPACK_DEFINE(msg, content, channel);
};


struct MsgSelectRoles
{
	MsgSelectRoles() {}
	MsgHead msg{ .id = MsgId::Say };
	std::vector<double> ids;//TypeScriptֻ��FLOAT64,û��POSITIVE_INTEGER��NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, ids);
};

struct MsgAddRoleRet
{
	MsgAddRoleRet(Entity&);
	MsgHead msg{ .id = AddRoleRet };
	uint64_t entityId;
	std::string nickName;
	std::string entityName;
	std::string prefabName;
	MSGPACK_DEFINE(msg, entityId, nickName, entityName, prefabName);
};

struct MsgDelRoleRet
{
	MsgDelRoleRet(uint64_t entityId) :entityId(entityId) {}
	MsgHead msg{ .id = DelRoleRet };
	uint64_t entityId;
	MSGPACK_DEFINE(msg, entityId);
};

struct MsgNotifyPos
{
	MsgNotifyPos(Entity& ref);
	MsgHead msg{ .id = NotifyPos };
	uint64_t entityId;
	float x;
	float z;
	int eulerAnglesY;
	int hp = 0;
	MSGPACK_DEFINE(msg, entityId, x, z, eulerAnglesY, hp);
};
struct MsgChangeSkeleAnim
{
	MsgChangeSkeleAnim(Entity& ref, const std::string &refName, bool loop) :entityId((uint64_t)&ref), loop(loop), clipName(refName) {}
	MsgHead msg{ .id = ChangeSkeleAnim };
	uint64_t entityId;
	bool loop;
	std::string clipName;
	MSGPACK_DEFINE(msg, entityId, loop, clipName);
};

struct MsgGateת��
{
	MsgGateת��()
	{

	}
	MsgGateת��(const void* buf, int len, uint64_t uGateClientSessionId, uint32_t snParam) :vecByte(len), gateClientSessionId(uGateClientSessionId)
	{
		msg.sn = snParam;
		uint8_t* pBegin = (uint8_t*)buf;
		std::copy(pBegin, pBegin + len, vecByte.begin());
	}
	MsgHead msg{ .id = Gateת�� };
	uint64_t gateClientSessionId;
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, gateClientSessionId, vecByte);
};

struct MsgGateAddSession
{
	MsgHead msg{ .id = GateAddSession };
	std::string nickName;

	MSGPACK_DEFINE(msg, nickName);
};

struct MsgGateDeleteSession
{
	MsgHead msg{ .id = GateDeleteSession };
	MSGPACK_DEFINE(msg);
};

struct MsgGateDeleteSessionResponce
{
	MsgHead msg{ .id = GateDeleteSessionResponce };
	MSGPACK_DEFINE(msg);
};

struct Msg�ɼ�
{
	MsgHead msg{ .id = �ɼ� };
	double idĿ����Դ;//TypeScriptֻ��FLOAT64,û��POSITIVE_INTEGER��NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, idĿ����Դ);
};

struct Msg���ر�
{
	MsgHead msg{ .id = ���ر� };
	double idĿ��ر�;//TypeScriptֻ��FLOAT64,û��POSITIVE_INTEGER��NEGATIVE_INTEGER
	MSGPACK_DEFINE(msg, idĿ��ر�);
};

struct Msg���ر�
{
	MsgHead msg{ .id = ���ر� };
	double id�ر�;//TypeScriptֻ��FLOAT64,û��POSITIVE_INTEGER��NEGATIVE_INTEGER
	std::vector<uint64_t> vecId�ر��ڵ�λ;
	MSGPACK_DEFINE(msg, id�ر�, vecId�ر��ڵ�λ);
};

struct Msg��Դ
{
	MsgHead msg{ .id = ��Դ };
	uint32_t ȼ����;
	uint32_t ���λ;
	uint32_t ���λ����;
	MSGPACK_DEFINE(msg, ȼ����, ���λ, ���λ����);
};

struct Msg��Space
{
	MsgHead msg{ .id = ��Space};
	uint32_t idSapce;
	MSGPACK_DEFINE(msg, idSapce);
};

struct Msg�����˾��鸱��
{
	MsgHead msg{ .id = �����˾��鸱�� };
	MSGPACK_DEFINE(msg);
};

struct Msg��ʾ����
{
	MsgHead msg{ .id = ��ʾ���� };
	enum ��������
	{
		ѡ���ͼ,
	};
	�������� ui;
	MSGPACK_DEFINE(msg,ui);
};
MSGPACK_ADD_ENUM(Msg��ʾ����::��������);