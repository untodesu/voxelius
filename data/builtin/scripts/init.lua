print("mod:", core.NAMESPACE)
print("engine version:", core.VERSION)

dofile("blocks/dirt.lua")
dofile("blocks/grass.lua")
dofile("blocks/stone.lua")
dofile("blocks/wood.lua")

blocks.add("bush", {
  render = blocks.RENDER_ALPHA,
  textures = {
    default = {
      "bush.png",
    }
  },

  model_name = "cross",
  bcoll_name = "bush",

  replaceable = true,
})

biomes.add("plains", {
  realm = biomes.REALM_SURFACE,

  lut_temp = 50,
  lut_humd = 50,
  lut_axis = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "dirt" },
    surface = { name = "grass" },
  }
})

biomes.add("testing", {
  realm = biomes.REALM_SURFACE,

  lut_temp = 40,
  lut_humd = 90,

  palette = {
    basic = { name = "stone" },
    filler = { name = "stone" },
    surface = { name = "stone" }
  }
})

biomes.add("testing2", {
  realm = biomes.REALM_SURFACE,

  lut_temp = 90,
  lut_humd = 10,

  palette = {
    basic = { name = "stone" },
    filler = { name = "stone" },
    surface = { name = "dirt" }
  }
})

