/*
 * ResetCodes.h
 *
 *  Created on: 15 Jan 2020
 *      Author: CasperBroekhuizen
 */

#ifndef RESETCODES_H_
#define RESETCODES_H_

//Product Family Reset Codes:
#include "reset.h"

//Product Specific Reset Codes  (6.8.1.2 p119):

////RSTCTL_HARDRESET_STAT
//#define RESET_HARD_SYSTEMREQ        RESET_SRC_0
//#define RESET_HARD_WDTTIME          RESET_SRC_1
//#define RESET_HARD_WDTPW_SRC        RESET_SRC_2
//#define RESET_HARD_FCTL             RESET_SRC_3
//#define RESET_HARD_CS               RESET_SRC_14
//#define RESET_HARD_PCM              RESET_SRC_15
//
////RSTCTL_SOFTRESET_STAT
//#define RESET_SOFT_CPULOCKUP        RESET_SRC_0
//#define RESET_SOFT_WDTTIME          RESET_SRC_1
//#define RESET_SOFT_WDTPW_SRC        RESET_SRC_2
//
//
////RSTCTL_PSSRESET_STAT
//#define RESET_PSS_VCCDET            RESET_VCCDET
//#define RESET_PSS_SVSH_TRIP          RESET_SVSH_TRIP
//#define RESET_PSS_BGREF_BAD         RESET_BGREF_BAD
//
////RSTCTL_PCMRESET_STAT
//#define RESET_PCM_LPM35             RESET_LPM35
//#define RESET_PCM_LPM45             RESET_LPM45
//
////RSTCTL_PINRESET_STAT
//#define RESET_PIN_NMI               RSTCTL_PINRESET_STAT_RSTNMI
//
////RSTCTL_REBOOTRESET_STAT
//#define RESET_REBOOT                RSTCTL_REBOOTRESET_STAT_REBOOT
//
////RSTCTL_CSRESET_STAT
//#define RESET_CSRESET_DCOSHORT      RSTCTL_CSRESET_STAT_DCOR_SHT

//RSTCTL_HARDRESET_STAT

#define RESET_HARD_SYSTEMREQ        RESET_SRC_0
#define RESET_HARD_WDTTIME          RESET_SRC_1
#define RESET_HARD_WDTPW_SRC        RESET_SRC_2
#define RESET_HARD_FCTL             RESET_SRC_3
#define RESET_HARD_CS               RESET_SRC_4
#define RESET_HARD_PCM              RESET_SRC_5
#define RESET_SOFT_CPULOCKUP        RESET_SRC_6
#define RESET_SOFT_WDTTIME          RESET_SRC_7
#define RESET_SOFT_WDTPW_SRC        RESET_SRC_8
#define RESET_PSS_VCCDET            RESET_SRC_9
#define RESET_PSS_SVSH_TRIP         RESET_SRC_10
#define RESET_PSS_BGREF_BAD         RESET_SRC_11
#define RESET_PCM_LPM35             RESET_SRC_12
#define RESET_PCM_LPM45             RESET_SRC_13
#define RESET_PIN_NMI               RESET_SRC_14
#define RESET_REBOOT                RESET_SRC_15
#define RESET_CSRESET_DCOSHORT      ((uint32_t)0x00010000)

#endif /* RESETCODES_H_ */
