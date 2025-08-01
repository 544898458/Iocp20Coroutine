from __future__ import annotations
import argparse
import asyncio
import random
import ssl
import websockets
from msgpack import unpackb, packb
from enum import Enum

# 存储所有单位数据的字典
单位数据字典 = {}

# 存储玩家个人战局列表
玩家个人战局列表 = []

class MsgId(Enum):
    MsgId_Invalid_0 = 0
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
    已解锁单位 = 47
    所有单位属性等级 = 48
    升级单位属性 = 49
    苔蔓半径 = 50
    太岁分裂 = 51
    Notify属性 = 52
    进房虫 = 53
    出房虫 = 54

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


async def send(websocket, arr) -> bool:
    print('发arr', arr)
    await websocket.send(packb(arr))
async def onRecvWorldSvr(arr,websocket) -> bool:
    idxArr = 0
    
    msg, sn = arr[idxArr]
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
            await send(websocket, [[MsgId.进Space.value, 0, 0, 0],1,])
            return True
        case _:
            print('未处理',idMsg)
            return False
    
随机 = 10000
async def 随机说话(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 发送消息，随机说话
        await send(websocket, [[MsgId.Say.value, 0, 0], f'测试说话{random.randint(1, 5)}'])

#随机框选
async def 框选全图(websocket):
    # 发送消息，随机框选
    await send(websocket, [[MsgId.框选.value, 0, 0],[-100, -100],[100, 100]])
        
#随机坐标点
def 随机坐标点():
    return [random.randint(-100, 100),random.randint(-100, 100)]

#随机move
async def 随机move(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        await 框选全图(websocket)
        
        await send(websocket, [[MsgId.Move.value, 0, 0], 随机坐标点(), True])

#随机造炮台
async def 造炮台(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        await send(websocket, [[MsgId.AddBuilding.value, 0], 单位类型.炮台.value, 随机坐标点()])

#随机造基地
async def 造基地(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        await send(websocket, [[MsgId.AddBuilding.value, 0], 单位类型.基地.value, 随机坐标点()])

#随机造工程车
async def 造工程车(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        await send(websocket, [[MsgId.AddRole.value, 0], 单位类型.工程车.value, 随机坐标点()])

#随机造民房
async def 造民房(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        await send(websocket, [[MsgId.AddBuilding.value, 0], 单位类型.民房.value, 随机坐标点()])

#随机造建筑
async def 随机造建筑(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 所有可建造的建筑类型
        建筑类型列表 = [
            单位类型.基地.value,      # 基地
            单位类型.兵厂.value,      # 兵营
            单位类型.民房.value,      # 民房
            单位类型.地堡.value,      # 地堡
            单位类型.炮台.value,      # 炮台
            单位类型.虫巢.value,      # 虫巢
            单位类型.机场.value,      # 机场
            单位类型.重车厂.value,    # 重车厂
            单位类型.虫营.value,      # 虫营
            单位类型.飞塔.value,      # 飞塔
            单位类型.拟态源.value,    # 拟态源
            单位类型.太岁.value       # 太岁
        ]
        # 随机选择一个建筑类型
        随机建筑类型 = random.choice(建筑类型列表)
        print(f'随机建造建筑: {随机建筑类型}')
        await send(websocket, [[MsgId.AddBuilding.value, 0], 随机建筑类型, 随机坐标点()])

#随机造人类单位
async def 随机造人类单位(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 所有可建造的人类单位类型
        人类单位列表 = [
            单位类型.工程车.value,    # 工程车
            单位类型.枪兵.value,      # 枪兵
            单位类型.近战兵.value,    # 近战兵
            单位类型.三色坦克.value,  # 三色坦克
            单位类型.飞机.value,      # 飞机
            单位类型.医疗兵.value     # 医疗兵
        ]
        # 随机选择一个单位类型
        随机单位类型 = random.choice(人类单位列表)
        print(f'随机建造人类单位: {随机单位类型}')

        # 根据单位类型选择对应的建筑
        if 随机单位类型 in [单位类型.枪兵.value, 单位类型.近战兵.value, 单位类型.医疗兵.value]:
            # 选中兵营
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.兵厂.value])
            print('选中兵营')
        elif 随机单位类型 == 单位类型.三色坦克.value:
            # 选中重车厂
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.重车厂.value])
            print('选中重车厂')
        elif 随机单位类型 == 单位类型.飞机.value:
            # 选中机场
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.机场.value])
            print('选中机场')
        elif 随机单位类型 == 单位类型.工程车.value:
            # 选中基地
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.基地.value])
            print('选中基地')

        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 建造单位
        await send(websocket, [[MsgId.AddRole.value, 0], 随机单位类型, 随机坐标点()])

#随机选中工程车采矿
async def 随机工程车采矿(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有工程车
        工程车列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] == 单位类型.工程车.value:
                工程车列表.append(id)
        
        if not 工程车列表:
            print('没有找到工程车')
            return
            
        # 随机选择一个工程车
        随机工程车id = random.choice(工程车列表)
        print(f'随机选择工程车: ID={随机工程车id}')
        
        # 选中工程车
        await send(websocket, [[MsgId.SelectRoles.value, 0], [随机工程车id], False])
        print('选中工程车')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 随机选择采集燃气矿或晶体矿
        矿类型列表 = [
            单位类型.晶体矿.value,  # 晶体矿
            单位类型.燃气矿.value   # 燃气矿
        ]
        随机矿类型 = random.choice(矿类型列表)
        矿名称 = "晶体矿" if 随机矿类型 == 单位类型.晶体矿.value else "燃气矿"
        
        # 发送采集命令
        await send(websocket, [[MsgId.采集.value, 0], 随机矿类型])
        print(f'工程车开始采集{矿名称}')

#随机选择单位进入地堡
async def 随机进地堡(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有地堡
        地堡列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] == 单位类型.地堡.value:
                地堡列表.append(id)
        
        if not 地堡列表:
            print('没有找到地堡')
            return
            
        # 从单位数据字典中找出所有可以进入地堡的单位
        可进地堡单位列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] in [
                单位类型.枪兵.value,      # 枪兵
                单位类型.近战兵.value,    # 近战兵
                单位类型.医疗兵.value     # 医疗兵
            ]:
                可进地堡单位列表.append(id)
        
        if not 可进地堡单位列表:
            print('没有找到可以进入地堡的单位')
            return
            
        # 随机选择一个地堡
        随机地堡id = random.choice(地堡列表)
        print(f'随机选择地堡: ID={随机地堡id}')
        
        # 随机选择1-3个单位进入地堡
        进入单位数量 = random.randint(1, min(3, len(可进地堡单位列表)))
        随机单位列表 = random.sample(可进地堡单位列表, 进入单位数量)
        print(f'随机选择{进入单位数量}个单位进入地堡')
        
        # 选中这些单位
        await send(websocket, [[MsgId.SelectRoles.value, 0], 随机单位列表, False])
        print('选中单位')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 发送进地堡命令
        await send(websocket, [[MsgId.进地堡.value, 0], 随机地堡id])
        print('单位开始进入地堡')

#随机选择单位强制移动
async def 随机强制移动(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有可以移动的单位
        可移动单位列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] in [
                单位类型.枪兵.value,      # 枪兵
                单位类型.近战兵.value,    # 近战兵
                单位类型.医疗兵.value,    # 医疗兵
                单位类型.工程车.value,    # 工程车
                单位类型.三色坦克.value   # 三色坦克
            ]:
                可移动单位列表.append(id)
        
        if not 可移动单位列表:
            print('没有找到可以移动的单位')
            return
            
        # 随机选择1-5个单位
        移动单位数量 = random.randint(1, min(5, len(可移动单位列表)))
        随机单位列表 = random.sample(可移动单位列表, 移动单位数量)
        print(f'随机选择{移动单位数量}个单位强制移动')
        
        # 选中这些单位
        await send(websocket, [[MsgId.SelectRoles.value, 0], 随机单位列表, False])
        print('选中单位')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 随机选择一个目标位置
        目标位置 = 随机坐标点()
        
        # 发送强制移动命令
        await send(websocket, [[MsgId.Move.value, 0, 0], 目标位置, True])  # True表示强制移动
        print(f'单位开始强制移动到位置: {目标位置}')

#随机选择单位坚守原地
async def 随机坚守原地(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有可以战斗的单位
        可战斗单位列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] in [
                单位类型.枪兵.value,      # 枪兵
                单位类型.近战兵.value,    # 近战兵
                单位类型.医疗兵.value,    # 医疗兵
                单位类型.三色坦克.value,  # 三色坦克
                单位类型.炮台.value       # 炮台
            ]:
                可战斗单位列表.append(id)
        
        if not 可战斗单位列表:
            print('没有找到可以战斗的单位')
            return
            
        # 随机选择1-3个单位
        坚守单位数量 = random.randint(1, min(3, len(可战斗单位列表)))
        随机单位列表 = random.sample(可战斗单位列表, 坚守单位数量)
        print(f'随机选择{坚守单位数量}个单位坚守原地')
        
        # 选中这些单位
        await send(websocket, [[MsgId.SelectRoles.value, 0], 随机单位列表, False])
        print('选中单位')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 发送坚守原地命令
        await send(websocket, [[MsgId.原地坚守.value, 0]])
        print('单位开始坚守原地')

#随机建造虫族单位
async def 随机造虫族单位(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 所有可建造的虫族单位类型
        虫族单位列表 = [
            单位类型.工虫.value,      # 工虫
            单位类型.枪虫.value,      # 枪虫
            单位类型.近战虫.value,    # 近战虫
            单位类型.房虫.value,      # 房虫
            单位类型.飞虫.value,      # 飞虫
            单位类型.幼虫.value       # 幼虫
        ]
        # 随机选择一个单位类型
        随机单位类型 = random.choice(虫族单位列表)
        print(f'随机建造虫族单位: {随机单位类型}')

        # 根据单位类型选择对应的建筑
        if 随机单位类型 in [单位类型.枪虫.value, 单位类型.近战虫.value]:
            # 选中虫营
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.虫营.value])
            print('选中虫营')
        elif 随机单位类型 == 单位类型.飞虫.value:
            # 选中飞塔
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.飞塔.value])
            print('选中飞塔')
        elif 随机单位类型 == 单位类型.工虫.value:
            # 选中虫巢
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.虫巢.value])
            print('选中虫巢')
        elif 随机单位类型 == 单位类型.房虫.value:
            # 选中太岁
            await send(websocket, [[MsgId.SelectRoles.value, 0], 单位类型.太岁.value])
            print('选中太岁')

        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 建造单位
        await send(websocket, [[MsgId.AddRole.value, 0], 随机单位类型, 随机坐标点()])
        print(f'开始建造虫族单位: {随机单位类型}')

#随机选择单位进入房虫
async def 随机进房虫(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有房虫
        房虫列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] == 单位类型.房虫.value:
                房虫列表.append(id)
        
        if not 房虫列表:
            print('没有找到房虫')
            return
            
        # 从单位数据字典中找出所有可以进入房虫的单位
        可进房虫单位列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] in [
                单位类型.枪虫.value,      # 枪虫
                单位类型.近战虫.value,    # 近战虫
                单位类型.工虫.value       # 工虫
            ]:
                可进房虫单位列表.append(id)
        
        if not 可进房虫单位列表:
            print('没有找到可以进入房虫的单位')
            return
            
        # 随机选择一个房虫
        随机房虫id = random.choice(房虫列表)
        print(f'随机选择房虫: ID={随机房虫id}')
        
        # 随机选择1-3个单位进入房虫
        进入单位数量 = random.randint(1, min(3, len(可进房虫单位列表)))
        随机单位列表 = random.sample(可进房虫单位列表, 进入单位数量)
        print(f'随机选择{进入单位数量}个单位进入房虫')
        
        # 选中这些单位
        await send(websocket, [[MsgId.SelectRoles.value, 0], 随机单位列表, False])
        print('选中单位')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 发送进房虫命令
        await send(websocket, [[MsgId.进房虫.value, 0], 随机房虫id])
        print('单位开始进入房虫')

#随机选择地堡让里面单位出来
async def 随机出地堡(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有地堡
        地堡列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] == 单位类型.地堡.value:
                地堡列表.append(id)
        
        if not 地堡列表:
            print('没有找到地堡')
            return
            
        # 随机选择一个地堡
        随机地堡id = random.choice(地堡列表)
        print(f'随机选择地堡: ID={随机地堡id}')
        
        # 选中地堡
        await send(websocket, [[MsgId.SelectRoles.value, 0], [随机地堡id], False])
        print('选中地堡')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 发送出地堡命令
        await send(websocket, [[MsgId.出地堡.value, 0]])
        print('地堡内的单位开始出来')

#随机选择房虫让里面单位出来
async def 随机出房虫(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有房虫
        房虫列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] == 单位类型.房虫.value:
                房虫列表.append(id)
        
        if not 房虫列表:
            print('没有找到房虫')
            return
            
        # 随机选择一个房虫
        随机房虫id = random.choice(房虫列表)
        print(f'随机选择房虫: ID={随机房虫id}')
        
        # 选中房虫
        await send(websocket, [[MsgId.SelectRoles.value, 0], [随机房虫id], False])
        print('选中房虫')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 发送出房虫命令
        await send(websocket, [[MsgId.出房虫.value, 0]])
        print('房虫内的单位开始出来')

#随机选择建筑升级属性
async def 随机升级建筑属性(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有可升级的建筑
        可升级建筑列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] in [
                单位类型.基地.value,      # 基地
                单位类型.兵厂.value,      # 兵营
                单位类型.虫巢.value,      # 虫巢
                单位类型.虫营.value,      # 虫营
                单位类型.重车厂.value,    # 重车厂
                单位类型.机场.value       # 机场
            ]:
                可升级建筑列表.append(id)
        
        if not 可升级建筑列表:
            print('没有找到可升级的建筑')
            return
            
        # 随机选择一个建筑
        随机建筑id = random.choice(可升级建筑列表)
        print(f'随机选择建筑: ID={随机建筑id}')
        
        # 选中建筑
        await send(websocket, [[MsgId.SelectRoles.value, 0], [随机建筑id], False])
        print('选中建筑')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 随机选择一个属性升级
        属性列表 = [
            "攻击力",
            "防御力",
            "生命值",
            "移动速度",
            "攻击速度"
        ]
        随机属性 = random.choice(属性列表)
        print(f'选择升级属性: {随机属性}')
        
        # 发送升级属性命令
        await send(websocket, [[MsgId.升级单位属性.value, 0], 随机属性])
        print(f'开始升级建筑属性: {随机属性}')

#随机选择建筑解锁单位
async def 随机解锁单位(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有可以解锁单位的建筑
        可解锁建筑列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] in [
                单位类型.基地.value,      # 基地
                单位类型.兵厂.value,      # 兵营
                单位类型.虫巢.value,      # 虫巢
                单位类型.虫营.value,      # 虫营
                单位类型.重车厂.value,    # 重车厂
                单位类型.机场.value,      # 机场
                单位类型.飞塔.value,      # 飞塔
                单位类型.拟态源.value     # 拟态源
            ]:
                可解锁建筑列表.append(id)
        
        if not 可解锁建筑列表:
            print('没有找到可以解锁单位的建筑')
            return
            
        # 随机选择一个建筑
        随机建筑id = random.choice(可解锁建筑列表)
        print(f'随机选择建筑: ID={随机建筑id}')
        
        # 选中建筑
        await send(websocket, [[MsgId.SelectRoles.value, 0], [随机建筑id], False])
        print('选中建筑')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 根据建筑类型选择可解锁的单位
        可解锁单位列表 = []
        建筑类型 = 单位数据字典[随机建筑id]['类型']
        
        if 建筑类型 == 单位类型.基地.value:
            可解锁单位列表 = [单位类型.工程车.value]
        elif 建筑类型 == 单位类型.兵厂.value:
            可解锁单位列表 = [单位类型.枪兵.value, 单位类型.近战兵.value, 单位类型.医疗兵.value]
        elif 建筑类型 == 单位类型.虫巢.value:
            可解锁单位列表 = [单位类型.工虫.value, 单位类型.幼虫.value]
        elif 建筑类型 == 单位类型.虫营.value:
            可解锁单位列表 = [单位类型.枪虫.value, 单位类型.近战虫.value]
        elif 建筑类型 == 单位类型.重车厂.value:
            可解锁单位列表 = [单位类型.三色坦克.value]
        elif 建筑类型 == 单位类型.机场.value:
            可解锁单位列表 = [单位类型.飞机.value]
        elif 建筑类型 == 单位类型.飞塔.value:
            可解锁单位列表 = [单位类型.飞虫.value]
        elif 建筑类型 == 单位类型.拟态源.value:
            可解锁单位列表 = [单位类型.绿色坦克.value]
            
        if not 可解锁单位列表:
            print('该建筑没有可解锁的单位')
            return
            
        # 随机选择一个单位解锁
        随机单位类型 = random.choice(可解锁单位列表)
        print(f'选择解锁单位: {随机单位类型}')
        
        # 发送解锁单位命令
        await send(websocket, [[MsgId.解锁单位.value, 0], 随机单位类型])
        print(f'开始解锁单位: {随机单位类型}')

#随机进入单人剧情副本
async def 随机进单人剧情副本(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        print('尝试进入单人剧情副本')
        # 发送进入单人剧情副本命令
        await send(websocket, [[MsgId.进单人剧情副本.value, 0]])
        print('已发送进入单人剧情副本命令')

#随机进入其他玩家多人战局
async def 随机进多人战局(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        print('尝试获取玩家多人战局列表')
        # 发送获取玩家多人战局列表命令
        await send(websocket, [[MsgId.玩家多人战局列表.value, 0]])
        print('已发送获取玩家多人战局列表命令')

#随机创建多人战局
async def 随机创建多人战局(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        print('尝试创建多人战局')
        # 随机生成一个战局名称
        战局名称 = f'随机战局_{random.randint(1000, 9999)}'
        print(f'创建战局名称: {战局名称}')
        
        # 发送创建多人战局命令
        await send(websocket, [[MsgId.创建多人战局.value, 0], 战局名称])
        print(f'已发送创建多人战局命令: {战局名称}')

#随机选中太岁分裂
async def 随机太岁分裂(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        # 从单位数据字典中找出所有太岁
        太岁列表 = []
        for id, 数据 in 单位数据字典.items():
            if 数据['类型'] == 单位类型.太岁.value:
                太岁列表.append(id)
        
        if not 太岁列表:
            print('没有找到太岁')
            return
            
        # 随机选择一个太岁
        随机太岁id = random.choice(太岁列表)
        print(f'随机选择太岁: ID={随机太岁id}')
        
        # 选中太岁
        await send(websocket, [[MsgId.SelectRoles.value, 0], [随机太岁id], False])
        print('选中太岁')
        
        # 等待一小段时间让选中命令生效
        await asyncio.sleep(0.1)
        
        # 发送分裂命令
        await send(websocket, [[MsgId.太岁分裂.value, 0]])
        print('太岁开始分裂')

#随机进入其他玩家个人战局
async def 随机进个人战局(websocket):
    rand = random.randint(1, 随机)
    if rand == 1:
        print('尝试获取玩家个人战局列表')
        # 发送获取玩家个人战局列表命令
        await send(websocket, [[MsgId.玩家个人战局列表.value, 0]])
        print('已发送获取玩家个人战局列表命令')

async def onRecvGameSvr(arr, websocket) -> bool:
    idxArr = 0
    
    msg, sn = arr[idxArr]
    idxArr += 1
    
    idMsg = MsgId(msg)
    match idMsg:
        case MsgId.NotifyPos:
            id = arr[idxArr];  idxArr += 1
            posX = arr[idxArr];  idxArr += 1
            posZ = arr[idxArr];  idxArr += 1
            eulerAnglesY = arr[idxArr];  idxArr += 1
            
            # 更新单位的位置和朝向信息
            if id in 单位数据字典:
                单位数据字典[id].update({
                    'posX': posX,
                    'posZ': posZ,
                    'eulerAnglesY': eulerAnglesY
                })
                print(f'更新单位位置: ID={id}, 位置=({posX}, {posZ}), 朝向={eulerAnglesY}')
            
            # print('NotifyPos', id)
            await 随机说话(websocket)
            await 随机move(websocket)
            await 随机造建筑(websocket)
            await 随机造人类单位(websocket)
            await 随机造虫族单位(websocket)
            await 随机工程车采矿(websocket)
            await 随机进地堡(websocket)
            await 随机强制移动(websocket)
            await 随机坚守原地(websocket)
            await 随机进房虫(websocket)
            await 随机出地堡(websocket)
            await 随机出房虫(websocket)
            await 随机升级建筑属性(websocket)
            await 随机解锁单位(websocket)
            await 随机进单人剧情副本(websocket)
            await 随机进多人战局(websocket)
            await 随机创建多人战局(websocket)
            await 随机太岁分裂(websocket)
            await 随机进个人战局(websocket)
            return True
        case MsgId.玩家多人战局列表:
            # 收到多人战局列表
            战局列表 = arr[idxArr]
            idxArr += 1
            print(f'收到多人战局列表，共{len(战局列表)}个战局')
            
            if not 战局列表:
                print('没有可用的多人战局，无法加入')
                return True
                
            # 随机选择一个战局
            随机战局 = random.choice(战局列表)
            print(f'随机选择战局: {随机战局}')
            
            try:
                # 发送进入其他玩家多人战局命令
                await send(websocket, [[MsgId.进其他玩家多人战局.value, 0], 随机战局])
                print(f'已发送进入多人战局命令: {随机战局}')
            except Exception as e:
                print(f'进入多人战局失败: {str(e)}')
            return True
        case MsgId.剧情对话:
            # 收到剧情对话消息，自动回复已看完
            print('收到剧情对话，自动回复已看完')
            await send(websocket, [[MsgId.剧情对话已看完.value, 0]])
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
            id = arr[idxArr];  idxArr += 1
            nickName = arr[idxArr];  idxArr += 1
            entityName = arr[idxArr];  idxArr += 1
            prefabName = arr[idxArr];  idxArr += 1
            类型 = arr[idxArr];  idxArr += 1
            hpMax = arr[idxArr];  idxArr += 1
            能量Max = arr[idxArr];  idxArr += 1
            
            # 存储单位数据，初始化位置和朝向为0
            单位数据字典[id] = {
                'nickName': nickName,
                'entityName': entityName,
                'prefabName': prefabName,
                '类型': 类型,
                'hpMax': hpMax,
                '能量Max': 能量Max,
                'posX': 0,
                'posZ': 0,
                'eulerAnglesY': 0
            }
            print(f'收到单位数据: ID={id}, 类型={类型}, 名称={nickName}')
            return True
        case MsgId.资源:
            return True
        case MsgId.Entity描述:
            id = arr[idxArr];  idxArr += 1
            desc = arr[idxArr];  idxArr += 1
            print(f'收到单位描述: ID={id}, 描述={desc}')
            # 更新单位数据字典中的描述
            单位数据字典[id]['描述'] = desc
            return True
        case MsgId.玩家个人战局列表:
            arr玩家 = arr[idxArr]
            idxArr += 1
            
            # 更新全局玩家个人战局列表
            global 玩家个人战局列表
            玩家个人战局列表 = arr玩家
            print(f'更新玩家个人战局列表，共{len(arr玩家)}个战局')
            
            if not 玩家个人战局列表:
                print('没有可用的个人战局')
                return True
                
            # 随机选择一个战局
            随机战局 = random.choice(玩家个人战局列表)
            print(f'随机选择个人战局: {随机战局}')
            
            try:
                # 发送进入其他玩家个人战局命令
                await send(websocket, [[MsgId.进其他玩家个人战局.value, 0], 随机战局])
                print(f'已发送进入个人战局命令: {随机战局}')
            except Exception as e:
                print(f'进入个人战局失败: {str(e)}')
            return True
        case MsgId.DelRoleRet:
            id = arr[idxArr]
            idxArr += 1
            
            # 从单位数据字典中删除该单位
            if id in 单位数据字典:
                del 单位数据字典[id]
                print(f'单位已删除: ID={id}')
            
            return True
        case MsgId.Say:
            content = arr[idxArr];  idxArr += 1
            channel = arr[idxArr];  idxArr += 1
            print(channel, '频道:', content)
            return True
        case MsgId.单位属性等级:
            return True
        case MsgId.升级单位属性:
            return True
        case MsgId.设置视口:
            return True
        case MsgId.SelectRoles:
            return True
        case MsgId.弹丸特效:
            return True
        case MsgId.Notify属性:
            return True
        case MsgId.离开Space:
            print('离开Space，清空单位数据字典')
            单位数据字典.clear()
            return True
        case _:
            print('onRecvGameSvr 未处理',idMsg)
            return False
  
async def 收到消息(reply, websocket) -> bool:
    index = 0
    msg, sn = reply[index]
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

    async with websockets.connect('wss://test.rtsgame.online:12348', 
                                ssl=ssl_context,
                                ping_interval=60,  # Adjust the interval as needed
                                ping_timeout=60      # Adjust the timeout as needed
  ) as websocket:
        print('已连上')
        
        sendMsgSn = 0
        sendMsgSn += 1  # Increment correctly
        
        # Construct the object correctly
        obj = [
            [MsgId.Login.value, sendMsgSn],
            0,
            str登录名,
            'Hello, world!pwd',
            17  # 版本号
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