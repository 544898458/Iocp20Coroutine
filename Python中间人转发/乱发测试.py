from __future__ import annotations
import asyncio
import ssl
import websockets
from msgpack import unpackb, packb
from enum import Enum

# Define MsgId using Python's Enum
class MsgId(Enum):
    Invalid = 0
    Login = 1
    Move = 2
    AddRoleRet = 3
    NotifyPos = 4
    ChangeSkeleAnim = 5
    Say = 6
    SelectRoles = 7
    AddRole = 8
    DelRoleRet = 9
    ChangeMoney = 10
    ChangeMoneyResponce = 11
    AddBuilding = 12
    NotifyeMoney = 13
    Gate转发 = 14
    GateAddSession = 15
    GateAddSessionResponce = 16
    GateDeleteSession = 17
    GateDeleteSessionResponce = 18
    采集 = 19
    资源 = 20
    进地堡 = 21
    出地堡 = 22
    进Space = 23
    进单人剧情副本 = 24
    显示界面_没用到 = 25
    离开Space = 26
    Entity描述 = 27
    播放声音 = 28
    设置视口 = 29
    框选 = 30
    玩家个人战局列表 = 31
    进其他玩家个人战局 = 32
    创建多人战局 = 33
    玩家多人战局列表 = 34
    进其他玩家多人战局 = 35
    切换空闲工程车 = 36
    弹丸特效 = 37
    剧情对话 = 38
    剧情对话已看完 = 39
    在线人数 = 40
    GateSvr转发GameSvr消息给游戏前端 = 41
    GateSvr转发WorldSvr消息给游戏前端 = 42
    建筑产出活动单位的集结点 = 43
    播放音乐 = 44
    原地坚守 = 45
    解锁单位 = 46
    所有单位属性等级 = 47
    升级单位属性 = 48
    苔蔓半径 = 49
    太岁分裂 = 50


async def onRecvWorldSvr(arr,websocket) -> bool:
    idxArr = 0
    
    msg, sn, rpc = arr[idxArr]
    idxArr += 1
    
    idMsg = MsgId(msg)
    match idMsg:
        case MsgId.在线人数:
            人数 = arr[idxArr]
            idxArr += 1
            
            arr昵称 = arr[idxArr]
            idxArr += 1
            
            str在线人数 = str(人数) + '人在线:'
            
            # if (!this.b登录成功) {
            #     this.b登录成功 = true;
            #     this.scene登录.nodeSelectSpace.active = true
            # }
            for str昵称 in arr昵称:
                str在线人数 += str昵称 + '、'
                
            print('str在线人数', str在线人数)

            # this.显示在线人数()
            global login_successful  # Declare the variable as global to modify it
            login_successful = True  # Set the flag to True
           
            #发消息，进多人联机混战
            arr = [[MsgId.进Space.value, 0, 0, 0],1,]
            print('发arr', arr)
            await websocket.send(packb(arr))
            return True
        case _:
            print('未处理',idMsg)
            return False
    
async def onRecvGameSvr(arr, websocket) -> bool:
    idxArr = 0
    
    msg, sn, rpc = arr[idxArr]
    idxArr += 1
    
    idMsg = MsgId(msg)
    match idMsg:
        case MsgId.NotifyPos:
            id = arr[idxArr];  idxArr += 1
            posX = arr[idxArr];  idxArr += 1
            posZ = arr[idxArr];  idxArr += 1
            eulerAnglesY = arr[idxArr];  idxArr += 1
            hp = arr[idxArr];  idxArr += 1
            能量 = arr[idxArr];  idxArr += 1
            print('NotifyPos', id)
            
            return True
        case MsgId.进Space:
            print('进Space', idMsg)
            return True
        case MsgId.播放音乐:
            print('播放音乐', idMsg)
            return True
        case _:
            print('onRecvGameSvr 未处理',idMsg)
            return False
  
async def 收到消息(reply, websocket) -> bool:
    index = 0
    msg, sn, rpc = reply[index]
    index += 1
    #idMsg转为 idMsg
    idMsg = MsgId(msg)
    match idMsg:
        case MsgId.Login:
            print('登录成功')
            return True
        case MsgId.GateSvr转发GameSvr消息给游戏前端:
            arrGameMsg = reply[index]
            index += 1
            print('GateSvr转发GameSvr消息给游戏前端 收到消息', arrGameMsg)
            await onRecvGameSvr(unpackb(arrGameMsg, strict_map_key=False), websocket)
            return True
        case MsgId.GateSvr转发WorldSvr消息给游戏前端:
            arrWorldMsg = reply[index] 
            index += 1
            print('GateSvr转发WorldSvr消息给游戏前端 收到消息', arrWorldMsg)
            await onRecvWorldSvr(unpackb(arrWorldMsg), websocket)
            return True
        case _:
            print('未处理',idMsg)
            return False

async def main() -> None:
    # Create SSL context if necessary
    ssl_context = ssl.create_default_context()

    async with websockets.connect('wss://test.rtsgame.online:12348', ssl=ssl_context) as websocket:
        print('已连上')
        
        sendMsgSn = 0
        sendMsgSn += 1  # Increment correctly
        
        str登录名 = 'test'  # Assign a proper value
        
        # Construct the object correctly
        obj = [
            [MsgId.Login.value, sendMsgSn, 0, 0],
            str登录名,
            'Hello, world!pwd',
            15  # 版本号
        ]
        
        request = packb(obj)  # Pack the correct object
        await websocket.send(request)
        print('已发出')
        
        while True:
            reply = await websocket.recv()
            obj = unpackb(reply)
            print('reply', obj)
            await 收到消息(obj, websocket)
            

asyncio.run(main())