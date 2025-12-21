// C64 Hello World Program using cc65's conio.h
// Demonstrates basic screen operations and text output
// @ulasb, 2025/12/20

#include <conio.h>
#include <stdio.h>

// Screen position constants for better readability
#define CENTER_COL      11
#define CENTER_ROW      5
#define BOTTOM_COL      0
#define BOTTOM_ROW      24

// C64 color codes
#define C64_COLOR_BLACK        0
#define C64_COLOR_WHITE        1
#define C64_COLOR_RED          2
#define C64_COLOR_CYAN         3
#define C64_COLOR_PURPLE       4
#define C64_COLOR_GREEN        5
#define C64_COLOR_BLUE         6
#define C64_COLOR_YELLOW       7
#define C64_COLOR_ORANGE       8
#define C64_COLOR_BROWN        9
#define C64_COLOR_LIGHTRED     10
#define C64_COLOR_DARKGRAY     11
#define C64_COLOR_GRAY         12
#define C64_COLOR_LIGHTGREEN   13
#define C64_COLOR_LIGHTBLUE    14
#define C64_COLOR_LIGHTGRAY    15

int main(void) {
    // Clear the screen and set up initial display
    clrscr();

    // Set text color to cyan for the main message
    textcolor(C64_COLOR_CYAN);

    // Display the main greeting message in the center of the screen
    gotoxy(CENTER_COL, CENTER_ROW);
    cprintf("Hello, C64 World!");

    // Change color and display secondary message at bottom
    textcolor(C64_COLOR_GREEN);
    gotoxy(BOTTOM_COL, BOTTOM_ROW);
    cprintf("Press any key to exit...");

    // Wait for user input before exiting
    cgetc();

    return 0;
}
