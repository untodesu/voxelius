blocks.add("oak_leaves", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "oak_leaves.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,
  tools = { blocks.TOOL_XBLADE },

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_FOIL },

  emission = 0,
  dissipation = 14,
})
