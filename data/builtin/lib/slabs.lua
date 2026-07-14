local slabs = {}

print("LOL")

function slabs.on_place(identifier)
  return function(bx, by, bz, target, actor)
    local slab_id = blocks.get(identifier)
    local current = world.get_block(bx, by, bz)

    if target.stem == slab_id then
      local state = world.get_state(target.bx, target.by, target.bz, "orientation")

      if state == "bottom" and target.face == blocks.FACE_TOP then
        world.set_state(target.bx, target.by, target.bz, "orientation", "double")
        return nil
      elseif state == "top" and target.face == blocks.FACE_BOTTOM then
        world.set_state(target.bx, target.by, target.bz, "orientation", "double")
        return nil
      end
    end

    if current == slab_id then
      local state = world.get_state(bx, by, bz, "orientation")
      if state ~= "double" then
        world.set_state(bx, by, bz, "orientation", "double")
        return nil
      end
    end

    if target.face == blocks.FACE_BOTTOM or target.ry > 0.5 then
      return { orientation = "top" }
    else
      return { orientation = "bottom" }
    end
  end
end

function slabs.register(identifier, prototype, options)
  options = options or {}

  local health = options.health or prototype.health or 2
  local drop_name = options.drop_name or identifier

  local def = {
    model_name = "slab",
    bcoll_name = "slab",

    states = {
      orientation = {
        hint = { "top", "bottom", "double" },
        default = "bottom",
      }
    },

    variants = {
      {
        when = { orientation = "top" },
        overrides = {
          model_facing = blocks.FACE_TOP,
          bcoll_offset = { 0, 8, 0 },
          health = health,
          drops = { { items = { { name = drop_name, count = 1 } } } }
        }
      },
      {
        when = { orientation = "bottom" },
        overrides = {
          model_facing = blocks.FACE_BOTTOM,
          health = health,
          drops = { { items = { { name = drop_name, count = 1 } } } }
        }
      },
      {
        when = { orientation = "double" },
        overrides = {
          model_name = "cube",
          bcoll_name = "cube",
          health = health,
          drops = { { items = { { name = drop_name, count = 2 } } } }
        }
      }
    },

    on_place = options.on_place or slabs.on_place(identifier),
  }

  for k, v in pairs(options) do
    if def[k] == nil then
      def[k] = v
    end
  end

  return blocks.add(identifier, prototype, def)
end

return slabs
