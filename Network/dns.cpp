module;
#include <format>
#include <stdio.h>
#include "network_import.h"

module network.dns;
import <string>;
import <vector>;
import network.types;

using namespace std;
#ifdef WINDOWS
inline uint32_t ConvertIN_ADDR_To_uint32_t(const IN_ADDR& inAddr)
{
    return (static_cast<uint32_t>(inAddr.S_un.S_un_b.s_b1)) |
        (static_cast<uint32_t>(inAddr.S_un.S_un_b.s_b2) << 8) |
        (static_cast<uint32_t>(inAddr.S_un.S_un_b.s_b3) << 16) |
        (static_cast<uint32_t>(inAddr.S_un.S_un_b.s_b4) << 24);
}
#endif

vector<IP> get_dns_response(string_view hostname)
{
    vector<IP> response;
#ifdef WINDOWS
    WSAWrapper::instance();
    addrinfo* result = nullptr;
    addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int ret = getaddrinfo(hostname.data(), nullptr, &hints, &result);
    if (ret != 0)
        throw runtime_error(format("getaddrinfo failed: {}", ret));

    for (auto ptr = result; ptr; ptr = ptr->ai_next)
    {
        if (ptr->ai_family != AF_INET)
            continue;

        auto ipv4 = (sockaddr_in*)ptr->ai_addr;
        response.push_back(ConvertIN_ADDR_To_uint32_t(ipv4->sin_addr));
    }

    freeaddrinfo(result);
#elif defined(UNIX)
    auto hptr = gethostbyname(hostname.data());
    if (hptr == nullptr)
        throw runtime_error(format("gethostbyname failed for host {}: {}", hostname, hstrerror(h_errno));

    if (hptr->h_addrtype != AF_INET)
        throw runtime_error(format("gethostbyname returned an unsupported address type for host: {}", hostname));

    for (auto pptr = hptr->h_addr_list; *pptr; ++pptr)
    {
        uint32_t ip;
        memcpy(&ip, *pptr, sizeof(ip));
        response.push_back(ip);
    }
#endif

    return response;
}