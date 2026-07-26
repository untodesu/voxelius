local logdef = require("builtin:logdef")

logdef.add_block("oak_log", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "oak_wood_01.png",
      "oak_wood_02.png",
    },
    north = {
      "oak_log.png"
    },
    south = {
      "oak_log.png"
    },
  },

  health = 4,
  tools = { blocks.TOOL_AXE },

  emission = 0,
  dissipation = 15,

  touch = blocks.TOUCH_SOLID,
  tags = { blocks.TAG_WOOD },

  drops = {
    {
      items = {
        {
          name = "oak_log",
          count = 1
        }
      }
    }
  }
})


