blocks.add("dev_chromakey", {
  render = blocks.RENDER_SOLID,
  animated = false,

  albedo = {
    default = {
      "dev/chromakey.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,

  touch = blocks.TOUCH_SOLID,

  emission = 0,
  dissipation = 15,
})
