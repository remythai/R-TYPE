#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void print_entity(int type, int x, int y, float spawn_time, int is_last)
{
    printf("    {\n");
    printf("      \"type\": %d,\n", type);
    printf("      \"x\": %d,\n", x);
    printf("      \"y\": %d,\n", y);
    printf("      \"spawnTime\": %.2f,\n", spawn_time);
    printf("      \"spritePath\": \"assets/sprites/r-typesheet5.png\",\n");
    printf("      \"textureRect\": [0, 0, 33, 36]\n");
    printf("    }%s\n", is_last ? "" : ",");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_entities> <simultaneous|sequential>\n", argv[0]);
        return 1;
    }

    int num_entities = atoi(argv[1]);
    char *mode = argv[2];

    if (num_entities <= 0) {
        fprintf(stderr, "Error: Number of entities must be positive\n");
        return 1;
    }

    if (strcmp(mode, "simultaneous") != 0 && strcmp(mode, "sequential") != 0) {
        fprintf(stderr, "Error: Mode must be 'simultaneous' or 'sequential'\n");
        return 1;
    }

    int is_simultaneous = (strcmp(mode, "simultaneous") == 0);

    srand(time(NULL));

    printf("{\n");
    printf("  \"entities\": [\n");

    for (int i = 0; i < num_entities; i++) {
        int type = 1;
        int x = 1920;
        int y = 64 + (rand() % 801);
        float spawn_time;

        if (is_simultaneous) {
            spawn_time = 5.0;
        } else {
            spawn_time = (float)i;
        }

        print_entity(type, x, y, spawn_time, (i == num_entities - 1));
    }

    printf("  ]\n");
    printf("}\n");

    return 0;
}