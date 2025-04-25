from __future__ import annotations
import argparse
import asyncio
import random
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

class 单位类型(Enum):
    单位类型_Invalid_0 = 0  # Define the missing value
    特效 = 1
    视口 = 2
    苔蔓 = 3  # Creep
    方墩 = 4  # Player-built block

    资源Min非法 = 100
    晶体矿 = 101  # Minerals
    燃气矿 = 102  # Vespene Gas

    活动单位Min非法 = 200
    工程车 = 201  # Space Construction Vehicle
    枪兵 = 202  # Marine
    近战兵 = 203  # Firebat
    三色坦克 = 204
    工虫 = 205
    飞机 = 206
    枪虫 = 207  # Hydralisk
    近战虫 = 208  # Zergling
    幼虫 = 209  # Larva
    绿色坦克 = 210  # Swarm unit, actually a biological entity
    光刺 = 211  # Emitted by green tank, moves forward and explodes on contact
    房虫 = 212  # Overload
    飞虫 = 213  # Mutalisk
    医疗兵 = 214  # Medic

    活动单位Max非法 = 300

    建筑Min非法 = 300
    基地 = 301  # Command Center
    兵厂 = 302  # Barracks
    民房 = 303  # Supply Depot
    地堡 = 304  # Bunker
    炮台 = 305  # Photon Cannon
    虫巢 = 306  # Hatchery
    机场 = 307  # Spaceport
    重车厂 = 308  # Factory
    虫营 = 309  # Corresponding to barracks
    飞塔 = 310  # Spore Colony
    拟态源 = 311  # Original, prerequisite for green tanks
    太岁 = 312  # Creep Colony

    建筑Max非法 = 400

    怪Min非法 = 400
    枪虫怪 = 401
    近战虫怪 = 402
    工虫怪 = 403
    枪兵怪 = 404
    近战兵怪 = 405
    怪Max非法 = 500


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
    
随机 = 1000
async def 随机说话(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 发送消息，随机说话
        object =[[MsgId.Say.value, 0, 0], f'测试说话{random.randint(1, 5)}']
        await websocket.send(packb(object))

#随机框选
async def 随机框选(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 发送消息，随机框选
        object =[[MsgId.框选.value, 0, 0],[-100, -100],[100, 100]]
        await websocket.send(packb(object))
        
#随机坐标点
def 随机坐标点():
    return [random.randint(-100, 100),random.randint(-100, 100)]

#随机move
async def 随机move(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 发送消息，随机move
        object =[[MsgId.Move.value, 0, 0], 随机坐标点(), True]
        await websocket.send(packb(object))

#随机造炮台
async def 造炮台(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        object =[[MsgId.AddBuilding.value, 0, 0], 单位类型.炮台.value, 随机坐标点()]
        await websocket.send(packb(object))
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
            # print('NotifyPos', id)
            await 随机说话(websocket)
            await 随机框选(websocket)
            await 随机move(websocket)
            await 造炮台(websocket)
            return True
        case MsgId.进Space:
            print('进Space', idMsg)
            return True
        case MsgId.播放音乐:
            print('播放音乐', idMsg)
            return True
        case MsgId.ChangeSkeleAnim:
            return True
        case MsgId.播放声音:
            return True
        case MsgId.AddRoleRet:
            return True
        case MsgId.资源:
            return True
        case MsgId.Entity描述:
            return True
        case MsgId.DelRoleRet:
            return True
        case MsgId.Say:
            content = arr[idxArr];  idxArr += 1
            channel = arr[idxArr];  idxArr += 1
            print(channel, '频道:', content)
            return True
        case MsgId.所有单位属性等级:
            return True
        case MsgId.升级单位属性:
            return True
        case MsgId.设置视口:
            return True
        case MsgId.SelectRoles:
            return True
        case MsgId.弹丸特效:
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
            # print('GateSvr转发GameSvr消息给游戏前端 收到消息', arrGameMsg)
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
       
def parse_arguments():
    parser = argparse.ArgumentParser(description='中间人转发程序')
    parser.add_argument('--username', type=str, required=True, help='登录用户名')
    return parser.parse_args()
async def main() -> None:
    args = parse_arguments()
    str登录名 = args.username
    print(str登录名)
    # Create SSL context if necessary
    ssl_context = ssl.create_default_context()

    async with websockets.connect('wss://test.rtsgame.online:12348', ssl=ssl_context) as websocket:
        print('已连上')
        
        sendMsgSn = 0
        sendMsgSn += 1  # Increment correctly
        
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
            # print('reply', obj)
            await 收到消息(obj, websocket)
            


asyncio.run(main())