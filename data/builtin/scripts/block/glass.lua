blocks.add("glass", {
  render = blocks.RENDER_ALPHA,
  animated = false,

  textures = {
    default = {
      "glass/glass.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,

  touch = blocks.TOUCH_SOLID,

  emission = 0,
  dissipation = 15,

  drops = {
    {
      items = {
        {
          name = "glass",
          count = 1
        }
      }
    }
  }
})
