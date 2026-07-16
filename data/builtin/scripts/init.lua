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

biomes.add("plains", {
    realm = biomes.REALM_SURFACE,

    palette = {
        basic = "stone",
        filler = "dirt",
        surface = "grass",
    }
})

biomes.add("test", {
    realm = biomes.REALM_SURFACE,

    lut_temp = 70,
    lut_humd = 20,

    palette = {
        basic = "stone",
        filler = "dirt",
        surface = "stone",
    }
})
