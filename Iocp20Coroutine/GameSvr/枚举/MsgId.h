#pragma once

/// <summary>
/// ��ҳǿ��Ҫ��Э��:����Ƕ�����WS(WebSocket)���������� MsgPack ���л�
/// ΢��С����ǿ��Ҫ��Э�飺�����WSS��Ҳ�������㣬�����TLS1.3���м��Ƕ�����WS(WebSocket)���������� MsgPack ���л�
/// </summary>
enum MsgId :uint16_t
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
	������λ,
	�ѽ�����λ,
	���е�λ���Եȼ�,
	������λ����,
	̦���뾶,
	̫�����,
	Notify����,
};