blocks.add("sandstone", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "sandstone_01.png",
      "sandstone_02.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 2,
  tools = { blocks.TOOL_PICKAXE },

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_ROCK },

  emission = 0,
  dissipation = 15,

  drops = {
    {
      items = {
        {
          name = "sandstone",
          count = 1
        }
      }
    }
  },
})
