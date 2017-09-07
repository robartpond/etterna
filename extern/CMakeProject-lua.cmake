
file(GLOB LUA_HPP
    "LuaJIT-2.0.5/src/*.h"
)
file(GLOB LUA_SRC
    "LuaJIT-2.0.5/src/*.c"
)
source_group("" FILES ${LUA_SRC})
source_group("" FILES ${LUA_HPP})

add_library("LuaJIT-2.0.5" STATIC ${LUA_SRC} ${LUA_HPP})

set_property(TARGET "LuaJIT-2.0.5" PROPERTY FOLDER "External Libraries")

# include_directories(src)

if(MSVC)
  sm_add_compile_definition("LuaJIT-2.0.5")
endif(MSVC)

disable_project_warnings("LuaJIT-2.0.5")

