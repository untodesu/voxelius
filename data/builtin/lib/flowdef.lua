local flowdef = {}

-- A sentinel value used to indicate if
-- a slope search has failed to find a drop
local SLOPE_INF <const> = 1000

-- Horizontal offsets for the four cardinal directions
local HORIZONTAL <const> = {
  { 1, 0, 0 },
  { -1, 0, 0 },
  { 0, 0, 1 },
  { 0, 0, -1 },
}

local function default_config(config_src)
  config_src = config_src or {}

  result = {}
  result.full_height = config_src.full_height or 14
  result.max_level = config_src.max_level or 8
  result.level_decrease = config_src.level_decrease or 1
  result.delay = config_src.delay or 5
  result.infinite = config_src.infinite ~= false
  result.slope_range = config_src.slope_range or 4

  return result
end

local function read_cell(block_id, bx, by, bz)
  if world.get_block(bx, by, bz) ~= block_id then
    return nil
  end

  local level_state = world.get_state(bx, by, bz, "level")
  local source_state = world.get_state(bx, by, bz, "source")
  return {
    level = tonumber(level_state) or 0,
    source = source_state == "true",
  }
end

local function effective_level(cell, max_level)
  if not cell then
    return 0
  end

  if cell.source or cell.level >= max_level then
    return max_level
  end

  return cell.level
end

local function is_falling(cell, max_level)
  return cell and not cell.source and cell.level >= max_level
end

local function is_solid(block_id, bx, by, bz)
  local id = world.get_block(bx, by, bz)

  if id == blocks.NULL_BLOCK or id == block_id then
    return false
  end

  return not blocks.is_replaceable(id)
end

local function can_flow_into(block_id, bx, by, bz)
  local id = world.get_block(bx, by, bz)

  if id == block_id then
    local cell = read_cell(block_id, bx, by, bz)
    return cell ~= nil and not cell.source
  end

  return blocks.is_replaceable(id)
end

local function write_cell(block_id, bx, by, bz, level, source)
  local existing = read_cell(block_id, bx, by, bz)

  if not existing then
    if not blocks.is_replaceable(world.get_block(bx, by, bz)) then
      return false
    end

    world.set_block(bx, by, bz, block_id)
  end

  local matched = existing and existing.level == level and existing.source == source

  world.set_state(bx, by, bz, "level", tostring(level))
  world.set_state(bx, by, bz, "source", source and "true" or "false")

  return not matched
end

local function flow_into(block_id, cfg, bx, by, bz, level)
  if level < 1 or not can_flow_into(block_id, bx, by, bz) then
    return false
  end

  local existing = read_cell(block_id, bx, by, bz)

  if existing then
    if existing.source then
      return false
    end

    if existing.level >= level then
      return false
    end
  end

  if not write_cell(block_id, bx, by, bz, level, false) then
    return false
  end

  world.schedule(bx, by, bz, cfg.delay)
  return true
end

local function compute_level(block_id, cfg, bx, by, bz)
  local above = read_cell(block_id, bx, by + 1, bz)

  if above then
    return cfg.max_level
  end

  local best = 0

  for _, dir in ipairs(HORIZONTAL) do
    local neighbour = read_cell(block_id, bx + dir[1], by + dir[2], bz + dir[3])
    local value = effective_level(neighbour, cfg.max_level)

    if value > best then
      best = value
    end
  end

  return best - cfg.level_decrease
end

local function try_infinite_source(block_id, bx, by, bz)
  local below = read_cell(block_id, bx, by - 1, bz)

  if not is_solid(block_id, bx, by - 1, bz) and not (below and below.source) then
    return false
  end

  local sources = 0

  for _, dir in ipairs(HORIZONTAL) do
    local neighbour = read_cell(block_id, bx + dir[1], by + dir[2], bz + dir[3])

    if neighbour and neighbour.source then
      sources = sources + 1

      if sources >= 2 then
        return true
      end
    end
  end

  return false
end

local function slope_distance(block_id, bx, by, bz, depth, max_depth, from_dx, from_dz)
  if can_flow_into(block_id, bx, by - 1, bz) then
    return depth
  end

  if depth >= max_depth then
    return SLOPE_INF
  end

  local best = SLOPE_INF

  for _, dir in ipairs(HORIZONTAL) do
    if dir[1] ~= -from_dx or dir[3] ~= -from_dz then
      local nx = bx + dir[1]
      local nz = bz + dir[3]

      if can_flow_into(block_id, nx, by, nz) then
        local dist = slope_distance(block_id, nx, by, nz, depth + 1, max_depth, dir[1], dir[3])

        if dist < best then
          best = dist
        end
      end
    end
  end

  return best
end

local function preferred_spread_dirs(block_id, cfg, bx, by, bz)
  local best = SLOPE_INF
  local dirs = {}

  for _, dir in ipairs(HORIZONTAL) do
    local nx = bx + dir[1]
    local ny = by + dir[2]
    local nz = bz + dir[3]

    if can_flow_into(block_id, nx, ny, nz) then
      local weight

      if can_flow_into(block_id, nx, ny - 1, nz) then
        weight = 0
      else
        weight = slope_distance(block_id, nx, ny, nz, 1, cfg.slope_range, dir[1], dir[3])
      end

      if weight < best then
        best = weight
        dirs = { dir }
      elseif weight == best then
        table.insert(dirs, dir)
      end
    end
  end

  return dirs
end

local function update_fluid(block_id, config, bx, by, bz)
  local cell = read_cell(block_id, bx, by, bz)

  if not cell then
    return
  end

  local source = cell.source
  local level = effective_level(cell, config.max_level)
  local falling = is_falling(cell, config.max_level)

  if config.infinite and try_infinite_source(block_id, bx, by, bz) then
    if not source or cell.level ~= config.max_level then
      write_cell(block_id, bx, by, bz, config.max_level, true)
      source = true
      level = config.max_level
      falling = false
      world.schedule(bx, by, bz, config.delay)
    end
  elseif not source then
    local new_level = compute_level(block_id, config, bx, by, bz)

    if new_level < 1 then
      world.set_block(bx, by, bz, blocks.NULL_BLOCK)
      return
    end

    if new_level ~= cell.level then
      write_cell(block_id, bx, by, bz, new_level, false)
      level = effective_level({ level = new_level, source = false }, config.max_level)
      falling = new_level >= config.max_level
      world.schedule(bx, by, bz, config.delay)
    end
  end

  local below_open = can_flow_into(block_id, bx, by - 1, bz)

  if below_open then
    flow_into(block_id, config, bx, by - 1, bz, config.max_level)
  end

  if not source and below_open then
    return
  end

  local spread

  if falling then
    spread = config.max_level - config.level_decrease
  else
    spread = level - config.level_decrease
  end

  if spread < 1 then
    return
  end

  for _, dir in ipairs(preferred_spread_dirs(block_id, config, bx, by, bz)) do
    flow_into(block_id, config, bx + dir[1], by + dir[2], bz + dir[3], spread)
  end
end

function flowdef.states(config_src)
  local config = default_config(config_src)
  local level_hints = {}

  for level = 1, config.max_level do
    table.insert(level_hints, tostring(level))
  end

  return {
    level = {
      default = tostring(config.max_level),
      hint = level_hints,
    },
    source = {
      default = "false",
      hint = { "true", "false" },
    },
  }
end

function flowdef.variants(config_src)
  local config = default_config(config_src)

  local variants = {
    {
      when = { source = "true" },
      overrides = {
        fluid_level = config.full_height,
      },
    },
    {
      when = { source = "false", level = tostring(config.max_level) },
      overrides = {
        fluid_level = config.full_height,
      },
    }
  }

  for level = 1, config.max_level - 1 do
    local height = math.max(1, math.floor(level * (config.full_height - 1) / (config.max_level - 1)))

    table.insert(variants, {
      when = { source = "false", level = tostring(level) },
      overrides = {
        fluid_level = height,
      },
    })
  end

  return variants
end

function flowdef.on_place(config_src)
  local config = default_config(config_src)

  return function(bx, by, bz, target, occupant, actor)
    if not occupant.replaceable then
      return nil
    end

    world.schedule(bx, by, bz, config.delay)

    return {
      level = tostring(config.max_level),
      source = "true",
    }
  end
end

function flowdef.on_tick(config_src)
  local config = default_config(config_src)

  return function(bx, by, bz, source)
    if source == blocks.TICK_NEIGHBOUR then
      world.schedule(bx, by, bz, config.delay)
      return
    end

    if source ~= blocks.TICK_SCRIPTED and source ~= blocks.TICK_RANDOM then
      return
    end

    update_fluid(world.get_block(bx, by, bz), config, bx, by, bz)
  end
end

return flowdef
