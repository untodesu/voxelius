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

    

})
