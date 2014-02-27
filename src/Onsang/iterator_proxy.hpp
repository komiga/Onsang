
/**
@file iterator_proxy.hpp
@brief Iterator proxy.

@author Tim Howard
@copyright 2013-2014 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_ITERATOR_PROXY_HPP_
#define ONSANG_ITERATOR_PROXY_HPP_

#include <Onsang/config.hpp>

#include <functional>

namespace Onsang {

template<
	class C
>
struct iterator_proxy final {
public:
	using container_type = C;
	using iterator = typename C::iterator;
	using const_iterator = typename C::const_iterator;

	std::reference_wrapper<container_type> container;

private:
	iterator_proxy() = delete;

public:
// constructors, destructor, and operators
	~iterator_proxy() = default;
	iterator_proxy& operator=(iterator_proxy const&) = default;
	iterator_proxy(iterator_proxy const&) = default;
	iterator_proxy(iterator_proxy&&) = default;
	iterator_proxy& operator=(iterator_proxy&&) = default;

	iterator_proxy(
		container_type& container
	)
		: container(container)
	{}

// properties
	iterator
	begin() noexcept {
		return container.get().begin();
	}

	iterator
	end() noexcept {
		return container.get().end();
	}

	const_iterator
	begin() const noexcept {
		return container.get().begin();
	}

	const_iterator
	end() const noexcept {
		return container.get().end();
	}

	const_iterator
	cbegin() const noexcept {
		return container.get().cbegin();
	}

	const_iterator
	cend() const noexcept {
		return container.get().cend();
	}
};

template<
	class C
>
struct const_iterator_proxy final {
public:
	using container_type = C;
	using const_iterator = typename C::const_iterator;

	std::reference_wrapper<container_type const> container;

private:
	const_iterator_proxy() = delete;

public:
// constructors, destructor, and operators
	~const_iterator_proxy() = default;
	const_iterator_proxy& operator=(const_iterator_proxy const&) = default;
	const_iterator_proxy(const_iterator_proxy const&) = default;
	const_iterator_proxy(const_iterator_proxy&&) = default;
	const_iterator_proxy& operator=(const_iterator_proxy&&) = default;

	const_iterator_proxy(
		container_type const& container
	)
		: container(container)
	{}

// properties
	const_iterator
	begin() const noexcept {
		return container.get().begin();
	}

	const_iterator
	end() const noexcept {
		return container.get().end();
	}

	const_iterator
	cbegin() const noexcept {
		return container.get().cbegin();
	}

	const_iterator
	cend() const noexcept {
		return container.get().cend();
	}
};

} // namespace Onsang

#endif // ONSANG_ITERATOR_PROXY_HPP_
