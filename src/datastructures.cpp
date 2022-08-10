#include "datastructures.h"

RingArray::RingArray(int array_length) {
    float array[array_length];
    this->array = array;
    this->array_length = array_length;
}

float RingArray::set_value(float value) {
    this->array[this->index] = value;
    if (this->index == this->array_length - 1) {
        this->all_indexes_used = true;
    }
    this->index = (this->index + 1) % this->array_length;

    return this->get_avg();
}


float* RingArray::get_array(){
    return this->array;
}


float RingArray::get_avg() {
    int max_index = this->all_indexes_used ? this->array_length : this->index;
    if (max_index == 0) {
        return 0;
    }
    float summed_value = 0;
    for (size_t i = 0; i < max_index; i++)
    {
        summed_value += this->array[i];
    }
    return summed_value / max_index;
}


MovingAverage::MovingAverage(float factor) {
    this->factor = factor;
}

float MovingAverage::set_value(float value) {
    if (!this->moving_average_initialized) {
        this-> value = value;
        this->moving_average_initialized = true;
    } else {
        this->value = this->value * this->factor + value * (1 - this->factor);
    }
    return this->value;
}

float MovingAverage::get_value() {
    return this->value;
}

bool MovingAverage::get_moving_average_initialized() {
    return this->moving_average_initialized;
}