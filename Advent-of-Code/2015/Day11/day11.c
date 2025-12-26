#include <conio.h>
#include <stdio.h>
#include <string.h>

// Global password buffer (8 chars + null terminator)
char pwd[9];

// Forbidden characters
const char forbidden[] = "iol";

// Function: increment_pwd
// Increments the password string like a base-26 number (a-z).
void increment_pwd(char *p) {
  int i;
  int len = 8;

  // Start from the rightmost character
  for (i = len - 1; i >= 0; i--) {
    if (p[i] < 'z') {
      p[i]++;
      // Increment found non-z char.
      // Characters to the right (which were 'z') have already been reset to 'a'
      // in previous iterations of this loop (in the else block).
      break;
    } else {
      p[i] = 'a';
    }
  }
}

// Function: fast_skip
// Checks if password contains i, o, l. If so, increments that char,
// resets subsequent chars to 'a', and returns 1 (true) to indicate a skip
// happened. Returns 0 if no forbidden chars found.
int fast_skip(char *p) {
  int i, j;
  for (i = 0; i < 8; i++) {
    if (p[i] == 'i' || p[i] == 'o' || p[i] == 'l') {
      p[i]++; // Skip the forbidden char to the next one
      // Reset all rest to 'a' to get the lowest possible valid suffix
      for (j = i + 1; j < 8; j++) {
        p[j] = 'a';
      }
      return 1;
    }
  }
  return 0;
}

// Function: is_valid_pwd
// Checks the 3 rules.
int is_valid_pwd(const char *p) {
  int i;
  int has_straight = 0;
  int pair_count = 0;
  char first_pair_char = 0;

  // Rule 2: No i, o, l
  // Handled by valid_skip optimization in main loop, so we skip this check here
  // for speed. If you run this function on arbitrary input, ensure it has no
  // i/o/l first.

  // Rule 1: Increasing straight of 3 (abc)
  for (i = 0; i < 6; i++) {
    // Check p[i], p[i+1], p[i+2]
    if (p[i + 1] == p[i] + 1 && p[i + 2] == p[i] + 2) {
      has_straight = 1;
      break;
    }
  }
  if (!has_straight)
    return 0;

  // Rule 3: Two different non-overlapping pairs
  for (i = 0; i < 7; i++) {
    if (p[i] == p[i + 1]) {
      if (pair_count == 0) {
        pair_count++;
        first_pair_char = p[i];
        i++; // Skip next to ensure non-overlapping
      } else {
        // Check if it's a different pair (e.g. 'aa' then 'bb')
        if (p[i] != first_pair_char) {
          pair_count++;
          break;
        }
      }
    }
  }

  if (pair_count < 2)
    return 0;

  return 1;
}

void solve(const char *label) {
  unsigned long attempts = 0;

  // Ensure we start clean (though input strings usually safe or handled
  // quickly)
  fast_skip(pwd);

  cprintf("%s: Searching...\r\n", label);

  // Loop until valid
  while (1) {
    attempts++;

    // Progress display every 2048 iterations to minimize I/O overhead
    if ((attempts & 2047) == 0) {
      cprintf("\r%s (%lu)", pwd, attempts);
    }

    increment_pwd(pwd);

    // Optimization: Check forbidden chars causing big skips
    if (fast_skip(pwd)) {
      // Skip happened. pwd is now clean of i/o/l (up to the skip point, and
      // reset rest). The resulting string ends in 'aaaa...', which usually
      // fails the "straight" rule (needs distinct chars). So we can skip the
      // expensive is_valid_pwd check and continue immediately.
      continue;
    }

    // Check remaining rules (Straight, Pairs)
    if (is_valid_pwd(pwd)) {
      break;
    }
  }
  cprintf("\rFOUND: %s (after %lu)   \r\n", pwd, attempts);
}

int main() {
  // Set up screen
  bgcolor(COLOR_BLACK);
  bordercolor(COLOR_BLACK);
  textcolor(COLOR_GRAY3); // readable on black
  clrscr();

  // Initial password
  strcpy(pwd, "cqjxjnds");

  cprintf("Advent of Code 2015 - Day 11\r\n");
  cprintf("Input: %s\r\n\r\n", pwd);

  solve("Part 1"); // Find next

  // For Part 2, find next after Part 1
  solve("Part 2");

  // Wait for key
  cprintf("\r\nDone. Press any key.\r\n");
  while (!kbhit())
    ;
  cgetc();

  return 0;
}
