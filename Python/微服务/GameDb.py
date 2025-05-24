#pip install fastapi uvicorn aiosqlite pydantic
import os
from fastapi import FastAPI
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
    单人ID_非法_MAX = 11

    # 多人模式ID范围
    多人ID_非法_MIN = 100
    四方对战 = 101
    多人ID_非法_MAX = 102

    # 多人混战ID范围
    多人混战ID_非法_MIN = 200
    多玩家混战 = 201
    多人混战ID_非法_MAX = 202


# 配置文件
class Config:
    DB_FILE = 'GameDb.sqlite3'

    @staticmethod
    def get_type_stats_file(type_id: int, category: str) -> str:
        if category:
            return f'C:/inetpub/wwwroot/排行榜/战局_{type_id}_{category}.json'
        return f'C:/inetpub/wwwroot/排行榜/战局_{type_id}.json'

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
                nickname TEXT,
                type INTEGER,
                wins INTEGER DEFAULT 0,
                losses INTEGER DEFAULT 0,
                PRIMARY KEY (nickname, type)
            )
        ''')
        await db.commit()
    yield  # This is where the app runs
    # Shutdown phase (add cleanup logic here if needed)

# Create FastAPI app with lifespan
app = FastAPI(
    title="MyFirstMicroservice",
    version="1.0.0",
    lifespan=lifespan  # Use the new lifespan context manager
)

# 定义玩家数据模型
class PlayerStats(BaseModel):
    nickname: str
    wins: int = 0
    losses: int = 0

class 战局结果参数(BaseModel):
    nickName: str
    type: 战局类型
    win: bool
# 定义基础路由
@app.get("/")
async def root():
    return {"message": "Hello 微服务世界！"}
  
# 定义带参数的路由示例
@app.post("/BattleResult/")
async def 战局结果(request: 战局结果参数):
    async with aiosqlite.connect(Config.DB_FILE) as db:
        # 首先检查玩家在该类型战局中是否存在
        cursor = await db.execute(
            'SELECT wins, losses FROM player_stats WHERE nickname = ? AND type = ?',
            (request.nickName, request.type)
        )
        player = await cursor.fetchone()
        
        if player is None:
            # 如果玩家在该类型战局中不存在，创建新记录
            if request.win:
                await db.execute(
                    'INSERT INTO player_stats (nickname, type, wins, losses) VALUES (?, ?, 1, 0)',
                    (request.nickName, request.type)
                )
            else:
                await db.execute(
                    'INSERT INTO player_stats (nickname, type, wins, losses) VALUES (?, ?, 0, 1)',
                    (request.nickName, request.type)
                )
        else:
            # 如果玩家在该类型战局中存在，更新胜负次数
            if request.win:
                await db.execute(
                    'UPDATE player_stats SET wins = wins + 1 WHERE nickname = ? AND type = ?',
                    (request.nickName, request.type)
                )
            else:
                await db.execute(
                    'UPDATE player_stats SET losses = losses + 1 WHERE nickname = ? AND type = ?',
                    (request.nickName, request.type)
                )
        await db.commit()
        
        # 获取更新后的数据
        cursor = await db.execute(
            'SELECT wins, losses FROM player_stats WHERE nickname = ? AND type = ?',
            (request.nickName, request.type)
        )
        updated_stats = await cursor.fetchone()
        
        #整个表按赢排序后写入json文件
        cursor = await db.execute(
            'SELECT nickname, type, wins, losses FROM player_stats ORDER BY wins DESC'
        )
        stats = await cursor.fetchall()
        
        # 将数据转换为带字段名的字典列表
        stats_list = [
            {
                "nickname": row[0],
                "type": row[1],
                "wins": row[2],
                "losses": row[3],
                "total_games": row[2] + row[3]
            }
            for row in stats
        ]
        
        # 获取所有战局类型
        all_types = [e.value for e in 战局类型 if e.name.endswith("MAX") is False and e.name.endswith("MIN") is False]

        # 为每个type生成“赢”和“输”排行榜文件
        for t in all_types:
            # 赢排行榜
            cursor = await db.execute(
                'SELECT nickname, type, wins, losses FROM player_stats WHERE type = ? AND wins > 0 ORDER BY wins DESC',
                (t,)
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
            win_file = Config.get_type_stats_file(t, "赢")
            os.makedirs(os.path.dirname(win_file), exist_ok=True)
            with open(win_file, 'w', encoding='utf-8') as f:
                json.dump(win_stats_list, f, ensure_ascii=False, indent=2)

            # 输排行榜
            cursor = await db.execute(
                'SELECT nickname, type, wins, losses FROM player_stats WHERE type = ? AND losses > 0 ORDER BY losses DESC',
                (t,)
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
            lose_file = Config.get_type_stats_file(t, "输")
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

# 获取玩家所有战局类型的统计信息
@app.get("/player/{nickname}")
async def get_player_stats(nickname: str):
    async with aiosqlite.connect(Config.DB_FILE) as db:
        cursor = await db.execute(
            'SELECT type, wins, losses FROM player_stats WHERE nickname = ?',
            (nickname,)
        )
        stats = await cursor.fetchall()
        
        if not stats:
            return {
                "nickname": nickname,
                "total_stats": {
                    "wins": 0,
                    "losses": 0,
                    "total_games": 0
                },
                "type_stats": []
            }
        
        # 计算总场次
        total_wins = sum(row[1] for row in stats)
        total_losses = sum(row[2] for row in stats)
        
        # 按类型整理数据
        type_stats = [
            {
                "type": row[0],
                "wins": row[1],
                "losses": row[2],
                "total_games": row[1] + row[2]
            }
            for row in stats
        ]
        
        return {
            "nickname": nickname,
            "total_stats": {
                "wins": total_wins,
                "losses": total_losses,
                "total_games": total_wins + total_losses
            },
            "type_stats": type_stats
        }

# 获取特定类型战局的统计信息
@app.get("/player/{nickname}/type/{type}")
async def get_player_type_stats(nickname: str, type: int):
    async with aiosqlite.connect(Config.DB_FILE) as db:
        cursor = await db.execute(
            'SELECT wins, losses FROM player_stats WHERE nickname = ? AND type = ?',
            (nickname, type)
        )
        stats = await cursor.fetchone()
        
        if stats is None:
            return {
                "nickname": nickname,
                "type": type,
                "wins": 0,
                "losses": 0,
                "total_games": 0
            }
            
        return {
            "nickname": nickname,
            "type": type,
            "wins": stats[0],
            "losses": stats[1],
            "total_games": stats[0] + stats[1]
        }

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