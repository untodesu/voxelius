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
