
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
	"onsang-client",
	"C++", "ConsoleApp",
	"bin/", "out/",
	nil,
	{
		"onsang-client-deps"
	}
)

configuration {"debug"}
	targetsuffix("_d")

configuration {}
	defines {
		"ONSANG_CLIENT"
	}
	files {
		"src/**.cpp"
	}
	excludes {
		"src/Onsang/Server/**"
	}

precore.action_clean("out", "bin")
