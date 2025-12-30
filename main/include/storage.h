#ifndef STORAGE_H
#define STORAGE_H


#define STORAGE_CAPACITY 5

typedef struct {
    float measurements[STORAGE_CAPACITY];
    int num_measurements;
    int current_first;
} storage_handle;

void storage_add_measurement(storage_handle* handle, float temperature);
void storage_get_json_data(storage_handle* handle, char ** output_ptr);

#endif
