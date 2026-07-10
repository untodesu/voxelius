-- API allows using a prototype table to avoid
-- repeating the same basic definitions of a block
-- family across multiple blocks; the resulting block
-- definition is merged with the second argument's priority;
-- If blocks.add has two arguments, the second is the actual definition

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

  tags = { blocks.TAG_ROCK }
}

blocks.add("stone", stone_prototype, {
  model_name = "cube",
  bcoll_name = "cube",

  drops = {
    {
      when = { effects = { "silk_touch" } },
      items = {
        {
          name = "stone",
          count = 1
        }
      }
    },
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

blocks.add("stone_slab", stone_prototype, {
  bcoll_name = "slab",

  states = {
    orientation = {
      hint = { "top", "bottom", "double" },
      default = "bottom"
    }
  },

  variants = {
    {
      when = { orientation = "top" },
      overrides = {
        model_name = "slab",
        model_facing = blocks.FACE_SOUTH,
        bcoll_offset = { 0, 8, 0 },
        health = 2,
        drops = {
          {
            items = {
              {
                name = "stone_slab",
                count = 1
              }
            }
          }
        }
      }
    },
    {
      when = { orientation = "bottom" },
      overrides = {
        model_name = "slab",
        model_facing = blocks.FACE_NORTH,
        health = 2,
        drops = {
          {
            items = {
              {
                name = "stone_slab",
                count = 1
              }
            }
          }
        }
      }
    },
    {
      when = { orientation = "double" },
      overrides = {
        model_name = "cube",
        health = 2,
        drops = {
          {
            items = {
              {
                name = "stone_slab",
                count = 2
              }
            }
          }
        }
      }
    }
  },

  on_place = function(bx, by, bz, placement)
    -- If placing on top/bottom of an another slab,
    -- make it a double slab; blocking a placement event
    -- defines a nil return value, otherwise an empty table
    -- permits placement, values inside are state values

    if placement.tblock == blocks.get("stone_slab") then
      if placement.tface == blocks.FACE_TOP then
        world.set_state(placement.tx, placement.ty, placement.tz, "orientation", "double")
        return nil
      elseif placement.tface == blocks.FACE_BOTTOM then
        world.set_state(placement.tx, placement.ty, placement.tz, "orientation", "double")
        return nil
      end
    end

    if placement.tface == blocks.FACE_TOP then
      return { orientation = "bottom" }
    elseif placement.tface == blocks.FACE_BOTTOM then
      return { orientation = "top" }
    end

    return { orientation = "bottom" }
  end
})
