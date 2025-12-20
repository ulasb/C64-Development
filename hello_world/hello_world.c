// C64 Hello World Program using cc65's conio.h
// Demonstrates basic screen operations and text output
// @ulasb, 2025/12/20

#include <conio.h>
#include <stdio.h>

// Screen position constants for better readability
#define CENTER_COL      10
#define CENTER_ROW      5
#define BOTTOM_COL      0
#define BOTTOM_ROW      24

// C64 color codes (0-15):
// 0=BLACK, 1=WHITE, 2=RED, 3=CYAN, 4=PURPLE, 5=GREEN, 6=BLUE, 7=YELLOW,
// 8=ORANGE, 9=BROWN, 10=LIGHTRED, 11=DARKGRAY, 12=GRAY, 13=LIGHTGREEN,
// 14=LIGHTBLUE, 15=LIGHTGRAY

int main(void) {
    // Clear the screen and set up initial display
    clrscr();

    // Set text color to cyan for the main message (color code 3)
    textcolor(3);

    // Display the main greeting message in the center of the screen
    gotoxy(CENTER_COL, CENTER_ROW);
    cprintf("Hello, C64 World!");

    // Change color and display secondary message at bottom (color code 5)
    textcolor(5);
    gotoxy(BOTTOM_COL, BOTTOM_ROW);
    cprintf("Press any key to exit...");

    // Wait for user input before exiting
    cgetc();

    return 0;
}