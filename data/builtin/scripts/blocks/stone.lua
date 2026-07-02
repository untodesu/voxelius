blocks.add("builtin:stone", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "builtin:cube",
    animated = false,

    textures = {
        default = [
            "builtin:stone01.png",
            "builtin:stone02.png",
            "builtin:stone03.png",
            "builtin:stone04.png",
        ]
    },

    hardness = 1.5,
    tool = blocks.TOOL_PICKAXE,
    sounds = "builtin:stone",

    drops = [
        {
            item = "builtin:cobblestone",
            count = 1
        }
    ],
})

blocks.add("builtin:stone_slab", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "builtin:slab",
    animated = false,

    textures = {
        default = [
            "builtin:stone01.png",
            "builtin:stone02.png",
            "builtin:stone03.png",
            "builtin:stone04.png",
        ]
    },

    hardness = 1.5,
    tool = blocks.TOOL_PICKAXE,
    sounds = "builtin:stone",

    drops = [
        {
            item = "builtin:stone_slab",
            count = 1
        }
    ],

    states = {
        orientation = {
            type = blocks.STATE_ENUM,
            values = ["bottom", "top", "full"],
            default = "bottom"
        }
    },

    variants = [
        {
            when = { orientation = "bottom" },
            apply = {
                blockmodel = "builtin:slab",
                translate = [0, 0, 0],
            }
        },
        {
            when = { orientation = "top" },
            apply = {
                blockmodel = "builtin:slab",
                translate = [0, 0.5, 0],
            }
        },
        {
            when = { orientation = "full" },
            apply = {
                blockmodel = "builtin:cube",
                translate = [0, 0, 0],
                drops = [
                    {
                        item = "builtin:stone_slab",
                        count = 2
                    }
                ]
            }
        }
    ],

    on_place = function(world, bx, by, bz, face, placer)
        if face == blocks.FACE_TOP then
            world.sstate(bx, by, bz, "orientation", "bottom")
        elseif face == blocks.FACE_BOTTOM then
            world.sstate(bx, by, bz, "orientation", "top")
        end
    end
})
