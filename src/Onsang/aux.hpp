/**
@copyright MIT license; see @ref index or the accompanying LICENSE file.

@file
@brief Auxiliary stdlib specializations.
*/

#pragma once

#include <Onsang/config.hpp>

#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>

namespace Onsang {
namespace aux {

/**
	@addtogroup etc
	@{
*/
/**
	@addtogroup aux
	@{
*/

/**
	@c std::shared_ptr<T>.
*/
template<
	typename T
>
using shared_ptr = std::shared_ptr<T>;

/**
	@c std::weak_ptr<T>.
*/
template<
	typename T
>
using weak_ptr = std::weak_ptr<T>;

/**
	@c std::unique_ptr<T>.
*/
template<
	typename T,
	typename Deleter = std::default_delete<T>
>
using unique_ptr = std::unique_ptr<T>;

/** Alias for @c std::make_shared(). */
using std::make_shared;

/** Alias for @c std::owner_less<T>. */
using std::owner_less;

/** Alias for @c std::enable_shared_from_this<T>. */
using std::enable_shared_from_this;

/**
	@c std::basic_string<CharT, Traits>.
*/
template<
	typename CharT,
	class Traits = std::char_traits<CharT>
>
using basic_string
= std::basic_string<
	CharT, Traits,
	ONSANG_AUX_ALLOCATOR<CharT>
>;

/**
	@c std::vector<T>.
*/
template<
	typename T
>
using vector
= std::vector<
	T,
	ONSANG_AUX_ALLOCATOR<T>
>;

/**
	@c std::deque<T>.
*/
template<
	typename T
>
using deque
= std::deque<
	T,
	ONSANG_AUX_ALLOCATOR<T>
>;

/**
	@c std::unordered_map<Key, T, Hash, KeyEqual>.
*/
template<
	typename Key,
	typename T,
	class Compare = std::less<Key>
>
using map
= std::map<
	Key, T, Compare,
	ONSANG_AUX_ALLOCATOR<std::pair<Key const, T> >
>;

/**
	@c std::unordered_map<Key, T, Hash, KeyEqual>.
*/
template<
	typename Key,
	typename T,
	class Hash = std::hash<Key>,
	class KeyEqual = std::equal_to<Key>
>
using unordered_map
= std::unordered_map<
	Key, T, Hash, KeyEqual,
	ONSANG_AUX_ALLOCATOR<std::pair<Key const, T> >
>;

/**
	@c std::unordered_set<Key, Hash, KeyEqual>.
*/
template<
	typename Key,
	class Hash = std::hash<Key>,
	class KeyEqual = std::equal_to<Key>
>
using unordered_set
= std::unordered_set<
	Key, Hash, KeyEqual,
	ONSANG_AUX_ALLOCATOR<Key>
>;

/** @} */ // end of doc-group aux
/** @} */ // end of doc-group etc

} // namespace aux
} // namespace Onsang
