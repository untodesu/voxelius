print("mod:", core.NAMESPACE)
print("engine version:", core.VERSION)

dofile("blocks/dirt.lua")
dofile("blocks/grass.lua")
dofile("blocks/stone.lua")
dofile("blocks/wood.lua")
dofile("blocks/sand.lua")
dofile("blocks/sandstone.lua")

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

  temperature = 50,
  humidity = 55,
  continentalness = 50,
  weirdness = 10,

  palette = {
    basic = { name = "stone" },
    filler = { name = "dirt" },
    surface = { name = "grass" },
  }
})

biomes.add("desert", {
  realm = biomes.REALM_SURFACE,

  temperature = 85,
  humidity = 15,
  continentalness = 60,
  weirdness = 12,

  palette = {
    basic = { name = "sandstone" },
    filler = { name = "sand" },
    surface = { name = "sand" },
  }
})

biomes.add("stoney_peaks", {
  realm = biomes.REALM_SURFACE,

  temperature = 30,
  humidity = 40,
  continentalness = 80,
  weirdness = 90,

  palette = {
    basic = { name = "stone" },
    filler = { name = "stone" },
    surface = { name = "stone" },
  }
})
