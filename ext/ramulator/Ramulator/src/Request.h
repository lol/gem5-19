#ifndef __REQUEST_H
#define __REQUEST_H

#include <vector>
#include <functional>

using namespace std;

namespace ramulator
{

class Request
{
public:
    bool is_first_command;
    long addr;
    // long addr_row;
    vector<int> addr_vec;
    // specify which core this request sent from, for virtual address translation
    int coreid;

    enum class Type
    {
        READ,
        WRITE,
        REFRESH,
        POWERDOWN,
        SELFREFRESH,
        EXTENSION,
        MAX
    } type;

    long arrive = -1;
    long depart;
    function<void(Request&)> callback; // call back with more info
    // gagan : is prefetch
    bool is_prefetch;

 Request(long addr, Type type, int coreid = 0)
      : is_first_command(true), addr(addr), coreid(coreid), type(type), callback([](Request& req){}), is_prefetch(false) {}

 Request(long addr, Type type, function<void(Request&)> callback,  bool is_prefetch, int coreid = 0)
   : is_first_command(true), addr(addr), coreid(coreid), type(type), callback(callback), is_prefetch(is_prefetch) {}

 Request(vector<int>& addr_vec, Type type, function<void(Request&)> callback, bool is_prefetch = false, int coreid = 0)
      : is_first_command(true), addr_vec(addr_vec), coreid(coreid), type(type), callback(callback) {}

    Request()
        : is_first_command(true), coreid(0) {}

    void print()
    {
      switch(type)
	{
	case Request::Type::READ:
	  std::cout << "[READ] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	case Request::Type::WRITE:
	  assert(is_prefetch == false);
	  std::cout << "[WRITE] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	case Request::Type::REFRESH:
	  assert(is_prefetch == false);
	  std::cout << "[REFRESH] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	case Request::Type::POWERDOWN:
	  assert(is_prefetch == false);
	  std::cout << "[POWERDOWN] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	case Request::Type::SELFREFRESH:
	  assert(is_prefetch == false);
	  std::cout << "[SELFREFRESH] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	case Request::Type::EXTENSION:
	  assert(is_prefetch == false);
	  std::cout << "[EXTENSION] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	case Request::Type::MAX:
	  assert(is_prefetch == false);
	  std::cout << "[MAX] pa[0x" << std::hex << addr << "] " << std::dec << " r[" << addr_vec[1] << "] bg[" << addr_vec[2] << "] b["
		    << addr_vec[3] << "] ch[" << addr_vec[0] << "] row[" << addr_vec[4] << "] col[" << addr_vec[5] << "]" << std::endl;
	  break;
	default:
	  std::cout << "Invalid Request" << std::endl;
	}
    }

    int getRank()
    {
      return addr_vec[1];
    }
};

} /*namespace ramulator*/

#endif /*__REQUEST_H*/

