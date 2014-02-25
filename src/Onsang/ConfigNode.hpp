
/**
@file ConfigNode.hpp
@brief Configuration node.

@author Tim Howard
@copyright 2013 Tim Howard under the MIT license;
see @ref index or the accompanying LICENSE file for full text.
*/

#ifndef ONSANG_CONFIGNODE_HPP_
#define ONSANG_CONFIGNODE_HPP_

#include <Onsang/config.hpp>
#include <Onsang/aux.hpp>
#include <Onsang/utility.hpp>
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
		{}

		Entry(
			ConfigNode* node_tpl,
			Flags const flags = Flags::none
		)
			: flags(flags)
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

	struct ValidationInfo final {
		bool valid;
		bool has_iter;
		entry_map_type::const_iterator iter;
	};

private:
	duct::StateStore<Flags> m_flags;
	entry_map_type m_entries;
	node_map_type m_nodes;

	ConfigNode() = delete;
	ConfigNode& operator=(ConfigNode const&) = delete;

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
	{}

	ConfigNode(
		entry_initializer_list entries,
		node_initializer_list nodes
	)
		: m_flags(Flags::none)
		, m_entries(entries)
		, m_nodes(nodes)
	{}

	ConfigNode(
		Flags const flags,
		entry_initializer_list entries,
		node_initializer_list nodes
	)
		: m_flags(flags)
		, m_entries(entries)
		, m_nodes(nodes)
	{}

// properties
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
