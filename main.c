/**
SPDX-License-Identifier: MPL-2.0
--------------------------------
This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
If a copy of the MPL was not distributed with this file,
You can obtain one at https://mozilla.org/MPL/2.0/.

Good old Minesweeper game written in C Language.

Provided “as is”, without warranty of any kind.

Copyright © 2025 Alar Akilbekov. All rights reserved.

Third party copyrights are property of their respective owners.
*/

#include <stdio.h> // printf
#include <time.h> // time
#include <stdlib.h> // rand, srand, and RAND_MAX
#include <stdbool.h>


long nanoTime(){
    struct timespec ts;

    // CLOCK_REALTIME (can be changed) vs. CLOCK_MONOTONIC (only grows)
    clock_gettime(CLOCK_REALTIME, &ts);

    time_t seconds = ts.tv_sec;
    // time value nanoseconds
    long nanoseconds = ts.tv_nsec; // только nano часть

    // printf("Seconds: %lu \n", seconds);
    printf("Nanoseconds: %ld \n", nanoseconds); // nano = 10^9

    long total_nanoseconds = seconds * 1000000000l + nanoseconds;
    // printf("Total time in nanoseconds: %ld\n", total_nanoseconds);

    return nanoseconds;
}


void seedRandom(){
    // printf("Random Max: %d \n\n", RAND_MAX);

    // seed random with current time in seconds
	srand(nanoTime());

    // Generate seeded random numbers
    // Ranges between (0; RAND_MAX]
    printf("Seeded Random number 1: %d\n", rand());
    printf("Seeded Random number 2: %d\n", rand());
    printf("Seeded Random number 3: %d\n", rand());
}


void init_map(int rows, int cols, char ** map, bool ** view, bool ** flagged){
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            map[r][c] = '.';
            view[r][c] = false;
            flagged[r][c] = false;
        }
    }
}


void place_mines(int mines, int rows, int cols, char ** map) {
    for(int i = 0; i < mines; i++) {
        int r = rand() % rows;
        int c = rand() % cols;
        if (map[r][c] != '*') {
            map[r][c] = '*';
        }
    }
}


void print_map(int rows, int cols, char ** map, bool ** view, bool ** flagged, bool reveal) {
    printf("   ");
    for(int c = 0; c < cols; ++c) printf("%2d ", c);
    printf("\n");

    for(int r = 0; r < rows; ++r){
        printf("%2d ", r);
        for(int c = 0; c < cols; ++c){
            if(view[r][c] || reveal){
                // show revealed cell: dot if '.', else number or '*'
                printf("%2c ", map[r][c]);
            }
            else if (flagged[r][c]) {
                printf("%2c ", 'F');
            }
            else {
                printf("%2c ", '?');
            }
        }
        printf("\n");
    }
}


bool in_bounds(int rows, int cols, int r, int c) {
    return r >= 0 && r < rows && c >= 0 && c < cols;
}


int count_adjacent_mines(int rows, int cols, int r, int c, char ** map) {
    int cnt = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int _r = r + dr,
                _c = c + dc;
            if (in_bounds(rows, cols, _r, _c) && map[_r][_c] == '*'){
                cnt++;
            }
        }
    }
    return cnt;
}


void addProximities(int rows, int cols, char ** map) {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (map[r][c] == '*') continue;
            int cnt = count_adjacent_mines(rows, cols, r, c, map);
            if (cnt > 0){
                map[r][c] = (char)('0' + cnt);
            }
            // leave '.' as-is for 0
        }
    }
}


// Declare first
void reveal_zero_region(int rows, int cols, int r, int c, char ** map, bool ** view, bool ** flagged, bool recursively);

bool open_cell(int rows, int cols, int r, int c, char ** map, bool ** view, bool ** flagged) {
    system("clear");

    if (!in_bounds(rows, cols, r, c)) {
        printf("Out of bounds.\n");
        return false;
    }
    if (flagged[r][c]) {
        printf("Cell is flagged. Unflag first if you want to open it.\n");
        return false;
    }
    if (view[r][c]) return true; // already opened; harmless

    view[r][c] = true;
    if (map[r][c] == '*') return false; // BOOM

    if (map[r][c] == '.') {
        reveal_zero_region(rows, cols, r, c, map, view, flagged, true);
    }

    return true;
}


// when a '.' cell is opened, reveal its neighbors recursively.
void reveal_zero_region(int rows, int cols, int r, int c, char ** map, bool ** view, bool ** flagged, bool recursively) {
    if (!in_bounds(rows, cols, r, c)) return;
    if (flagged[r][c]) return; // don't auto-open flags
    view[r][c] = true;
    if (map[r][c] != '.') return; // stop if it's a number
    if(!recursively) return;
    // expand neighbors around a zero
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (in_bounds(rows, cols, nr, nc) && !view[nr][nc]) {
                // recursion
                reveal_zero_region(rows, cols, nr, nc, map, view, flagged, false);
            }
        }
    }
}


bool has_won(int mines, int rows, int cols, char ** map, bool ** view) {
    int opened = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (view[r][c] && map[r][c] != '*') opened++;
        }
    }
    return opened == (rows * cols - mines);
}


void game_loop(int MINES, int ROWS, int COLS, char ** map, bool ** view, bool ** flagged){
    // PART 3: play loop
    printf("Minesweeper %dx%d with %d mines\n", ROWS, COLS, MINES);
    printf("Commands:\n");
    printf("  o r c   -> open row r, col c\n");
    printf("  f r c   -> toggle flag at (r,c)\n");
    printf("  q       -> quit\n\n");

    print_map(ROWS, COLS, map, view, flagged, false);

    int step = 0;
    while (true) {
        step++;
        char cmd;
        int r, c;
        scanf("%c", &cmd);
        if (cmd == 'q' || cmd == 'Q') {
            printf("Quitting\n");
            break;
        }
        else if (cmd == 'o' || cmd == 'O') {
            if (scanf("%d %d", &r, &c) != 2) {
                printf("Oops! Expected: o r c\n");
                // clear bad input
                int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
                continue;
            }
            if (!open_cell(ROWS, COLS, r, c, map, view, flagged)) {
                // show boom and full map
                printf("\nBOOOOM!!!\n\n");
                // reveal all mines before printing to emphasize loss
                print_map(ROWS, COLS, map, view, flagged, true);
                break;
            }
            print_map(ROWS, COLS, map, view, flagged, false);
            if (has_won(MINES, ROWS, COLS, map, view)) {
                printf("\nYou win!\n");
                break;
            }
        }
        else if (cmd == 'f' || cmd == 'F') {
            if (scanf("%d %d", &r, &c) != 2) {
                printf("Oops! Expected: f r c\n");
                int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
                continue;
            }
            if (!in_bounds(ROWS, COLS, r, c)) {
                printf("Out of bounds.\n");
                int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
                continue;
            }
            if (view[r][c]) {
                printf("Cell already opened; cannot flag.\n");
            } else {
                flagged[r][c] = !flagged[r][c]; // toggle
            }
            print_map(ROWS, COLS, map, view, flagged, false);
        }
        else {
            printf("Step: %d\n", step);
            printf("Unknown command. Use: o/f/q\n");
        }

        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}


int main(){
    seedRandom();

    const int   MINES = 10,
                ROWS  = 12,
                COLS  = 30;

    // '*' = mine
    // '.' = empty (no proximity yet / shown as '.' if 0)
    // '1'..'8' = proximity counts
    char ** map     = (char**) malloc(ROWS * sizeof(char*));
    bool ** view    = (bool**) malloc(ROWS * sizeof(bool*));
    bool ** flagged = (bool**) malloc(ROWS * sizeof(bool*));
    for(int i = 0; i < ROWS; ++i){
        map[i]      = (char*) malloc(COLS * sizeof(char));
        view[i]     = (bool*) malloc(COLS * sizeof(bool));
        flagged[i]  = (bool*) malloc(COLS * sizeof(bool));
    }

    init_map(ROWS, COLS, map, view, flagged);
    place_mines(MINES, ROWS, COLS, map);
    addProximities(ROWS, COLS, map);
    print_map(ROWS, COLS, map, view, flagged, true);

    printf("Has won: %s\n", has_won(MINES, ROWS, COLS, map, view) ? "true" : "false");

    game_loop(MINES, ROWS, COLS, map, view, flagged);

    for(int i = 0; i < ROWS; ++i){
        free(map[i]);
        free(view[i]);
        free(flagged[i]);
    }
    free(map);
    free(view);
    free(flagged);

    return 0;
}
