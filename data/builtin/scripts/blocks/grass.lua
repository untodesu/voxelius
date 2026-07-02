blocks.add("builtin:grass", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "builtin:cube",
    animated = false,

    textures = {
        top = [
            "builtin:grass_top01.png",
            "builtin:grass_top02.png",
        ],

        bottom = [
            "builtin:dirt01.png",
            "builtin:dirt02.png",
            "builtin:dirt03.png",
            "builtin:dirt04.png",
        ],

        default = [
            "builtin:grass_side01.png",
            "builtin:grass_side02.png",
            "builtin:grass_side03.png",
            "builtin:grass_side04.png",
        ]
    },

    hardness = 0.6,
    tool = blocks.TOOL_SHOVEL,
    sounds = "builtin:grass",

    drops = [
        {
            item = "builtin:dirt", -- TODO: silk touch?
            count = 1
        },
    ],

    random_tick = function(world, bx, by, bz)
        local above = world.bget(bx, by + 1, bz)
        local light = world.lgetv(bx, by + 1, bz)

        if light < 9 and above ~= blocks.NULL_BLOCK then
            world.bset(bx, by, bz, blocks.get("builtin:dirt"))
            return true
        end

        return false
    end
})
