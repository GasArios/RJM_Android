#!/usr/bin/env python3
"""Write the editable Moonlit Harbor route terrain from its pier dimensions."""
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / 'godot-lab'
cells = set()

def pier(x, y, w, h):
    cells.update((x+i,y+j) for i in range(w) for j in range(h))

pier(0, 26, 77, 3)     # Waterfront, y832, with side walls.
pier(0, 5, 1, 21)
pier(76, 5, 1, 21)
pier(10, 25, 18, 1)    # Low pier, x320..895; rises 32px.
pier(28, 21, 18, 5)    # Upper pier, x896..1471; rises 128px.
pier(46, 19, 12, 7)    # Roof walk, x1472..1855; rises 64px.
pier(58, 17, 18, 9)    # Beacon terrace, x1856..2431; rises 64px.

data = struct.pack('<H', 0) + b''.join(
    struct.pack('<hhHHHH', x, y, 0, (x*5+y*7+x//4)%4+
                (4 if (x,y-1) not in cells else 0), 0, 0)
    for x, y in sorted(cells))
atlas = '\n'.join(f'{i}:0/0 = 0' for i in range(8))
packed = ', '.join(map(str, data))
(ROOT/'scenes/harbor_terrain.tscn').write_text(f'''[gd_scene load_steps=4 format=3]

[ext_resource type="Texture2D" path="res://assets/tiles.svg" id="1"]

[sub_resource type="TileSetAtlasSource" id="TileSetAtlasSource_pier"]
texture = ExtResource("1")
texture_region_size = Vector2i(32, 32)
{atlas}

[sub_resource type="TileSet" id="TileSet_pier"]
tile_size = Vector2i(32, 32)
sources/0 = SubResource("TileSetAtlasSource_pier")

[node name="RouteTerrain" type="TileMapLayer"]
texture_filter = 1
tile_map_data = PackedByteArray({packed})
tile_set = SubResource("TileSet_pier")
''')
print(f'{len(cells)} route cells')
