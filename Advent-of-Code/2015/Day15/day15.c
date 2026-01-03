/*
 * Advent of Code 2015 - Day 15 (C64 Edition)
 * Science for Hungry People - Cookie Recipe Optimizer
 * 
 * This program finds the optimal cookie recipe that maximizes score.
 * Part 1: Find the highest score possible with any recipe (100 teaspoons total).
 * Part 2: Find the highest score with exactly 500 calories.
 */

#include <stdio.h>
#include <string.h>

#define MAX_INGREDIENTS 4       // Maximum number of ingredients
#define TOTAL_TEASPOONS 100     // Total teaspoons to use
#define CALORIE_TARGET 500      // Target calories for Part 2

/* Ingredient structure */
typedef struct {
    char name[16];
    int capacity;
    int durability;
    int flavor;
    int texture;
    int calories;
} Ingredient;

/* Global data */
static Ingredient ingredients[MAX_INGREDIENTS];
static int ingredient_count = 0;
static long best_score_part1 = 0;
static long best_score_part2 = 0;

/* Function prototypes */
void add_ingredient(const char* name, int capacity, int durability, int flavor, int texture, int calories);
long calculate_score(int amounts[], int check_calories);
void find_optimal_recipe(void);
void run_tests(void);

/*
 * Add an ingredient to the list
 */
void add_ingredient(const char* name, int capacity, int durability, int flavor, int texture, int calories) {
    if (ingredient_count >= MAX_INGREDIENTS) {
        printf("ERROR: TOO MANY INGREDIENTS\n");
        return;
    }
    
    strcpy(ingredients[ingredient_count].name, name);
    ingredients[ingredient_count].capacity = capacity;
    ingredients[ingredient_count].durability = durability;
    ingredients[ingredient_count].flavor = flavor;
    ingredients[ingredient_count].texture = texture;
    ingredients[ingredient_count].calories = calories;
    ingredient_count++;
}

/*
 * Calculate score for a given recipe
 * Returns the score if valid, or 0 if invalid
 * 
 * If check_calories is true, only returns score if calories == CALORIE_TARGET
 */
long calculate_score(int amounts[], int check_calories) {
    long total_capacity = 0;
    long total_durability = 0;
    long total_flavor = 0;
    long total_texture = 0;
    long total_calories = 0;
    int i;
    
    /* Calculate totals for each property */
    for (i = 0; i < ingredient_count; i++) {
        total_capacity += (long)ingredients[i].capacity * amounts[i];
        total_durability += (long)ingredients[i].durability * amounts[i];
        total_flavor += (long)ingredients[i].flavor * amounts[i];
        total_texture += (long)ingredients[i].texture * amounts[i];
        total_calories += (long)ingredients[i].calories * amounts[i];
    }
    
    /* Check calorie constraint for Part 2 */
    if (check_calories && total_calories != CALORIE_TARGET) {
        return 0;
    }
    
    /* Set negative values to 0 */
    if (total_capacity < 0) total_capacity = 0;
    if (total_durability < 0) total_durability = 0;
    if (total_flavor < 0) total_flavor = 0;
    if (total_texture < 0) total_texture = 0;
    
    /* Calculate final score (multiply all four properties) */
    return total_capacity * total_durability * total_flavor * total_texture;
}

/*
 * Recursive function to try all combinations of ingredient amounts
 * 
 * current_index: Which ingredient we're currently setting
 * remaining: How many teaspoons are left to distribute
 * amounts: Array of current amounts for each ingredient
 */
void try_combinations(int current_index, int remaining, int amounts[]) {
    long score;
    int amount;
    
    /* Base case: Last ingredient gets all remaining teaspoons */
    if (current_index == ingredient_count - 1) {
        amounts[current_index] = remaining;
        
        /* Calculate score for Part 1 (no calorie constraint) */
        score = calculate_score(amounts, 0);
        if (score > best_score_part1) {
            best_score_part1 = score;
        }
        
        /* Calculate score for Part 2 (with calorie constraint) */
        score = calculate_score(amounts, 1);
        if (score > best_score_part2) {
            best_score_part2 = score;
        }
        
        return;
    }
    
    /* Try different amounts for current ingredient */
    for (amount = 0; amount <= remaining; amount++) {
        amounts[current_index] = amount;
        try_combinations(current_index + 1, remaining - amount, amounts);
    }
}

/*
 * Find the optimal recipe by trying all possible combinations
 */
void find_optimal_recipe(void) {
    int amounts[MAX_INGREDIENTS];
    int i;
    
    /* Initialize amounts array */
    for (i = 0; i < MAX_INGREDIENTS; i++) {
        amounts[i] = 0;
    }
    
    /* Reset best scores */
    best_score_part1 = 0;
    best_score_part2 = 0;
    
    printf("EVALUATING %d INGREDIENTS...\n", ingredient_count);
    
    /* Start recursive search */
    try_combinations(0, TOTAL_TEASPOONS, amounts);
}

/*
 * Run test cases to validate the implementation
 */
void run_tests(void) {
    int amounts[MAX_INGREDIENTS];
    long score;
    long calories;
    
    printf("\n=== RUNNING TESTS ===\n\n");
    
    /* Clear any existing ingredients */
    ingredient_count = 0;
    
    /* Add test ingredients */
    add_ingredient("Butterscotch", -1, -2, 6, 3, 8);
    add_ingredient("Cinnamon", 2, 3, -2, -1, 3);
    
    printf("TEST INGREDIENTS:\n");
    printf("  BUTTERSCOTCH: CAP -1, DUR -2, FLA 6, TEX 3, CAL 8\n");
    printf("  CINNAMON: CAP 2, DUR 3, FLA -2, TEX -1, CAL 3\n\n");
    
    /* Test Part 1: 44 Butterscotch, 56 Cinnamon */
    printf("PART 1 TEST:\n");
    printf("  44 BUTTERSCOTCH + 56 CINNAMON\n");
    amounts[0] = 44;
    amounts[1] = 56;
    score = calculate_score(amounts, 0);
    printf("  EXPECTED SCORE: 62842880\n");
    printf("  ACTUAL SCORE:   %ld", score);
    if (score == 62842880L) {
        printf(" (PASS)\n");
    } else {
        printf(" (FAIL)\n");
    }
    
    /* Calculate individual properties for verification */
    {
        long cap = 44 * -1 + 56 * 2;
        long dur = 44 * -2 + 56 * 3;
        long fla = 44 * 6 + 56 * -2;
        long tex = 44 * 3 + 56 * -1;
        
        printf("  DETAILS: CAP=%ld, DUR=%ld, FLA=%ld, TEX=%ld\n", cap, dur, fla, tex);
    }
    
    /* Test Part 2: 40 Butterscotch, 60 Cinnamon */
    printf("\nPART 2 TEST:\n");
    printf("  40 BUTTERSCOTCH + 60 CINNAMON\n");
    amounts[0] = 40;
    amounts[1] = 60;
    
    /* Calculate calories */
    calories = 40L * 8 + 60L * 3;
    printf("  CALORIES: %ld (TARGET: %d)\n", calories, CALORIE_TARGET);
    
    score = calculate_score(amounts, 0);  /* Don't check calories for this test */
    printf("  EXPECTED SCORE: 57600000\n");
    printf("  ACTUAL SCORE:   %ld", score);
    if (score == 57600000L) {
        printf(" (PASS)\n");
    } else {
        printf(" (FAIL)\n");
    }
    
    /* Calculate individual properties for verification */
    {
        long cap = 40 * -1 + 60 * 2;
        long dur = 40 * -2 + 60 * 3;
        long fla = 40 * 6 + 60 * -2;
        long tex = 40 * 3 + 60 * -1;
        
        printf("  DETAILS: CAP=%ld, DUR=%ld, FLA=%ld, TEX=%ld\n", cap, dur, fla, tex);
    }
    
    /* Now find optimal recipe */
    printf("\n=== FINDING OPTIMAL RECIPE ===\n\n");
    find_optimal_recipe();
    
    printf("PART 1 - BEST SCORE: %ld\n", best_score_part1);
    printf("  EXPECTED: 62842880");
    if (best_score_part1 == 62842880L) {
        printf(" (PASS)\n");
    } else {
        printf(" (FAIL)\n");
    }
    
    printf("\nPART 2 - BEST SCORE (500 CAL): %ld\n", best_score_part2);
    printf("  EXPECTED: 57600000");
    if (best_score_part2 == 57600000L) {
        printf(" (PASS)\n");
    } else {
        printf(" (FAIL)\n");
    }
}

/*
 * Main program
 */
int main(void) {
    printf("\n");
    printf("ADVENT OF CODE 2015 - DAY 15\n");
    printf("COOKIE RECIPE OPTIMIZER\n");
    printf("C64 EDITION\n");
    
    run_tests();
    
    printf("\n=== ALL TESTS COMPLETE ===\n");
    
    return 0;
}
