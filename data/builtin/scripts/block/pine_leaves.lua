blocks.add("pine_leaves", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "pine/pine_leaves_01.png",
      "pine/pine_leaves_02.png",
      "pine/pine_leaves_03.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,
  tools = { blocks.TOOL_XBLADE },

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_SFOIL },

  emission = 0,
  dissipation = 14,
})
