blocks.add("sand", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "sand_01.png",
      "sand_02.png",
      "sand_03.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,
  tools = { blocks.TOOL_SHOVEL },

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_SOIL },

  emission = 0,
  dissipation = 15,

  drops = {
    {
      items = {
        {
          name = "sand",
          count = 1
        }
      }
    }
  }
})
