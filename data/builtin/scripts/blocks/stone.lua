local slabs = require("builtin:slabs.lua")
require("builtin:slabs.lua")
require("builtin:slabs.lua")
require("builtin:slabs.lua")
require("builtin:slabs.lua")

local stone_prototype = {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "stone_01.png",
      "stone_02.png",
      "stone_03.png",
      "stone_04.png",
    }
  },

  health = 3,
  tools = { blocks.TOOL_PICKAXE },
  emission = 0,
  dissipation = 15,
  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_ROCK }
}

blocks.add("stone", stone_prototype, {
  model_name = "cube",
  bcoll_name = "cube",

  drops = {
    {
      when = { effects = { "silk_touch" } },
      items = { { name = "stone", count = 1 } }
    },
    {
      items = { { name = "cobblestone", count = 1 } }
    }
  }
})

slabs.register("stone_slab", stone_prototype, {
  health = 2,
  textures = {
    north = { "test_north.png" },
    south = { "test_south.png" },
    east = { "test_east.png" },
    west = { "test_west.png" },
    top = { "test_top.png" },
    bottom = { "test_bottom.png" },
  }
})
