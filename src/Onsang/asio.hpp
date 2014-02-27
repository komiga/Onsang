/**
@file asio.hpp
@brief Asio imports.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_ASIO_HPP_
#define ONSANG_ASIO_HPP_

#include <Onsang/config.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <boost/asio.hpp>
#pragma GCC diagnostic pop

namespace Onsang {

#define ONSANG_LOCALIZE_NS_ASIO() \
	namespace asio = boost::asio

ONSANG_LOCALIZE_NS_ASIO();

} // namespace Onsang

#endif // ONSANG_ASIO_HPP_
