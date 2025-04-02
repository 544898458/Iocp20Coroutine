#pragma once
#include <msgpack.hpp>
#include "../IocpNetwork/MsgQueue.h"
#include "../CoRoutine/CoTask.h"

class Entity;
struct Position
{
	float x;
	float z;
	MSGPACK_DEFINE(x, z);
	bool operator==(const Position& refRight)const
	{
		return x == refRight.x && z == refRight.z;
	}
	void operator+=(const Position& refRight)
	{
		x += refRight.x;
		z += refRight.z;
	}
	Position operator-(const Position& refRight)const
	{
		Position pos(*this);
		pos.x -= refRight.x;
		pos.z -= refRight.z;
		return pos;
	}
	Position operator*(float f)const;
	Position operator+(const Position& refRight)const
	{
		Position pos(*this);
		pos.x += refRight.x;
		pos.z += refRight.z;
		return pos;
	}
	bool DistanceLessEqual(const Position& refPos, float fDistance)const;

	float DistancePow2(const Position& refPos)const;
	float Distance(const Position& refPos)const;

	float LengthPow2() const;

	float Length() const;
	
	/// <summary>
	/// ������һ��(Vector Normalization)
	/// </summary>
	Position ��һ��()const;
};
struct Rect
{
	Position pos����;
	Position pos����;
	float ��()const
	{
		return pos����.x - pos����.x;
	}
	int32_t ��Int32()const
	{
		return (int32_t)��();
	}
	float ��()const
	{
		return pos����.z - pos����.z;
	}
	int32_t ��Int32()const
	{
		return (int32_t)��();
	}
	bool �����˵�(const Position& pos)const
	{
		return
			pos����.x < pos.x && pos.x < pos����.x &&
			pos����.z < pos.z && pos.z < pos����.z;
	}
};

template <class _Traits>
std::basic_ostream<char, _Traits>& operator<<(std::basic_ostream<char, _Traits>& _Ostr, const Position& _ref)
{
	return _Ostr << "(" << _ref.x << "," << _ref.z << ")";
}

/// <summary>
/// ��ҳǿ��Ҫ��Э��:����Ƕ�����WS(WebSocket)���������� MsgPack ���л�
/// ΢��С����ǿ��Ҫ��Э�飺�����WSS��Ҳ�������㣬�����TLS1.3���м��Ƕ�����WS(WebSocket)���������� MsgPack ���л�
/// </summary>
enum MsgId
{
	/// <summary>
	/// ��Ч
	/// </summary>
	MsgId_Invalid_0,
	/// <summary>
	/// ��¼ 
	/// C=>S MsgLogin
	/// S=>C ��ӦMsgLoginResponce
	/// </summary>
	Login,
	/// <summary>
	/// �����ѡ�еĵ�λ�ƶ���Ŀ��λ��
	/// C=>S MsgMove
	/// </summary>
	Move,
	/// <summary>
	/// �����мӵ�λ��֪ͨǰ��
	/// S=>C MsgAddRoleRet
	/// </summary>
	AddRoleRet,
	/// <summary>
	/// ֪ͨǰ�ˣ���λλ�ú�Ѫ���仯
	/// S=>C MsgNotifyPos
	/// </summary>
	NotifyPos,
	/// <summary>
	/// ֪ͨǰ�ˣ���λ���Ź���������������
	/// S=>C MsgChangeSkeleAnim
	/// </summary>
	ChangeSkeleAnim,
	/// <summary>
	/// ϵͳ��ʾ���������
	/// S=>C C=>S MsgSay 
	/// </summary>
	Say,
	/// <summary>
	/// �������ѡ�е�λ
	/// C=>S	MsgSelectRoles
	/// </summary>
	SelectRoles,
	/// <summary>
	/// �����������λ��ѵ��������ս�������칤�̳���̹�ˣ�
	/// C=>S MsgAddRole
	/// </summary>
	AddRole,
	/// <summary>
	/// ֪ͨǰ��ɾ��һ����λ
	/// S=>C MsgDelRoleRet
	/// </summary>
	DelRoleRet,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	ChangeMoney,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	ChangeMoneyResponce,
	/// <summary>
	/// ��������콨����λ
	/// C=>S MsgAddBuilding
	/// </summary>
	AddBuilding,
	/// <summary>
	/// ֪ͨǰ��Ǯ�仯������Ǯû�ã�ǰ�˲�����ʾ�����Բ��������Ӳ߻�����˵��Ǯ���ھ������ݣ�
	/// </summary>
	NotifyeMoney,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	Gateת��,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	GateAddSession,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	GateAddSessionResponce,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	GateDeleteSession,
	/// <summary>
	/// ��̨���̼��ڲ�ͨ��
	/// </summary>
	GateDeleteSessionResponce,
	/// <summary>
	/// �������ɼ�
	/// </summary>
	�ɼ�,
	/// <summary>
	/// ֪ͨǰ�ˣ�ȼ���󡢾���󡢻��λ��
	/// Msg��Դ
	/// </summary>
	��Դ,
	/// <summary>
	/// �������ѡ�еĵ�λ���ر�
	/// </summary>
	���ر�,
	/// <summary>
	/// �������ѡ�еĵر��еı�ȫ������
	/// </summary>
	���ر�,
	/// <summary>
	/// ��������������ͼ������������ͼ��
	/// C=>S �ɹ��� S=>C
	/// ע�⣺�����˾��鸱���ɹ��������˶Ծֳɹ���Ҳ�Ƿ���ǰ�� Msg��Space
	/// </summary>
	��Space,
	/// <summary>
	/// �����������˾��鸱����ѵ��ս������ս��
	/// </summary>
	�����˾��鸱��,
	/// <summary>
	/// û��
	/// </summary>
	��ʾ����_û�õ�,
	/// <summary>
	/// ��������뿪������ͼ������������ͼ��
	/// </summary>
	�뿪Space,
	/// <summary>
	/// ֪ͨǰ����ʾ��λ������������ȡ�ѵ�����λ���ȡ��ɼ����ȣ�
	/// </summary>
	Entity����,
	/// <summary>
	/// ֪ͨǰ�˲�����������Ч
	/// </summary>
	��������,
	/// <summary>
	/// ������������׼ĳ��
	/// </summary>
	�����ӿ�,
	/// <summary>
	/// ��������ѡ��λ
	/// C=>S
	/// </summary>
	��ѡ,
	/// <summary>
	/// �����ȡȫ�ָ���ս���б�
	/// </summary>
	��Ҹ���ս���б�,
	/// <summary>
	/// �����������˵ĸ���ս�֣�ѵ��ս������ս��
	/// </summary>
	��������Ҹ���ս��,
	/// <summary>
	/// ��������ս�֣��ķ���ս��
	/// </summary>
	��������ս��,
	/// <summary>
	/// ��ȡȫ�ֶ���ս���б�
	/// </summary>
	��Ҷ���ս���б�,
	/// <summary>
	/// �����˵Ķ���ս��
	/// </summary>
	��������Ҷ���ս��,
	/// <summary>
	/// ѡ�п��й��̳�
	/// </summary>
	�л����й��̳�,
	/// <summary>
	/// ֪ͨǰ�˲�����̨������Ч������̨����Ŀ�꣩
	/// </summary>
	������Ч,
	/// <summary>
	/// ֪ͨǰ����ʾ����Ի�����
	/// S=>C
	/// </summary>
	����Ի�,
	/// <summary>
	/// ǰ�˸��߷������Ѿ������ҳ����Ի�
	/// C=>S
	/// </summary>
	����Ի��ѿ���,
	��������,
	GateSvrת��GameSvr��Ϣ����Ϸǰ��,
	GateSvrת��WorldSvr��Ϣ����Ϸǰ��,
	�����������λ�ļ����,
	��������,
	ԭ�ؼ���,
};
MSGPACK_ADD_ENUM(MsgId);

enum ��λ����
{
	��λ����_Invalid_0,

	��Ч,
	�ӿ�,

	��ԴMin�Ƿ� = 100,
	�����,//Minerals
	ȼ����,//Vespene Gas
	��ԴMax�Ƿ�,

	���λMin�Ƿ� = 200,
	���̳�,//�ռ乤�̳�Space Construction Vehicle�����Բɿ󣬲�����Ҳ���Լ򵥹���
	ǹ��,//½ս��ԱMarine��ֻ�ܹ��������ܲɿ�
	��ս��,//��������Firebat
	��ɫ̹��,//�����ǣ�����̹�ˣ�Siege Tank��
	����,//Drone
	�ɻ�,
	ǹ��,//Hydralisk
	��ս��,//Zergling
	�׳�,//Larva
	��ɫ̹��,//��Ⱥ��λ��ʵ������������
	���,//����ɫ̹�˷��䣬ֱ��ǰ�������б�ը
	����,//overload
	�ɳ�,//Mutalisk

	���λMax�Ƿ�,

	����Min�Ƿ� = 300,
	����,//ָ������(Command Center),���칤�̳�
	��Ӫ,//��Ӫ(Barracks)���������ս����̹�ˣ�����Ҫ�ع�����
	��,//����վ(Supply Depot)
	�ر�,//����; �ر�(Bunker),���Խ���
	��̨,//Photon Cannon
	�泲,//hatchery
	����,//Spaceport
	�ع���,//Factory 
	��Ӫ,//��Ӧ��Ӫ
	����,//Spore Conlony

	����Max�Ƿ�,

	��Min�Ƿ� = 400,
	ǹ���,
	��ս���,
	�����,
	ǹ����,
	��ս����,
	���̳���,
	��Max�Ƿ�,
};
MSGPACK_ADD_ENUM(��λ����);


struct MsgHead
{
	//void SetSn(uint32_t snParam) const
	//{
	//	const_cast<Msg*>(this)->sn = snParam;
	//}
	static MsgHead GetMsgId(msgpack::object obj)
	{
		if (obj.type != MSGPACK_OBJECT_ARRAY)
		{
			LOG(WARNING) << "���ݰ�ͷ���ʹ�:" << obj.type;
			return {};
		}
		if (obj.via.array.size < 1)
		{
			LOG(WARNING) << "���ݰ�ͷ����̫С:" << obj.via.array.size;
			return {};
		}
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
	uint32_t u32�汾��;
	MSGPACK_DEFINE(msg, name, pwd, u32�汾��);
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
		�ͻ��˰汾��ƥ��,
	};
	Error  result = OK;
	std::string str��ʾ;
	MSGPACK_DEFINE(msg, result, str��ʾ);
};
MSGPACK_ADD_ENUM(MsgLoginResponce::Error);

struct MsgAddRole
{
	MsgHead msg;
	��λ���� ����;
	MSGPACK_DEFINE(msg, ����);
};

struct MsgAddBuilding
{
	MsgHead msg;
	��λ���� ����;
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
	Position pos;
	bool b���������Զ�����;
	MSGPACK_DEFINE(msg, pos, b���������Զ�����);
};

enum SayChannel
{
	ϵͳ,
	����,
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
	MsgHead msg{ .id = MsgId::SelectRoles };
	std::vector<double> ids;//TypeScriptֻ��FLOAT64,û��POSITIVE_INTEGER��NEGATIVE_INTEGER
	bool b׷�� = false;
	MSGPACK_DEFINE(msg, ids, b׷��);
};

struct MsgAddRoleRet
{
	MsgAddRoleRet(Entity&);
	MsgHead msg{ .id = AddRoleRet };
	uint64_t entityId;
	std::string nickName;
	std::string entityName;
	std::string prefabName;
	int32_t	i32HpMax;
	��λ���� ����;
	MSGPACK_DEFINE(msg, entityId, nickName, entityName, prefabName, i32HpMax, ����);
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
	MsgChangeSkeleAnim(Entity& ref, const std::string& refName, bool loop) :entityId((uint64_t)&ref), loop(loop), clipName(refName) {}
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

struct MsgGateSvrת��GameSvr��Ϣ����Ϸǰ��
{
	MsgGateSvrת��GameSvr��Ϣ����Ϸǰ��()
	{

	}
	MsgGateSvrת��GameSvr��Ϣ����Ϸǰ��(const void* buf, int len) :vecByte(len)
	{
		uint8_t* pBegin = (uint8_t*)buf;
		std::copy(pBegin, pBegin + len, vecByte.begin());
	}
	MsgHead msg{ .id = GateSvrת��GameSvr��Ϣ����Ϸǰ�� };
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, vecByte);
};
struct MsgGateSvrת��WorldSvr��Ϣ����Ϸǰ��
{
	MsgGateSvrת��WorldSvr��Ϣ����Ϸǰ��()
	{

	}
	MsgGateSvrת��WorldSvr��Ϣ����Ϸǰ��(const void* buf, int len) :vecByte(len)
	{
		uint8_t* pBegin = (uint8_t*)buf;
		std::copy(pBegin, pBegin + len, vecByte.begin());
	}
	MsgHead msg{ .id = GateSvrת��WorldSvr��Ϣ����Ϸǰ�� };
	std::vector<uint8_t> vecByte;
	MSGPACK_DEFINE(msg, vecByte);
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
	uint32_t �����;
	uint32_t ȼ����;
	uint32_t ���λ;
	uint32_t ���λ����;
	MSGPACK_DEFINE(msg, �����, ȼ����, ���λ, ���λ����);
};

struct Msg��Space
{
	MsgHead msg{ .id = ��Space };
	uint32_t idSapce;
	MSGPACK_DEFINE(msg, idSapce);
};

enum ����ID
{
	����ID_�Ƿ�_MIN,
	ѵ��ս,
	����ս,
	����ս,
	ѵ��ս_��,
	����ս_��,
	����ID_�Ƿ�_MAX,

	����ID_�Ƿ�_MIN = 100,
	�ķ���ս,
	����ID_�Ƿ�_MAX,

	����������ͼ,
};
MSGPACK_ADD_ENUM(����ID);

struct Msg�����˾��鸱��
{
	MsgHead msg{ .id = �����˾��鸱�� };
	����ID id;
	MSGPACK_DEFINE(msg, id);
};

struct Msg��ʾ����
{
	MsgHead msg{ .id = ��ʾ����_û�õ� };
	enum ��������
	{
		ѡ���ͼ,
	};
	�������� ui;
	MSGPACK_DEFINE(msg, ui);
};
MSGPACK_ADD_ENUM(Msg��ʾ����::��������);


struct Msg�뿪Space
{
	MsgHead msg{ .id = �뿪Space };
	MSGPACK_DEFINE(msg);
};

struct MsgEntity����
{
	MsgHead msg{ .id = Entity���� };
	uint64_t idEntity;
	std::string str����;
	MSGPACK_DEFINE(msg, idEntity, str����);
};

struct Msg��������
{
	MsgHead msg{ .id = �������� };
	std::string str����;
	std::string str�ı�;
	MSGPACK_DEFINE(msg, str����, str�ı�);
};
struct Msg������������
{
	MsgHead msg{ .id = �������� };
	std::string strHttpsMp3;
	MSGPACK_DEFINE(msg, strHttpsMp3);
};

struct Msg�����ӿ�
{
	MsgHead msg{ .id = �����ӿ� };
	Position pos�ӿ�;
	MSGPACK_DEFINE(msg, pos�ӿ�);
};

struct Msg��ѡ
{
	MsgHead msg{ .id = ��ѡ };
	Position pos��ʼ;//������������
	Position pos����;
	MSGPACK_DEFINE(msg, pos��ʼ, pos����);
};

struct Msg��Ҹ���ս���б�
{
	MsgHead msg{ .id = ��Ҹ���ս���б� };
	MSGPACK_DEFINE(msg);
};


struct ��ҳ���
{
	std::string nickName;
	std::string sceneName;
	MSGPACK_DEFINE(nickName, sceneName);
};

struct Msg��Ҹ���ս���б�Responce
{
	MsgHead msg{ .id = ��Ҹ���ս���б� };
	std::vector<��ҳ���> vec����ս���е����;
	MSGPACK_DEFINE(msg, vec����ս���е����);
};

struct Msg��������Ҹ���ս��
{
	MsgHead msg{ .id = ��������Ҹ���ս�� };
	std::string nickName�������;
	MSGPACK_DEFINE(msg, nickName�������);
};

struct Msg��������ս��
{
	MsgHead msg{ .id = ��������ս�� };
	����ID id;
	MSGPACK_DEFINE(msg, id);
};

struct Msg��Ҷ���ս���б�
{
	MsgHead msg{ .id = ��Ҷ���ս���б� };
	MSGPACK_DEFINE(msg);
};

struct Msg��Ҷ���ս���б�Responce
{
	MsgHead msg{ .id = ��Ҷ���ս���б� };
	std::vector<��ҳ���> vec����ս���е�Host���;
	MSGPACK_DEFINE(msg, vec����ս���е�Host���);
};

struct Msg��������Ҷ���ս��
{
	MsgHead msg{ .id = ��������Ҷ���ս�� };
	std::string nickName�������;
	MSGPACK_DEFINE(msg, nickName�������);
};

struct Msg�л����й��̳�
{
	MsgHead msg{ .id = �л����й��̳� };
	MSGPACK_DEFINE(msg);
};

struct Msg������Ч
{
	//Msg������Ч(Entity& ref, const Position& refTarget) :entityId(ref.Id), pos��ʼ(ref.Pos()) {}
	MsgHead msg{ .id = ������Ч };
	uint64_t idEntity;
	uint64_t idEntityTarget;
	std::string ��Чname;
	MSGPACK_DEFINE(msg, idEntity, idEntityTarget, ��Чname);
};

struct Msg����Ի�
{
	MsgHead msg{ .id = ����Ի� };
	std::string strͷ����;
	std::string str������;
	std::string strͷ����;
	std::string str������;
	std::string str�Ի�����;
	bool b��ʾ�˳�������ť;
	MSGPACK_DEFINE(msg, strͷ����, str������, strͷ����, str������, str�Ի�����, b��ʾ�˳�������ť);
};
struct Msg����Ի��ѿ���
{
	MsgHead msg{ .id = ����Ի��ѿ��� };
	MSGPACK_DEFINE(msg);
};

struct Msg��������
{
	MsgHead msg{ .id = �������� };
	uint16_t u16����;
	std::vector<std::string> vec���NickName;
	MSGPACK_DEFINE(msg, u16����, vec���NickName);
};

struct Msg�����������λ�ļ����
{
	MsgHead msg{ .id = �����������λ�ļ���� };
	Position pos;
	MSGPACK_DEFINE(msg, pos);
};

struct Msgԭ�ؼ���
{
	MsgHead msg{ .id = ԭ�ؼ��� };
	MSGPACK_DEFINE(msg);
};
