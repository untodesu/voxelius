blocks.add("vgame::stone", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "block/cube.json",
    animated = false,

    textures = {
        default = [
            "block/stone01.png",
            "block/stone02.png",
            "block/stone03.png",
            "block/stone04.png",
        ]
    }
})

blocks.add("vgame::stone_slab", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "block/slab.json",
    animated = false,

    textures = {
        default = [
            "block/stone01.png",
            "block/stone02.png",
            "block/stone03.png",
            "block/stone04.png",
        ]
    },

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
                blockmodel = "block/slab.json",
                translate = [0, 0, 0],
            }
        },
        {
            when = { orientation = "top" },
            apply = {
                blockmodel = "block/slab.json",
                translate = [0, 0.5, 0],
            }
        },
        {
            when = { orientation = "full" },
            apply = {
                blockmodel = "block/cube.json",
                translate = [0, 0, 0],
            }
        }
    ]
})
