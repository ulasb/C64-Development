/*
 * Fireworks Simulation for Commodore 64 - Final Optimization
 *
 * Optimizations:
 * 1. Zero-division math (Scale 256).
 * 2. Inlined plotting (No function calls in loop).
 * 3. Delta-Drawing (Only erase if moved).
 * 4. Reduced particle count for safety.
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
#define MAX_Y_SCALED (25 << 8)

/* Physics Constants */
#define GRAVITY 38
#define P_SPEED_MIN 30
#define P_SPEED_MAX 120
#define ROCKET_VY -140

#define LIFE_MAX 30
#define MAX_FIREWORKS 3
/* Reduced to 48 to maintain framerate with multiple explosions */
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

typedef struct {
  unsigned char active;
  int x, y;
  int vx, vy;
  int target_y;
  unsigned char color;
  unsigned char exploded;
} Firework;

Firework fireworks[MAX_FIREWORKS];

/* Sound System */
void init_sound() {
  SID_HW->volume = 15;
  SID_HW->ad1 = 0x09;
  SID_HW->sr1 = 0x00;
  SID_HW->ad3 = 0x09;
  SID_HW->sr3 = 0x00;
}

void sfx_launch() {
  SID_HW->ctrl1 = 0;
  SID_HW->freq1 = 1000 + (rand() % 500); /* Varied pitch */
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
  unsigned char i;
  unsigned char count = 0;
  unsigned char p_count = 10 + (rand() & 7); /* 10-17 particles (lighter) */

  sfx_explode();

  for (i = 0; i < MAX_PARTICLES; ++i) {
    if (!p_active[i]) {
      int speed = P_SPEED_MIN + (rand() % (P_SPEED_MAX - P_SPEED_MIN));
      p_active[i] = 1;
      p_x[i] = x;
      p_y[i] = y;
      p_color[i] = color;
      p_life[i] = LIFE_MAX;
      p_vx[i] = (rand() % (speed * 2)) - speed;
      p_vy[i] = (rand() % (speed * 2)) - speed;
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

  /* FIREWORKS */
  for (i = 0; i < MAX_FIREWORKS; ++i) {
    if (fireworks[i].active && !fireworks[i].exploded) {
      old_sx = (unsigned char)(fireworks[i].x >> 8);
      old_sy = (unsigned char)(fireworks[i].y >> 8);

      fireworks[i].y += fireworks[i].vy;

      if (fireworks[i].y <= fireworks[i].target_y) {
        /* Erase old */
        if (old_sy < 24) {
          off = row_offsets[old_sy] + old_sx;
          VIDRAM[off] = ' ';
        }
        fireworks[i].exploded = 1;
        spawn_explosion(fireworks[i].x, fireworks[i].y, fireworks[i].color);
        fireworks[i].active = 0;
      } else {
        sx = (unsigned char)(fireworks[i].x >> 8);
        sy = (unsigned char)(fireworks[i].y >> 8);

        /* Delta Erase/Draw */
        if (sx != old_sx || sy != old_sy) {
          if (old_sy < 24)
            VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
          if (sy < 24) {
            off = row_offsets[sy] + sx;
            VIDRAM[off] = '^';
            COLRAM[off] = 1; /* White */
          }
        }
      }
    }
  }

  /* PARTICLES */
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
        if (old_sy < 24 && old_sx < 40) {
          VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
        }
      } else {
        sx = (unsigned char)(p_x[i] >> 8);
        sy = (unsigned char)(p_y[i] >> 8);
        ch = (p_life[i] < 10) ? '.' : '*';

        /* Delta Draw */
        if (sy < 24 && sx < 40) {
          off = row_offsets[sy]; /* Optimization: fetch row once? compiler might
                                  */
          off += sx;

          if (sx != old_sx || sy != old_sy) {
            /* Erase Old */
            if (old_sy < 24 && old_sx < 40) {
              VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
            }
            /* Draw New */
            VIDRAM[off] = ch;
            COLRAM[off] = p_color[i];
          } else {
            /* Overwrite (refresh char) */
            VIDRAM[off] = ch;
            /* color is same, skip */
          }
        } else if (old_sy < 24 && old_sx < 40) {
          /* Moved off screen, erase old */
          VIDRAM[row_offsets[old_sy] + old_sx] = ' ';
        }
      }
    }
  }
}

void launch_firework() {
  unsigned char i;
  for (i = 0; i < MAX_FIREWORKS; ++i) {
    if (!fireworks[i].active) {
      fireworks[i].active = 1;
      fireworks[i].x = (1280 + rand() % 7680);
      fireworks[i].y = MAX_Y_SCALED - 256;
      fireworks[i].target_y = (1280 + rand() % 2560);
      fireworks[i].vx = 0;
      fireworks[i].vy = ROCKET_VY;
      fireworks[i].color = PALETTE[rand() & 7];
      fireworks[i].exploded = 0;
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

  memset(fireworks, 0, sizeof(fireworks));
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
