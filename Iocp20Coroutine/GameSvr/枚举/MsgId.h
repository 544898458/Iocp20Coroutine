#pragma once

/// <summary>
/// 网页强制要求协议:外层是二进制WS(WebSocket)，二进制用 MsgPack 序列化
/// 微信小程序强制要求协议：外层是WSS，也就是三层，最外层TLS1.3，中间是二进制WS(WebSocket)，最里面是 MsgPack 序列化
/// </summary>
enum MsgId :uint16_t
{
	/// <summary>
	/// 无效
	/// </summary>
	MsgId_Invalid_0,
	/// <summary>
	/// 登录 
	/// C=>S MsgLogin
	/// S=>C 回应MsgLoginResponce
	/// </summary>
	Login,
	/// <summary>
	/// 请求把选中的单位移动到目标位置
	/// C=>S MsgMove
	/// </summary>
	Move,
	/// <summary>
	/// 场景中加单位，通知前端
	/// S=>C MsgAddRoleRet
	/// </summary>
	AddRoleRet,
	/// <summary>
	/// 通知前端，单位位置和血量变化
	/// S=>C MsgNotifyPos
	/// </summary>
	NotifyPos,
	/// <summary>
	/// 通知前端，单位播放骨骼动画（动作）
	/// S=>C MsgChangeSkeleAnim
	/// </summary>
	ChangeSkeleAnim,
	/// <summary>
	/// 系统提示和玩家聊天
	/// S=>C C=>S MsgSay 
	/// </summary>
	Say,
	/// <summary>
	/// 玩家请求选中单位
	/// C=>S	MsgSelectRoles
	/// </summary>
	SelectRoles,
	/// <summary>
	/// 玩家请求造活动单位（训练兵、近战兵，制造工程车、坦克）
	/// C=>S MsgAddRole
	/// </summary>
	AddRole,
	/// <summary>
	/// 通知前端删除一个单位
	/// S=>C MsgDelRoleRet
	/// </summary>
	DelRoleRet,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	ChangeMoney,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	ChangeMoneyResponce,
	/// <summary>
	/// 玩家请求建造建筑单位
	/// C=>S MsgAddBuilding
	/// </summary>
	AddBuilding,
	/// <summary>
	/// 通知前端钱变化（现在钱没用，前端不会显示，所以不用做。从策划上来说，钱属于局外数据）
	/// </summary>
	NotifyeMoney,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	Gate转发,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateAddSession,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateAddSessionResponce,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateDeleteSession,
	/// <summary>
	/// 后台进程间内部通信
	/// </summary>
	GateDeleteSessionResponce,
	/// <summary>
	/// 玩家请求采集
	/// </summary>
	采集,
	/// <summary>
	/// 通知前端，燃气矿、晶体矿、活动单位数
	/// Msg资源
	/// </summary>
	资源,
	/// <summary>
	/// 玩家请求选中的单位进地堡
	/// </summary>
	进地堡,
	/// <summary>
	/// 玩家请求选中的地堡中的兵全部出来
	/// </summary>
	出地堡,
	/// <summary>
	/// 玩家请求进公共地图（多人联机地图）
	/// C=>S 成功后 S=>C
	/// 注意：进单人剧情副本成功、进多人对局成功，也是发给前端 Msg进Space
	/// </summary>
	进Space,
	/// <summary>
	/// 玩家请求进单人剧情副本（训练战、防守战）
	/// </summary>
	进单人剧情副本,
	/// <summary>
	/// 没用
	/// </summary>
	显示界面_没用到,
	/// <summary>
	/// 玩家请求离开公共地图（多人联机地图）
	/// </summary>
	离开Space,
	/// <summary>
	/// 通知前端显示单位描述（建造进度、训练活动单位进度、采集进度）
	/// </summary>
	Entity描述,
	/// <summary>
	/// 通知前端播放语音或音效
	/// </summary>
	播放声音,
	/// <summary>
	/// 让玩家摄像机对准某处
	/// </summary>
	设置视口,
	/// <summary>
	/// 玩家请求框选单位
	/// C=>S
	/// </summary>
	框选,
	/// <summary>
	/// 玩家拉取全局个人战局列表
	/// </summary>
	玩家个人战局列表,
	/// <summary>
	/// 玩家请求进别人的个人战局（训练战、防守战）
	/// </summary>
	进其他玩家个人战局,
	/// <summary>
	/// 创建多人战局（四方对战）
	/// </summary>
	创建多人战局,
	/// <summary>
	/// 拉取全局多人战局列表
	/// </summary>
	玩家多人战局列表,
	/// <summary>
	/// 进别人的多人战局
	/// </summary>
	进其他玩家多人战局,
	/// <summary>
	/// 选中空闲工程车
	/// </summary>
	切换空闲工程车,
	/// <summary>
	/// 通知前端播放炮台攻击特效（从炮台飞向目标）
	/// </summary>
	弹丸特效,
	/// <summary>
	/// 通知前端显示剧情对话界面
	/// S=>C
	/// </summary>
	剧情对话,
	/// <summary>
	/// 前端告诉服务器已经看完此页剧情对话
	/// C=>S
	/// </summary>
	剧情对话已看完,
	在线人数,
	GateSvr转发GameSvr消息给游戏前端,
	GateSvr转发WorldSvr消息给游戏前端,
	建筑产出活动单位的集结点,
	播放音乐,
	原地坚守,
	解锁单位,
	已解锁单位,
	所有单位属性等级,
	升级单位属性,
	苔蔓半径,
	太岁分裂,
	Notify属性,
};