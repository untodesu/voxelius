blocks.add("vgame::grass", {
    render = blocks.RENDER_OPAQUE,
    blockmodel = "block/cube.json",
    animated = false,

    textures = {
        top = [
            "block/grass_top01.png",
            "block/grass_top02.png",
        ],

        bottom = [
            "block/dirt01.png",
            "block/dirt02.png",
            "block/dirt03.png",
            "block/dirt04.png",
        ],

        default = [
            "block/grass_side01.png",
            "block/grass_side02.png",
            "block/grass_side03.png",
            "block/grass_side04.png",
        ]
    }
})

blocks.tick("vgame::grass", function(world, bx, by, bz)
    local above = world.bget(bx, by + 1, bz)
    local light = world.lgetv(bx, by + 1, bz)

    if light < 9 and above ~= blocks.NULL_BLOCK then
        world.bset(bx, by, bz, blocks.get("vgame::dirt"))
        return true
    end

    return false
end)
