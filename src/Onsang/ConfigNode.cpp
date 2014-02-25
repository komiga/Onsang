
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
		ConfigNode splice{*node_tpl};
		splice.import(var);
		// TODO: Generate name if var.get_name().empty()?
		cnode.emplace_node(var.get_name(), std::move(splice));
	} else {
		if (!validate(var)) {
			return false;
		}
		value = var;
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
	"invalid variable '%s' (type = %s)"
);
} // anonymous namespace

void
ConfigNode::import(
	duct::Var const& node
) {
	DUCT_ASSERTE(node.is_type(duct::VarType::node));
	for (auto const& var : node) {
		auto entry_it = m_entries.find(var.get_name());
		if (var.is_type(duct::VarType::node) && m_entries.cend() != entry_it) {
			entry_it = m_entries.find(String{});
		}
		if (m_entries.cend() != entry_it) {
			if (entry_it->second.assign(*this, var)) {
				continue;
			} else {
				ONSANG_THROW_FMT(
					ErrorCode::config_var_invalid,
					s_err_var_invalid,
					var.get_name(),
					duct::var_type_name(var.get_type())
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
ConfigNode::validate() const noexcept {
	if (!m_flags.test_any(Flags::supplied)) {
		return {false, false, m_entries.cend()};
	}
	for (
		auto it = m_entries.cbegin();
		m_entries.cend() != it;
		++it
	) {
		if (!it->second.supplied()) {
			return {false, true, it};
		}
	}
	for (auto const& node : m_nodes) {
		auto vinfo = node.second.validate();
		if (!vinfo.valid) {
			return vinfo;
		}
	}
	return {true, false, m_entries.cend()};
}

#undef ONSANG_SCOPE_CLASS // ConfigNode

} // namespace Onsang
