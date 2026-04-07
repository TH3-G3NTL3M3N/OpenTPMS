# Fusion 360 Electronics — Schematic Guide for OpenTPMS

## Quick Reference Shortcuts

| Action | Shortcut | Menu Path |
|---|---|---|
| Add/Place Component | `A` | Place > Component |
| Draw Net (electrical) | `N` | Place > Net |
| Place Label | `L` | Place > Label |
| Set Value | `V` | Edit > Value |
| Move | `M` | Edit > Move |
| Copy | `C` | Edit > Copy |
| Delete | `Delete` | Edit > Delete |
| Rotate (while placing) | `R` | — |
| Mirror (while placing) | `Shift+R` | — |
| Undo | `Cmd+Z` | Edit > Undo |
| Smash (detach text) | `S` | Edit > Smash |
| ERC | — | Tools > ERC |
| Switch to PCB | — | Design > Switch to PCB |

**Important:** Use **Net** (`N`) to draw electrical connections, NOT "Wire" — Wire draws non-electrical graphic lines.

---

## Step 1: Switch to the Schematic

The Library editor and Schematic editor are separate document tabs. Click the **"OpenTMS v1"** tab at the top. If no schematic exists yet: **File → New Electronics Design**.

---

## Step 2: Place Components

Press **`A`** (or **Place → Component**). Search your "OpenTMS Library v1":

| Place this | Quantity | Search term | Notes |
|---|---|---|---|
| MDBT42Q-512KV2 | 1 | `MDBT42Q` | MCU + Radio |
| LIS2DH12TR | 1 | `LIS2DH` | Accelerometer |
| MS583730BA01-50 | 1 | `MS5837` | Pressure sensor |
| CR1225_PAD | 1 | Custom footprint | Battery contact: ~10mm neg pad + VCC pad for lid spring wire |
| C1206 | 1 | `C1206` | 220µF buffer cap |
| C0402 | 5 | `C0402` | 3x 100nF decoupling (one per IC) + 2x ~150pF NFC tuning caps |
| R0402 | 2 | `R0402` | 4.7kΩ I2C pull-ups |
| L0402 or L0603 | 1 | `L0402` or search `inductor` | 10µH DC-DC buck converter inductor (VDD to DCC) |

Press `R` while placing to rotate. Click to drop. `Escape` when done.

**Suggested layout on the sheet:**

```
        [R1 4.7k]  [R2 4.7k]
             │         │
[LIS2DH12]──┼──[MDBT42Q-512KV2]──┼──[MS5837-30BA]
             │         │                │
          [C1 100n] [C2 100n][C4 220µ] [C3 100n]
                       │
                  [BK-885]
```

---

## Step 3: Add Power Symbols

Press **`A`** and search in the built-in libraries:
- **`VCC`** — place near each IC's power pin (from `supply1` or `supply2` library)
- **`GND`** — place near each IC's ground pin

Every VCC symbol is automatically connected to every other VCC symbol (same for GND). No need to draw wires between them.

---

## Step 4: Set Component Values

Click a component → **Properties panel** (right side) → set **Value** field.
Or press **`V`** then click a component.

| Component | Value |
|---|---|
| C1 (0402, near LIS2DH12) | `100nF` |
| C2 (0402, near MDBT42Q) | `100nF` |
| C3 (0402, near MS5837) | `100nF` |
| C4 (1206, near MDBT42Q) | `220uF` |
| C5 (0402, NFC tuning) | `150pF` |
| C6 (0402, NFC tuning) | `150pF` |
| R1 | `4.7k` |
| R2 | `4.7k` |
| L1 (0402/0603, DC-DC) | `10uH` |

---

## Step 5: Wire Everything

Press **`N`** (Net tool). Click pin-to-pin to draw connections.

### I2C Bus
1. MDBT42Q **P0.26** → LIS2DH12 **SDA** → MS5837 **SDA**
2. MDBT42Q **P0.27** → LIS2DH12 **SCL** → MS5837 **SCL**
3. R1 (4.7kΩ) between SDA line and VCC
4. R2 (4.7kΩ) between SCL line and VCC

### Interrupt
5. LIS2DH12 **INT1** → MDBT42Q **P0.11**

### Power (use VCC/GND symbols)
6. BK-885 **+** → VCC symbol
7. BK-885 **-** → GND symbol
8. Each IC's VDD/VCC pin → VCC symbol
9. Each IC's GND pin → GND symbol
10. C1: one side to LIS2DH12 VCC, other side to GND
11. C2: one side to MDBT42Q VDD, other side to GND
12. C3: one side to MS5837 VDD, other side to GND
13. C4 (220µF): one side to MDBT42Q VDD, other side to GND

### NFC Antenna (hardware-ready for firmware v2)
14. C5 (150pF): one side to MDBT42Q **NFC1** pin, other side to antenna pad 1
15. C6 (150pF): one side to MDBT42Q **NFC2** pin, other side to antenna pad 2
16. On the PCB, antenna pads connect to a ~10x15mm rectangular trace loop on the bottom copper layer (no component needed — just a routed trace with ground pour gap underneath)

### DC-DC Buck Converter (30% radio power savings)
17. L1 (10µH inductor): connect between MDBT42Q **VDD** (pin 11) and **DCC** (pin 10)
18. Place the inductor physically close to the MDBT42Q module on the PCB

### Full Circuit Diagram

```
                        VCC (CR1225 3.0V)
                            │
                     ┌──────┼──────────────────────────┐
                     │      │                           │
                  100nF  100nF                       100nF
                  C1│    C2│                         C3│
                     │      │                           │
                     │      │    4.7kΩ   4.7kΩ         │
                     │      │    R1│      R2│          │
                     │      │      │        │           │
              ┌──────┴──┐ ┌─┴────────────────┴──┐  ┌───┴────────┐
              │         │ │  MDBT42Q-512KV2     │  │ MS5837-30BA│
              │LIS2DH12 │ │                     │  │            │
              │         │ │  P0.26 ──── SDA ────┼──┤ SDA        │
              │ SDA ────┼─┤  P0.27 ──── SCL ────┼──┤ SCL        │
              │ SCL ────┼─┤                     │  │            │
              │         │ │  P0.11 ◄── INT1     │  │            │
              │ INT1 ───┘ │                     │  └────────────┘
              │         │ │  SWDIO ──── pad     │
              └─────────┘ │  SWCLK ──── pad     │
                          │                     │
                          │  VDD ── 220µF ── GND│  (TX buffer cap C4)
                          │  VDD ── 10µH ── DCC │  (DC-DC inductor L1)
                          └─────────────────────┘
                                    │
                                   GND

          ┌─────────────┐
          │ BK-885    │
          │  + ── VCC   │
          │  - ── GND   │
          │ (CR1225)    │
          └─────────────┘

          SWD Test Pads: SWDIO, SWCLK, VCC, GND

          NFC Antenna (PCB trace loop, bottom layer):
          MDBT42Q NFC1 ──┬── C5 150pF ──┐
                         │              │  ┌────────────┐
                         └──────────────┼──┤ PCB trace  │
                                        │  │ loop antenna│
                         ┌──────────────┼──┤ ~10x15mm   │
                         │              │  └────────────┘
          MDBT42Q NFC2 ──┴── C6 150pF ──┘
```

---

## Step 6: Name the Nets

Select a wire → **Properties panel** (right side) → set **Name** field:
- SDA wire → name `SDA`
- SCL wire → name `SCL`
- Interrupt wire → name `ACCEL_INT1`

Press **`L`** (Label tool) and click each named net to display the name visually on the schematic.

Two nets with the same name are electrically connected even without a drawn wire.

---

## Step 7: Add SWD Test Pads

Press `A`, search for **`TP`** or **`testpad`** in built-in libraries. Place 4 test pads:

| Test Pad | Connect to |
|---|---|
| TP1 | MDBT42Q **SWDIO** (pin 37) |
| TP2 | MDBT42Q **SWCLK** (pin 36) |
| TP3 | VCC |
| TP4 | GND |

---

## Step 8: Run ERC

**Tools → ERC** (or Inspect → ERC)

- Fix all errors
- Unused MDBT42Q GPIO pins (~32 of them) will show as "unconnected" warnings — approve these
- Click an error to zoom to it on the schematic

---

## Step 9: Switch to PCB Layout

Once schematic is clean: **Design → Switch to PCB**

This creates the PCB document with all components and netlist ready for layout.

---

## Component Library Sources

| Component | Source | Format |
|---|---|---|
| MDBT42Q-512KV2 | [SnapEDA](https://www.snapeda.com/parts/MDBT42Q-512KV2/Raytac%20Corporation/view-part/) | Eagle .lbr |
| MS583730BA01-50 | [SnapEDA](https://www.snapeda.com/parts/MS583730BA01-50/TE%20Connectivity/view-part/) | Eagle .lbr |
| CR1225_PAD | Custom — create in Fusion library editor | Two pads: ~10mm circle (GND/neg) + small rect (VCC/pos for lid wire) |
| LIS2DH12TR | SnapEDA or Fusion built-in | Eagle .lbr |
| C0402, C1206, R0402 | Fusion built-in library | Search `CAPC1005` (0402) or `CAPC3216` (1206) |

Import `.lbr` files via: **Library Manager → Import → Eagle Library**
