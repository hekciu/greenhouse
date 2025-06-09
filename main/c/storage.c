#include <float.h>
#include <memory.h>
#include <stdio.h>

#include "esp_log.h"


#define STORAGE_CAPACITY 5

#define CHARS_FOR_ONE_FLOAT_VALUE 24


static const char * TAG = "STORAGE";


static float measurements[STORAGE_CAPACITY] = {0};
static int num_measurements = 0;
static int current_first = 0;


static void marshal_to_json(char * output) {
    *output = '[';

    int currently_written = 0;

    for (int i = 0; i < num_measurements; i++) {
        int index = (i + current_first) % STORAGE_CAPACITY;

        char * value_ptr = (output + currently_written + 1);

        int n_written = snprintf(value_ptr, CHARS_FOR_ONE_FLOAT_VALUE, "%f", measurements[index]);

        currently_written += n_written;

        if (i != (num_measurements - 1)) {
            *(value_ptr + n_written) = ',';
        } else {
            *(value_ptr + n_written) = ']';
            *(value_ptr + n_written + 1) = '\0';
        }

        currently_written += 1;
    }
};


void storage_add_measurement(const float temperature) {
    int index;

    if (num_measurements == STORAGE_CAPACITY) {
        if (current_first == (STORAGE_CAPACITY - 1)) {
            current_first = 0;
            index = STORAGE_CAPACITY - 1;
        } else {
            index = current_first;
            current_first = current_first + 1;
        }
    } else {
        index = num_measurements;
        num_measurements++;
    }

    measurements[index] = temperature;
};


void storage_get_json_data(char ** output_ptr) {
    const int buffer_size = STORAGE_CAPACITY *
        CHARS_FOR_ONE_FLOAT_VALUE
        + STORAGE_CAPACITY // semicolons
        + 2 // square brackers
        + 1; // null termination

    *output_ptr = malloc(buffer_size);

    marshal_to_json(*output_ptr);
}
