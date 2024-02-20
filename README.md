# Iocp20Coroutine

#### 介绍
C++20,IOCP,Coroutine,TCP Server Framework  
用协程将IOCP异步回调改为同步编程
客户端项目在CocosCreator38Demo

#### 第一个协程，服务器玩家移动
功能：玩家走向目标点，每帧移动0.5米。  
注意：目标点的坐标，没有记录在任何成员变量或全局变量里，而是记录在协程的局部变量里。
这是此项目中真实运行的代码，并不是伪代码。
```c++
void MsgQueue::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	m_pSession->m_entity.ReplaceCo(	//替换协程
		[targetX, targetZ](Entity* pEntity, float& x, float& z, bool& stop)->CoTask<int>
		{
			const auto localTargetX = targetX;
			const auto localTargetZ = targetZ;
			while (true)
			{
				co_yield 0;//服务器主工作线程大循环，每次循环触发一次
				if (stop)
				{
					LOG(INFO) << "走向" << localTargetX << "," << localTargetZ << "的协程正常退出";
					co_return 0;
				}

				const auto step = 0.5f;
				x += localTargetX < x ? -step : step;
				z += localTargetZ < z ? -step : step;

				MsgNotifyPos msg = { (int)NotifyPos , (uint64_t)pEntity, x,z };
				Broadcast(msg);
			}
		});
}
```
如此简单的编程手段早就在lua、python、typescript、java、C#广泛使用，而C++却只能用boost或者腾讯协程库。  
boost或腾讯协程库内部都包含汇编语言代码，可见C++协程已经无法依赖语言内部的机制实现，必须通过偏门手段骗过操作系统。  
腾讯协程库的使用还有重大限制，局部变量的尺寸很小，稍微大一点就崩溃，异常抛出和捕获以及longjump都很容易崩溃，而这些在没用协程时完全正常。  
C++20标准采纳了微软的协程方案，此方案和C#的协程方案很像。没有任何汇编代码，在VS2022和GCC里代码相同，运行结果相同，而且不再有特殊的局部变量尺寸限制、异常抛出捕获限制、longjump限制。  
#### 软件架构
软件架构说明



#### 安装教程

1.  xxxx
2.  xxxx
3.  xxxx

#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
