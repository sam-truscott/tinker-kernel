/*
 *
 * TINKER Source Code
 * 
 *
 *  [2009] - [2013] Samuel Steven Truscott
 *  All Rights Reserved.
 */
#include "uart16550.h"

#include "tinker_api.h"
#include "tgt.h"
#include "utils/util_memcpy.h"
#include "tgt_io.h"

#define LSR_DR 0x01
#define LSR_OV_ERR 0x02
#define LSR_PA_ERR 0x04
#define LSR_FR_ERR 0x08
#define LSR_BI_ERR 0x10
#define LSR_TH_ERR 0x20
#define LSR_TE_ERR 0x40
#define LSR_RF_ERR 0x80

#define MSR_DEL_CTS 0x01
#define MSR_DEL_DSR 0x02
#define MSR_TEI 0x04
#define MSR_DEL_DCD 0x08
#define MSR_CTS 0x10
#define MSR_DSR 0x20
#define MSR_RI 0x40
#define MSR_DCD 0x80

typedef struct
{
	bool_t data_ready;
	bool_t overrun_error;
	bool_t parity_error;
	bool_t framing_error;
	bool_t break_interrupt;
	bool_t tx_holding_register;
	bool_t tx_empty;
	bool_t rx_fifo_error;
} uart_16550_line_status_t;

typedef struct
{
	bool_t delta_clear_to_send;
	bool_t delta_data_set_ready;
	bool_t trailing_edge_indicator;
	bool_t delta_data_carrier_detect;
	bool_t clear_to_send;
	bool_t data_set_ready;
	bool_t ring_indicator;
	bool_t data_carrier_detect;
} uart_16550_modem_status_t;

/* Interrupt causes */
#define INT_RECV_LINE_STATUS 6
#define INT_RECV_DATA_AVAIL 4
#define INT_CHAR_TIMEOUT 12
#define INT_TRAN_EMPTY 2
#define INT_STATUS 0

/* register structure */
#pragma pack(push,1)
typedef struct
{
	uint8_t reg0_tx_rx_ls;
	uint8_t reg1_ier_ms;
	uint8_t reg2_iir;
	uint8_t reg3_lcr;
	uint8_t reg4_mcr;
	uint8_t reg5_lsr;
	uint8_t reg6_msr;
	uint8_t reg7_scratch;
} uart_16550_device_map_t;
#pragma pack(pop)

static error_t uart16550_initialise(
		kernel_device_t * device_info,
		void * param,
		const uint32_t param_size);

static error_t uart16550_write_register(const void * const usr_data, const uint32_t id, const uint32_t val);

static error_t uart16550_read_register(const void * const usr_data, const uint32_t id, uint32_t * const val);

static error_t uart16650_write_buffer(
		const void * const usr_data,
		const uint32_t dst,
		const void * const src,
		const uint32_t src_size);

static error_t uart16550_read_buffer(
		const void * const usr_data,
		const uint32_t src,
		void * const dst,
		const uint32_t dst_size);

static error_t uart16550_isr(
		const void * const usr_data,
		const uint32_t vector);

static const kernel_device_t UART16550_DEVICE_INFO =
{
		.initialise = uart16550_initialise,
		.write_register = uart16550_write_register,
		.read_register = uart16550_read_register,
		.write_buffer = uart16650_write_buffer,
		.read_buffer = uart16550_read_buffer,
		.isr = uart16550_isr,
};

static void serial_process_lsr(
		uart_16550_line_status_t * info,
		uint8_t val);

static void serial_process_msr(
		uart_16550_modem_status_t *info,
		uint8_t val);

void uart16550_get_device(
		const void * const base_address,
		kernel_device_t * const device)
{
	kernel_device_t new_device;
	util_memcpy(
			&new_device,
			&UART16550_DEVICE_INFO,
			sizeof(UART16550_DEVICE_INFO));

	new_device.user_data = base_address;

	if (device)
	{
		util_memcpy(
				device,
				&new_device,
				sizeof(kernel_device_t));
	}
}

error_t uart16550_initialise(
		kernel_device_t * device_info,
		void * param,
		const uint32_t param_size)
{
	error_t ret = NO_ERROR;
	uart_16550_device_map_t *raw_com_port = NULL;

	raw_com_port = (uart_16550_device_map_t*)device_info->user_data;

	/* check the device exists - params should be empty*/
	if (raw_com_port == NULL || param || param_size)
	{
		ret = DEVICE_USER_DATA_INVALID;
	}
	else
	{
		/* set DLAB bit in LCR */
		out_u8(&raw_com_port->reg3_lcr, 0x80u);

		/* set the low byte for 192,000 baud */
		out_u8(&raw_com_port->reg0_tx_rx_ls, 0x06u);
		out_u8(&raw_com_port->reg1_ier_ms, 0x00u);

		/* no parity, 8bits, 1 stop bit */
		out_u8(&raw_com_port->reg3_lcr, 0x03u);

		/* set up the FCR */
		out_u8(&raw_com_port->reg2_iir, 0xC7u);
		out_u8(&raw_com_port->reg4_mcr, 0x03u);


		/* set up the interrupts */
		/* FOR SIMULATOR USE 0x0E coz READ IS BLOCKING */
		/* RXTX empty -> raw_com_port->reg1 = 0x0E;*/
		/* TX empty -> raw_com_port->reg1 = 0x0E;*/
		out_u8(&raw_com_port->reg1_ier_ms, 0x0Cu);
	}

	return ret;
}

error_t uart16550_read_register(const void * const usr_data, const uint32_t id, uint32_t * const val)
{
	error_t ret = NO_ERROR;
	uart_16550_device_map_t *raw_com_port = NULL;

	raw_com_port = (uart_16550_device_map_t*)usr_data;

	/* check the device exists */
	if ( raw_com_port == NULL)
	{
		ret = DEVICE_USER_DATA_INVALID;
	}
	else
	{
		uint8_t* dst = (uint8_t*)val + 3;
		switch(id)
		{
			case REG_0_RX_TX_LS:
				*dst = in_u8(&raw_com_port->reg0_tx_rx_ls);
				break;
			case REG_1_ISR_MS:
				*dst = in_u8(&raw_com_port->reg1_ier_ms);
				break;
			case REG_2_IIR:
				*dst = in_u8(&raw_com_port->reg2_iir);
				break;
			case REG_3_LCR:
				*dst = in_u8(&raw_com_port->reg3_lcr);
				break;
			case REG_4_MCR:
				*dst = in_u8(&raw_com_port->reg4_mcr);
				break;
			case REG_5_LSR:
				*dst = in_u8(&raw_com_port->reg5_lsr);
				break;
			case REG_6_MSR:
				*dst = in_u8(&raw_com_port->reg6_msr);
				break;
			case REG_7_SCRATCH:
				*dst = in_u8(&raw_com_port->reg7_scratch);
				break;
			default:
				ret = DEVICE_REGISTER_INVALID;
				break;
		}
	}

	return ret;
}

error_t uart16550_write_register(const void * const usr_data, const uint32_t id, const uint32_t val)
{
	error_t ret = NO_ERROR;
	uart_16550_device_map_t *raw_com_port = NULL;

	raw_com_port = (uart_16550_device_map_t*)usr_data;

	/* check the device exists */
	if ( raw_com_port == NULL)
	{
		ret = DEVICE_USER_DATA_INVALID;
	}
	else
	{
		const uint8_t sval = (uint8_t)val;
		switch(id)
		{
			case REG_0_RX_TX_LS:
				out_u8(&raw_com_port->reg0_tx_rx_ls, sval);
				break;
			case REG_1_ISR_MS:
				out_u8(&raw_com_port->reg1_ier_ms, sval);
				break;
			case REG_2_IIR:
				out_u8(&raw_com_port->reg2_iir, sval);
				break;
			case REG_3_LCR:
				out_u8(&raw_com_port->reg3_lcr, sval);
				break;
			case REG_4_MCR:
				out_u8(&raw_com_port->reg4_mcr, sval);
				break;
			case REG_5_LSR:
				out_u8(&raw_com_port->reg5_lsr, sval);
				break;
			case REG_6_MSR:
				out_u8(&raw_com_port->reg6_msr, sval);
				break;
			case REG_7_SCRATCH:
				out_u8(&raw_com_port->reg7_scratch, sval);
				break;
			default:
				ret = DEVICE_REGISTER_INVALID;
				break;
		}
	}

	return ret;
}

error_t uart16650_write_buffer(
		const void * const usr_data,
		const uint32_t dst,
		const void * const src,
		const uint32_t src_size)
{
	error_t retval = NO_ERROR;
	uart_16550_device_map_t *raw_com_port = NULL;

	/* check that we're intending to read
	 * from the FIFO */
	if ( dst != 0 )
	{
		retval = DEVICE_WRITE_BAD_ADDRESS;
	}

	raw_com_port = (uart_16550_device_map_t*)usr_data;

	/* check the device exists */
	if ( raw_com_port == NULL)
	{
		retval = DEVICE_USER_DATA_INVALID;
	}

	if ( retval == NO_ERROR )
	{
		for(uint32_t counter=0 ; counter < src_size ; counter++ )
		{
			while ( (in_u8(&raw_com_port->reg6_msr) & 0x10) != 0x00 )
			{
			}

			out_u8(&raw_com_port->reg0_tx_rx_ls, (((uint8_t*)src) + counter)[0]);
			out_u8(&raw_com_port->reg4_mcr, 3);
			while ( (in_u8(&raw_com_port->reg5_lsr) & 0x40) != 0x40 )
			{
			}
			out_u8(&raw_com_port->reg4_mcr, 1);
		}
	}

	return retval;
}

error_t uart16550_read_buffer(
		const void * const usr_data,
		const uint32_t src,
		void * const dst,
		const uint32_t dst_size)
{
	error_t retval = NO_ERROR;
	uart_16550_device_map_t *raw_com_port = NULL;

	/* check that we're intending to read
	 * from the FIFO */
	if ( src != 0 )
	{
		retval = DEVICE_READ_BAD_ADDRESS;
	}

	raw_com_port = (uart_16550_device_map_t*)usr_data;

	/* check the device exists */
	if ( raw_com_port == NULL)
	{
		retval = DEVICE_USER_DATA_INVALID;
	}

	if ( retval == NO_ERROR )
	{
		for(uint32_t counter=0 ; counter < dst_size ; counter++ )
		{
			uint8_t raw_input = 0;

			/* this code extract waits for data on the
			 * com port and stores it */
			while( (in_u8(&raw_com_port->reg5_lsr) & LSR_DR) != LSR_DR)
			{
			}

			/* read the actual data */
			while( (raw_input = in_u8(&raw_com_port->reg0_tx_rx_ls)) == 0)
			{
			}

			*(((uint8_t*)dst) + counter) = raw_input;
		}
	}

	return retval;
}

error_t uart16550_isr(
		const void * const usr_data,
		const uint32_t vector)
{
	error_t retval = NO_ERROR;
	uart_16550_device_map_t *raw_com_port = NULL;

	raw_com_port = (uart_16550_device_map_t*)usr_data;

	/* check the device exists */
	if ( raw_com_port == NULL && vector)
	{
		retval = DEVICE_USER_DATA_INVALID;
	}
	else
	{
		uint8_t ir_iden = 0;
		ir_iden = in_u8(&raw_com_port->reg2_iir);

		/* interrupt is pending, get the id */
		if  ( (ir_iden & 0x01) == 0x00)
		{
			if ( (ir_iden & INT_RECV_LINE_STATUS) == INT_RECV_LINE_STATUS)
			{
				/* Read the line status register */
				uart_16550_line_status_t lsr;
				serial_process_lsr(&lsr,raw_com_port->reg5_lsr);
			}
			else if ( (ir_iden & INT_RECV_DATA_AVAIL) == INT_RECV_DATA_AVAIL)
			{
				const uint8_t new_data = raw_com_port->reg0_tx_rx_ls;
				if (new_data)
				{

				}
			}
			else if ( (ir_iden & INT_CHAR_TIMEOUT) == INT_CHAR_TIMEOUT)
			{
				/* Read the RX buffer */
			}
			else if ( (ir_iden & INT_TRAN_EMPTY) == INT_TRAN_EMPTY)
			{
				/* Read the IIR ( already done ) and/or write more data */
			}
			else if ( (ir_iden & INT_STATUS) == INT_STATUS)
			{
				/* Read Modem status register */
				uart_16550_modem_status_t msr;
				serial_process_msr(&msr,raw_com_port->reg6_msr);
			}
			else
			{
				retval = UNKNOWN_INTERRUPT_CAUSE;
			}
		}
	}

	return retval;
}

void serial_process_lsr(
		uart_16550_line_status_t * lsr,
		uint8_t val)
{
	if ( lsr )
	{
		lsr->data_ready = ((val & LSR_DR) == LSR_DR);
		lsr->overrun_error = ((val & LSR_OV_ERR) == LSR_OV_ERR);
		lsr->parity_error = ((val & LSR_PA_ERR)  == LSR_PA_ERR);
		lsr->framing_error = ((val & LSR_FR_ERR) == LSR_FR_ERR);
		lsr->break_interrupt = ((val & LSR_BI_ERR) == LSR_BI_ERR);
		lsr->tx_holding_register = ((val & LSR_TH_ERR) == LSR_TH_ERR);
		lsr->tx_empty = ((val & LSR_TE_ERR) == LSR_TE_ERR);
		lsr->rx_fifo_error = ((val & LSR_RF_ERR) == LSR_RF_ERR);
	}
}

void serial_process_msr(
		uart_16550_modem_status_t *info,
		uint8_t val)
{
	if ( info )
	{
		info->delta_clear_to_send = ((val & MSR_DEL_CTS) == MSR_DEL_CTS);
		info->delta_data_set_ready = ((val & MSR_DEL_DSR) == MSR_DEL_DSR);
		info->trailing_edge_indicator = ((val & MSR_TEI) == MSR_TEI);
		info->delta_data_carrier_detect = ((val & MSR_DEL_DCD) == MSR_DEL_DCD);
		info->clear_to_send = ((val & MSR_CTS) == MSR_CTS);
		info->data_set_ready = ((val & MSR_DSR) == MSR_DSR);
		info->ring_indicator = ((val & MSR_RI) == MSR_RI);
		info->data_carrier_detect = ((val & MSR_DCD) == MSR_DCD);
	}
}
