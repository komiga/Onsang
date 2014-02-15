
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
	nil, nil
)

configuration {"debug"}
	targetsuffix("_d")

configuration {}
	files {
		"src/**.cpp"
	}
	excludes {
		"src/Onsang/Server/**"
	}

action_clean()
