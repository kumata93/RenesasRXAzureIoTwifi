/*
* Copyright (c) 2016 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/***********************************************************************************************************************
* File Name        : Pin.c
* Version          : 1.0.2
* Device(s)        : R5F565NEDxFP
* Description      : This file implements SMC pin code generation.
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Pins_Create
* Description  : This function initializes Smart Configurator pins
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Pins_Create(void)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_MPC);

    /* Set CTS0# pin */
    MPC.P23PFS.BYTE = 0x0BU;
    PORT2.PMR.BYTE |= 0x08U;

    /* Set RXD0 pin */
    MPC.P21PFS.BYTE = 0x0AU;
    PORT2.PMR.BYTE |= 0x02U;

    /* Set RXD1 pin */
    MPC.P30PFS.BYTE = 0x0AU;
    PORT3.PMR.BYTE |= 0x01U;

    /* Set RXD5 pin */
    MPC.PA3PFS.BYTE = 0x0AU;
    PORTA.PMR.BYTE |= 0x08U;

    /* Set TXD0 pin */
    PORT2.PODR.BYTE |= 0x01U;
    MPC.P20PFS.BYTE = 0x0AU;
    PORT2.PDR.BYTE |= 0x01U;
    // PORT2.PMR.BIT.B0 = 1U; // Please set the PMR bit after TE bit is set to 1.

    /* Set TXD1 pin */
    PORT2.PODR.BYTE |= 0x40U;
    MPC.P26PFS.BYTE = 0x0AU;
    PORT2.PDR.BYTE |= 0x40U;
    // PORT2.PMR.BIT.B6 = 1U; // Please set the PMR bit after TE bit is set to 1.

    /* Set TXD5 pin */
    PORTA.PODR.BYTE |= 0x10U;
    MPC.PA4PFS.BYTE = 0x0AU;
    PORTA.PDR.BYTE |= 0x10U;
    // PORTA.PMR.BIT.B4 = 1U; // Please set the PMR bit after TE bit is set to 1.

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_MPC);
}

