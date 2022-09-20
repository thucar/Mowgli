/****************************************************************************
* Title                 :   Blade motor driver
* Filename              :   blademotor.h
* Author                :   Nekraus
* Origin Date           :   17/08/2022
* Version               :   1.0.0

*****************************************************************************/
/** \file blademotor.h
*  \brief 
*
*/
#ifndef __BLADEMOTOR_H
#define __BLADEMOTOR_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdbool.h>

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* PUBLIC Function Prototypes
*******************************************************************************/

void BLADEMOTOR_Init(void);
void BLADEMOTOR_App(void);
void BLADEMOTOR_ReceiveIT(void);

void BLADEMOTOR_Set(uint8_t on_off);


#ifdef __cplusplus
}
#endif

#endif /*BLADEMOTOR_H*/ 

/*** End of File **************************************************************/