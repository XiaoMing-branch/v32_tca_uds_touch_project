#ifndef __FFF_TCPL03X_LL_SCI_H__
#define __FFF_TCPL03X_LL_SCI_H__


#ifdef ENABLE_TEST_MODE
    #include "fff_tcpl03x_ll_def.h"
#else
    #include "tcpl03x_ll_def.h"
#endif

#include "fff.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define SCI_INT_OFFSET                (0)

/** @defgroup LIN_SCI_INT
  * @{
  */
#define SCI_INT_RX_1BYTE_DONE        0x00000001    //Receive each byte data done
#define SCI_INT_RX_DONE              0x00000002          //Receive all data and checksum done(when rx_num_mode =1)
#define SCI_INT_RX_PID_DONE          0x00000004      //Receive pid done
#define SCI_INT_RX_PTY_CHK_ERR       0x00000008   //Receive parity check error (in LIN parity check only in PID).
#define SCI_INT_RX_CHKSUM_ERR        0x00000010    //Receive checksum error
#define SCI_INT_RX_STP_ERR           0x00000020       //Receive no stop bit error
#define SCI_INT_RX_FIFO_FULL         0x00000040     //Receive fifo full
#define SCI_INT_RX_FIFO_OVF_ERR      0x00000080  //Receive fifo overflow
#define SCI_INT_TX_DONE              0x00000100          //Transmit done(when tx_num_mode =1)
#define SCI_INT_TX_FIFO_EMPTY        0x00000200    //Transmit fifo empty
//SCI_INT_TX_COL_DET_ERR
#define SCI_INT_BRK_DET              0x00000400         //Break bits detect
#define SCI_INT_SYNC_DET             0x00000800         //Sync bits detect
#define SCI_INT_SYNC_VAL_ERR         0x00001000     //Sync value check error
#define SCI_INT_TX_PID_DONE          0x00002000      //tx pid done
#define SCI_INT_TX_RX_CONF           0x00004000       //tx rx conflict
//..new
#define SCI_INT_MP_MODE_ADDR         0x00008000     //multi_processor mode address received
#define SCI_INT_SLV_SELECTED         0x00010000    //LIN AA done
#define SCI_INT_AUTO_ADDR_DONE       0x00020000    //LIN AA done
#define SCI_INT_TX_1BYTE_DONE        0x00040000    //tx 1byte done
#define SCI_INT_SLV_TX_BRK_DONE      0x00080000  //slave tx break done
#define SCI_INT_RX_DATA_DONE         0x00100000     //Receive all data done(when rx_num_mode =1)
#define SCI_INT_SHORT_TO_GND         0x00200000    //Short to GND detected interrupt
#define SCI_INT_RX_VALID             0x00400000

/**
  * @brief  ll sci bus enumeration
  */
typedef enum
{
    LL_SCI_BUS_0 = 0,
    LL_SCI_BUS_1,
    LL_SCI_BUS_2,
    LL_SCI_BUS_MAX
} ll_sci_bus_e;

/**
  * @brief  ll sic mode enumeration
  */
typedef enum
{
    SCI_MODE_LIN_S         = 0,
    SCI_MODE_LIN_M,
    SCI_MODE_UART,
    SCI_MODE_MAX,
} ll_sci_mode_e;

/**
  * @brief  ll sic clear enumeration
  */
typedef enum
{
    SCI_CLEAR_NULL          = (0),
    SCI_CLEAR_TX_FIFO     = (0x01 << 0),
    SCI_CLEAR_RX_FIFO     = (0x01 << 1),
    SCI_CLEAR_TX_ABORT    = (0x01 << 2),
    SCI_CLEAR_RX_ABORT    = (0x01 << 3),
} ll_sci_clear_type_e;

/**
  * @brief  ll sic mode enumeration
  */
typedef enum
{
    LIN_CHECKSUM_CLASSIC           = 0,
    LIN_CHECKSUM_ENHANCED          = 1,
    LIN_CHECKSUM_MAX,
} ll_lin_checksum_e;

typedef struct
{
    ll_clk_config_t clk_cfg;
    ll_isr_config_t isr_cfg;
    ll_sci_mode_e mode;
    uint32_t baudrate;
} sci_config_t;

#define IS_SCI_BUS(__BUS__)         (((__BUS__) == LL_SCI_BUS_0) || ((__BUS__) ==LL_SCI_BUS_1) )
#define IS_SCI_MODE(__MODE__)       ((__MODE__) < SCI_MODE_MAX )


DECLARE_FAKE_VOID_FUNC(ll_sci_deinit,ll_sci_bus_e);
DECLARE_FAKE_VOID_FUNC(ll_sci_init,ll_sci_bus_e,sci_config_t*,ISR_FUNC_CALLBACK);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_baudrate_config,ll_sci_bus_e,uint32_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_isr_enabl,ll_sci_bus_e,bool);
DECLARE_FAKE_VOID_FUNC(ll_sci_state_clear,ll_sci_bus_e,ll_sci_clear_type_e);

DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_rx_delay_set,ll_sci_bus_e,uint8_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_wakeup_enable,ll_sci_bus_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_aa_enable,ll_sci_bus_e,lin_aa_type_e,bool,uint16_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_aa_disable,ll_sci_bus_e);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_aa_ready_set,ll_sci_bus_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_transmit,ll_sci_bus_e ,uint8_t * ,uint16_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_sci_receive,ll_sci_bus_e ,uint8_t *,uint16_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_transmit,ll_sci_bus_e,uint8_t,uint8_t *,uint16_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_receive,ll_sci_bus_e,uint8_t,uint8_t *,uint16_t);

DECLARE_FAKE_VALUE_FUNC(uint8_t,ll_lin_checksum_calib_func,uint8_t,uint8_t *,uint16_t);

DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_pid_read,ll_sci_bus_e,uint8_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_read_byte,ll_sci_bus_e,uint8_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_auto_baudrate_read,ll_sci_bus_e,uint32_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_baudrate_read,ll_sci_bus_e,uint32_t *);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_ctrl_glben,ll_sci_bus_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_ctrl_rx_abort,ll_sci_bus_e,bool);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_ctrl_brk_tx,ll_sci_bus_e,uint8_t);
DECLARE_FAKE_VALUE_FUNC(ll_status_e,ll_lin_tx_header,ll_sci_bus_e,uint8_t);


#if defined(__cplusplus)
}
#endif
#endif /* __TCPL03X_LL_SCI_H__ */
