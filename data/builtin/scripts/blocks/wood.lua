local logs = require("builtin:logs.lua")

logs.add_block("oak_log", {
    render = blocks.RENDER_SOLID,

    health = 4,
    tools = { blocks.TOOL_AXE },

    emission = 0,
    dissipation = 15,

    drops = {
        {
            items = { name = "oak_log", count = 1 }
        }
    },

    textures = {
        default = {
            "oak_wood_01.png",
            "oak_wood_02.png",
        },
        north = { "oak_log.png" },
        south = { "oak_log.png" },
    }
})
