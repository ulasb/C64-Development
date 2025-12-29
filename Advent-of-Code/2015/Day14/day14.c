#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define MAX_REINDEER 10
#define NAME_LEN 16
#define RACE_DURATION 2503

/* Reindeer struct - similar to Python dataclass */
typedef struct {
    char name[NAME_LEN];
    int speed;
    int fly_time;
    int rest_time;
    int distance;
    int points;
    int flying;  /* 1 for flying, 0 for resting */
    int time_in_current_state;
} Reindeer;

/* Global Variables */
Reindeer reindeer[MAX_REINDEER];
int reindeer_count = 0;

/* Function Prototypes */
void init_reindeer(void);
void add_reindeer(const char *name, int speed, int fly_time, int rest_time);
void reset_reindeer(Reindeer *r);
void update_reindeer_position(Reindeer *r);
void simulate_second(Reindeer reindeer_list[], int count, int award_points);
void run_race_simulation(Reindeer reindeer_list[], int count, int total_seconds, int award_points);
int simulate_race(Reindeer reindeer_list[], int count, int total_seconds);
int simulate_race_with_points(Reindeer reindeer_list[], int count, int total_seconds);

/* Initialize reindeer array */
void init_reindeer(void) {
    reindeer_count = 0;
    memset(reindeer, 0, sizeof(reindeer));
}

/* Add a reindeer to the array */
void add_reindeer(const char *name, int speed, int fly_time, int rest_time) {
    int idx;

    if (reindeer_count >= MAX_REINDEER) {
        printf("ERROR: Maximum reindeer count exceeded\n");
        return;
    }

    idx = reindeer_count++;
    strncpy(reindeer[idx].name, name, NAME_LEN - 1);
    reindeer[idx].name[NAME_LEN - 1] = '\0';  /* Ensure null termination */
    reindeer[idx].speed = speed;
    reindeer[idx].fly_time = fly_time;
    reindeer[idx].rest_time = rest_time;
    reset_reindeer(&reindeer[idx]);
}

/* Reset reindeer to initial state */
void reset_reindeer(Reindeer *r) {
    r->distance = 0;
    r->points = 0;
    r->flying = 1;  /* Start flying */
    r->time_in_current_state = 0;
}

/* Update a single reindeer's position and state for one second */
void update_reindeer_position(Reindeer *r) {
    if (r->flying) {
        r->distance += r->speed;
        r->time_in_current_state++;

        /* Check if we need to switch to resting */
        if (r->time_in_current_state >= r->fly_time) {
            r->flying = 0;
            r->time_in_current_state = 0;
        }
    } else {
        r->time_in_current_state++;

        /* Check if we need to switch to flying */
        if (r->time_in_current_state >= r->rest_time) {
            r->flying = 1;
            r->time_in_current_state = 0;
        }
    }
}

/* Simulate one second for all reindeer */
void simulate_second(Reindeer reindeer_list[], int count, int award_points) {
    int i;
    int current_max_distance;

    /* Update all positions for this second */
    for (i = 0; i < count; i++) {
        update_reindeer_position(&reindeer_list[i]);
    }

    if (award_points) {
        /* Find the current maximum distance */
        current_max_distance = 0;
        for (i = 0; i < count; i++) {
            if (reindeer_list[i].distance > current_max_distance) {
                current_max_distance = reindeer_list[i].distance;
            }
        }

        /* Award points to all reindeer at the current maximum distance */
        for (i = 0; i < count; i++) {
            if (reindeer_list[i].distance == current_max_distance) {
                reindeer_list[i].points++;
            }
        }
    }
}

/* Run the race simulation for specified duration */
void run_race_simulation(Reindeer reindeer_list[], int count, int total_seconds, int award_points) {
    int i;

    /* Reset all reindeer to initial state */
    for (i = 0; i < count; i++) {
        reset_reindeer(&reindeer_list[i]);
    }

    /* Run simulation */
    for (i = 0; i < total_seconds; i++) {
        simulate_second(reindeer_list, count, award_points);
    }
}

/* Simulate race and return max distance */
int simulate_race(Reindeer reindeer_list[], int count, int total_seconds) {
    int max_distance;
    int i;

    run_race_simulation(reindeer_list, count, total_seconds, 0);

    max_distance = 0;
    for (i = 0; i < count; i++) {
        if (reindeer_list[i].distance > max_distance) {
            max_distance = reindeer_list[i].distance;
        }
    }
    return max_distance;
}

/* Simulate race with points and return max points */
int simulate_race_with_points(Reindeer reindeer_list[], int count, int total_seconds) {
    int max_points;
    int i;

    run_race_simulation(reindeer_list, count, total_seconds, 1);

    max_points = 0;
    for (i = 0; i < count; i++) {
        if (reindeer_list[i].points > max_points) {
            max_points = reindeer_list[i].points;
        }
    }
    return max_points;
}

/* Test cases from the problem description */
void run_test_part1(void) {
    int max_distance;

    printf("\n--- Part 1 Test ---\n");
    init_reindeer();

    /* Add test reindeer */
    add_reindeer("Comet", 14, 10, 127);
    add_reindeer("Dancer", 16, 11, 162);

    printf("Testing reindeer:\n");
    printf("Comet: %d km/s for %d seconds, rest %d seconds\n",
           reindeer[0].speed, reindeer[0].fly_time, reindeer[0].rest_time);
    printf("Dancer: %d km/s for %d seconds, rest %d seconds\n",
           reindeer[1].speed, reindeer[1].fly_time, reindeer[1].rest_time);

    /* Test after 1000 seconds */
    max_distance = simulate_race(reindeer, reindeer_count, 1000);

    printf("\nAfter 1000 seconds:\n");
    printf("Comet traveled: %d km\n", reindeer[0].distance);
    printf("Dancer traveled: %d km\n", reindeer[1].distance);
    printf("Winner traveled: %d km\n", max_distance);

    if (max_distance == 1120 && reindeer[0].distance == 1120 && reindeer[1].distance == 1056) {
        printf("PART 1: (PASS)\n");
    } else {
        printf("PART 1: (FAIL) Expected Comet: 1120 km, Dancer: 1056 km\n");
    }
}

void run_test_part2(void) {
    int max_points;

    printf("\n--- Part 2 Test ---\n");
    init_reindeer();

    /* Add test reindeer */
    add_reindeer("Comet", 14, 10, 127);
    add_reindeer("Dancer", 16, 11, 162);

    /* Test after 1000 seconds with points */
    max_points = simulate_race_with_points(reindeer, reindeer_count, 1000);

    printf("After 1000 seconds:\n");
    printf("Comet points: %d\n", reindeer[0].points);
    printf("Dancer points: %d\n", reindeer[1].points);
    printf("Winner points: %d\n", max_points);

    if (max_points == 689 && reindeer[0].points == 312 && reindeer[1].points == 689) {
        printf("PART 2: (PASS)\n");
    } else {
        printf("PART 2: (FAIL) Expected Dancer: 689 points, Comet: 312 points\n");
    }
}

void run_final_race(void) {
    int max_distance;
    int max_points;

    printf("\n--- Final Race (2503 seconds) ---\n");
    init_reindeer();

    /* Add reindeer - these would normally come from input file */
    /* For now, using test reindeer since no input file is provided */
    add_reindeer("Comet", 14, 10, 127);
    add_reindeer("Dancer", 16, 11, 162);

    /* Part 1: Distance-based winner */
    printf("Running Part 1: Distance-based race...\n");
    max_distance = simulate_race(reindeer, reindeer_count, RACE_DURATION);
    printf("Part 1 - After %d seconds, the winning reindeer traveled %d km!\n",
           RACE_DURATION, max_distance);

    /* Part 2: Points-based winner */
    printf("Running Part 2: Points-based race...\n");
    max_points = simulate_race_with_points(reindeer, reindeer_count, RACE_DURATION);
    printf("Part 2 - After %d seconds, the winning reindeer earned %d points!\n",
           RACE_DURATION, max_points);
}

int main(void) {
    clrscr();
    printf("AoC 2015 Day 14: Reindeer Olympics\n");
    printf("==================================\n");

    run_test_part1();
    run_test_part2();
    run_final_race();

    printf("\nPress any key to exit...\n");
    cgetc();

    return 0;
}
