print("mod:", core.NAMESPACE)
print("engine version:", core.VERSION)

dofile("blocks/dirt.lua")
dofile("blocks/grass.lua")
dofile("blocks/stone.lua")

blocks.add("bush", {
    render = blocks.RENDER_ALPHA,
    textures = {
        default = {
            "bush.png",
        }
    },

    model_name = "cross",
    bcoll_name = "bush",
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
    bcoll_name = "cube",
    
    touch = blocks.TOUCH_SOLID,
})
