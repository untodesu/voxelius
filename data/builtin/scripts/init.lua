core.log_info("mod: " .. core.NAMESPACE)
core.log_info("engine version: " .. core.VERSION)

core.do_file("blocks/air.lua")
core.do_file("blocks/dirt.lua")
core.do_file("blocks/grass.lua")
core.do_file("blocks/stone.lua")
