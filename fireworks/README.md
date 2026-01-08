# C64 Fireworks Simulation

This project contains a C64 solution simulating fireworks, written in C and compiled using `cc65`.
It is a port of the Python version found [here](https://github.com/ulasb/Python-Algorithms/tree/main/fireworks).

## Description

A visual simulation of fireworks using the C64 text mode.
- **Visuals**: Uses 40x25 text mode characters to represent rockets and particles.
- **Physics**: Implements gravity, velocity, and drag using fixed-point arithmetic for the 6502 processor.
- **Audio**: Uses the SID chip for launch and explosion sound effects.
- **Controls**: Interactive launch and quit functions.

## Project Structure

- `main.c`: The main C source code.
- `Makefile`: Build script for `cl65`.

## Building and Running

1.  Make sure `cc65` is installed and in your PATH.
2.  Run `make` to build the `fireworks.prg` executable.
3.  Run `fireworks.prg` in a C64 emulator (like VICE).

```bash
make
x64sc fireworks.prg
```

## Controls

- **SPACE**: Launch a firework.
- **Q** or **ESC**: Quit the simulation.

## Performance Optimizations

To achieve a smooth framerate on the 1MHz 6502 processor, several critical optimizations were implemented compared to the original Python code:

1.  **Zero-Overhead Coordinate System (Scale 256)**:
    - The simulation physics were re-mapped so that 256 internal units = 1 screen character.
    - This allows coordinate mapping (`screen_x = world_x >> 8`) to be compiled as a single "take high byte" instruction, completely eliminating 6502-expensive division/modulo operations from the inner loop.
    - This provided a 10x speedup in coordinate calculation, allowing for many more simultaneous particles.

2.  **Structure of Arrays (SoA)**:
    - Converted the particle system from an "Array of Structures" to a "Structure of Arrays".
    - On the 6502, accessing `int x[i]` is significantly faster than `structs[i].x` because it avoids complex pointer arithmetic (6502 does not have a hardware multiplier). This speeds up particle loops by 2-3x.

3.  **Fixed-Point Algebra (16-bit)**:
    - Floating-point math is irrelevant. Using `int` (16-bit) handles the physics range of 0..10240 perfectly.

4.  **Direct Video Memory Access**:
    - The standard `conio` library (`cputc`, `gotoxy`) carries significant overhead.
    - This implementation writes directly to the Video RAM at `0x0400` and Color RAM at `0xD800`.
    - A **pre-calculated row offset table** (`row_offsets`) allows for single-addition screen indexing.

5.  **Delta Clearing / Object-Based Erasing**:
    - The simulation now only "erases" (draws a space) at the previous position of each active object before moving it.

## Sound Implementation
- The **SID (Sound Interface Device)** chip is accessed directly at `0xD400`.
- **Voice 1**: Used for the launch sound (Triangle wave).
- **Voice 3**: Used for explosions (Noise waveform with envelope decay).

## License
MIT / MPL 2.0 (Inherited from original project)
