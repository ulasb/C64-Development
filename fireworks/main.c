/*
 * Fireworks Simulation for Commodore 64 - SoA Everywhere
 *
 * Optimizations:
 * 1. Zero-division math (Scale 256).
 * 2. SoA for BOTH Particles AND Rockets.
 * 3. Fast PRNG replacing rand().
 * 4. Inlined plotting & Delta Drawing.
 * 5. Sound Effects (SID).
 */

#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Screen Memory */
#define VIDRAM ((unsigned char *)0x0400)
#define COLRAM ((unsigned char *)0xD800)

/* SID Registers */
#define SID_BASE 0xD400
typedef struct {
  unsigned int freq1;
  unsigned int pw1;
  unsigned char ctrl1;
  unsigned char ad1;
  unsigned char sr1;
  unsigned int freq2;
  unsigned int pw2;
  unsigned char ctrl2;
  unsigned char ad2;
  unsigned char sr2;
  unsigned int freq3;
  unsigned int pw3;
  unsigned char ctrl3;
  unsigned char ad3;
  unsigned char sr3;
  unsigned char filt_l;
  unsigned char filt_h;
  unsigned char filt_v;
  unsigned char volume;
} SIDPTR;

#define SID_HW ((volatile SIDPTR *)SID_BASE)

/* Dimensions */
#define SCREEN_W 40
#define SCREEN_H 25

/* Scale 256 */
#define SCALE 8

/* Max screen height in scaled units */
/* (25 * 256 = 6400) */
#define MAX_Y_SCALED (SCREEN_H << SCALE)

/* Physics Constants (Scaled by 256) */
#define GRAVITY 38
#define P_SPEED_MIN 30
#define P_SPEED_MAX 120
/* -0.55 chars/frame */
#define ROCKET_VY -140

/* Launch configuration */
#define LAUNCH_X_MIN (5 << SCALE)
#define LAUNCH_X_RANGE (30 << SCALE)
#define TARGET_Y_MIN (5 << SCALE)
#define TARGET_Y_RANGE (10 << SCALE)

#define LIFE_MAX 30
#define MAX_FIREWORKS 3
#define MAX_PARTICLES 48

/* Colors */
const unsigned char PALETTE[] = {2, 5, 6, 7, 4, 3, 8, 14};

/* Helper Tables */
unsigned int row_offsets[25];

/* SoA for Particles */
unsigned char p_active[MAX_PARTICLES];
int p_x[MAX_PARTICLES];
int p_y[MAX_PARTICLES];
int p_vx[MAX_PARTICLES];
int p_vy[MAX_PARTICLES];
unsigned char p_color[MAX_PARTICLES];
signed char p_life[MAX_PARTICLES];

/* SoA for Fireworks (Rockets) */
/* Breaking the struct to Arrays eliminates 13x multiplication overhead per
 * access */
unsigned char f_active[MAX_FIREWORKS];
int f_x[MAX_FIREWORKS];
int f_y[MAX_FIREWORKS];
int f_vx[MAX_FIREWORKS];
int f_vy[MAX_FIREWORKS];
int f_target_y[MAX_FIREWORKS];
unsigned char f_color[MAX_FIREWORKS];
unsigned char f_exploded[MAX_FIREWORKS];

/* Simple Fast PRNG State */
unsigned char seed = 123;

/* Sound System */
void init_sound() {
  SID_HW->volume = 15;
  SID_HW->ad1 = 0x09;
  SID_HW->sr1 = 0x00;
  SID_HW->ad3 = 0x09;
  SID_HW->sr3 = 0x00;
}

/* Fast 8-bit PRNG */
unsigned char fast_rand() {
  seed ^= seed << 2;
  seed ^= seed >> 5;
  seed ^= seed << 3;
  return seed;
}

/* Helper for 16-bit random using fast 8-bit parts */
unsigned int fast_rand16() {
  return (unsigned int)fast_rand() | ((unsigned int)fast_rand() << 8);
}

void sfx_launch() {
  SID_HW->ctrl1 = 0;
  SID_HW->freq1 = 1000 + (fast_rand() << 2);
  SID_HW->ad1 = 0x59;
  SID_HW->sr1 = 0x00;
  SID_HW->ctrl1 = 17;
}

void sfx_explode() {
  SID_HW->ctrl3 = 0;
  SID_HW->freq3 = 4000;
  SID_HW->ad3 = 0x08;
  SID_HW->sr3 = 0x05;
  SID_HW->ctrl3 = 129;
}

void init_tables() {
  int i;
  for (i = 0; i < SCREEN_H; ++i) {
    row_offsets[i] = i * SCREEN_W;
  }
}

void spawn_explosion(int x, int y, unsigned char color) {
  register unsigned char i;
  unsigned char count = 0;
  unsigned char p_count = 10 + (fast_rand() & 7);

  sfx_explode();

  for (i = 0; i < MAX_PARTICLES; ++i) {
    if (!p_active[i]) {
      int speed = P_SPEED_MIN + (fast_rand() % (P_SPEED_MAX - P_SPEED_MIN));
      p_active[i] = 1;
      p_x[i] = x;
      p_y[i] = y;
      p_color[i] = color;
      p_life[i] = LIFE_MAX;

      p_vx[i] = (fast_rand() % (speed * 2)) - speed;
      p_vy[i] = (fast_rand() % (speed * 2)) - speed;
      count++;
      if (count >= p_count)
        break;
    }
  }
}

void update_simulation() {
  register unsigned char i;
  register unsigned char sx, sy;
  register unsigned char old_sx, old_sy;
  unsigned int off;
  unsigned char ch;

  /* FIREWORKS (SoA Optimized) */
  for (i = 0; i < MAX_FIREWORKS; ++i) {
    if (f_active[i] && !f_exploded[i]) {
      old_sx = (unsigned char)(f_x[i] >> 8);
      old_sy = (unsigned char)(f_y[i] >> 8);

      f_y[i] += f_vy[i];

      if (f_y[i] <= f_target_y[i]) {
        /* Explode */
        /* Erase old */
        if (old_sy < 24 && old_sx < SCREEN_W) {
          off = row_offsets[old_sy] + old_sx;
          VIDRAM[off] = ' ';
        }
        f_exploded[i] = 1;
        spawn_explosion(f_x[i], f_y[i], f_color[i]);
        f_active[i] = 0;
      } else {
        /* Update */
        sx = (unsigned char)(f_x[i] >> 8);
        sy = (unsigned char)(f_y[i] >> 8);

        /* Delta Erase/Draw */
        if (sx != old_sx || sy != old_sy) {
          if (old_sy < 24 && old_sx < SCREEN_W) {
            VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
          }
          if (sy < 24 && sx < SCREEN_W) {
            off = row_offsets[sy] + sx;
            VIDRAM[off] = '^';
            COLRAM[off] = 1; /* White */
          }
        }
      }
    }
  }

  /* PARTICLES (SoA) */
  for (i = 0; i < MAX_PARTICLES; ++i) {
    if (p_active[i]) {
      old_sx = (unsigned char)(p_x[i] >> 8);
      old_sy = (unsigned char)(p_y[i] >> 8);

      p_vy[i] += GRAVITY;
      p_x[i] += p_vx[i];
      p_y[i] += p_vy[i];
      p_vx[i] -= (p_vx[i] >> 4);
      p_life[i]--;

      if (p_y[i] > MAX_Y_SCALED)
        p_life[i] = 0;

      if (p_life[i] <= 0) {
        p_active[i] = 0;
        /* Erase last position */
        if (old_sy < 24 && old_sx < SCREEN_W) {
          VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
        }
      } else {
        sx = (unsigned char)(p_x[i] >> 8);
        sy = (unsigned char)(p_y[i] >> 8);
        ch = (p_life[i] < 10) ? '.' : '*';

        /* Delta Draw */
        if (sy < 24 && sx < SCREEN_W) {
          off = row_offsets[sy] + sx;

          if (sx != old_sx || sy != old_sy) {
            /* Erase Old */
            if (old_sy < 24 && old_sx < SCREEN_W) {
              VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
            }
            /* Draw New */
            VIDRAM[off] = ch;
            COLRAM[off] = p_color[i];
          } else {
            /* Refresh char only if needed */
            if (VIDRAM[off] != ch) {
              VIDRAM[off] = ch;
            }
          }
        } else if (old_sy < 24 && old_sx < SCREEN_W) {
          /* Moved off screen, erase */
          VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
        }
      }
    }
  }
}

void launch_firework() {
  register unsigned char i;
  for (i = 0; i < MAX_FIREWORKS; ++i) {
    if (!f_active[i]) {
      f_active[i] = 1;
      /* Launch X: LAUNCH_X_MIN + random */
      f_x[i] = LAUNCH_X_MIN + (fast_rand16() % LAUNCH_X_RANGE);
      f_y[i] = MAX_Y_SCALED - 256;

      /* Target Y: */
      f_target_y[i] = TARGET_Y_MIN + (fast_rand16() % TARGET_Y_RANGE);

      f_vx[i] = 0;
      f_vy[i] = ROCKET_VY;
      f_color[i] = PALETTE[fast_rand() & 7];
      f_exploded[i] = 0;
      sfx_launch();
      break;
    }
  }
}

int main() {
  clrscr();
  bgcolor(0);
  bordercolor(0);
  init_tables();
  init_sound();

  memset(f_active, 0, sizeof(f_active));
  memset(p_active, 0, sizeof(p_active));

  gotoxy(0, 24);
  textcolor(15);
  cprintf("SPACE:Launch Q:Quit");

  while (1) {
    if (kbhit()) {
      char c = cgetc();
      if (c == ' ')
        launch_firework();
      if (c == 'q')
        break;
    }
    update_simulation();
  }

  SID_HW->volume = 0;
  SID_HW->ctrl1 = 0;
  SID_HW->ctrl3 = 0;

  clrscr();
  return 0;
}
