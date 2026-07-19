print("mod:", core.NAMESPACE)
print("engine version:", core.VERSION)

fluids.add("water", {
  gravity = fluids.GRAVITY_DOWN,
  max_level = 14,
  textures = {
    still = {
      "water_still_01.png",
      "water_still_02.png",
      "water_still_03.png",
      "water_still_04.png",
      "water_still_05.png",
      "water_still_06.png",
      "water_still_07.png",
      "water_still_08.png",
      "water_still_09.png",
      "water_still_10.png",
      "water_still_11.png",
      "water_still_12.png",
      "water_still_13.png",
      "water_still_14.png",
      "water_still_15.png",
      "water_still_16.png",
    },
    flowing = {
      "water_flowing_01.png",
      "water_flowing_02.png",
      "water_flowing_03.png",
      "water_flowing_04.png",
      "water_flowing_05.png",
      "water_flowing_06.png",
      "water_flowing_07.png",
      "water_flowing_08.png",
      "water_flowing_09.png",
      "water_flowing_10.png",
      "water_flowing_11.png",
      "water_flowing_12.png",
      "water_flowing_13.png",
      "water_flowing_14.png",
      "water_flowing_15.png",
      "water_flowing_16.png",
    }
  }
})

dofile("blocks/dirt.lua")
dofile("blocks/grass.lua")
dofile("blocks/stone.lua")
dofile("blocks/wood.lua")
dofile("blocks/sand.lua")
dofile("blocks/sandstone.lua")
dofile("blocks/water.lua")

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
    fluid = { name = "water", states = { level = "14" } },
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
    fluid = { name = "water", states = { level = "14" } },
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
    fluid = { name = "water", states = { level = "14" } },
  }
})
