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

1.  **Structure of Arrays (SoA)**:
    - BOTH the **Particle** system and the **Firework (Rocket)** system utilize "Structure of Arrays" (e.g. `p_x[]`, `f_x[]`) instead of Array of Structures.
    - On the 6502, calculating `struct_base + index * sizeof(struct)` is extremely expensive (multiplication). 
    - Separate arrays allow indexed addressing (`LDA base,X`), which is 10-20x faster for member access.

2.  **Zero-Overhead Coordinate System (Scale 256)**:
    - The simulation physics were re-mapped so that 256 internal units = 1 screen character.
    - This allows coordinate mapping (`screen_x = world_x >> 8`) to be compiled as a single "take high byte" instruction, completely eliminating division logic.

3.  **Fast Xorshift Random Number Generator**:
    - The standard `rand()` function was replaced with a custom 8-bit Xorshift implementation (`fast_rand`).
    - This drastically eliminates the overhead of generating random numbers for particle velocities during explosions.

4.  **Delta Drawing & Conditional Writes**:
    - The screen is never cleared completely. Instead, particles only "erase" their old position if they have moved.
    - **Optimization**: If a particle is at the same screen location as the previous frame, the Video RAM write is skipped entirely (unless the character shape assumedly changed), saving valuable cycles.

5.  **Direct Video Memory Access with Pre-calculated Offsets**:
    - Writes directly to the Video RAM at `0x0400` and Color RAM at `0xD800` using pre-calculated row offsets, avoiding all multiplication in the draw loop.

## Sound Implementation
- The **SID (Sound Interface Device)** chip is accessed directly at `0xD400`.
- **Voice 1**: Used for the launch sound (Triangle wave).
- **Voice 3**: Used for explosions (Noise waveform with envelope decay).

## License
MIT / MPL 2.0 (Inherited from original project)
