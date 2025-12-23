#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants for demonstration grid (50x50 fits in C64 memory ~2.5KB)
#define DEMO_ROWS 50
#define DEMO_COLS 50

// Global grids to avoid cc65 local variable limits
char part1_grid[DEMO_ROWS][DEMO_COLS];
unsigned char part2_grid[DEMO_ROWS][DEMO_COLS];

// Test instructions for Part 1 (scaled for 50x50 demo grid)
#define PART1_TEST_COUNT 3
const char* part1_instructions[PART1_TEST_COUNT] = {
    "turn on 0,0 through 49,49",     // Turn on all lights in 50x50 grid
    "toggle 0,0 through 49,0",       // Toggle first row (50 lights)
    "turn off 24,24 through 25,25"   // Turn off 2x2 square in middle (4 lights)
};

const unsigned long part1_expected[PART1_TEST_COUNT] = {
    2500,  // All 50x50 grid on: 50 * 50 = 2,500
    2450,  // First row toggled off: 2,500 - 50 = 2,450
    2446   // Middle 4 turned off: 2,450 - 4 = 2,446
};

// Test instructions for Part 2 (scaled for 50x50 demo grid)
#define PART2_TEST_COUNT 2
const char* part2_instructions[PART2_TEST_COUNT] = {
    "turn on 0,0 through 0,0",     // Single light +1 brightness
    "toggle 0,0 through 49,49"     // All lights toggle +2 each
};

const unsigned long part2_expected[PART2_TEST_COUNT] = {
    1,      // Single light brightness +1
    5000    // All 50x50 lights toggle: 2,500 * 2 = 5,000
};

// Structure to represent a light command
typedef struct {
    char command[10];  // "turn on", "turn off", or "toggle"
    int start_x, start_y;
    int end_x, end_y;
} LightCommand;

// Simple atoi for C64 (no stdlib dependency)
int atoi(const char* str) {
    int result = 0;
    int i = 0;
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return result;
}

// Parse a single instruction line (C64-compatible)
void parse_instruction(const char* line, LightCommand* cmd) {
    int i = 0;
    int pos = 0;
    char num_str[10];
    int num_pos;

    // Parse command
    if (line[0] == 't' && line[1] == 'u' && line[2] == 'r' && line[3] == 'n') {
        // "turn on/off"
        pos = 5; // skip "turn "
        if (line[pos] == 'o' && line[pos+1] == 'n') {
            strcpy(cmd->command, "turn on");
            pos += 3; // skip "on "
        } else {
            strcpy(cmd->command, "turn off");
            pos += 4; // skip "off "
        }
    } else {
        // "toggle"
        strcpy(cmd->command, "toggle");
        pos = 7; // skip "toggle "
    }

    // Parse start_x
    num_pos = 0;
    while (line[pos] >= '0' && line[pos] <= '9') {
        num_str[num_pos++] = line[pos++];
    }
    num_str[num_pos] = '\0';
    cmd->start_x = atoi(num_str);
    pos++; // skip comma

    // Parse start_y
    num_pos = 0;
    while (line[pos] >= '0' && line[pos] <= '9') {
        num_str[num_pos++] = line[pos++];
    }
    num_str[num_pos] = '\0';
    cmd->start_y = atoi(num_str);

    // Skip " through "
    while (line[pos] != ' ' && pos < 50) pos++;
    pos += 9; // skip " through "

    // Parse end_x
    num_pos = 0;
    while (line[pos] >= '0' && line[pos] <= '9') {
        num_str[num_pos++] = line[pos++];
    }
    num_str[num_pos] = '\0';
    cmd->end_x = atoi(num_str);
    pos++; // skip comma

    // Parse end_y
    num_pos = 0;
    while (line[pos] >= '0' && line[pos] <= '9') {
        num_str[num_pos++] = line[pos++];
    }
    num_str[num_pos] = '\0';
    cmd->end_y = atoi(num_str);
}

// Count lights on in a grid (general solution)
unsigned long count_lights_on(const char grid[DEMO_ROWS][DEMO_COLS]) {
    unsigned long count = 0;
    int x, y;
    for (x = 0; x < DEMO_ROWS; x++) {
        for (y = 0; y < DEMO_COLS; y++) {
            if (grid[x][y]) {
                count++;
            }
        }
    }
    return count;
}

// Process a single command on a grid (general algorithm)
void process_command_part1(char grid[DEMO_ROWS][DEMO_COLS], const LightCommand* cmd) {
    int x, y;
    int start_x = cmd->start_x;
    int start_y = cmd->start_y;
    int end_x = cmd->end_x;
    int end_y = cmd->end_y;

    // Clamp coordinates to grid bounds for demo
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x >= DEMO_ROWS) end_x = DEMO_ROWS - 1;
    if (end_y >= DEMO_COLS) end_y = DEMO_COLS - 1;

    for (x = start_x; x <= end_x; x++) {
        for (y = start_y; y <= end_y; y++) {
            if (strcmp(cmd->command, "turn on") == 0) {
                grid[x][y] = 1;
            } else if (strcmp(cmd->command, "turn off") == 0) {
                grid[x][y] = 0;
            } else if (strcmp(cmd->command, "toggle") == 0) {
                grid[x][y] = !grid[x][y];
            }
        }
    }
}

// Simulate Part 1 with actual grid operations (scaled for C64)
unsigned long simulate_part1_full(const LightCommand* commands, int num_commands) {
    int x, y, i;
    unsigned long count;

    // Initialize grid to off
    for (x = 0; x < DEMO_ROWS; x++) {
        for (y = 0; y < DEMO_COLS; y++) {
            part1_grid[x][y] = 0;
        }
    }

    // Process each command and show intermediate results
    for (i = 0; i < num_commands; i++) {
        process_command_part1(part1_grid, &commands[i]);
        count = count_lights_on(part1_grid);
        cprintf("Step %d: %lu lights\r\n", i+1, count);
    }

    return count;
}

// Calculate total brightness in a grid
unsigned long calculate_total_brightness(const unsigned char grid[DEMO_ROWS][DEMO_COLS]) {
    unsigned long total = 0;
    int x, y;
    for (x = 0; x < DEMO_ROWS; x++) {
        for (y = 0; y < DEMO_COLS; y++) {
            total += grid[x][y];
        }
    }
    return total;
}

// Process a single command on brightness grid (Part 2)
void process_command_part2(unsigned char grid[DEMO_ROWS][DEMO_COLS], const LightCommand* cmd) {
    int x, y;
    int start_x = cmd->start_x;
    int start_y = cmd->start_y;
    int end_x = cmd->end_x;
    int end_y = cmd->end_y;

    // Clamp coordinates to grid bounds for demo
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    if (end_x >= DEMO_ROWS) end_x = DEMO_ROWS - 1;
    if (end_y >= DEMO_COLS) end_y = DEMO_COLS - 1;

    for (x = start_x; x <= end_x; x++) {
        for (y = start_y; y <= end_y; y++) {
            if (strcmp(cmd->command, "turn on") == 0) {
                grid[x][y] += 1;
            } else if (strcmp(cmd->command, "turn off") == 0) {
                if (grid[x][y] > 0) {
                    grid[x][y] -= 1;
                }
            } else if (strcmp(cmd->command, "toggle") == 0) {
                grid[x][y] += 2;
            }
        }
    }
}

// Simulate Part 2 with actual grid operations (scaled for C64)
unsigned long simulate_part2_full(const LightCommand* commands, int num_commands) {
    int x, y, i;
    unsigned long brightness;

    // Initialize brightness grid to 0
    for (x = 0; x < DEMO_ROWS; x++) {
        for (y = 0; y < DEMO_COLS; y++) {
            part2_grid[x][y] = 0;
        }
    }

    // Process each command and show intermediate results
    for (i = 0; i < num_commands; i++) {
        process_command_part2(part2_grid, &commands[i]);
        brightness = calculate_total_brightness(part2_grid);
        cprintf("Step %d: %lu brightness\r\n", i+1, brightness);
    }

    return brightness;
}

// Demonstration with small grid (works on real C64)
void demo_with_small_grid(void) {
    cprintf("DEMO: turn on 0,0-49,49 -> 2500 on\r\n");
    cprintf("DEMO: toggle 0,0-49,0 -> 2450 on\r\n");
    cprintf("DEMO: turn off 24,24-25,25 -> 2446 on\r\n");
}

void test_part1(void) {
    int i;
    LightCommand commands[PART1_TEST_COUNT];
    unsigned long result;

    cprintf("PART 1:\r\n");

    // Parse all instructions
    for (i = 0; i < PART1_TEST_COUNT; i++) {
        parse_instruction(part1_instructions[i], &commands[i]);
    }

    // Simulate on actual 50x50 grid
    result = simulate_part1_full(commands, PART1_TEST_COUNT);

    if (result == 2446) {
        cprintf("FINAL: PASS\r\n");
    } else {
        cprintf("FINAL: FAIL (got %lu)\r\n", result);
    }
}

void test_part2(void) {
    int i;
    LightCommand commands[PART2_TEST_COUNT];
    unsigned long result;

    cprintf("PART 2:\r\n");

    // Parse all instructions
    for (i = 0; i < PART2_TEST_COUNT; i++) {
        parse_instruction(part2_instructions[i], &commands[i]);
    }

    // Simulate on actual 50x50 brightness grid
    result = simulate_part2_full(commands, PART2_TEST_COUNT);

    if (result == 5001) {
        cprintf("FINAL: PASS\r\n");
    } else {
        cprintf("FINAL: FAIL (got %lu)\r\n", result);
    }
}

int main(void) {
    clrscr();

    cprintf("AOC 2015 Day 6 - 50x50 Grid\r\n");
    cprintf("==========================\r\n");

    // Show small grid demonstration
    demo_with_small_grid();
    cprintf("\r\n");

    // Run the actual tests
    test_part1();
    test_part2();

    cprintf("\r\nPress any key...\r\n");
    cgetc();

    return 0;
}
