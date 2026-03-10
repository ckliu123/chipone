#ifndef CTS_PATTERN_H
#define CTS_PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Define different patterns */
/****************************************************************
 *            PATTERN_TYPE_1 - DP382_BOE_Riyadh                 *
 ****************************************************************
 *	   RX00----RX35 		*		  RX00----RX35 RX36----RX71 *
 * TX00 		   TX00 	*	  TX00 o  ---- o	x  ---- x   *
 *	|				|		*	   |   .  ---- .	.  ---- .	*
 *	|				|		*	   |   .  ---- .	.  ---- .	*
 * TX33 		   TX33 	*	  TX33 o  ---- o	x  ---- x   *
 * -------------------- 	*	  ----------------------------- *
 * TX34 		   TX34 	*	  TX34 x  ---- x	o  ---- o   *
 *	|				|		*	   |   .  ---- .	.  ---- .	*
 * TX50 		   TX50 	*	  TX50 x  ---- x	o  ---- o	*
 *	   RX36----RX71 		*									*
 ****************************************************************/
#define PATTERN_TYPE_1					0


/*
 * ROWS & COLS:		application channels, maybe different with tx_num/rx_num which get from firmware
 * TR_NUM_MAX:		TR(TX/RX), max physical channels supported by IC
 * TX_NUM & RX_NUM:	Panel physical channels
 */
#define ROWS							35//35//18//36
#define COLS							21//31//39
#define TX_NUM							21//43//18//36
#define RX_NUM							35//31//39


#if PATTERN_TYPE_1
#define ROWS_PATTERN					51
#define COLS_PATTERN					(COLS/2)
#else
#define ROWS_PATTERN					ROWS
#define COLS_PATTERN					COLS
#endif

#ifdef __cplusplus
}
#endif

#endif /* CTS_PATTERN_H */

