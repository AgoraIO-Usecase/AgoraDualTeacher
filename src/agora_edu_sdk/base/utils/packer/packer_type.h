//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include "utils/tools/sys_type.h"
#include <cstdint>
#include "utils/packer/packer.h"
#include "utils/net/ip_type.h"

namespace agora { namespace commons {

    // NOTE: port should use network order
    DECLARE_PACKABLE_2_START(address_info_v1, ipv4::ip_t, ip, uint16_t, port)
	address_info_v1(const ip::sockaddr_t& address)
            : ip(ipv4::from_address(address.sin))
            , port(htons(ip::address_to_port(address)))
        {}
		std::string to_string() const
		{
			return ip::to_string(to_address());
		}
		ip::sockaddr_t to_address() const
		{
			ip::sockaddr_t addr;
			addr.sin = ipv4::to_address(ip, port);
			return addr;
		}
		friend std::ostream& operator<<(std::ostream& oss, const address_info_v1& a)
		{
			return oss << a.to_string();
		}
		DECLARE_STRUCT_END
		typedef std::vector<address_info_v1> address_list_v1;
		
		DECLARE_PACKABLE_2_START(address_info_v2, std::string, ip, std::vector<uint16_t>, ports)
		address_info_v2(const ip::sockaddr_t& address)
		: ip(ip::from_address(address))
		, ports({ htons(ip::address_to_port(address)) })
		{}
		std::string to_string() const
		{
			return ip::to_string(to_address());
		}
		ip::sockaddr_t to_address() const
		{
			return ip::to_address(ip, port());
		}
		friend std::ostream& operator<<(std::ostream& oss, const address_info_v2& a)
		{
			return oss << a.to_string();
		}
		uint16_t port() const
		{
			return ports.empty() ? 0 : ports.front();
		}
		DECLARE_STRUCT_END
	typedef std::vector<address_info_v2> address_list_v2;
    DECLARE_PACKABLE_2_START(address_info_v3, std::string, ip, uint16_t, port0)
    address_info_v3(const ip::sockaddr_t& address)
        : ip(ip::from_address(address))
        , port0(htons(ip::address_to_port(address)))
    {}
    std::string to_string() const
    {
        return ip::to_string(to_address());
    }
    ip::sockaddr_t to_address() const
    {
        return ip::to_address(ip, port0);
    }
    friend std::ostream& operator<<(std::ostream& oss, const address_info_v3& a)
    {
        return oss << a.to_string();
    }
    uint16_t port() const
    {
        return port0;
    }
    DECLARE_STRUCT_END
    typedef std::vector<address_info_v3> address_list_v3;

    using address_info = address_info_v2;
	using address_list = address_list_v2;
}
}

