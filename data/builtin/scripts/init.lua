print("mod:", core.NAMESPACE)
print("engine version:", core.VERSION)

dofile("blocks/air.lua")
dofile("blocks/dirt.lua")
dofile("blocks/grass.lua")
dofile("blocks/stone.lua")

blocks.add("vtest", {
    render = blocks.RENDER_SOLID,
    animated = true,
    textures = {
        default = {
            "vtest_F1.png",
            "vtest_F2.png",
            "vtest_F3.png",
            "vtest_F4.png",
        }
    },

    model_name = "cross",
})

blocks.add("chip", {
    render = blocks.RENDER_SOLID,
    animated = true,
    textures = {
        default = {
            "chip_F1.png",
            "chip_F2.png",
            "chip_F3.png",
            "chip_F4.png",
            "chip_F5.png",
            "chip_F6.png",
            "chip_F7.png",
            "chip_F8.png",
        }
    },

    model_name = "cube",
})
