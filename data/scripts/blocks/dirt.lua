blocks.add("dirt", {
  render = blocks.RENDER_SOLID,
  animated = false,

  textures = {
    default = {
      "dirt01.png",
      "dirt02.png",
      "dirt03.png",
      "dirt04.png",
    }
  },

  model_name = "cube",
  bcoll_name = "cube",

  health = 1,
  tools = { blocks.TOOL_SHOVEL },

  emission = 0,
  dissipation = 15,

  drops = {
    {
      items = {
        {
          name = "dirt",
          count = 1
        }
      }
    }
  },

  on_rtick = function(world, bx, by, bz)
    local above = world:bget(bx, by + 1, bz)
    local light = world:lget(bx, by + 1, bz)

    if not blocks.has_tag(above, blocks.TAG_GAS) or light < 9 then
      return
    end

    local grass_id = blocks.get("grass")
    local has_grass = false

    for dx = -1, 1 do
      for dz = -1, 1 do
        if grass_id == world:bget(bx + dx, by, bz + dz) then
          has_grass = true
          break
        end
      end

      if has_grass then
        break
      end
    end

    if has_grass then
      world:bset(bx, by, bz, grass_id)
    end
  end
})
