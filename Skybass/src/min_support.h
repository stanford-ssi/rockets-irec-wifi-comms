#ifndef MIN_SUPPORT_H
#define MIN_SUPPORT_H

#include <Arduino.h>
#include "min.h"

//this file adapts the MIN C implementation to the Arduino Framework. You should adapt the switch-cases
//for your port configuration. for each port you will need a min context, initialized to the right number.

//ask @timv if you have questions.
//This should eventually be rolled into a min C++ arduino implementation.



uint32_t min_time_ms(void)
{
    return millis();
}

void min_tx_start(uint8_t port){
//nada
}

void min_tx_finished(uint8_t port){
//nada
}

#endif