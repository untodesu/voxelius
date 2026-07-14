blocks.add("grass", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    top = {
      "grass_01.png",
      "grass_02.png",
    },
    bottom = {
      "dirt_01.png",
      "dirt_02.png",
      "dirt_03.png",
      "dirt_04.png",
    },
    default = {
      "grass_side_01.png",
      "grass_side_02.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 2,
  tools = { blocks.TOOL_SHOVEL },

  touch = blocks.TOUCH_SOLID,
  
  emission = 0,
  dissipation = 15,

  drops = {
    {
      when = { effects = { "silk_touch" } },
      items = {
        {
          name = "grass",
          count = 1
        }
      }
    },
    {
      items = {
        {
          name = "dirt",
          count = 1
        }
      }
    }
  },

})