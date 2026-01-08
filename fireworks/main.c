/*
 * Fireworks Simulation for Commodore 64 - Zero-Division Optimization
 *
 * Optimizations:
 * 1. 16-bit math with Scale 8 (1.0 = 256).
 *    - Allows ">> 8" (high byte) for coordinate mapping.
 *    - Eliminates ALL division and complex shifting in the inner loops.
 * 2. Structure of Arrays (SoA).
 * 3. Direct screen/SID access.
 * 4. Delta clearing.
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

/*
 * CRITICAL OPTIMIZATION:
 * Coordinate System is now Chars * 256.
 * X: 0 .. 40*256 (10240)
 * Y: 0 .. 25*256 (6400)
 * Screen X = val >> 8
 * Screen Y = val >> 8
 */
#define SCALE 8
/* Max screen height in scaled units */
#define MAX_Y_SCALED (25 << 8)

/* Physics Constants (Scaled by 256) */
/* Gravity 0.15 * 256 = 38 */
#define GRAVITY 38

/* Speed in chars/frame approx */
/* Min speed 0.1 chars/frame -> 25 */
/* Max speed 0.4 chars/frame -> 100 */
#define P_SPEED_MIN 30
#define P_SPEED_MAX 120

/* Rocket VY: -0.6 chars/frame? */
/* -12.0 python pixels. 24 pixels/char -> 0.5 chars/frame */
/* 0.5 * 256 = 128 */
#define ROCKET_VY -140

#define LIFE_MAX 30
#define MAX_FIREWORKS 3
/* We can probably handle more particles now */
#define MAX_PARTICLES 64

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
  SID_HW->freq1 = 1000;
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

/* Fast plot without boundary checks for speed in known ranges?
   Better safe than sorry, but maybe we can optimize.
   Since we use >> 8, values 0..10240 map to 0..40.
   Negatives map to ... big unsigned.
*/
void fast_plot(unsigned char sx, unsigned char sy, unsigned char ch,
               unsigned char color) {
  /* Manual boundary check using unsigned char cast tricks (wraps negative to
   * large positive). Limit Y to 24 to protect footer at row 24. */
  if (sx < SCREEN_W && sy < 24) {
    unsigned int off = row_offsets[sy] + sx;
    VIDRAM[off] = ch;
    COLRAM[off] = color;
  }
}

void spawn_explosion(int x, int y, unsigned char color) {
  unsigned char i;
  unsigned char count = 0;
  /* Increase particle count slightly */
  unsigned char p_count = 15 + (rand() & 15);

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
  unsigned char i;
  unsigned char sx, sy; /* Bytes are enough for screen coords 0..40 */

  /* FIREWORKS */
  for (i = 0; i < MAX_FIREWORKS; ++i) {
    if (fireworks[i].active && !fireworks[i].exploded) {
      /* High byte is screen coordinate ( >> 8 ) */
      sx = (unsigned char)(fireworks[i].x >> 8);
      sy = (unsigned char)(fireworks[i].y >> 8);
      fast_plot(sx, sy, ' ', 0);

      fireworks[i].y += fireworks[i].vy;

      if (fireworks[i].y <= fireworks[i].target_y) {
        fireworks[i].exploded = 1;
        spawn_explosion(fireworks[i].x, fireworks[i].y, fireworks[i].color);
        fireworks[i].active = 0;
      } else {
        sx = (unsigned char)(fireworks[i].x >> 8);
        sy = (unsigned char)(fireworks[i].y >> 8);
        fast_plot(sx, sy, '^', 1);
      }
    }
  }

  /* PARTICLES */
  for (i = 0; i < MAX_PARTICLES; ++i) {
    /* Optimization: cache active check to avoid array lookup? No, byte lookup
     * is fast */
    if (p_active[i]) {
      sx = (unsigned char)(p_x[i] >> 8);
      sy = (unsigned char)(p_y[i] >> 8);

      fast_plot(sx, sy, ' ', 0);

      p_vy[i] += GRAVITY;
      p_x[i] += p_vx[i];
      p_y[i] += p_vy[i];

      /* Drag: vx -= vx/16 */
      p_vx[i] -= (p_vx[i] >> 4);

      p_life[i]--;

      /* Bounds check: > 6400 (visible height) */
      if (p_y[i] > MAX_Y_SCALED) {
        p_life[i] = 0;
      }

      if (p_life[i] <= 0) {
        p_active[i] = 0;
      } else {
        unsigned char ch = (p_life[i] < 10) ? '.' : '*';
        /* Recalculate screen coords */
        sx = (unsigned char)(p_x[i] >> 8);
        sy = (unsigned char)(p_y[i] >> 8);

        fast_plot(sx, sy, ch, p_color[i]);
      }
    }
  }
}

void launch_firework() {
  unsigned char i;
  for (i = 0; i < MAX_FIREWORKS; ++i) {
    if (!fireworks[i].active) {
      fireworks[i].active = 1;
      /* Random X: 5 to 35 chars -> 1280 to 8960 */
      fireworks[i].x = (1280 + rand() % 7680);
      fireworks[i].y = MAX_Y_SCALED - 256; /* Bottom */

      /* Target: 5 to 15 chars from top -> 1280 to 3840 */
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

  /* Silence SID_HW */
  SID_HW->volume = 0;
  SID_HW->ctrl1 = 0;
  SID_HW->ctrl3 = 0;

  clrscr();
  return 0;
}
