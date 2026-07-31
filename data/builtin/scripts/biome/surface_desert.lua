biomes.add("surface_desert", {
  realm = biomes.REALM_SURFACE,

  temperature = 80,
  humidity = 20,
  continentalness = 60,
  erosion = 65,
  weirdness = 50,

  palette = {
    basic = { name = "stone" },
    filler = { name = "sandstone" },
    surface = { name = "sand" },
    fluid = { name = "water", states = { level = "8", source = "true" } },
  }
})
