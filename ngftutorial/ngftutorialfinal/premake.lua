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

if (windows) then
   table.insert(package.defines, "WIN32") -- To fix a problem on Windows.
end

-- Include and library search paths, system dependent (I don't assume a directory structure)

package.includepaths = {
-- Edit include directories here. Add Ogre and OIS include directories if you don't use
-- pkg-config
"<boostdir>",                                                           -- Boost.
"<exampleappdir>",                                                      -- ExampleApplication

-- You don't have to edit the directories below, they're relative.
"../../include",                                                        -- NGF.
"include"                                                               -- NGFExample files.
}

package.libpaths = {
-- Edit library directories here. Add Ogre and OIS library directories if you don't use
-- pkg-config
}

-- Libraries to link to ---------------------------------------------------------------------

package.links = {
-- Add Ogre and OIS libraries here, if you don't use pkg-config.
}

-- pkg-configable stuff ---------------------------------------------------------------------

if (linux) then
    package.buildoptions = {
    "`pkg-config OGRE --cflags`" ..
    "`pkg-config OIS --cflags`"
    }

    package.linkoptions = {
    "`pkg-config OGRE --libs`" ..
    "`pkg-config OIS --libs`"
    }
end

-- Files ------------------------------------------------------------------------------------

package.files = {
matchrecursive("*.h", "*.cpp"),
"../../Ngf.cpp"
}

-- Debug configuration ----------------------------------------------------------------------

debug = package.config["Debug"]
debug.defines = { "DEBUG", "_DEBUG" }
debug.objdir = "obj/debug"
debug.target = "debug/" .. package.name .. "_d"

debug.buildoptions = { "-g" }

-- Release configuration --------------------------------------------------------------------

release = package.config["Release"]
release.objdir = "obj/release"
release.target = "release/" .. package.name
