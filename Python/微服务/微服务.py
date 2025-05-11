#环境变量要设置好 ALIBABA_CLOUD_ACCESS_KEY_ID ALIBABA_CLOUD_ACCESS_KEY_SECRET
#pip install fastapi
#pip install uvicorn
#pip install alibabacloud_green20220302==2.2.8
import os
from fastapi import FastAPI
import uvicorn
from alibabacloud_green20220302.client import Client
from alibabacloud_green20220302 import models
from alibabacloud_tea_openapi.models import Config
import json

# 创建FastAPI应用实例
app = FastAPI(title="MyFirstMicroservice", version="1.0.0")

# 定义基础路由
@app.get("/")
async def root():
    return {"message": "Hello 微服务世界！"}
  
# 定义带参数的路由示例
@app.get("/阿里云内容安全/{name}")
async def 阿里云内容安全(name: str, age: int = 20):
    # return {"message": f"你好 {name}！推测你大约{age}岁"}
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
        'content': '测试文本内容'
    }
    textModerationPlusRequest = models.TextModerationPlusRequest(
        # 检测类型
        service='llm_query_moderation',
        service_parameters=json.dumps(serviceParameters)
    )
  
    try:
        response = clt.text_moderation_plus(textModerationPlusRequest)
        if response.status_code == 200:
            # 调用成功
            result = response.body
            print('response success. result:{}'.format(result))
        else:
            print('response not success. status:{} ,result:{}'.format(response.status_code, response))
    except Exception as err:
        print(err)

    return result.data.risk_level


# 启动服务（仅在直接运行时执行）
if __name__ == "__main__":
    print("启动微服务（HTTPS模式）")
    uvicorn.run(
        app="__main__:app",
        host="0.0.0.0",  # 允许外部访问
        port=8000,       # 服务端口
        reload=True,     # 开发模式自动重载（生产环境应关闭）
        # HTTPS 配置（需要替换为你的证书路径）
        ssl_keyfile="test.rtsgame.online-private.key",  # 私钥文件路径
        ssl_certfile="test.rtsgame.online-cert.crt"  # 证书文件路径
    )
