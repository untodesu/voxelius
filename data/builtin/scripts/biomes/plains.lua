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
  }
})
