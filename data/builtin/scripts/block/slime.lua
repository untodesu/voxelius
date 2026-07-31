blocks.add("slime", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "slime/slime_01.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,

  touch = blocks.TOUCH_BOUNCE,

  emission = 0,
  dissipation = 15,

  drops = {
    {
      items = {
        {
          name = "slime",
          count = 1
        }
      }
    }
  }
})
