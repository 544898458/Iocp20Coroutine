#include <string>
enum 单位类型;
namespace AliyunGreen
{
	bool Check(const std::string& refContentGbk);
	bool 记录战局结束(const uint32_t uSvrId, const std::string& nickName, int 战局类型, bool is赢);
	bool 记录击杀(const uint32_t uSvrId, const int 战局类型, const std::string& nickName攻击, const 单位类型 单位类型攻击, const std::string& nickName阵亡, const 单位类型 单位类型阵亡);
}