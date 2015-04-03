
local S, G, R = precore.helpers()

precore.import(G"${DEP_PATH}/duct")
precore.import(G"${DEP_PATH}/trait_wrangler")
precore.import(G"${DEP_PATH}/ceformat")
precore.import(G"${DEP_PATH}/Cacophony")
precore.import(G"${DEP_PATH}/am")
precore.import(G"${DEP_PATH}/Beard")
precore.import(G"${DEP_PATH}/Hord")

precore.make_config("onsang.dep.boost", nil, {
{project = function(_)
	configuration {}
		includedirs {
			G"${DEP_PATH}/boost/include/",
		}
		libdirs {
			G"${DEP_PATH}/boost/lib/",
		}
		links {"boost_system"}
		links {"boost_filesystem"}
end}})

precore.make_config_scoped("onsang.env", {
	once = true,
}, {
{global = function()
	precore.define_group("ONSANG", os.getcwd())
end}})

precore.make_config("onsang.strict", nil, {
{project = function()
	configuration {}
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

precore.make_config("onsang.dep", nil, {
"duct.dep",
"trait_wrangler.dep",
"ceformat.dep",
"cacophony.dep",
"am.dep",
"beard.dep",
"hord.dep",
"onsang.dep.boost",
{project = function(_)
	configuration {}
		includedirs {
			G"${ONSANG_ROOT}/src/",
		}
end}})

precore.make_config_scoped("onsang.projects", {
	once = true,
}, {
{global = function()
	precore.make_solution(
		"Onsang",
		{"debug", "release"},
		{"x64", "x32"},
		nil,
		{
			"precore.generic",
		}
	)

	precore.make_project(
		"onsang",
		"C++", "ConsoleApp",
		G"${ONSANG_BUILD}/bin/",
		G"${ONSANG_BUILD}/out/",
		nil, {
			"onsang.strict",
			"onsang.dep",
		}
	)

	configuration {"debug"}
		targetsuffix("_debug")

	configuration {}
		files {
			"src/**.cpp",
		}
end}})

precore.apply_global({
	"precore.env-common",
	"onsang.env",
})
