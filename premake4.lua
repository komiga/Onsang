
dofile("premake_common.lua")

-- Core solution

precore.make_solution(
	"Onsang",
	{"debug", "release"},
	{"x64", "x32"},
	nil,
	{
		"precore-generic",
		"onsang-strict",
		"onsang-deps",
		"onsang-import"
	}
)

-- Client

precore.make_project(
	"onsang",
	"C++", "ConsoleApp",
	"bin/", "out/",
	nil,
	{
		"onsang-client-deps"
	}
)

configuration {"debug"}
	targetsuffix("_debug")

configuration {}
	files {
		"src/**.cpp"
	}

precore.action_clean("out", "bin")
