local slabdef = require("builtin:slabdef")

local cobblestone_prototype = {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "cobblestone_01.png",
      "cobblestone_02.png",
    }
  },

  health = 2,
  tools = { blocks.TOOL_PICKAXE },

  emission = 0,
  dissipation = 15,

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_ROCK }
}

blocks.add("cobblestone", cobblestone_prototype, {
  model_name = "cube",
  bcoll_name = "cube",

  drops = {
    {
      items = {
        {
          name = "cobblestone",
          count = 1
        }
      }
    }
  }
})

slabdef.add_block("cobblestone_slab", cobblestone_prototype, {
  health = 1,
})
