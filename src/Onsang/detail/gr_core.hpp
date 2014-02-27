/**
@file detail/gr_core.hpp
@brief GR core configuration.

@author Timothy Howard
@copyright 2013-2014 Timothy Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_DETAIL_GR_CORE_HPP_
#define ONSANG_DETAIL_GR_CORE_HPP_

#include <Onsang/config.hpp>
#include <Onsang/String.hpp>
#include <Onsang/Error.hpp>

#ifndef ONSANG_DETAIL_GR_CORE_NS_GUARD_
#define ONSANG_DETAIL_GR_CORE_NS_GUARD_

namespace Onsang {
namespace detail {
namespace GR {
	using Error = ::Onsang::Error;
}}}

#endif // ONSANG_DETAIL_GR_CORE_NS_GUARD_

#define DUCT_GR_ROOT_NAMESPACE \
	::Onsang::detail::GR

#define DUCT_GR_STR_LIT(s_) \
	ONSANG_STR_LIT(s_)

#define DUCT_GR_MSG_IMPL_(s_, m_) \
	"{" s_ "} " m_

#define DUCT_GR_SCOPE_PREFIX \
	Onsang::

#define DUCT_GR_SCOPE_CLASS \
	ONSANG_SCOPE_CLASS

#define DUCT_GR_SCOPE_FUNC \
	ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FQN_STR_LIT \
	DUCT_GR_SCOPE_FQN_STR_LIT

#include <duct/GR/common.hpp>
#include <duct/GR/throw.hpp>

#define ONSANG_THROW(ec_, m_) \
	DUCT_GR_THROW(ec_, m_)

#define ONSANG_THROW_CLASS(ec_, m_) \
	DUCT_GR_THROW_CLASS(ec_, m_)

#define ONSANG_THROW_FUNC(ec_, m_) \
	DUCT_GR_THROW_FUNC(ec_, m_)

#define ONSANG_THROW_FQN(ec_, m_) \
	DUCT_GR_THROW_FQN(ec_, m_)

#endif // ONSANG_DETAIL_GR_CORE_HPP_
