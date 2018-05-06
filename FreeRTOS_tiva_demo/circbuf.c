/*
 * circbuf.c
 *
 *  Created on: Sep 25, 2017
 *      Author: Kayla
 */

#include "circbuf.h"
#include <stdint.h>
#include <stdlib.h>

//Setup buffer in heap and initialize structure variable
CB_Status CB_initialize_buffer(CircBuf_t ** buf, uint32_t length){
    (*buf)->buffer = (uint16_t *)malloc(length * sizeof(uint16_t));
    if((*buf)->buffer == NULL)        //buffer is zero -> return an error
        {
            return CB_STATUS_ERROR;
        }
    (*buf)->head = (*buf)->buffer;
    (*buf)->tail = (*buf)->buffer;
    (*buf)->length = length;
    (*buf)->num_items = 0;
    return CB_STATUS_INITIAL;
 }

//Set contents of buffer to zero and reset buffer pointer to a clean state
CB_Status CB_clear_buffer(CircBuf_t * buf){
    buf->num_items = 0;
    buf->head = buf->buffer;
    buf->tail = buf->buffer;
    return CB_STATUS_INITIAL;

}

//Delete buffer from dynamic memory allocation
CB_Status CB_delete_buffer(CircBuf_t * buf){
    free(buf);
    return CB_STATUS_EMPTY;
}

//Return non-zero value if buffer is full, zero value if not
int8_t CB_is_buffer_full(CircBuf_t * buf){
    if(buf->num_items == buf->length){
        return 1;
    }
    else{
        return 0;
    }
}

//Return zero of buffer not empty, non-zero value if empty
int8_t CB_is_buffer_empty(CircBuf_t * buf){
    if(buf->num_items == 0){
        return 1;
    }
    else{
        return 0;
    }
}

//Add new item to buffer and adjust the params in the structure
CB_Status CB_add_item_to_buffer(CircBuf_t * buf, uint16_t item)
{
    if(!buf)
    {
        return CB_STATUS_ERROR;
    }
    if(buf->num_items == BUFFER)
    {
        if(buf->head == buf->buffer + buf->length)
        {
            buf->head = buf->buffer;
        }
        if(buf->tail == buf->buffer + buf->length)
        {
            buf->tail = buf->buffer;
        }
        *buf->head = item;
        buf->head++;
        buf->tail++;
        return CB_STATUS_MID;
    }
    else
    {
        if(buf->head == buf->buffer + buf->length)
         {
             buf->head = buf->buffer;
             *buf->head = item;
             buf->head++;
             buf->num_items++;
             return CB_STATUS_MID;
         }
         *buf->head = item;
         buf->head++;
         buf->num_items++;
         return CB_STATUS_MID;
    }
}

//Remove item from buffer and adjust params in the structure
uint16_t CB_remove_item_from_buffer(CircBuf_t * buf)
{
    if(buf->num_items == 0)
    {
        return CB_STATUS_EMPTY;
    }

    if(buf->tail == buf->buffer + buf->length)
    {
        uint32_t temp = *buf->tail;
        buf->tail = buf->buffer;
        buf->num_items--;

    }

    uint16_t temp = *buf->tail;
    buf->tail++;
    buf->num_items--;

    return temp;
}

