core.log_info("mod: " .. core.NAMESPACE)
core.log_info("engine version: " .. core.VERSION)

core.log_info("info test")
core.log_warning("warning test")
core.log_error("error test")
core.log_critical("critical test")
core.log_debug("debug test")

core.do_file("blocks/air.lua")
core.do_file("blocks/dirt.lua")
core.do_file("blocks/grass.lua")
core.do_file("blocks/stone.lua")
