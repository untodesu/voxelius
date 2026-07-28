biomes.add("deep_ocean", {
  realm = biomes.REALM_SURFACE,

  temperature = 50,
  humidity = 70,
  continentalness = 10,
  erosion = 50,
  weirdness = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "stone" },
    surface = { name = "stone" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  }
})
