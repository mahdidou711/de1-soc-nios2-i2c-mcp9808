from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ASSETS = Path(__file__).resolve().parents[1] / "assets"
INK = "#182c40"
SECONDARY = "#445a70"
LINE = "#597389"
BLUE = "#dceaf8"
GREEN = "#dceee9"
GRAY = "#ebeff3"


def canvas(height):
    image = Image.new("RGB", (1080, height), "#f7f9fb")
    return image, ImageDraw.Draw(image)


def label(draw, position, text, size=23, anchor="mm", fill=INK):
    draw.text(position, text, font=ImageFont.load_default(size=size),
              fill=fill, anchor=anchor)


def block(draw, bounds, title, detail, fill=BLUE):
    left, top, right, bottom = bounds
    center = (left + right) // 2
    draw.rounded_rectangle(bounds, radius=12, fill=fill, outline=LINE, width=2)
    label(draw, (center, top + 24), title)
    label(draw, (center, bottom - 20), detail, size=18, fill=SECONDARY)


def arrow_down(draw, x, start, end):
    draw.line((x, start, x, end), fill=LINE, width=3)
    draw.polygon([(x - 6, end - 8), (x + 6, end - 8), (x, end)], fill=LINE)


def architecture():
    image, draw = canvas(444)
    block(draw, (410, 18, 670, 98), "Nios II/f", "50 MHz system clock")
    arrow_down(draw, 540, 98, 125)
    draw.rounded_rectangle((35, 125, 1045, 172), 12, fill=INK)
    label(draw, (540, 149), "Avalon-MM interconnect", fill="white")

    peripherals = [
        (155, "JTAG UART", "Console output", GRAY),
        (412, "Timer2", "IRQ 4 / 250 ms", GRAY),
        (668, "I2C PIO", "GPIO_1 SCL / SDA", BLUE),
        (925, "HEX_PIO", "16-bit packed BCD", BLUE),
    ]
    for x, title, detail, fill in peripherals:
        arrow_down(draw, x, 172, 215)
        block(draw, (x - 120, 215, x + 120, 295), title, detail, fill)

    arrow_down(draw, 668, 295, 342)
    arrow_down(draw, 925, 295, 342)
    block(draw, (548, 342, 788, 422), "MCP9808", "I2C address 0x18", GREEN)
    block(draw, (805, 342, 1045, 422), "VHDL decoder", "HEX3 through HEX0", GREEN)

    image.save(ASSETS / "architecture.png", optimize=True)


def temperature_register():
    image, draw = canvas(286)
    label(draw, (36, 30), "Ambient temperature register (0x05)", size=28, anchor="lm")
    left, bit_width = 36, 63
    for bit in range(15, -1, -1):
        x = left + (15 - bit) * bit_width
        label(draw, (x + bit_width // 2, 80), str(bit), size=18, fill=SECONDARY)

    fields = [
        (0, 3, "Alert flags", GRAY),
        (3, 1, "Sign", GREEN),
        (4, 12, "Lower 12 bits", BLUE),
    ]
    for start, width, title, fill in fields:
        x = left + start * bit_width
        right = x + width * bit_width
        draw.rectangle((x, 99, right, 155), fill=fill, outline=LINE, width=2)
        label(draw, ((x + right) // 2, 127), title, size=20)

    label(draw, (36, 185), "Bits 12 to 0 encode a signed value in steps of 1/16 °C.",
          size=21, anchor="lm")
    label(draw, (36, 229), "Example: 0x017C encodes 380 / 16 = 23.75 °C.",
          size=21, anchor="lm", fill=SECONDARY)
    label(draw, (36, 257), "The firmware writes packed BCD 0x2375 to HEX_PIO.",
          size=21, anchor="lm", fill=SECONDARY)

    image.save(ASSETS / "temperature_register.png", optimize=True)


if __name__ == "__main__":
    ASSETS.mkdir(exist_ok=True)
    architecture()
    temperature_register()
