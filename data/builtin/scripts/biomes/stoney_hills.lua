biomes.add("stoney_hills", {
  realm = biomes.REALM_SURFACE,

  temperature = 30,
  humidity = 40,
  continentalness = 80,
  weirdness = 90,

  palette = {
    basic = { name = "stone" },
    filler = { name = "stone" },
    surface = { name = "stone" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  }
})
