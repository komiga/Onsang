/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief GR ceformat definitions.
*/

#ifndef ONSANG_DETAIL_GR_CEFORMAT_HPP_
#define ONSANG_DETAIL_GR_CEFORMAT_HPP_

#include <Onsang/config.hpp>
#include <Onsang/detail/gr_core.hpp>

#include <duct/GR/ceformat.hpp>

#define ONSANG_DEF_FMT(ident_, fmt_) \
	DUCT_GR_DEF_CEFMT(ident_, fmt_)

#define ONSANG_DEF_FMT_CLASS(ident_, fmt_) \
	DUCT_GR_DEF_CEFMT_CLASS(ident_, fmt_)

#define ONSANG_DEF_FMT_FUNC(ident_, fmt_) \
	DUCT_GR_DEF_CEFMT_FUNC(ident_, fmt_)

#define ONSANG_DEF_FMT_FQN(ident_, fmt_) \
	DUCT_GR_DEF_CEFMT_FQN(ident_, fmt_)

#define ONSANG_THROW_FMT(ec_, cefmt_, ...) \
	DUCT_GR_THROW_CEFMT(ec_, cefmt_, __VA_ARGS__)

#endif // ONSANG_DETAIL_GR_CEFORMAT_HPP_
