# Iocp20Coroutine

#### 介绍

* C++20,IOCP,Coroutine,TCP Server Framework  
* 用协程将IOCP异步回调改为同步编程  
* 客户端项目在CocosCreator38Demo

#### 实例1：协程可以消除部分成员变量和全局变量

* 功能：玩家走向目标点，每帧移动0.5米。  
* 注意：目标点的坐标，没有记录在任何成员变量或全局变量里，而是记录在协程的局部变量里。  
* 这是此项目中真实运行的代码，并不是伪代码。

```C++
void MsgQueue::OnRecv(const MsgMove& msg)
{
	LOG(INFO) << "收到点击坐标:" << msg.x << "," << msg.z;
	const auto targetX = msg.x;
	const auto targetZ = msg.z;
	m_pSession->m_entity.ReplaceCo(	//替换协程
		[targetX, targetZ](Entity* pEntity, float& x, float& z, std::function<void()>& funCancel)->CoTask<int>
		{
			KeepCancel kc(funCancel);
			const auto localTargetX = targetX;
			const auto localTargetZ = targetZ;

			Broadcast(MsgChangeSkeleAnim(pEntity, "run"));
			
			while (true)
			{
				if (co_await CoTimer::WaitNextUpdate(funCancel))//服务器主工作线程大循环，每次循环触发一次
				{
					LOG(INFO) << "走向" << localTargetX << "," << localTargetZ << "的协程取消了";
					co_return 0;
				}

				const auto step = 0.5f;
				if (std::abs(localTargetX - x) < step && std::abs(localTargetZ - z) < step) {
					LOG(INFO) << "已走到" << localTargetX << "," << localTargetZ << "附近，协程正常退出";
					Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));
					co_return 0;
				}

				x += localTargetX < x ? -step : step;
				z += localTargetZ < z ? -step : step;

				Broadcast(MsgNotifyPos(pEntity, x, z));
			}
		});
	m_pSession->m_entity.m_coWalk.Run();//协程离开开始运行（运行到第一个co_await
}
```
* 如此简单的编程手段早就在lua、python、typescript、java、C#、golang广泛使用，而C++却只能用boost或者腾讯协程库。  
* boost或腾讯协程库内部都包含汇编语言代码，可见C++协程已经无法依赖语言内部的机制实现，必须通过偏门手段骗过操作系统。  
* 腾讯协程库的使用还有重大限制，局部变量的尺寸很小，稍微大一点就崩溃，异常抛出和捕获以及longjump都很容易崩溃，而这些在没用协程时完全正常。  
* C++20标准采纳了微软的协程方案，此方案和C#的协程方案很像。没有任何汇编代码，在VS2022和GCC里代码相同，运行结果相同，而且不再有特殊的局部变量尺寸限制、异常抛出捕获限制、longjump限制。

#### 实例2：协程替代OOP的多态、Switch/Case、分支逻辑

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

#### 实例3：协程同步等待一段时间替代定时器事件

* 功能：技能过程为前摇3秒，然后造成3段伤害，各段伤害之间有间隔，最后是后摇（硬直）和公共冷却（不可释放其它已冷却的技能）。  
* 注意：协程Wait把定时器事件改为同步编程方式，相同的有顺序关系的代码也按顺序关系排在一起（还支持循环和if/switch分支）。如果没有协程等待而用传统定时器事件实现，代码必然分散。  
* 这是此项目中真实运行的代码，并不是伪代码。

```C++
CoTask<int> Attack(Entity* pEntity, Entity* pDefencer, float& x, float& z, std::function<void()> &cancel)
{
	KeepCancel kc(cancel);

	Broadcast(MsgChangeSkeleAnim(pEntity, "attack"));//播放攻击动作
		
	if (co_await CoTimer::Wait(3000ms, cancel))//等3秒	前摇
		co_return 0;//协程取消

	pDefencer->Hurt(1);//第一次让对方伤1点生命

	if (co_await CoTimer::Wait(500ms, cancel))//等0.5秒
		co_return 0;//协程取消

	pDefencer->Hurt(3);//第二次让对方伤3点生命

	if(co_await CoTimer::Wait(500ms, cancel))//等0.5秒
		co_return 0;//协程取消

	pDefencer->Hurt(10);//第三次让对方伤10点生命

	if(co_await CoTimer::Wait(3000ms, cancel))//等3秒	后摇
		co_return 0;//协程取消

	Broadcast(MsgChangeSkeleAnim(pEntity, "idle"));//播放休闲待机动作
	
	if(co_await CoTimer::Wait(5000ms, cancel))//等5秒	公共冷却
		co_return 0;//协程取消
	
	co_return 0;//协程正常退出
}
```

#### 实例4：用协程辅助实现RPC网络消息的同步接收

* 同步接收回应的意思就是，前一行代码发送Reqire消息，下一行代码就能直接接收Response消息，而不是设置回调处理，也不是在某个很远处的代码用switch/函数字典接收回应。

```C++
//以同步编程的方式，向另一个服务器发送请求并等待返回
MsgConsumeMoneyResponce responce = co_await CoRpc<MsgConsumeMoneyResponce>::Send<MsgConsumeMoney>({ .consumeMoney = 3 }, SendToWorldSvr);```
```

#### 实例5：用协程操作数据库

```C++
DbPlayer &refDb = ...;
const bool ok = co_await g_CoDbPlayer.Save(refDb, fun);//可能存DB用了5秒，就会在5秒后执行下一句
if(!ok)
{
    LOG(ERROR) << "存数据库失败";
}
```

#### 软件架构

用相同数据库数据的一组服务器称为一个区。

* **GameSvr**：计算密集型，AOI，同步战斗；  
* **GateSvr**：网络收发送缓冲密集型，假设GameSvr里有1000玩家同时在线，每个玩家动作要同步给所有玩家，就是发1000个消息，这个消息发送完之前，都要有内存存着（1000个连接就是1000个缓存，哪怕发送内容相同也要复制出相同的缓存，不可优化）。使用2个GateSvr后，每个GateSvr负载500个玩家，GameSvr只与这2个GateSvr有连接，就只要向两个GateSvr各发1个消息，每个GateSvr收到消息后再广播给自己的500个玩家，这样就把GameSvr的网络连接数和内存用量都减少到原来的1/500；  
* **WorldSvr**:当出现2个GameSvr时，就出现了同账号踢出问题，所以必须有一种不可水平扩展中心服务器，称为WorldSvr，它负责管理同号踢出以及必须串行的数据库操作；

#### 安装教程

1. 安装VS2022社区版的C++桌面开发后就能直接打开解决方案;
2. 把此文件复制到C盘Windows文件夹下：Iocp20Coroutine/x64/Release/本地机器专用.ini
3. 用这个批处理一次启动三个服务器：Iocp20Coroutine/x64/Release/启动全部服务器.bat
4. 前端项目在 https://gitee.com/griffon2/cocos-creator38-demo

### 贡献名单

* 关卡：阳光下的一个虾(QQ 1179331661)  
* 首页底图：不问清阴(QQ 3969340418)  
* 3D模型：TripoAI、混元AI	
* 抠图：豆包AI  
* 地面纹理：夸克AI  
* 头像图片：文小言AI  
* 音乐：SunoAI  
* 绑定/动作：mixamo.com  
* 绑定/动作：小白阿姨(QQ 3276812319)、九里庆安(QQ 284102069)  
* 绑定/动作：大呲花(QQ 3029823814)、恒(QQ 2682677034)  
* 绑定/动作：天涳の翼(QQ 349070005)、适中(QQ 1136976220)  
* 绑定/动作：哈哈哈哈(QQ 2517969872)、一方狂三(QQ 1256604813)  
* 模型/动作：<span style="color: #a0ff50">荧瞳(QQ 287859992)、荒野乱斗(QQ 2930801690)</span>  
* 语音：若有道心(QQ 1602576119)、潭(QQ 1514475926)、<span style="color: #a0ff50">* 凌枭(QQ 2862703087)</span>  
* 程序：火凤凰(QQ 75187631)、<span style="color:#a0ff50">江沉晚吟时(QQ 3380125833)</span>  
* 程序：kunnka(QQ 954436840)、樱木花道(QQ 251949672)

### 协议
1. 使用此项目造成的一切风险由使用者承担；
2. 允许任何个人或组织将此项目用于学习和非商业用途；
3. 本项目贡献者有权（上文参与贡献名单中的个人或组织，以及项目所引用的第三方库的作者）将此项目用于闭源商业用途，无需向其它贡献者支付任何利益；
4. 任何个人或组织在未修改本项目任何内容的前提下，可以将此项目用于商业用途（比如用于教学盈利）；
5. 任何个人或组织在修改本项目内容并且开源修改后的内容的前提下，可以将此项目用于商业用途（开源的意思是你要先开源然后才能发布产品，而不是发布产品之后过了十年再开源，那不叫开源，那叫闭源）；
6. 参与贡献的方式非常宽松，贡献以下内容进入主分支都算参与贡献：一行代码、一个模型、一个动作、一段音效、一段音乐、一个图片；