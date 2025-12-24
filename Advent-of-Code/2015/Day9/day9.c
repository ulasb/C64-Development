/*
 * Advent of Code 2015 - Day 9
 * Traveling Santa Problem
 * C64 Version for cc65
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

/* Constants */
#define MAX_CITIES 10
#define MAX_NAME_LEN 16

/* Types */
typedef struct {
    char name[MAX_NAME_LEN];
} City;

/* Globals */
City cities[MAX_CITIES];
int num_cities = 0;
unsigned int dist_matrix[MAX_CITIES][MAX_CITIES];
int visited[MAX_CITIES];

/* Results */
unsigned int min_route_dist;
unsigned int max_route_dist;

/* Function Prototypes */
void init_systems(void);
int get_city_index(const char *name);
void add_route(const char *from, const char *to, unsigned int dist);
void solve_tsp(int current_city, int count, unsigned int current_dist);
void solve_all(void);
void run_tests(void);

/* Main Entry Point */
int main(void) {
    init_systems();
    
    printf("\n--- Advent of Code 2015 Day 9 ---\n");
    printf("   --- Santa's Route Plan ---\n\n");
    
    run_tests();
    
    return 0;
}

/* Initialize globals */
void init_systems(void) {
    int i, j;
    num_cities = 0;
    for (i = 0; i < MAX_CITIES; i++) {
        for (j = 0; j < MAX_CITIES; j++) {
            dist_matrix[i][j] = 0;
        }
    }
}

/* Get index of a city, adding it if new */
int get_city_index(const char *name) {
    int i;
    for (i = 0; i < num_cities; i++) {
        if (strcmp(cities[i].name, name) == 0) {
            return i;
        }
    }
    
    /* Add new city */
    if (num_cities < MAX_CITIES) {
        strncpy(cities[num_cities].name, name, MAX_NAME_LEN - 1);
        cities[num_cities].name[MAX_NAME_LEN - 1] = '\0';
        return num_cities++;
    }
    
    printf("Error: Too many cities!\n");
    return -1;
}

/* Register a distance between two cities */
void add_route(const char *from, const char *to, unsigned int dist) {
    int c1 = get_city_index(from);
    int c2 = get_city_index(to);
    
    if (c1 >= 0 && c2 >= 0) {
        dist_matrix[c1][c2] = dist;
        dist_matrix[c2][c1] = dist;
    }
}

/* Recursive TSP Solver (DFS) */
/* Finds longest and shortest paths visiting all cities */
void solve_tsp(int current_city, int count, unsigned int current_dist) {
    int i;
    
    visited[current_city] = 1;
    
    /* Base case: All cities visited */
    if (count == num_cities) {
        if (current_dist < min_route_dist) min_route_dist = current_dist;
        if (current_dist > max_route_dist) max_route_dist = current_dist;
        visited[current_city] = 0; /* Backtrack */
        return;
    }
    
    /* Try visiting all unvisited neighbors */
    for (i = 0; i < num_cities; i++) {
        if (!visited[i] && dist_matrix[current_city][i] > 0) {
            solve_tsp(i, count + 1, current_dist + dist_matrix[current_city][i]);
        }
    }
    
    visited[current_city] = 0; /* Backtrack */
}

/* Wrapper to start TSP from every city */
void solve_all(void) {
    int i;
    
    min_route_dist = UINT_MAX;
    max_route_dist = 0;
    
    /* Reset visited array */
    for (i = 0; i < MAX_CITIES; i++) visited[i] = 0;
    
    /* Try starting path at every city */
    for (i = 0; i < num_cities; i++) {
        solve_tsp(i, 1, 0);
    }
}

/* Test runner with provided sample data */
void run_tests(void) {
    printf("Running Sample Tests...\n");
    
    /* 
     * Test Data:
     * London to Dublin = 464
     * London to Belfast = 518
     * Dublin to Belfast = 141
     */
     
    /* Clear old data */
    init_systems();
    
    /* Add routes */
    add_route("London", "Dublin", 464);
    add_route("London", "Belfast", 518);
    add_route("Dublin", "Belfast", 141);
    
    printf("Cities processed: %d\n", num_cities);
    printf("Resolving routes...\n");
    
    solve_all();
    
    printf("\nResults:\n");
    
    /* Part 1 Check */
    printf("Part 1 - Shortest Route: %u ", min_route_dist);
    if (min_route_dist == 605) {
        printf("(PASS)\n");
    } else {
        printf("(FAIL - Expected 605)\n");
    }
    
    /* Part 2 Check */
    printf("Part 2 - Longest Route:  %u ", max_route_dist);
    if (max_route_dist == 982) {
        printf("(PASS)\n");
    } else {
        printf("(FAIL - Expected 982)\n");
    }
}
