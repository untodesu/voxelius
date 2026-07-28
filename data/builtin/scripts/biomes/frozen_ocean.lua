biomes.add("frozen_ocean", {
  realm = biomes.REALM_SURFACE,

  temperature = 10,
  humidity = 50,
  continentalness = 20,
  erosion = 50,
  weirdness = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "sand" },
    surface = { name = "sand" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  }
})
