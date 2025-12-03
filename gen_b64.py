import base64

moon_svg = """<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24'><text x='50%' y='50%' dominant-baseline='central' text-anchor='middle' font-size='16'>🌙</text></svg>"""
sun_svg = """<svg xmlns='http://www.w3.org/2000/svg' width='24' height='24' viewBox='0 0 24 24'><text x='50%' y='50%' dominant-baseline='central' text-anchor='middle' font-size='16'>☀️</text></svg>"""

print("MOON_B64:", base64.b64encode(moon_svg.encode('utf-8')).decode('utf-8'))
print("SUN_B64:", base64.b64encode(sun_svg.encode('utf-8')).decode('utf-8'))
