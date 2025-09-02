#环境变量要设置好 ALIBABA_CLOUD_ACCESS_KEY_ID ALIBABA_CLOUD_ACCESS_KEY_SECRET
#Windows开始按钮搜"编辑系统环境变量"
#pip install fastapi
#pip install uvicorn
#pip install alibabacloud_green20220302==2.2.8
#pip install pydantic
# pip install aiosqlite
# pip install requests
#https://www.rtsgame.online:8000/阿里云内容安全/你好呀
import os
from fastapi import FastAPI
import uvicorn
from alibabacloud_green20220302.client import Client
from alibabacloud_green20220302 import models
from alibabacloud_tea_openapi.models import Config
import json
from pydantic import BaseModel  # Add Pydantic import
import requests  # 添加requests库导入

# 新增数据库依赖导入
import aiosqlite
from contextlib import asynccontextmanager  # New import for lifespan
import time  # 添加time模块用于时间计算

# 新增：应用启动时初始化数据库表
# Replace @app.on_event with lifespan context manager
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup phase: Initialize database
    async with aiosqlite.connect('content_cache.db') as db:
        await db.execute('''
            CREATE TABLE IF NOT EXISTS content_cache (
                content TEXT PRIMARY KEY,  -- 使用内容作为唯一标识
                risk_level TEXT NOT NULL,  -- 存储风险等级
                cached_at DATETIME DEFAULT CURRENT_TIMESTAMP  -- 缓存时间
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

class 内容安全参数(BaseModel):
    content: str
# 定义基础路由
@app.get("/")
async def root():
    return {"message": "Hello 微服务世界！"}
  
# 定义带参数的路由示例
@app.post("/AliyunGreen/")
async def 阿里云内容安全(text_request: 内容安全参数):  # Use Pydantic model
    print('content', text_request.content)  # Get content from request body

    # 新增：先查询数据库缓存
    async with aiosqlite.connect('content_cache.db') as db:
        cursor = await db.execute('''
            SELECT risk_level FROM content_cache WHERE content = ?
        ''', (text_request.content,))
        cached_result = await cursor.fetchone()  # 返回 (risk_level,) 或 None
    
    if cached_result:
        print(f"找到缓存，直接返回。内容：{text_request.content[:20]}... 风险等级：{cached_result[0]}")
        return cached_result[0]  # 直接返回缓存的风险等级

    # 未找到缓存时，继续调用阿里云接口
    config = Config(
        # 阿里云账号AccessKey拥有所有API的访问权限，建议您使用RAM用户进行API访问或日常运维。
        # 强烈建议不要把AccessKey ID和AccessKey Secret保存到工程代码里，否则可能导致AccessKey泄露，威胁您账号下所有资源的安全。
        # 常见获取环境变量方式：
        # 获取RAM用户AccessKey ID：os.environ['ALIBABA_CLOUD_ACCESS_KEY_ID']
        # 获取RAM用户AccessKey Secret：os.environ['ALIBABA_CLOUD_ACCESS_KEY_SECRET']
        access_key_id = os.environ['ALIBABA_CLOUD_ACCESS_KEY_ID'],
        access_key_secret = os.environ['ALIBABA_CLOUD_ACCESS_KEY_SECRET'],
        # 连接超时时间 单位毫秒(ms)
        connect_timeout=10000,
        # 读超时时间 单位毫秒(ms)
        read_timeout=3000,
        region_id='cn-hangzhou',
        endpoint='green-cip.cn-hangzhou.aliyuncs.com'
    )
    clt = Client(config)
    serviceParameters = {
        'content': text_request.content  # Update to use model data
    }
    textModerationPlusRequest = models.TextModerationPlusRequest(
        # 检测类型
        service='llm_query_moderation',
        service_parameters=json.dumps(serviceParameters)
    )
  
    try:
        response = clt.text_moderation_plus(textModerationPlusRequest)
        if response.status_code == 200:
            result = response.body
            print('response success. result:{}'.format(result))
            

            # 新增：缓存结果到数据库（原有逻辑保持不变）
            risk_level = result.data.risk_level
            async with aiosqlite.connect('content_cache.db') as db:

                await db.execute('''
                    INSERT OR REPLACE INTO content_cache (content, risk_level)
                    VALUES (?, ?)
                ''', (text_request.content, risk_level))
                await db.commit()
                print(f"已缓存内容：{text_request.content[:20]}... 风险等级：{risk_level}")
        else:
            print('response not success. status:{} ,result:{}'.format(response.status_code, response))
    except Exception as err:
        print(err)

    return result.data.risk_level

@app.post("/WxaGameContentSpam/")
async def 微信游戏内容安全(text_request: 内容安全参数):
    """
    微信游戏内容安全检测接口
    参考文档：https://api.weixin.qq.com/wxa/game/content_spam/msg_sec_check?access_token=ACCESS_TOKEN
    """
    # 获取access_token（这里需要您实现获取access_token的逻辑）
    access_token = await get_wechat_access_token()
    if not access_token:
        return {"error": "无法获取微信access_token"}
    
    url = f"https://api.weixin.qq.com/wxa/game/content_spam/msg_sec_check?access_token={access_token}"
    
    # 构建请求数据
    data = {
        "content": text_request.content,
        "version": 2,  # 使用v2版本
        "scene": 1,    # 场景值，1表示游戏
        "openid": "test_openid"  # 可选，用于标识用户
    }
    
    try:
        response = requests.post(url, json=data, timeout=10)
        if response.status_code == 200:
            result = response.json()
            print(f"微信内容安全检测结果: {result}")
            
            # 处理返回结果
            if result.get("errcode") == 0:
                # 成功
                return {
                    "success": True,
                    "risk_level": "pass" if result.get("result", {}).get("suggest") == "pass" else "reject",
                    "detail": result.get("result", {})
                }
            else:
                # 失败
                return {
                    "success": False,
                    "error": result.get("errmsg", "未知错误"),
                    "errcode": result.get("errcode")
                }
        else:
            return {"error": f"HTTP请求失败: {response.status_code}"}
            
    except requests.exceptions.RequestException as e:
        print(f"请求异常: {e}")
        return {"error": f"网络请求异常: {str(e)}"}
    except Exception as e:
        print(f"其他异常: {e}")
        return {"error": f"处理异常: {str(e)}"}

# 全局变量用于缓存access_token
_wechat_token_cache = {
    "access_token": None,
    "expires_at": 0
}

async def get_wechat_access_token():
    """
    获取微信access_token，带1小时缓存机制
    """
    global _wechat_token_cache
    
    # 检查缓存是否还有效（提前5分钟刷新）
    current_time = time.time()
    if (_wechat_token_cache["access_token"] and 
        current_time < _wechat_token_cache["expires_at"] - 300):
        print("使用缓存的access_token")
        return _wechat_token_cache["access_token"]
    
    # 缓存过期或不存在，重新获取
    appid = os.environ.get('WECHAT_APPID')
    appsecret = os.environ.get('WECHAT_APPSECRET')
    
    if not appid or not appsecret:
        print("警告: 未设置微信APPID或APPSECRET环境变量")
        return None
    
    # 获取access_token的接口
    token_url = f"https://api.weixin.qq.com/cgi-bin/token?grant_type=client_credential&appid={appid}&secret={appsecret}"
    
    try:
        response = requests.get(token_url, timeout=10)
        if response.status_code == 200:
            result = response.json()
            if "access_token" in result:
                access_token = result["access_token"]
                expires_in = result.get("expires_in", 7200)  # 默认2小时
                
                # 更新缓存
                _wechat_token_cache["access_token"] = access_token
                _wechat_token_cache["expires_at"] = current_time + expires_in
                
                print(f"成功获取新的access_token，有效期至: {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(_wechat_token_cache['expires_at']))}")
                return access_token
            else:
                print(f"获取access_token失败: {result}")
                return None
        else:
            print(f"获取access_token HTTP失败: {response.status_code}")
            return None
    except Exception as e:
        print(f"获取access_token异常: {e}")
        return None

# 添加新的路由
@app.post("/WechatGreen/")
async def 微信内容安全接口(text_request: 内容安全参数):
    """微信游戏内容安全检测接口"""
    return await 微信游戏内容安全(text_request)

# 启动服务（仅在直接运行时执行）
if __name__ == "__main__":
    print("启动微服务（HTTPS模式）")
    uvicorn.run(
        app="__main__:app",
        host="0.0.0.0",  # 允许外部访问
        port=8000,       # 服务端口
        reload=True,     # 开发模式自动重载（生产环境应关闭）
        # HTTPS 配置（需要替换为你的证书路径）
        ssl_keyfile="rtsgame.online-private.key",  # 私钥文件路径
        ssl_certfile="rtsgame.online-cert.crt"  # 证书文件路径
    )


# Define request body model
class TextRequest(BaseModel):
    content: str