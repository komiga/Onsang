
-- Import precore

local env_name = "PRECORE_ROOT"
local root_path = os.getenv(env_name)

if nil == root_path or 0 == #root_path then
	error(
		"Environment variable '" .. env_name .. "' is not " ..
		"defined or is blank; assign it to precore's root directory"
	)
end

dofile(path.join(os.getenv(env_name), "precore.lua"))

-- Utilities

function action_clean()
	if "clean" == _ACTION then
		for _, pc_sol in pairs(precore.state.solutions) do
			for _, pc_proj in pairs(pc_sol.projects) do
				os.rmdir(path.join(pc_proj.obj.basedir, "out"))
			end
		end
	end
end

-- Custom precore configs

precore.make_config(
"onsang-strict", {{
project = function()
	-- NB: -Werror is a pita for GCC. Good for testing, though,
	-- since its error checking is better.
	configuration {"clang"}
		flags {
			"FatalWarnings"
		}
		buildoptions {
			"-Wno-missing-braces"
		}

	configuration {"linux"}
		buildoptions {
			"-pedantic-errors",
			"-Wextra",

			"-Wuninitialized",
			"-Winit-self",

			"-Wmissing-field-initializers",
			"-Wredundant-decls",

			"-Wfloat-equal",
			"-Wold-style-cast",

			"-Wnon-virtual-dtor",
			"-Woverloaded-virtual",

			"-Wunused"
		}
end}})

precore.make_config(
"onsang-deps", {{
project = function()
	configuration {}
		includedirs {
			precore.subst("${ROOT}/dep/duct/"),
			precore.subst("${ROOT}/dep/murk/include/"),
			precore.subst("${ROOT}/dep/trait_wrangler/"),
			precore.subst("${ROOT}/dep/ceformat/"),
			precore.subst("${ROOT}/dep/Hord/include/"),
			precore.subst("${ROOT}/dep/boost/include/"),
		}

		libdirs {
			precore.subst("${ROOT}/dep/murk/lib"),
			precore.subst("${ROOT}/dep/Hord/lib"),
			precore.subst("${ROOT}/dep/boost/lib"),
		}

		links {"boost_system"}
		--links {"boost_filesystem"}

	configuration {"debug"}
		links {"hord_d"}
		links {"murk_d"}

	configuration {"release"}
		links {"hord"}
		links {"murk"}
end}})

precore.make_config(
"onsang-client-deps", {{
project = function()
	configuration {}
		includedirs {
			precore.subst("${ROOT}/dep/am/"),
			precore.subst("${ROOT}/dep/Beard/include/"),
		}

		libdirs {
			precore.subst("${ROOT}/dep/Beard/lib"),
		}

	configuration {"debug"}
		links {"beard_d"}

	configuration {"release"}
		links {"beard"}
end}})

precore.make_config(
"onsang-import", {{
project = function()
	configuration {}
		includedirs {
			"src/"
		}
end}})

-- Initialize precore

precore.init(
	nil,
	{
		"opt-clang",
		"c++11-core",
		"precore-env-root"
	}
)
