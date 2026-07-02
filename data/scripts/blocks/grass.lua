blocks.add("grass", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    top = {
      "grass_top01.png",
      "grass_top02.png",
    },
    bottom = {
      "dirt01.png",
      "dirt02.png",
      "dirt03.png",
      "dirt04.png",
    },
    default = {
      "grass_side01.png",
      "grass_side02.png",
      "grass_side03.png",
      "grass_side04.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 2,
  tools = { blocks.TOOL_SHOVEL },

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