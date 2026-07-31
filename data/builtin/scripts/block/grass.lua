blocks.add("grass", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    top = {
      "grass/grass_01.png",
      "grass/grass_02.png",
    },
    bottom = {
      "dirt/dirt_01.png",
      "dirt/dirt_02.png",
      "dirt/dirt_03.png",
      "dirt/dirt_04.png",
    },
    default = {
      "grass/grass_side_01.png",
      "grass/grass_side_02.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 2,
  tools = { blocks.TOOL_SHOVEL },

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_TURF },

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
