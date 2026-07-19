local fluids = {}

local HORIZONTAL = {
  { 1, 0, 0 },
  { -1, 0, 0 },
  { 0, 0, 1 },
  { 0, 0, -1 },
}

-- Sentinel used by the Minecraft slope search when no drop is found.
local SLOPE_INF = 1000

local function defaults(options)
  options = options or {}
  return {
    -- Fullness units: source / falling = max_level, thinnest flowing = 1.
    -- Minecraft water uses 8 (still getLevel) with decrease 1 → 7-block reach.
    max_level = options.max_level or 8,
    -- How much fullness drops per horizontal step (Minecraft water=1, lava=2).
    level_decrease = options.level_decrease or 1,
    delay = options.delay or 5,
    infinite = options.infinite ~= false,
    -- Minecraft water searches 4 blocks for a drop; lava uses 2.
    slope_range = options.slope_range or 4,
  }
end

--- Build `states` for a Minecraft-like fluid block.
---
--- `source=true` — infinite/full source block (fluid_level = 14/16).
--- `source=false`, `level` 1 .. max_level-1 — flowing, always shorter than source.
--- `source=false`, `level` = max_level — falling column (same 14/16; mesher
--- makes it a full block only when the same fluid is above).
function fluids.states(options)
  local cfg = defaults(options)
  local level_hints = {}

  for level = 1, cfg.max_level do
    level_hints[#level_hints + 1] = tostring(level)
  end

  return {
    level = {
      default = tostring(cfg.max_level),
      hint = level_hints,
    },
    source = {
      default = "false",
      hint = { "true", "false" },
    },
  }
end

--- Build `variants` mapping level/source to `fluid_level` (1/16ths).
---
--- Source blocks are always full height (14/16). Flowing blocks use levels
--- 1 .. max_level-1 and are always shorter than a source. Level == max_level
--- with source=false is falling water: same 14/16 height as a source; the
--- mesher only lifts it to a full block when the same fluid is above.
function fluids.variants(options)
  local cfg = defaults(options)
  local full_height = 14
  local result = {
    {
      when = { source = "true" },
      overrides = {
        fluid_level = full_height,
      },
    },
    {
      when = { source = "false", level = tostring(cfg.max_level) },
      overrides = {
        fluid_level = full_height,
      },
    },
  }

  -- Horizontal flowing: strictly below source height.
  for level = 1, cfg.max_level - 1 do
    local height = math.max(1, math.floor(level * (full_height - 1) / (cfg.max_level - 1)))

    result[#result + 1] = {
      when = { source = "false", level = tostring(level) },
      overrides = {
        fluid_level = height,
      },
    }
  end

  return result
end

function fluids.on_place(options)
  local cfg = defaults(options)

  return function(bx, by, bz, target, occupant, actor)
    if not occupant.replaceable then
      return nil
    end

    world.schedule(bx, by, bz, cfg.delay)

    return {
      level = tostring(cfg.max_level),
      source = "true",
    }
  end
end

local function read_cell(block_id, bx, by, bz)
  if world.get_block(bx, by, bz) ~= block_id then
    return nil
  end

  return {
    level = tonumber(world.get_state(bx, by, bz, "level")) or 0,
    source = world.get_state(bx, by, bz, "source") == "true",
  }
end

-- Minecraft treats falling fluid (meta >= 8 / FALLING) as full for neighbour
-- strength, and sources as max fullness.
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

-- Minecraft canFlowInto: replaceable (incl. air) or non-source same fluid.
-- Sources block both spreading and the slope pathfinder.
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

  -- Always run set_state: a stem reports default level/source via get_state but
  -- still has fluid_level 0 until resolve_variant runs. Skipping set_state when
  -- defaults "match" left invisible water in the world.
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

    -- Only raise / refresh; never lower a neighbour from here.
    if existing.level >= level then
      return false
    end
  end

  if not write_cell(block_id, bx, by, bz, level, false) then
    return false
  end

  -- Newly written cells are not themselves notified by set_block, so they
  -- must be scheduled or flow stops after the first ring of neighbours.
  world.schedule(bx, by, bz, cfg.delay)
  return true
end

-- Minecraft getUpdatedState / getSmallestFlowDecay in fullness units:
--   new_level = max(neighbour effective levels) - level_decrease
--   fluid above → falling at max_level
local function compute_level(block_id, cfg, bx, by, bz)
  local above = read_cell(block_id, bx, by + 1, bz)

  if above then
    return cfg.max_level
  end

  local best = 0

  for _, d in ipairs(HORIZONTAL) do
    local neighbour = read_cell(block_id, bx + d[1], by + d[2], bz + d[3])
    local value = effective_level(neighbour, cfg.max_level)

    if value > best then
      best = value
    end
  end

  return best - cfg.level_decrease
end

-- Minecraft: ≥2 adjacent sources + solid (or same-fluid source) below → source.
local function try_infinite_source(block_id, bx, by, bz)
  local below = read_cell(block_id, bx, by - 1, bz)

  if not is_solid(block_id, bx, by - 1, bz) and not (below and below.source) then
    return false
  end

  local sources = 0

  for _, d in ipairs(HORIZONTAL) do
    local neighbour = read_cell(block_id, bx + d[1], by + d[2], bz + d[3])

    if neighbour and neighbour.source then
      sources = sources + 1

      if sources >= 2 then
        return true
      end
    end
  end

  return false
end

-- Shortest walk from (bx,by,bz) to a cell that can fall, capped at max_depth.
-- Sources are impassable (Minecraft canFlowInto). Never reverse immediately.
local function slope_distance(block_id, bx, by, bz, depth, max_depth, from_dx, from_dz)
  if can_flow_into(block_id, bx, by - 1, bz) then
    return depth
  end

  if depth >= max_depth then
    return SLOPE_INF
  end

  local best = SLOPE_INF

  for _, d in ipairs(HORIZONTAL) do
    if d[1] ~= -from_dx or d[3] ~= -from_dz then
      local nx = bx + d[1]
      local nz = bz + d[3]

      if can_flow_into(block_id, nx, by, nz) then
        local dist = slope_distance(block_id, nx, by, nz, depth + 1, max_depth, d[1], d[3])

        if dist < best then
          best = dist
        end
      end
    end
  end

  return best
end

-- Minecraft: assign each side a weight = distance to nearest drop (or 1000),
-- then only spread into the minimum-weight directions.
local function preferred_spread_dirs(block_id, cfg, bx, by, bz)
  local best = SLOPE_INF
  local dirs = {}

  for _, d in ipairs(HORIZONTAL) do
    local nx = bx + d[1]
    local ny = by + d[2]
    local nz = bz + d[3]

    if can_flow_into(block_id, nx, ny, nz) then
      local weight

      if can_flow_into(block_id, nx, ny - 1, nz) then
        weight = 0
      else
        weight = slope_distance(block_id, nx, ny, nz, 1, cfg.slope_range, d[1], d[3])
      end

      if weight < best then
        best = weight
        dirs = { d }
      elseif weight == best then
        dirs[#dirs + 1] = d
      end
    end
  end

  return dirs
end

local function update_fluid(block_id, cfg, bx, by, bz)
  local cell = read_cell(block_id, bx, by, bz)

  if not cell then
    return
  end

  local source = cell.source
  local level = effective_level(cell, cfg.max_level)
  local falling = is_falling(cell, cfg.max_level)

  -- 1) Recalculate this cell (Minecraft getUpdatedState).
  if cfg.infinite and try_infinite_source(block_id, bx, by, bz) then
    if not source or cell.level ~= cfg.max_level then
      write_cell(block_id, bx, by, bz, cfg.max_level, true)
      source = true
      level = cfg.max_level
      falling = false
      world.schedule(bx, by, bz, cfg.delay)
    end
  elseif not source then
    local new_level = compute_level(block_id, cfg, bx, by, bz)

    if new_level < 1 then
      world.set_block(bx, by, bz, blocks.NULL_BLOCK)
      return
    end

    if new_level ~= cell.level then
      write_cell(block_id, bx, by, bz, new_level, false)
      level = effective_level({ level = new_level, source = false }, cfg.max_level)
      falling = new_level >= cfg.max_level
      world.schedule(bx, by, bz, cfg.delay)
    end
  end

  -- 2) Always try to flow downward as falling fluid (full height).
  local below_open = can_flow_into(block_id, bx, by - 1, bz)

  if below_open then
    flow_into(block_id, cfg, bx, by - 1, bz, cfg.max_level)
  end

  -- 3) Horizontal spread only if this is a source OR the block below blocks
  --    downward flow. Flowing water over a hole falls only — no side spray.
  --    (Minecraft: decay == 0 || !canFlowInto(below))
  if not source and below_open then
    return
  end

  -- Falling fluid spreads sideways as if next to a source (decay 8 → next 1).
  local spread

  if falling then
    spread = cfg.max_level - cfg.level_decrease
  else
    spread = level - cfg.level_decrease
  end

  if spread < 1 then
    return
  end

  for _, d in ipairs(preferred_spread_dirs(block_id, cfg, bx, by, bz)) do
    flow_into(block_id, cfg, bx + d[1], by + d[2], bz + d[3], spread)
  end
end

--- Produce an `on_tick` handler for a fluid block family.
---
--- options:
---   max_level (default 8) — source / falling fullness
---   level_decrease (default 1) — fullness lost per horizontal step
---   delay (default 5) — ticks between neighbour debounce and flow update
---   infinite (default true) — 2+ adjacent sources form a new source
---   slope_range (default 4) — how far to search for a drop when choosing sides
function fluids.on_tick(block_name, options)
  local cfg = defaults(options)

  return function(bx, by, bz, source)
    if source == blocks.TICK_NEIGHBOUR then
      world.schedule(bx, by, bz, cfg.delay)
      return
    end

    if source ~= blocks.TICK_SCRIPTED and source ~= blocks.TICK_RANDOM then
      return
    end

    update_fluid(blocks.get(block_name), cfg, bx, by, bz)
  end
end

return fluids
