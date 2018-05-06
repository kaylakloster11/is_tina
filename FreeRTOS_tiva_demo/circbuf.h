/*
 * circbuf.h
 *
 *  Created on: Sep 25, 2017
 *      Author: Kayla
 */

#ifndef CIRCBUF_H_
#define CIRCBUF_H_

#include <stdint.h>


#define BUFFER (20)
#define LENGTH  20.0

typedef struct {
    uint8_t * buffer; //pointer to base of buffer in heap
    volatile uint16_t * head; //Pointer to first item
    volatile uint16_t * tail; //Pointer to last item
    volatile uint32_t num_items; //number items in buffer
    volatile uint32_t length; //max number of items buffer is holding
} CircBuf_t;

typedef enum CB_Status{
    // Circular Buffer error (general)
    CB_STATUS_EMPTY = 0,
    CB_STATUS_ERROR = 0,
    CB_STATUS_FULL = 1,
    CB_STATUS_OVERWRITE = 1,
    CB_STATUS_INITIAL = 1,
    CB_STATUS_WRAP = 1,
    CB_STATUS_MID = 2,
} CB_Status;

CB_Status CB_initialize_buffer(CircBuf_t ** buf, uint32_t length);
CB_Status CB_clear_buffer(CircBuf_t * buf);
CB_Status CB_delete_buffer(CircBuf_t * buf);
int8_t CB_is_buffer_full(CircBuf_t * buf);
int8_t CB_is_buffer_empty(CircBuf_t * buf);
CB_Status CB_add_item_to_buffer(CircBuf_t * buf, uint16_t item);
uint16_t CB_remove_item_from_buffer(CircBuf_t * buf);



#endif /* CIRCBUF_H_ */
