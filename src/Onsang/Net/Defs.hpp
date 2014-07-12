/**
@file Net/Defs.hpp
@brief Net definitions.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#pragma once

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
