#ifndef TEST_MOCK_R_SCI_RX_IF_H
#define TEST_MOCK_R_SCI_RX_IF_H

#include <stdint.h>

typedef void *sci_hdl_t;

typedef enum {
    SCI_SUCCESS = 0,
    SCI_ERR_INSUFFICIENT_SPACE = -1
} sci_err_t;

typedef enum {
    SCI_MODE_ASYNC = 0
} sci_mode_t;

typedef enum {
    SCI_EVT_TEI = 0
} sci_cb_evt_t;

typedef struct {
    sci_cb_evt_t event;
} sci_cb_args_t;

#define SCI_CH5           5
#define SCI_CLK_INT       0
#define SCI_DATA_8BIT     0
#define SCI_PARITY_OFF    0
#define SCI_EVEN_PARITY   0
#define SCI_STOPBITS_1    0

typedef struct {
    struct {
        uint32_t baud_rate;
        int clk_src;
        int data_size;
        int parity_en;
        int parity_type;
        int stop_bits;
        int int_priority;
    } async;
} sci_cfg_t;

sci_err_t R_SCI_Open(uint8_t chan, sci_mode_t mode, sci_cfg_t *p_cfg, void (*p_callback)(void *p_args), sci_hdl_t *p_hdl);
sci_err_t R_SCI_Send(sci_hdl_t hdl, uint8_t *p_src, uint16_t length);

#endif
