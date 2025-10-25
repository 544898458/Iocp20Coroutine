#pip install fastapi uvicorn aiosqlite pydantic
import os
from fastapi import FastAPI, Request
from fastapi.exceptions import RequestValidationError
from fastapi.responses import JSONResponse
import uvicorn
import json
from pydantic import BaseModel  # Add Pydantic import

# 新增数据库依赖导入
import aiosqlite
from contextlib import asynccontextmanager  # New import for lifespan
# 枚举类在Python中使用enum模块实现，原代码是类似C++/Java的枚举语法，在Python中需要重写
from enum import IntEnum

class 战局类型(IntEnum):
    # 单人模式ID范围
    单人ID_非法_MIN = 0
    新手训练_单位介绍_人 = 1
    新手训练_单位介绍_虫 = 2
    新手训练_反空降战_人 = 3
    新手训练_空降战_虫 = 4
    新手训练_战斗_人 = 5
    新手训练_战斗_虫 = 6
    防守战_人 = 7
    防守战_虫 = 8
    攻坚战_人 = 9
    攻坚战_虫 = 10
    中央防守_人 = 11
    新手训练_防空战_人 = 12
    新手训练_反防空战_虫 = 13
    单人ID_非法_MAX = 13

    # 多人模式ID范围
    多人ID_非法_MIN = 100
    四方对战 = 101
    多人ID_非法_MAX = 102

    # 多人混战ID范围
    多人混战ID_非法_MIN = 200
    多玩家混战 = 201
    多人混战ID_非法_MAX = 202

class 单位类型(IntEnum):
    单位类型_Invalid_0 = 0

    特效 = 1
    视口 = 2
    苔蔓 = 3  # Creep
    方墩 = 4  # 玩家造的阻挡

    资源Min非法 = 100
    晶体矿 = 101  # Minerals
    燃气矿 = 102  # Vespene Gas
    资源Max非法 = 103

    活动单位Min非法 = 200
    工程车 = 201  # 空间工程车Space Construction Vehicle。可以采矿，采气，也可以简单攻击
    枪兵 = 202  # 陆战队员Marine。只能攻击，不能采矿
    近战兵 = 203  # 火蝠，喷火兵Firebat
    三色坦克 = 204  # 不是攻城坦克（Siege Tank）
    工虫 = 205  # Drone
    飞机 = 206
    枪虫 = 207  # Hydralisk
    近战虫 = 208  # Zergling
    幼虫 = 209  # Larva
    绿色坦克 = 210  # 虫群单位，实际上是生物体
    光刺 = 211  # 由绿色坦克发射，直线前进，遇敌爆炸
    房虫 = 212  # overload
    飞虫 = 213  # Mutalisk
    医疗兵 = 214  # Medic
    防空兵 = 215
    活动单位Max非法 = 216

    建筑Min非法 = 300
    基地 = 301  # 指挥中心(Command Center),可造工程车
    兵营 = 302  # 兵营(Barracks)，造兵、近战兵、坦克（不需要重工厂）
    民房 = 303  # 供给站(Supply Depot)
    地堡 = 304  # 掩体; 地堡(Bunker),可以进兵
    炮台 = 305  # Photon Cannon
    虫巢 = 306  # hatchery
    机场 = 307  # Spaceport
    重车厂 = 308  # Factory
    虫营 = 309  # 对应兵营
    飞塔 = 310  # Spore Conlony
    拟态源 = 311  # 拟态源，原创，绿色坦克前置建筑
    太岁 = 312  # Creep Colony
    建筑Max非法 = 313

    怪Min非法 = 400
    枪虫怪 = 401
    近战虫怪 = 402
    工虫怪 = 403
    枪兵怪 = 404
    近战兵怪 = 405
    工程车怪 = 406
    幼虫怪 = 407
    飞虫怪 = 408
    绿色坦克怪 = 409
    房虫怪 = 410
    怪Max非法 = 411

# 配置文件
class Config:
    DB_FILE = 'GameDb.sqlite3'

    @staticmethod
    def get_type_stats_file(svr_id: int, type_id: int, category: str) -> str:
        if category:
            return f'C:/inetpub/wwwroot/排行榜/战局_{svr_id}_{type_id}_{category}.json'
        return f'C:/inetpub/wwwroot/排行榜/战局_{svr_id}_{type_id}.json'

# 创建FastAPI应用实例
# app = FastAPI(title="MyFirstMicroservice", version="1.0.0")

# 新增：应用启动时初始化数据库表
# Replace @app.on_event with lifespan context manager
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup phase: Initialize database
    async with aiosqlite.connect(Config.DB_FILE) as db:
        await db.execute('''
            CREATE TABLE IF NOT EXISTS player_stats (
                svr_id INTEGER,
                nickname TEXT,
                type INTEGER,
                wins INTEGER DEFAULT 0,
                losses INTEGER DEFAULT 0,
                PRIMARY KEY (svr_id, nickname, type)
            )
        ''')
        await db.commit()
        await db.execute('''
            CREATE TABLE IF NOT EXISTS unit_kill (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                svr_id INTEGER,
                battle_type INTEGER,
                killer TEXT,
                victim TEXT,
                killer_unit INTEGER,
                victim_unit INTEGER,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ''')
    yield  # This is where the app runs
    # Shutdown phase (add cleanup logic here if needed)

# Create FastAPI app with lifespan
app = FastAPI(
    title="MyFirstMicroservice",
    version="1.0.0",
    lifespan=lifespan  # Use the new lifespan context manager
)

# 添加全局异常处理器
@app.exception_handler(RequestValidationError)
async def validation_exception_handler(request: Request, exc: RequestValidationError):
    print("=== 请求验证错误 ===")
    print(f"错误详情: {exc}")
    print(f"请求URL: {request.url}")
    print(f"请求方法: {request.method}")
    print(f"请求头: {dict(request.headers)}")
    
    try:
        body = await request.body()
        print(f"请求体: {body}")
        if body:
            try:
                body_str = body.decode('utf-8')
                print(f"请求体字符串: {body_str}")
                
                # 检查是否是JSON解析错误
                if "json_invalid" in str(exc):
                    print("=== JSON解析错误分析 ===")
                    print("可能的原因：")
                    print("1. 中文字符编码问题")
                    print("2. 控制字符干扰")
                    print("3. JSON格式错误（缺少引号、逗号等）")
                    print("4. 特殊字符未正确转义")
                    
                    # 尝试修复常见的JSON问题
                    try:
                        import re
                        # 移除控制字符
                        cleaned_body = re.sub(r'[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]', '', body_str)
                        if cleaned_body != body_str:
                            print(f"清理后的JSON: {cleaned_body}")
                            
                        # 尝试解析清理后的JSON
                        import json
                        parsed = json.loads(cleaned_body)
                        print(f"清理后的JSON可以正常解析: {parsed}")
                    except Exception as parse_error:
                        print(f"清理后仍然无法解析: {parse_error}")
                        
            except UnicodeDecodeError as e:
                print(f"UTF-8解码错误: {e}")
                print("尝试使用其他编码...")
                try:
                    body_str = body.decode('gbk')
                    print(f"使用GBK解码: {body_str}")
                except:
                    print("GBK解码也失败")
            except:
                print("请求体无法解码为字符串")
    except Exception as e:
        print(f"获取请求体时出错: {e}")
    
    return JSONResponse(
        status_code=422,
        content={
            "error": "请求参数验证失败", 
            "details": str(exc), 
            "errcode": -20,
            "suggestion": "请检查JSON格式，确保中文字符正确编码，移除控制字符，检查引号和逗号是否正确"
        }
    )


class 战局结果参数(BaseModel):
    svr_id: int
    nickName: str
    type: 战局类型
    win: bool

class UnitKillRequest(BaseModel):
    svr_id: int
    battle_type: 战局类型
    killer: str
    killer_unit: 单位类型
    victim: str
    victim_unit: 单位类型

# 定义基础路由
@app.get("/")
async def root():
    return {"message": "Hello 微服务世界！"}
  
# 定义带参数的路由示例
@app.post("/BattleResult/")
async def 战局结果(request: 战局结果参数):
    print(f"=== 收到战局结果请求 ===")
    print(f"svr_id: {request.svr_id} (类型: {type(request.svr_id)})")
    print(f"nickName: {request.nickName} (类型: {type(request.nickName)})")
    print(f"type: {request.type} (类型: {type(request.type)})")
    print(f"win: {request.win} (类型: {type(request.win)})")
    async with aiosqlite.connect(Config.DB_FILE) as db:
        # 首先检查玩家在该类型战局中是否存在
        cursor = await db.execute(
            'SELECT wins, losses FROM player_stats WHERE svr_id = ? AND nickname = ? AND type = ?',
            (request.svr_id, request.nickName, request.type)
        )
        player = await cursor.fetchone()
        
        if player is None:
            # 如果玩家在该类型战局中不存在，创建新记录
            if request.win:
                await db.execute(
                    'INSERT INTO player_stats (svr_id, nickname, type, wins, losses) VALUES (?, ?, ?, 1, 0)',
                    (request.svr_id, request.nickName, request.type)
                )
            else:
                await db.execute(
                    'INSERT INTO player_stats (svr_id, nickname, type, wins, losses) VALUES (?, ?, ?, 0, 1)',
                    (request.svr_id, request.nickName, request.type)
                )
        else:
            # 如果玩家在该类型战局中存在，更新胜负次数
            if request.win:
                await db.execute(
                    'UPDATE player_stats SET wins = wins + 1 WHERE svr_id = ? AND nickname = ? AND type = ?',
                    (request.svr_id, request.nickName, request.type)
                )
            else:
                await db.execute(
                    'UPDATE player_stats SET losses = losses + 1 WHERE svr_id = ? AND nickname = ? AND type = ?',
                    (request.svr_id, request.nickName, request.type)
                )
        await db.commit()
        
        # 获取更新后的数据
        cursor = await db.execute(
            'SELECT wins, losses FROM player_stats WHERE svr_id = ? AND nickname = ? AND type = ?',
            (request.svr_id, request.nickName, request.type)
        )
        updated_stats = await cursor.fetchone()
                
        # 获取所有战局类型
        all_types = [e.value for e in 战局类型 if e.name.endswith("MAX") is False and e.name.endswith("MIN") is False]

        # 为每个type生成“赢”和“输”排行榜文件
        for t in all_types:
            # 赢排行榜
            cursor = await db.execute(
                'SELECT nickname, type, wins, losses FROM player_stats WHERE svr_id = ? AND type = ? AND wins > 0 ORDER BY wins DESC LIMIT 16',
                (request.svr_id, t)
            )
            win_stats = await cursor.fetchall()
            win_stats_list = [
                {
                    "nickname": row[0],
                    "type": row[1],
                    "wins": row[2],
                    "losses": row[3],
                    "total_games": row[2] + row[3]
                }
                for row in win_stats
            ]
            win_file = Config.get_type_stats_file(request.svr_id, t, "赢")
            os.makedirs(os.path.dirname(win_file), exist_ok=True)
            with open(win_file, 'w', encoding='utf-8') as f:
                json.dump(win_stats_list, f, ensure_ascii=False, indent=2)

            # 输排行榜
            cursor = await db.execute(
                'SELECT nickname, type, wins, losses FROM player_stats WHERE svr_id = ? AND type = ? AND losses > 0 ORDER BY losses DESC LIMIT 16',
                (request.svr_id, t)
            )
            lose_stats = await cursor.fetchall()
            lose_stats_list = [
                {
                    "nickname": row[0],
                    "type": row[1],
                    "wins": row[2],
                    "losses": row[3],
                    "total_games": row[2] + row[3]
                }
                for row in lose_stats
            ]
            lose_file = Config.get_type_stats_file(request.svr_id, t, "输")
            os.makedirs(os.path.dirname(lose_file), exist_ok=True)
            with open(lose_file, 'w', encoding='utf-8') as f:
                json.dump(lose_stats_list, f, ensure_ascii=False, indent=2)

        return {
            "message": "战局记录已更新",
            "nickname": request.nickName,
            "type": request.type,
            "wins": updated_stats[0],
            "losses": updated_stats[1],
            "total_games": updated_stats[0] + updated_stats[1]
        }

@app.post("/UnitKill/")
async def add_unit_kill(request: UnitKillRequest):
    print(f"=== 收到单位击杀请求 ===")
    print(f"svr_id: {request.svr_id} (类型: {type(request.svr_id)})")
    print(f"battle_type: {request.battle_type} (类型: {type(request.battle_type)})")
    print(f"killer: {request.killer} (类型: {type(request.killer)})")
    print(f"killer_unit: {request.killer_unit} (类型: {type(request.killer_unit)})")
    print(f"victim: {request.victim} (类型: {type(request.victim)})")
    print(f"victim_unit: {request.victim_unit} (类型: {type(request.victim_unit)})")
    
    # 清理victim字段中的控制字符
    if request.victim:
        import re
        # 移除控制字符（除了换行符和制表符）
        cleaned_victim = re.sub(r'[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]', '', request.victim)
        if cleaned_victim != request.victim:
            print(f"清理victim字段: '{request.victim}' -> '{cleaned_victim}'")
            request.victim = cleaned_victim
    async with aiosqlite.connect(Config.DB_FILE) as db:
        cursor = await db.execute(
            '''
            INSERT INTO unit_kill (svr_id, battle_type, killer, victim, killer_unit, victim_unit)
            VALUES (?, ?, ?, ?, ?, ?)
            ''',
            (request.svr_id, request.battle_type, request.killer, request.victim, request.killer_unit, request.victim_unit)
        )
        await db.commit()
        last_id = cursor.lastrowid
        
        # 获取该战局类型的最新X条击杀记录
        cursor = await db.execute(
            'SELECT * FROM unit_kill WHERE svr_id = ? AND battle_type = ? ORDER BY timestamp DESC LIMIT 200',
            (request.svr_id, request.battle_type)
        )
        unit_kills = await cursor.fetchall()
        
        # 将查询结果转换为字典列表
        unit_kills_list = []
        for row in unit_kills:
            unit_kills_list.append({
                "id": row[0],
                "svr_id": row[1],
                "battle_type": row[2],
                "killer": row[3],
                "victim": row[4],
                "killer_unit": row[5],
                "victim_unit": row[6],
                "timestamp_utc": f"{row[7]} UTC"
            })
        
        # 写入JSON文件
        json_file = f'C:/inetpub/wwwroot/战报/战局_{request.svr_id}_{request.battle_type}.json'
        os.makedirs(os.path.dirname(json_file), exist_ok=True)
        with open(json_file, 'w', encoding='utf-8') as f:
            json.dump(unit_kills_list, f, ensure_ascii=False, indent=2)
        
        await 写入战报排行榜(request.svr_id, request.battle_type)
        
    return {
        "message": "击杀事件已记录并更新JSON文件", 
        "id": last_id,
        "json_file": json_file
    }

async def 写入战报排行榜(svr_id: int, battle_type: 战局类型):
    print(f"=== 收到单位击杀排行榜请求 ===")
    print(f"svr_id: {svr_id} (类型: {type(svr_id)})")
    print(f"battle_type: {battle_type} (类型: {type(battle_type)})")
    
    # 定义统计配置：字段名、文件名后缀、显示名称
    stats_configs = [
        {
            "field": "killer",
            "key_name": "killer", 
            "file_suffix": "击败单位数",
            "display_name": "击杀"
        },
        {
            "field": "victim", 
            "key_name": "victim",
            "file_suffix": "被击败单位数", 
            "display_name": "被击杀"
        }
    ]
    
    async with aiosqlite.connect(Config.DB_FILE) as db:
        for config in stats_configs:
            # 执行查询
            cursor = await db.execute(
                f'SELECT {config["field"]}, COUNT(*) FROM unit_kill WHERE svr_id = ? AND battle_type = ? GROUP BY {config["field"]} ORDER BY COUNT(*) DESC LIMIT 200',
                (svr_id, battle_type)
            )
            results = await cursor.fetchall()
            
            # 将查询结果转换为字典列表
            stats_list = [
                {
                    config["key_name"]: row[0],
                    "count": row[1]
                }
                for row in results
            ]
            
            # 写入JSON文件
            json_file = f'C:/inetpub/wwwroot/战报/战报排行_{svr_id}_{battle_type}_{config["file_suffix"]}.json'
            os.makedirs(os.path.dirname(json_file), exist_ok=True)
            with open(json_file, 'w', encoding='utf-8') as f:
                json.dump(stats_list, f, ensure_ascii=False, indent=2)
            
            print(f"已生成{config['display_name']}排行榜: {json_file}")

        

# 启动服务（仅在直接运行时执行）
if __name__ == "__main__":
    print("启动微服务（HTTPS模式）")
    uvicorn.run(
        app="__main__:app",
        host="0.0.0.0",  # 允许外部访问
        port=8001,       # 服务端口
        reload=True,     # 开发模式自动重载（生产环境应关闭）
        # HTTPS 配置（需要替换为你的证书路径）
        ssl_keyfile="rtsgame.online-private.key",  # 私钥文件路径
        ssl_certfile="rtsgame.online-cert.crt"  # 证书文件路径
    )


# Define request body model
class TextRequest(BaseModel):
    content: str