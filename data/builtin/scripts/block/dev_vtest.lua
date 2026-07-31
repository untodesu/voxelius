blocks.add("dev_vtest", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "dev/vtest_F1.png",
      "dev/vtest_F2.png",
      "dev/vtest_F3.png",
      "dev/vtest_F4.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,

  touch = blocks.TOUCH_SOLID,

  emission = 0,
  dissipation = 15,
})
