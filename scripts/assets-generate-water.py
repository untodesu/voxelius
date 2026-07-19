#!/usr/bin/env python3

# This is vibecoded lol

from __future__ import annotations

import argparse
import math
import struct
import zlib
from pathlib import Path

def clamp(value: float, lo: float, hi: float) -> float:
    return lo if value < lo else hi if value > hi else value

def fade(t: float) -> float:
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0)

def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t

def write_png_rgba(path: Path, width: int, height: int, pixels: bytes) -> None:
    def chunk(tag: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)

    raw = bytearray()
    stride = width * 4
    for y in range(height):
        raw.append(0)
        raw.extend(pixels[y * stride : (y + 1) * stride])

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b"")
    path.write_bytes(png)

class HashNoise:
    def __init__(self, seed: int = 0) -> None:
        self.seed = seed & 0xFFFFFFFF

    def _hash(self, *coords: int) -> float:
        h = self.seed
        for c in coords:
            h ^= (c * 374761393) & 0xFFFFFFFF
            h = (h * 668265263) & 0xFFFFFFFF
            h ^= h >> 13
        h = (h * 1274126177) & 0xFFFFFFFF
        return (h & 0xFFFFFF) / float(0xFFFFFF)

    def noise4(self, x: float, y: float, z: float, w: float) -> float:
        x0 = math.floor(x)
        y0 = math.floor(y)
        z0 = math.floor(z)
        w0 = math.floor(w)
        fx = fade(x - x0)
        fy = fade(y - y0)
        fz = fade(z - z0)
        fw = fade(w - w0)
        ix0, iy0, iz0, iw0 = int(x0), int(y0), int(z0), int(w0)

        def corner(dx: int, dy: int, dz: int, dw: int) -> float:
            return self._hash(ix0 + dx, iy0 + dy, iz0 + dz, iw0 + dw)

        n0000 = corner(0, 0, 0, 0)
        n1000 = corner(1, 0, 0, 0)
        n0100 = corner(0, 1, 0, 0)
        n1100 = corner(1, 1, 0, 0)
        n0010 = corner(0, 0, 1, 0)
        n1010 = corner(1, 0, 1, 0)
        n0110 = corner(0, 1, 1, 0)
        n1110 = corner(1, 1, 1, 0)
        n0001 = corner(0, 0, 0, 1)
        n1001 = corner(1, 0, 0, 1)
        n0101 = corner(0, 1, 0, 1)
        n1101 = corner(1, 1, 0, 1)
        n0011 = corner(0, 0, 1, 1)
        n1011 = corner(1, 0, 1, 1)
        n0111 = corner(0, 1, 1, 1)
        n1111 = corner(1, 1, 1, 1)

        x00 = lerp(lerp(n0000, n1000, fx), lerp(n0100, n1100, fx), fy)
        x10 = lerp(lerp(n0010, n1010, fx), lerp(n0110, n1110, fx), fy)
        x01 = lerp(lerp(n0001, n1001, fx), lerp(n0101, n1101, fx), fy)
        x11 = lerp(lerp(n0011, n1011, fx), lerp(n0111, n1111, fx), fy)
        return lerp(lerp(x00, x10, fz), lerp(x01, x11, fz), fw)

    def fbm4(self, x: float, y: float, z: float, w: float, octaves: int = 4, lacunarity: float = 2.0, gain: float = 0.5) -> float:
        amp = 1.0
        freq = 1.0
        total = 0.0
        norm = 0.0
        for _ in range(octaves):
            total += amp * self.noise4(x * freq, y * freq, z * freq, w * freq)
            norm += amp
            amp *= gain
            freq *= lacunarity
        return total / norm if norm else 0.0

def heat_to_rgba(heat: float, alpha_min: int, alpha_max: int) -> tuple[int, int, int, int]:
    heat = clamp(heat, 0.0, 1.0)
    h2 = heat * heat
    r = int(32.0 + h2 * 32.0)
    g = int(50.0 + h2 * 64.0)
    b = 255
    a = int(round(alpha_min + h2 * (alpha_max - alpha_min)))
    a = int(clamp(a, 0, 255))
    return r, g, b, a

def render_field(width: int, height: int, sample_heat, alpha_min: int, alpha_max: int) -> bytes:
    out = bytearray(width * height * 4)
    for y in range(height):
        for x in range(width):
            r, g, b, a = heat_to_rgba(sample_heat(x, y), alpha_min, alpha_max)
            o = (y * width + x) * 4
            out[o] = r
            out[o + 1] = g
            out[o + 2] = b
            out[o + 3] = a
    return bytes(out)

def torus2(x: float, y: float, size: float, rx: float, ry: float) -> tuple[float, float, float, float]:
    u = 2.0 * math.pi * (x / size)
    v = 2.0 * math.pi * (y / size)
    return rx * math.cos(u), rx * math.sin(u), ry * math.cos(v), ry * math.sin(v)

def sample_tiled(noise: HashNoise, x: float, y: float, size: float, rx: float, ry: float, tz: float = 0.0, tw: float = 0.0, octaves: int = 5) -> float:
    px, py, qx, qy = torus2(x, y, size, rx, ry)
    wx, wy, wq, wr = torus2(x + 3.7, y - 2.1, size, rx * 0.55, ry * 0.55)
    warp = noise.fbm4(wx + tz * 0.25, wy + tw * 0.25, wq, wr, octaves=2)
    return noise.fbm4(px + warp * 0.35 + tz * 0.4, py + warp * 0.1, qx + tw * 0.4, qy, octaves=octaves)

def still_heat(noise: HashNoise, x: float, y: float, size: float, theta: float, rx: float, ry: float, time_radius: float) -> float:
    tz = time_radius * math.cos(theta)
    tw = time_radius * math.sin(theta)
    return sample_tiled(noise, x, y, size, rx, ry, tz, tw)

def flowing_base_field(noise: HashNoise, size: int, rx: float, ry: float) -> list[float]:
    field = [0.0] * (size * size)
    for y in range(size):
        for x in range(size):
            field[y * size + x] = sample_tiled(noise, float(x), float(y), float(size), rx, ry)
    return field

def nn_upscale_rgba(src: bytes, src_w: int, src_h: int, scale: int) -> bytes:
    dst_w = src_w * scale
    dst_h = src_h * scale
    out = bytearray(dst_w * dst_h * 4)
    for y in range(dst_h):
        sy = y // scale
        for x in range(dst_w):
            sx = x // scale
            si = (sy * src_w + sx) * 4
            di = (y * dst_w + x) * 4
            out[di : di + 4] = src[si : si + 4]
    return bytes(out)

def tile_2x2_rgba(src: bytes, size: int) -> bytes:
    out_w = size * 2
    out = bytearray(out_w * out_w * 4)
    for y in range(out_w):
        sy = y % size
        for x in range(out_w):
            sx = x % size
            si = (sy * size + sx) * 4
            di = (y * out_w + x) * 4
            out[di : di + 4] = src[si : si + 4]
    return bytes(out)

def write_debug_gif(path: Path, frames_rgba: list[bytes], src_size: int, fps: int = 10) -> None:
    try:
        from PIL import Image
    except ImportError as exc:
        raise SystemExit("Pillow is required for --debug-gif (pip install pillow)") from exc

    tiled_size = src_size * 2
    if 512 % tiled_size != 0:
        raise SystemExit(f"--debug-gif needs 512 divisible by 2*size (got size={src_size})")
    up = 512 // tiled_size
    duration_ms = int(round(1000 / fps))

    images: list[Image.Image] = []
    for rgba in frames_rgba:
        tiled = tile_2x2_rgba(rgba, src_size)
        big = nn_upscale_rgba(tiled, tiled_size, tiled_size, up)
        img = Image.frombytes("RGBA", (512, 512), big)
        bg = Image.new("RGBA", (512, 512), (20, 40, 80, 255))
        composed = Image.alpha_composite(bg, img).convert("P", palette=Image.ADAPTIVE, colors=256)
        images.append(composed)

    images[0].save(path, save_all=True, append_images=images[1:], duration=duration_ms, loop=0, optimize=False)

def flowing_radii(direction: str, along: float, across: float) -> tuple[float, float]:
    if direction in ("py", "ny"):
        # vertical bands: high rx, low ry
        return across, along
    if direction in ("px", "nx"):
        # horizontal bands: low rx, high ry
        return along, across
    raise ValueError(f"unknown scroll direction: {direction}")

def sample_scrolled(field: list[float], size: int, x: int, y: int, frame: int, direction: str) -> float:
    if direction == "px":
        sx, sy = (x + frame) % size, y
    elif direction == "nx":
        sx, sy = (x - frame) % size, y
    elif direction == "py":
        sx, sy = x, (y + frame) % size
    elif direction == "ny":
        sx, sy = x, (y - frame) % size
    else:
        raise ValueError(f"unknown scroll direction: {direction}")
    return field[sy * size + sx]

def sample_flowing_heat(field_slow: list[float], field_fast: list[float], size: int, x: int, y: int, frame: int, direction: str) -> float:
    # Two layers at 1x / 2x scroll — parallax so it reads as flowing, not sliding
    slow = sample_scrolled(field_slow, size, x, y, frame, direction)
    fast = sample_scrolled(field_fast, size, x, y, frame * 2, direction)
    return 0.62 * slow + 0.38 * fast

def generate(out_dir: Path, frames: int, size: int, seed: int, rx: float, ry: float, flow_along: float, flow_across: float, time_radius: float, still_prefix: str, flowing_prefix: str, scroll_dir: str, alpha_min: int, alpha_max: int, debug_gif: bool) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    noise = HashNoise(seed=seed)

    still_frames_rgba: list[bytes] = []
    still_digits = max(2, int(math.log10(max(frames, 1))) + 1)
    for frame in range(frames):
        theta = 2.0 * math.pi * (frame / frames)

        def sample(x: int, y: int, _theta: float = theta) -> float:
            return still_heat(noise, float(x), float(y), float(size), _theta, rx, ry, time_radius)

        rgba = render_field(size, size, sample, alpha_min, alpha_max)
        still_frames_rgba.append(rgba)
        path = out_dir / f"{still_prefix}_{frame + 1:0{still_digits}d}.png"
        write_png_rgba(path, size, size, rgba)
        print(f"wrote {path.name}")

    flow_rx, flow_ry = flowing_radii(scroll_dir, flow_along, flow_across)
    noise_fast = HashNoise(seed=seed ^ 0xA5A5A5A5)
    field_slow = flowing_base_field(noise, size, flow_rx, flow_ry)
    field_fast = flowing_base_field(noise_fast, size, flow_rx * 1.15, flow_ry * 1.1)

    flowing_frames = size
    flowing_digits = max(2, int(math.log10(max(flowing_frames, 1))) + 1)
    flowing_frames_rgba: list[bytes] = []

    for frame in range(flowing_frames):
        def sample(x: int, y: int, _frame: int = frame) -> float:
            return sample_flowing_heat(field_slow, field_fast, size, x, y, _frame, scroll_dir)
        rgba = render_field(size, size, sample, alpha_min, alpha_max)
        flowing_frames_rgba.append(rgba)
        path = out_dir / f"{flowing_prefix}_{frame + 1:0{flowing_digits}d}.png"
        write_png_rgba(path, size, size, rgba)
        print(f"wrote {path.name}")

    if debug_gif:
        still_gif = out_dir / f"{still_prefix}_debug.gif"
        flowing_gif = out_dir / f"{flowing_prefix}_debug.gif"
        write_debug_gif(still_gif, still_frames_rgba, size, fps=10)
        write_debug_gif(flowing_gif, flowing_frames_rgba, size, fps=10)
        print(f"wrote {still_gif.name} / {flowing_gif.name} (512x512 @ 10 FPS, 2x2 tiled)")

    print(f"still: {frames} frames (toroidal fbm + time circle, seamless)")
    print(f"flowing: {flowing_frames} frames (scroll {scroll_dir}, flow rx/ry={flow_rx:.2f}/{flow_ry:.2f}, dual-layer, seamless)")
    print(f"alpha: {alpha_min} .. {alpha_max} (heat-modulated)")

def main() -> None:
    repo = Path(__file__).resolve().parents[1]
    default_out = repo / "data" / "builtin" / "textures" / "block"

    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-o", "--out-dir", type=Path, default=default_out)
    parser.add_argument("-n", "--frames", type=int, default=16, help="still loop length (default: 16)")
    parser.add_argument("--size", type=int, default=16)
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--rx", type=float, default=1.0, help="still: X torus radius (higher = shorter horizontal dashes)")
    parser.add_argument("--ry", type=float, default=10.0, help="still: Y torus radius (higher = thinner horizontal bands)")
    parser.add_argument("--flow-along", type=float, default=0.25, help="flowing: noise radius along scroll (low = streaks stretch with the flow)")
    parser.add_argument("--flow-across", type=float, default=16.0, help="flowing: noise radius across scroll (high = thin bands parallel to flow)")
    parser.add_argument("--time-radius", type=float, default=0.55, help="time-circle radius in noise space")
    parser.add_argument("--still-prefix", default="water_still")
    parser.add_argument("--flowing-prefix", default="water_flowing")
    parser.add_argument("--scroll", choices=("px", "nx", "py", "ny"), default="ny", help="flowing scroll direction: px/nx/py/ny (+X/-X/+Y/-Y). Default: ny (down)")
    parser.add_argument(
        "--alpha-min",
        type=int,
        default=146,
        help="alpha at heat=0 (default: 146, Classic)",
    )
    parser.add_argument(
        "--alpha-max",
        type=int,
        default=196,
        help="alpha at heat=1 (default: 196 = 146+50, Classic)",
    )
    parser.add_argument(
        "--alpha",
        type=int,
        default=None,
        help="constant alpha (overrides --alpha-min/--alpha-max)",
    )
    parser.add_argument("--debug-gif", action="store_true", help="also write 512x512 debug GIFs at 10 FPS (2x2 tiled NN upscale; needs Pillow)")
    args = parser.parse_args()

    if args.frames < 1:
        parser.error("--frames must be >= 1")
    if args.size < 1:
        parser.error("--size must be >= 1")
    if args.rx <= 0 or args.ry <= 0:
        parser.error("--rx/--ry must be > 0")
    if args.flow_along <= 0 or args.flow_across <= 0:
        parser.error("--flow-along/--flow-across must be > 0")

    if args.alpha is not None:
        alpha_min = alpha_max = args.alpha
    else:
        alpha_min, alpha_max = args.alpha_min, args.alpha_max

    for label, value in (("alpha-min", alpha_min), ("alpha-max", alpha_max), ("alpha", args.alpha)):
        if value is not None and not (0 <= value <= 255):
            parser.error(f"--{label} must be in 0..255")

    generate(
        out_dir=args.out_dir,
        frames=args.frames,
        size=args.size,
        seed=args.seed,
        rx=args.rx,
        ry=args.ry,
        flow_along=args.flow_along,
        flow_across=args.flow_across,
        time_radius=args.time_radius,
        still_prefix=args.still_prefix,
        flowing_prefix=args.flowing_prefix,
        scroll_dir=args.scroll,
        alpha_min=alpha_min,
        alpha_max=alpha_max,
        debug_gif=args.debug_gif,
    )

if __name__ == "__main__":
    main()
