/*************************************************************************
 *
 * SOS 2 Source Code
 * __________________
 *
 *  [2009] - [2011] Samuel Steven Truscott
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Samuel Truscott and suppliers, if any.
 * The intellectual and technical concepts contained herein
 * are proprietary to Samuel Truscott and its suppliers and
 * may be covered by UK and Foreign Patents, patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Samuel Truscott.
 */

#ifndef SOS2_API_ERRORS_H_
#define SOS2_API_ERRORS_H_

typedef enum error_enum
{
	UNKNOWN_ERROR = 0,
	NO_ERROR,
	OUT_OF_MEMORY,
	SIZE_TOO_SMALL,

	PARAMETERS_NULL,
	PARAMETERS_INVALID,
	PARAMETERS_OUT_OF_RANGE,

	INVALID_CONTEXT,
	OBJECT_TABLE_FULL,
	INVALID_OBJECT,
	OBJECT_ADD_FAILED,
	OBJECT_IN_INVALID_STATE,
	SEMAPHORE_EMPTY,
	ALARM_ID_UNKNOWN,

	DEVICE_ID_INVALID,
	DEVICE_USER_DATA_INVALID,
	DEVICE_REGISTER_INVALID,
	DEVICE_READ_BAD_ADDRESS,
	DEVICE_WRITE_BAD_ADDRESS,
	DEVICE_OPERATION_NOT_SUPPORTED,

	UNEXPECTED_INTERRUPT,
	UNKNOWN_INTERRUPT_CAUSE,
	UNKNOWN_EXTERNAL_INTERRUPT_VECTOR,
	INTERRUPT_NO_EXT_VECTOR_SPECIFIED,
} error_t;

#endif /* SOS2_API_ERRORS_H_ */
