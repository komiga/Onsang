/**
@file Net/Defs.hpp
@brief Net definitions.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_NET_DEFS_HPP_
#define ONSANG_NET_DEFS_HPP_

#include <Onsang/config.hpp>
#include <Onsang/asio.hpp>

#include <duct/IO/dynamic_streambuf.hpp>

namespace Onsang {
namespace Net {

using Socket = asio::generic::stream_protocol::socket;
using StreamProtocol = asio::generic::stream_protocol;

using DynamicStreamBuf
= duct::IO::basic_dynamic_streambuf<
	char,
	std::char_traits<char>,
	ONSANG_AUX_ALLOCATOR<char>
>;

} // namespace Net
} // namespace Onsang

#endif // ONSANG_NET_DEFS_HPP_
