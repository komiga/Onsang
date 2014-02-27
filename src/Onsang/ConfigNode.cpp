
#include <Onsang/utility.hpp>
#include <Onsang/detail/gr_core.hpp>
#include <Onsang/detail/gr_ceformat.hpp>
#include <Onsang/ConfigNode.hpp>

#include <duct/debug.hpp>
#include <duct/VarType.hpp>

namespace Onsang {

// struct ConfigNode::Entry implementation

bool
ConfigNode::Entry::assign(
	ConfigNode& cnode,
	duct::Var const& var
) {
	if (node_tpl) {
		if (
			flags.test(Flags::node_matcher_named) &&
			var.get_name().empty()
		) {
			return false;
		}
		ConfigNode splice{*node_tpl};
		splice.set_flags(Flags::node_built, true);
		splice.import(var);
		cnode.emplace_node(var.get_name(), std::move(splice));
		++node_count;
	} else {
		if (!validate(var)) {
			return false;
		}
		if (flags.test(Flags::collect)) {
			value.emplace_back(var);
		} else {
			value = var;
		}
	}
	flags.enable(Flags::assigned);
	return true;
}

// class ConfigNode implementation

#define ONSANG_SCOPE_CLASS ConfigNode

#define ONSANG_SCOPE_FUNC emplace_node
namespace {
ONSANG_DEF_FMT_FQN(
	s_err_node_already_exists,
	"config node '%s' already exists"
);
} // anonymous namespace

void
ConfigNode::emplace_node(
	String name,
	ConfigNode&& node
) {
	if (m_nodes.end() != m_nodes.find(name)) {
		ONSANG_THROW_FMT(
			ErrorCode::config_node_already_exists,
			s_err_node_already_exists,
			name
		);
	}
	m_nodes.emplace(std::move(name), std::move(node));
}
#undef ONSANG_SCOPE_FUNC

#define ONSANG_SCOPE_FUNC import
namespace {
ONSANG_DEF_FMT_FQN(
	s_err_var_unknown,
	"unknown variable '%s' (type = %s)"
);

ONSANG_DEF_FMT_FQN(
	s_err_var_invalid,
	"invalid variable '%s' (type = %s): %s"
);
} // anonymous namespace

void
ConfigNode::import(
	duct::Var const& node
) {
	DUCT_ASSERTE(node.is_type(duct::VarType::node));
	auto const matcher_it
		= m_has_node_matcher
		? m_entries.find(m_node_matcher)
		: m_entries.end()
	;
	for (auto const& var : node) {
		auto entry_it = m_entries.find(var.get_name());
		if (
			m_entries.cend() == entry_it &&
			m_has_node_matcher &&
			var.is_type(duct::VarType::node)
		) {
			entry_it = matcher_it;
		}
		if (m_entries.cend() != entry_it) {
			if (entry_it->second.assign(*this, var)) {
				continue;
			} else {
				ONSANG_THROW_FMT(
					ErrorCode::config_var_invalid,
					s_err_var_invalid,
					var.get_name(),
					duct::var_type_name(var.get_type()),
					entry_it->second.flags.test(Flags::node_matcher_named)
					? "node requires name"
					: "template validation failed"
				);
			}
		} else if (var.is_type(duct::VarType::node)) {
			auto const node_it = m_nodes.find(var.get_name());
			if (m_nodes.cend() != node_it) {
				node_it->second.import(var);
				continue;
			}
		}
		ONSANG_THROW_FMT(
			ErrorCode::config_var_unknown,
			s_err_var_unknown,
			var.get_name(),
			duct::var_type_name(var.get_type())
		);
	}
	m_flags.enable(Flags::assigned);
}
#undef ONSANG_SCOPE_FUNC

ConfigNode::ValidationInfo
ConfigNode::validate(
	String&& name
) const noexcept {
	if (!m_flags.test_any(Flags::supplied)) {
		return {false, false, std::move(name), m_entries.cend()};
	}
	for (
		auto it = m_entries.cbegin();
		m_entries.cend() != it;
		++it
	) {
		if (!it->second.supplied()) {
			return {false, true, std::move(name), it};
		}
	}
	for (auto const& npair : m_nodes) {
		auto vinfo = npair.second.validate(String{npair.first});
		if (!vinfo.valid) {
			return vinfo;
		}
	}
	return {true, false, std::move(name), m_entries.cend()};
}

ConfigNode::ValidationInfo
ConfigNode::validate() const noexcept {
	return validate("<root>");
}

#undef ONSANG_SCOPE_CLASS // ConfigNode

} // namespace Onsang
