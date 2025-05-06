#pragma once
#include "../IocpNetwork/MsgPack.h"
#include "../IocpNetwork/StrConv.h"

#include <fstream>


template<class T>
T 从文件里读出对象(const std::string nickName)
{
	std::ostringstream oss;
	oss << StrConv::Utf8ToGbk(typeid(T).name()) << "_" << nickName << ".msgpack";
	const auto& strFileName = oss.str();
	// 打开文件
	std::ifstream in(strFileName, std::ios::binary);
	// 检查文件是否成功打开
	T objT;
	if (in)
	{
		// 获取文件大小
		in.seekg(0, std::ios::end);
		std::streamsize size = in.tellg();
		in.seekg(0, std::ios::beg);

		// 读取文件内容到vector
		std::vector<char> buffer(size);
		if (in.read(buffer.data(), size)) {
			// 成功读取数据，buffer中包含文件内容
			LOG(INFO) << "文件大小: " << size << " 字节" << std::endl;
		}
		else {
			LOG(ERROR) << "读取文件失败" << std::endl;
		}
		msgpack::object_handle oh = msgpack::unpack(buffer.data(), buffer.size());//没判断越界，要加try
		msgpack::object obj = oh.get();
		objT = obj.as<T>();
		in.close();
		LOG(INFO) << "已读出" << strFileName;
	}
	else
	{
		LOG(WARNING) << "无法打开文件" << strFileName;
		//dequeLocal.pop_front();
	}

	return objT;
}

template<class T>
T 写对象进文件(const T& ref, const std::string& strNickName)
{
	std::ostringstream oss;
	oss << StrConv::Utf8ToGbk(typeid(T).name()) << "_" << strNickName << ".msgpack";
	const auto& strFileName = oss.str();

	// 打开文件
	std::ofstream out(strFileName, std::ios::binary);

	// 检查文件是否成功打开
	if (!out)
	{
		LOG(ERROR) << "无法打开文件" << strFileName;
		//dequeLocal.pop_front();
		return ref;
	}

	MsgPack::SendMsgpack(ref, [&out](const void* buf, int len) { out.write((const char*)buf, len); }, false);

	// 关闭文件
	out.close();
	LOG(INFO) << "已写入" << strFileName;
	//模拟写硬盘很卡
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	return ref;
};