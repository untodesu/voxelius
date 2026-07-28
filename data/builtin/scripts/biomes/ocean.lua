biomes.add("ocean", {
  realm = biomes.REALM_SURFACE,

  temperature = 50,
  humidity = 65,
  continentalness = 25,
  erosion = 50,
  weirdness = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "sand" },
    surface = { name = "sand" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  }
})
