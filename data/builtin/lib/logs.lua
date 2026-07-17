local logs = {}

function logs.on_place(identifier)
  return function(bx, by, bz, target, actor)
    local current = world.get_block(bx, by, bz)

    if current ~= blocks.NULL_BLOCK then
      return nil
    end

    if target.face == blocks.FACE_NORTH or target.face == blocks.FACE_SOUTH then
      return { orientation = "north" }
    elseif target.face == blocks.FACE_EAST or target.face == blocks.FACE_WEST then
      return { orientation = "east" }
    else
      return { orientation = "up" }
    end
  end
end

function logs.add_block(identifier, prototype)
  local def = {
    model_name = "log",
    bcoll_name = "cube",

    states = {
      orientation = {
        hint = { "north", "east", "up" },
        default = "up",
      }
    },

    variants = {
      {
        when = { orientation = "north" },
        overrides = {
          model_facing = blocks.FACE_NORTH,
        }
      },
      {
        when = { orientation = "east" },
        overrides = {
          model_facing = blocks.FACE_EAST,
        }
      },
      {
        when = { orientation = "up" },
        overrides = {
          model_facing = blocks.FACE_UP,
        }
      }
    },

    on_place = logs.on_place(identifier),
  }

  return blocks.add(identifier, prototype, def)
end

return logs
