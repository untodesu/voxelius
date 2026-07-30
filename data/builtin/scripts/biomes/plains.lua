biomes.add("plains", {
  realm = biomes.REALM_SURFACE,

  temperature = 50,
  humidity = 55,
  continentalness = 55,
  erosion = 70,
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
      chance = 1.0 / 4.0,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
    },
    {
      feature = "oak_tree_01",
      chance = 1.0 / 32.0,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
      padding = 2,
    },
    {
      feature = "oak_tree_02",
      chance = 1.0 / 32.0,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
      padding = 2,
    },
    {
      feature = "oak_tree_03",
      chance = 1.0 / 32.0,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
      padding = 2,
    },
    {
      feature = "oak_tree_04",
      chance = 1.0 / 32.0,
      need_above = { blocks.TAG_GAS },
      need_below = { blocks.TAG_TURF },
      padding = 2,
    },
  },
})
