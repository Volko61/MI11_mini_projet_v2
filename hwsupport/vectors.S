/*
 * vectors.S
 *
 *  Created on: 7 avr. 2021
 *      Author: bonnetst
 */

.title "vectors.S"

.thumb
.syntax unified

.section .vectors, "ax", %progbits

/* Pointeur de pile initial */
.long   __tos

/* Vecteurs système */
.long   _start
.long   _nmi
.long   _hard_fault
.long   _mem_fault
.long   _bus_fault
.long   _use_fault
.long   0
.long   0
.long   0
.long   0
.long   _svc
.long   _debug_mon
.long   0
.long   _pend_svc
.long   schedule

/* Vecteurs IRQ */

.weak   _nmi
.thumb_set _nmi,_default_handler

.weak   _hard_fault
.thumb_set _hard_fault,_default_handler

.weak   _mem_fault
.thumb_set _mem_fault,_default_handler

.weak   _bus_fault
.thumb_set _bus_fault,_default_handler

.weak   _use_fault
.thumb_set _use_fault,_default_handler

.weak   _svc
.thumb_set _svc,_default_handler

.weak   _debug_mon
.thumb_set _debug_mon,_default_handler

.weak   _pend_svc
.thumb_set _pend_svc,_default_handler

.weak   _systick
.thumb_set _systick,_default_handler

.text

.thumb_func
_default_handler:
        b   .

.end
