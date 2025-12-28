/**
 * Copyright (c) 2023 mjcross
 *
 * SPDX-License-Identifier: BSD-3-Clause
**/

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include <stdlib.h>
#include "onewire_library.h"

bool ow_init (OW *ow, PIO pio, uint offset, uint gpio) {
    int sm = pio_claim_unused_sm (pio, false);
    if (sm == -1) {
        return false;
    }
    gpio_init (gpio);               // enable the gpio and clear any output value
    pio_gpio_init (pio, gpio);      // set the function to PIO output
    ow->gpio = gpio;
    ow->pio = pio;
    ow->offset = offset;
    ow->sm = (uint)sm;
    ow->jmp_reset = onewire_reset_instr (ow->offset);   // assemble the bus reset instruction
    onewire_sm_init (ow->pio, ow->sm, ow->offset, ow->gpio, 8); // set 8 bits per word
    return true;
}

void ow_send (OW *ow, uint data) {
    pio_sm_put_blocking (ow->pio, ow->sm, (uint32_t)data);
    pio_sm_get_blocking (ow->pio, ow->sm);  // discard the response
}

uint8_t ow_read (OW *ow) {
    pio_sm_put_blocking (ow->pio, ow->sm, 0xff);    // generate read slots
    return (uint8_t)(pio_sm_get_blocking (ow->pio, ow->sm) >> 24);  // shift response into bits 0..7
}

bool ow_reset (OW *ow) {
    pio_sm_exec_wait_blocking (ow->pio, ow->sm, ow->jmp_reset);
    if ((pio_sm_get_blocking (ow->pio, ow->sm) & 1) == 0) {     // apply pin mask (see pio program)
        return true;    // a slave pulled the bus low
    }
    return false;
}

int ow_romsearch (OW *ow, uint64_t *romcodes, int maxdevs, uint command) {
    int index;
    uint64_t romcode = 0ull;
    int branch_point;
    int next_branch_point = -1;
    int num_found = 0;
    bool finished = false;

    onewire_sm_init (ow->pio, ow->sm, ow->offset, ow->gpio, 1); // set driver to 1-bit mode

    while (finished == false && (maxdevs == 0 || num_found < maxdevs )) {
        finished = true;
        branch_point = next_branch_point;
        if (ow_reset (ow) == false) {
            num_found = 0;     // no slaves present
            finished = true;
            break;
        }
        for (int i = 0; i < 8; i += 1) {    // send search command as single bits
            ow_send (ow, command >> i);
        }
        for (index = 0; index < 64; index += 1) {   // determine romcode bits 0..63 (see ref)
            uint a = ow_read (ow);
            uint b = ow_read (ow);
            if (a == 0 && b == 0) {         // (a, b) = (0, 0)
                if (index == branch_point) {
                    ow_send (ow, 1);
                    romcode |= (1ull << index);
                } else {
                    if (index > branch_point || (romcode & (1ull << index)) == 0) {
                        ow_send(ow, 0);
                        finished = false;
                        romcode &= ~(1ull << index);
                        next_branch_point = index;
                    } else {                // index < branch_point or romcode[index] = 1
                        ow_send (ow, 1);
                    }
                }
            } else if (a != 0 && b != 0) {  // (a, b) = (1, 1) error (e.g. device disconnected)
                num_found = -2;             // function will return -1
                finished = true;
                break;                      // terminate for loop
            } else {
                if (a == 0) {               // (a, b) = (0, 1) or (1, 0)
                    ow_send (ow, 0);
                    romcode &= ~(1ull << index);
                } else {
                    ow_send (ow, 1);
                    romcode |= (1ull << index);
                }
            }
        }                                   // end of for loop

        if (romcodes != NULL) {
            romcodes[num_found] = romcode;  // store the romcode
        }
        num_found += 1;
    }                                       // end of while loop

    onewire_sm_init (ow->pio, ow->sm, ow->offset, ow->gpio, 8); // restore 8-bit mode
    return num_found;
}

uint64_t *Ids;
double	*_Temps;
int16_t Anzahl = 0;
OW WIREBUS;

uint64_t *ds18b20_getIds(){ return Ids; }
int16_t ds18b20_init(PIO pio, uint8_t gpio){
    uint offset;
	if (pio_can_add_program (pio, &onewire_program)) {
		offset = pio_add_program (pio, &onewire_program);

        // claim a state machine and initialise a driver instance
        if (ow_init (&WIREBUS, pio, offset, gpio)) {
			uint64_t romcode[MAX_DEVS];
            Anzahl = ow_romsearch (&WIREBUS, romcode, MAX_DEVS, OW_SEARCH_ROM);
			Ids = (uint64_t *) calloc(Anzahl, sizeof(uint64_t));
			_Temps = (double *) calloc(Anzahl, sizeof(double));
            for (int i = 0; i < Anzahl; i += 1) Ids[i] = romcode[i];
            if(Anzahl != 10)return Anzahl;
            else return 0;
		}else return 0;
	}else return 0;
}
double ds18b20_getTemp(uint64_t id){
	ow_reset (&WIREBUS);
	ow_send (&WIREBUS, OW_SKIP_ROM);
	ow_send (&WIREBUS, DS18B20_CONVERT_T);
	while (ow_read(&WIREBUS) == 0);
	ow_reset (&WIREBUS);
	ow_send (&WIREBUS, OW_MATCH_ROM);
	for (int b = 0; b < 64; b += 8) {
		ow_send (&WIREBUS, id >> b);
	}
	ow_send (&WIREBUS, DS18B20_READ_SCRATCHPAD);
	int16_t temp = ow_read (&WIREBUS) | (ow_read (&WIREBUS) << 8);
	return temp / 16.0;
}
double *ds18b20_getTemps(){
	ow_reset (&WIREBUS);
	ow_send (&WIREBUS, OW_SKIP_ROM);
	ow_send (&WIREBUS, DS18B20_CONVERT_T);

	// wait for the conversions to finish
	while (ow_read(&WIREBUS) == 0);

	// read the result from each device
	for (int i = 0; i < Anzahl; i += 1) {
		ow_reset (&WIREBUS);
		ow_send (&WIREBUS, OW_MATCH_ROM);
		for (int b = 0; b < 64; b += 8) {
			ow_send (&WIREBUS, Ids[i] >> b);
		}
		ow_send (&WIREBUS, DS18B20_READ_SCRATCHPAD);
		int16_t temp = 0;
		temp = ow_read (&WIREBUS) | (ow_read (&WIREBUS) << 8);
		_Temps[i] = temp / 16.0;
	}
	return _Temps;
}
