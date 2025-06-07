#include <float.h>
#include <memory.h>
#include <string.h>


#define STORAGE_CAPACITY 4000

#define CHARS_FOR_ONE_FLOAT_VALUE 24


static float measurements[STORAGE_CAPACITY] = {0};
static int num_measurements = 0;
static int current_first = 0;


static void marshal_to_json(char * output) {
    *output = '{';

    for (int i = current_first; i < num_measurement; i++) {
        int index = i < STORAGE_CAPACITY ? i : i - STORAGE_CAPACITY;

        *(output + index)
    }
};


void storage_add_measurement(const float temperature) {};


void storage_get_json_data(char ** output_ptr) {
    const int buffer_size = num_measurements *
        CHARS_FOR_ONE_FLOAT_VALUE *
        + num_measurements // semicolons
        + 2 // curly brackers
        + 1; // null terminationh

    *output_ptr = malloc(buffer_size);

    marshal_to_json(*output_ptr);
}
