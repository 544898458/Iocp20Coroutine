# Iocp20Coroutine

#### 介绍
C++20,IOCP,Coroutine,TCP Server Framework  
用协程将IOCP异步回调改为同步编程  
客户端项目在CocosCreator38Demo  

#### 一个实例：协程可以消除部分成员变量和全局变量
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
如此简单的编程手段早就在lua、python、typescript、java、C#、golang广泛使用，而C++却只能用boost或者腾讯协程库。  
boost或腾讯协程库内部都包含汇编语言代码，可见C++协程已经无法依赖语言内部的机制实现，必须通过偏门手段骗过操作系统。  
腾讯协程库的使用还有重大限制，局部变量的尺寸很小，稍微大一点就崩溃，异常抛出和捕获以及longjump都很容易崩溃，而这些在没用协程时完全正常。  
C++20标准采纳了微软的协程方案，此方案和C#的协程方案很像。没有任何汇编代码，在VS2022和GCC里代码相同，运行结果相同，而且不再有特殊的局部变量尺寸限制、异常抛出捕获限制、longjump限制。  

#### 另一个实例，协程替代OOP的多态、Switch/Case、分支逻辑

功能：有三个完成事件要区分处理，分别是Accept完成、Recv完成、Send完成，分别要执行完全不同的代码；  
注意：没用OOP多态、没用Switch/Case以及函数字典等任何分支逻辑，唯一核心调用就是resume协程；  
这是此项目中真实运行的代码，并不是伪代码。  
```C++
	void Overlapped::OnComplete(SocketCompeletionKey* pKey, const HANDLE port, const DWORD number_of_bytes, const BOOL bGetQueuedCompletionStatusReturn, const int lastErr)
	{
		this->numberOfBytesTransferred = number_of_bytes;
		this->GetQueuedCompletionStatusReturn = bGetQueuedCompletionStatusReturn;
		this->GetLastErrorReturn = lastErr;
		this->coTask.Run();//resume此协程
	}
```
上面代码中,Overlapped重叠结构在初始化时，就对coTask赋值了三个不同的协程中的一个，一旦协程resume，就会执行对应的协程的co_yield后面的代码，所以不用写任何逻辑分支判断。  
程序员思考的逻辑对应代码就是，有三种协程Accept、Recv、Send，它们在等待完成时suspend了，一旦事件完成，就resume。  
可以想象这三个协程内部也是一个while(true)循环，中间有co_yiled;  

#### 第三个实例，协程同步等待一段时间替代定时器事件
功能：技能过程为前摇3秒，然后造成3段伤害，伤害之间有间隔，最后是后摇和公共冷却。  
注意：协程Wait把定时器事件改为同步编程方式，相同的有顺序关系的代码也按顺序关系排在一起（还支持循环和if/switch分支）。如果没有协程等待而用传统定时器事件实现，代码必然分散。  
这是此项目中真实运行的代码，并不是伪代码。  
```C++
	CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, bool& stop)
	{
		{
			MsgChangeSkeleAnim msg(pEntity, "attack");//播放攻击动作
			Broadcast(msg);
		}

		co_await CoTimer::Wait(3000ms);//等3秒	前摇
		pDefencer->Hurt(1);//第一次让对方伤1点生命
		co_await CoTimer::Wait(500ms);//等0.5秒
		pDefencer->Hurt(3);//第二次让对方伤3点生命
		co_await CoTimer::Wait(500ms);//等0.5秒
		pDefencer->Hurt(10);//第三次让对方伤10点生命
		co_await CoTimer::Wait(3000ms);//等3秒	后摇
		{
			MsgChangeSkeleAnim msg(pEntity, "idle");//播放休闲待机动作
			Broadcast(msg);
		}

		co_await CoTimer::Wait(5000ms);//等5秒	公共冷却
	
		co_return 0;
	}
```

#### 用协程辅助实现RPC网络消息的同步接收


#### 软件架构
软件架构说明



#### 安装教程

1.  安装VS2022的C++桌面开发后就能直接打开解决方案运行;
2.  客户端项目在 https://gitee.com/griffon2/cocos-creator38-demo

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
