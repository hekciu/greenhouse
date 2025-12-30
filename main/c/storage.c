#include <float.h>
#include <memory.h>
#include <stdio.h>

#include "esp_log.h"

#include "storage.h"

#define CHARS_FOR_ONE_FLOAT_VALUE 24


static const char * TAG = "STORAGE";


static void marshal_to_json(storage_handle* handle, char * output) {
    *output = '[';

    int currently_written = 0;

    for (int i = 0; i < handle->num_measurements; i++) {
        int index = (i + handle->current_first) % STORAGE_CAPACITY;

        char * value_ptr = (output + currently_written + 1);

        int n_written = snprintf(value_ptr, CHARS_FOR_ONE_FLOAT_VALUE, "%f", handle->measurements[index]);

        currently_written += n_written;

        if (i != (handle->num_measurements - 1)) {
            *(value_ptr + n_written) = ',';
        } else {
            *(value_ptr + n_written) = ']';
            *(value_ptr + n_written + 1) = '\0';
        }

        currently_written += 1;
    }
};


void storage_add_measurement(storage_handle* handle, float temperature) {
    int index;

    if (handle->num_measurements == STORAGE_CAPACITY) {
        if (handle->current_first == (STORAGE_CAPACITY - 1)) {
            handle->current_first = 0;
            index = STORAGE_CAPACITY - 1;
        } else {
            index = handle->current_first;
            handle->current_first++;
        }
    } else {
        index = handle->num_measurements;
        handle->num_measurements++;
    }

    handle->measurements[index] = temperature;
};


void storage_get_json_data(storage_handle* handle, char ** output_ptr) {
    const int buffer_size = STORAGE_CAPACITY *
        CHARS_FOR_ONE_FLOAT_VALUE
        + STORAGE_CAPACITY // semicolons
        + 2 // square brackers
        + 1; // null termination

    *output_ptr = malloc(buffer_size);

    marshal_to_json(handle, *output_ptr);
}
