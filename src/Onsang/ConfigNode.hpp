
/**
@file ConfigNode.hpp
@brief Configuration node.

@author Tim Howard
@copyright 2013-2014 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_CONFIGNODE_HPP_
#define ONSANG_CONFIGNODE_HPP_

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
#include <Onsang/iterator_proxy.hpp>
#include <Onsang/String.hpp>

#include <duct/debug.hpp>
#include <duct/StateStore.hpp>
#include <duct/VarType.hpp>
#include <duct/Var.hpp>
#include <duct/VarTemplate.hpp>

#include <utility>
#include <memory>

namespace Onsang {

class ConfigNode final {
public:
	enum class Flags : unsigned {
		none = 0u,
		optional = 1u << 0,
		assigned = 1u << 1,

		// Only used for Entry
		collect = 1u << 2,
		node_matcher = 1u << 3,
		node_matcher_named = 1u << 4,

		// Only used for ConfigNode
		node_built = 1u << 5,

		supplied
			= optional
			| assigned
		,
	};

	struct Entry final {
		duct::StateStore<Flags> flags;
		duct::VarTemplate tpl{};
		duct::Var value{duct::VarType::null};
		std::shared_ptr<ConfigNode> node_tpl{nullptr};
		std::size_t node_count{0u};

		~Entry() = default;
		Entry() = delete;
		Entry(Entry&&) = default;
		Entry(Entry const&) = default;
		Entry& operator=(Entry const&) = delete;
		Entry& operator=(Entry&&) = default;

		Entry(
			duct::VarTemplate&& tpl,
			Flags const flags = Flags::none
		)
			: flags(flags)
			, tpl(std::move(tpl))
			, value(
				enum_bitand(flags, Flags::collect)
				? duct::VarType::node
				: duct::VarType::null
			)
		{}

		Entry(
			ConfigNode* node_tpl,
			Flags const flags = Flags::none
		)
			: flags(enum_combine(flags, Flags::node_matcher))
			, node_tpl(std::move(node_tpl))
		{
			DUCT_ASSERTE(nullptr != node_tpl);
		}

		bool
		assigned() const noexcept {
			return flags.test_any(Flags::assigned);
		}

		bool
		supplied() const noexcept {
			return flags.test_any(Flags::supplied);
		}

		bool
		validate(
			duct::Var const& var
		) {
			return
				tpl.validate_type(var) &&
				(
					!var.is_type_of(duct::VarMask::collection) ||
					tpl.validate_layout(var)
				)
			;
		}

		bool
		assign(
			ConfigNode& cnode,
			duct::Var const& var
		);
	};

	using entry_map_type = aux::unordered_map<String, Entry>;
	using node_map_type = aux::unordered_map<String, ConfigNode>;

	using entry_initializer_list = std::initializer_list<
		entry_map_type::value_type
	>;
	using node_initializer_list = std::initializer_list<
		node_map_type::value_type
	>;

	using entry_iterator_proxy = iterator_proxy<entry_map_type>;
	using const_entry_iterator_proxy = const_iterator_proxy<entry_map_type>;
	using node_iterator_proxy = iterator_proxy<node_map_type>;
	using const_node_iterator_proxy = const_iterator_proxy<node_map_type>;

	struct ValidationInfo final {
		bool valid;
		bool has_iter;
		String node;
		entry_map_type::const_iterator iter;
	};

private:
	duct::StateStore<Flags> m_flags;
	entry_map_type m_entries;
	node_map_type m_nodes;
	bool m_has_node_matcher{false};
	String m_node_matcher{};

	ConfigNode() = delete;
	ConfigNode& operator=(ConfigNode const&) = delete;

	void
	designate_node_matcher() {
		for (auto& epair : m_entries) {
			if (epair.second.flags.test(Flags::node_matcher)) {
				// There should only be one builder
				DUCT_ASSERTE(!m_has_node_matcher);
				m_has_node_matcher = true;
				m_node_matcher = epair.first;
			}
		}
	}

public:
// constructors, destructor, and operators
	~ConfigNode() = default;
	ConfigNode(ConfigNode const&) = default;
	ConfigNode(ConfigNode&&) = default;
	ConfigNode& operator=(ConfigNode&&) = default;

	ConfigNode(
		entry_initializer_list entries
	)
		: m_flags(Flags::none)
		, m_entries(entries)
		, m_nodes()
	{
		designate_node_matcher();
	}

	ConfigNode(
		entry_initializer_list entries,
		node_initializer_list nodes
	)
		: m_flags(Flags::none)
		, m_entries(entries)
		, m_nodes(nodes)
	{
		designate_node_matcher();
	}

	ConfigNode(
		Flags const flags,
		entry_initializer_list entries
	)
		: m_flags(flags)
		, m_entries(entries)
		, m_nodes()
	{
		designate_node_matcher();
	}

	ConfigNode(
		Flags const flags,
		entry_initializer_list entries,
		node_initializer_list nodes
	)
		: m_flags(flags)
		, m_entries(entries)
		, m_nodes(nodes)
	{
		designate_node_matcher();
	}

// properties
	void
	set_flags(
		Flags const flags,
		bool const enable
	) noexcept {
		m_flags.set(flags, enable);
	}

	bool
	built() const noexcept {
		return m_flags.test(Flags::node_built);
	}

	entry_iterator_proxy
	entry_proxy() noexcept {
		return {m_entries};
	}

	const_entry_iterator_proxy
	entry_proxy() const noexcept {
		return {m_entries};
	}

	node_iterator_proxy
	node_proxy() noexcept {
		return {m_nodes};
	}

	const_node_iterator_proxy
	node_proxy() const noexcept {
		return {m_nodes};
	}

	// TODO: Use a custom hasher and add overload for char const*
	/** @throws std::out_of_range */
	Entry&
	entry(
		String const& name
	) {
		return m_entries.at(name);
	}

	/** @throws std::out_of_range */
	Entry const&
	entry(
		String const& name
	) const {
		return m_entries.at(name);
	}

	/** @throws std::out_of_range */
	ConfigNode&
	node(
		String const& name
	) {
		return m_nodes.at(name);
	}

	/** @throws std::out_of_range */
	ConfigNode const&
	node(
		String const& name
	) const {
		return m_nodes.at(name);
	}

// operations
	/** @throws Error{ErrorCode::config_node_already_exists} */
	void
	emplace_node(
		String name,
		ConfigNode&& node
	);

	/**
		@throws Error{ErrorCode::config_var_unknown}
		@throws Error{ErrorCode::config_var_invalid}
	*/
	void
	import(
		duct::Var const& node
	);

private:
	ValidationInfo
	validate(
		String&& name
	) const noexcept;

public:
	/**
		@returns
		-# {false, m_entries.cend()} if the node isn't supplied;
		-# {false, <iterator to entry>} if some entry is not supplied,
		   with an iterator to the entry.
		-# {true, m_entries.cend()} if all entries and nodes are
		   supplied;
	*/
	ValidationInfo
	validate() const noexcept;
};

} // namespace Onsang

#endif // ONSANG_CONFIGNODE_HPP_
