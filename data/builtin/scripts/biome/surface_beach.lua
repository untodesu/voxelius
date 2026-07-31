biomes.add("surface_beach", {
  realm = biomes.REALM_SURFACE,

  temperature = 60,
  humidity = 60,
  continentalness = 35,
  erosion = 80,
  weirdness = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "sand" },
    surface = { name = "sand" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  }
})
