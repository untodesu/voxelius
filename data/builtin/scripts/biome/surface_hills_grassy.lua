biomes.add("surface_hills_grassy", {
  realm = biomes.REALM_SURFACE,

  temperature = 40,
  humidity = 50,
  continentalness = 70,
  erosion = 25,
  weirdness = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "dirt" },
    surface = { name = "grass" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  },

  scatter = {
    {
      feature = "bush",
      chance = 1.0 / 8.0,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
    },
  },
})
