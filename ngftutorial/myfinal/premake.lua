---------------------------------------------------------------------------------------------
----------------------------- NGFExample 'Premake.lua' file ---------------------------------
---------------------------------------------------------------------------------------------

-- Project ----------------------------------------------------------------------------------

project.name = "NGFExample"
project.bindir = "bin"

-- Package ----------------------------------------------------------------------------------

package = newpackage()

package.name = "NGFExample"
package.kind = "exe"
package.language = "c++"
package.configs = { "Debug", "Release" } -- If you add more, configure them at the bottom.
package.prebuildcommands = { "pydef/runpydef.sh" }

-- Include and library search paths, system dependent (I don't assume a directory structure)

package.includepaths = {
    -- Edit include directories here. Add Ogre and OIS include directories if you don't use
    -- pkg-config
    -- "<boostdir>",                                                           	-- Boost.
    "/home/nikki/Development/Libraries/ogre/ogre-1.60/Samples/Common/include/", 	-- ExampleApplication
    "/home/nikki/Development/Libraries/ogre/btogre/include/",

    -- You don't have to edit the directories below, they're relative.
    "../../include",                                                        -- NGF.
    "include",                                                               -- NGFExample files.
    "."                                                               -- NGFExample files.
}

package.libpaths = {
    -- Edit library directories here. Add Ogre and OIS library directories if you don't use
    -- pkg-config
    "/home/nikki/Development/Libraries/squirrel/lib"
}

-- Libraries to link to ---------------------------------------------------------------------

package.links = {
    -- Add Ogre and OIS libraries here, if you don't use pkg-config.
    "python2.6",
    "boost_python"
}

-- pkg-configable stuff ---------------------------------------------------------------------

if (linux) then
    package.buildoptions = {
	"`pkg-config OGRE --cflags`" ..
	"`pkg-config OIS --cflags`" ..
	"`pkg-config MyGUI --cflags`" ..
	"`pkg-config bullet --cflags`"
	--"`python-config --cflags`"
    }

    package.linkoptions = {
	"`pkg-config OGRE --libs`" ..
	"`pkg-config OIS --libs`" ..
	"`pkg-config MyGUI --libs`" ..
	"`pkg-config bullet --libs`"
	--"`python-config --libs`"
    }
end

-- Files ------------------------------------------------------------------------------------

package.files = {
    matchrecursive("*.h", "*.cpp"),
    "../../ConfigScript.cpp",
    "../../Ngf.cpp",
    "../../plugins/ngfpython/NgfPython.cpp"
}

-- Debug configuration ----------------------------------------------------------------------

debug = package.config["Debug"]
debug.defines = { "DEBUG", "_DEBUG" }
debug.objdir = "obj/debug"
debug.target = "debug/" .. package.name .. "_d"

debug.buildoptions = { "-g", "-pg" }
debug.linkoptions = { "-pg" }

-- Release configuration --------------------------------------------------------------------

release = package.config["Release"]
release.objdir = "obj/release"
release.target = "release/" .. package.name

release.buildoptions = { "-O3", "-funroll-loops" }
release.linkoptions = { "-pg" }
