#include <stdint.h> // uint8_t
#include <string.h> //memecpy
#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include "CAN_utils.h"
//#include "circularByteBuffer.h"
#include "stm32f4xx_hal.h"
/*
* Little-endian e.g., 0x87654321 0x87 = a [3],.. 0x21 = a [0]  
*/
//#define DISABLE_CAN1_TX_PENDING_WAIT

static void Error_Handler(char * str);

extern CAN_TxHeaderTypeDef txHeader; //declare a specific header for message transmittions
extern CAN_RxHeaderTypeDef rxHeader; //declare header for message reception
extern uint32_t TxMailbox;
extern CAN_FilterTypeDef sFilterConfig; //declare CAN filter structure
extern circularByteBuffer_t cb_han;
extern CAN_HandleTypeDef hcan1;


bool init_CAN_bus (void) {

	/*
	 * CAN_TxHeaderTypeDef txHeader, CAN_RxHeaderTypeDef rxHeader are for future development
	 * For time being, simply reset the circular buffer of CAN-Bus
	 */
	return circularByteBuffer_Reset(&cb_han);

}


int32_t cb_read_CAN_frame(uint8_t *arr)
{

	uint32_t cb_element;
	int32_t len;
	int i;
	uint8_t arr_val [8];

	printf("\n\rcb_read_CAN_frame() entered \n\r");
	len = circularByteBuffer_Count(&cb_han);
	printf("\n\rcircularByteBuffer length %d  \n\r", (int)len);

	if (len > 0 ) //CAN1_FRAME_SIZE)
	{

#if 1
		//read std_ID
		CANreadElement(&cb_han, &cb_element);
		printf("\n\rCAN ID, 0x%x \n\r", (unsigned int)cb_element);


		//read CAN DLC
		CANreadElement(&cb_han, &cb_element);
		printf("\n\rCAN DLC, 0x%x \n\r", (unsigned int)cb_element);
		len = cb_element;

		//read CAN DATA
		CANreadPayload(&cb_han, arr_val,  len);  //cb_element = CAN DLC
		printf("\n\rCAN payload: ");
		for (i=0; i< len; i++) {
			printf("data[%d] = 0x%x ",i, (unsigned int)arr_val[i]);
		}
		printf("\n\r");
		memcpy(arr, arr_val, len);

#else
		int i = 0;
		//read std_ID
		for (i=0; i<4; i++)
		{
			 circularByteBuffer_Dequeue(&cb_han, &arr_val[i]);
		}

		printf("\n\rCAN std_ID, 0x%x 0x%x 0x%x 0x%x \n\r", arr_val[3] ,arr_val[2] ,arr_val[1] ,arr_val[0]);

		//read DLC
		for (i=0; i<4; i++)
		{
			arr_val[i] = circularByteBuffer_Dequeue(&cb_han, &arr_val[i]);
		}

		printf("\n\rCAN DLC, 0x%x 0x%x 0x%x 0x%x \n\r", arr_val[3] ,arr_val[2] ,arr_val[1] ,arr_val[0]);

		//read data
		for (i=0; i<4; i++)
		{
			arr_val[i] = circularByteBuffer_Dequeue(&cb_han, &arr_val[i]);
		}

		printf("\n\rCAN DATA, 0x%x 0x%x 0x%x 0x%x \n\r", arr_val[3] ,arr_val[2] ,arr_val[1] ,arr_val[0]);
#endif
	}

	return len;

}



void EnqueueFrame(int id, int dlc, uint8_t * arr)
{
	uint8_t arr_val[4];
	int i;
	/*
		 * Store the CAN frame in this structure only
		 * i.e., | rxHeader.StdId (4 bytes) | rxHeader.DLC (4 bytes) | data (4 bytes) |
		 *
		 * Note: the term "element" depicts data type of uint32_t
		 */

	/* convert rxHeader.StdId to 4 bytes */
    circularByteBuffer_Int2Bytes(id, arr_val);

	/* Store rxHeader.StdId, i.e, arr_val in the circular buffer */
	circularByteBuffer_element_Enqueue(&cb_han, arr_val);
	for (i=0; i<4; i++)
	{
		printf("\n\rEnqueue id[%d] %x \n\r", i, arr_val[i]);
	}


	/* convert rxHeader.DCL to bytes */
	circularByteBuffer_Int2Bytes(dlc, arr_val);
	/* Store rxHeader.StdId, i.e, arr_val in the circular buffer */
	circularByteBuffer_element_Enqueue(&cb_han, arr_val);

	for (i=0; i<4; i++)
	{
		printf("\n\rEnqueue DCL[%d] %x \n\r", i, arr_val[i]);
	}

	/* Store in the circular buffer byte by byte*/
	for (i=0; i< dlc; i++) {
		circularByteBuffer_Enqueue(&cb_han, arr[i]);
	}

}


void circularByteBuffer_Int2Bytes(uint32_t num, uint8_t *a)
{
 
  a[3] = (num>>24) & 0xFF;
  printf("\n\r num 0x%x a[3] = %x\n\r", (unsigned int)num, (unsigned int)a[3]);

  a[2] = (num>>16) & 0xFF;
  printf("\n\r num 0x%x a[2] = %x\n\r", (unsigned int)num, (unsigned int)a[2]);

  a[1] = (num>>8) & 0xFF;
  printf("\n\r num 0x%x a[1] = %x\n\r", (unsigned int)num, (unsigned int)a[1]);

  a[0] = num & 0xFF;
  printf("\n\r num 0x%x a[0] = %x\n\r", (unsigned int)num, (unsigned int)a[0]);

}

/*
* Little-endian 0x87654321 0x87 = a [3],.. 0x21 = a [0]
*/
void circularByteBuffer_Bytes2Int(uint8_t *a, uint32_t *num)
{

  *num = ((a[3]<<24) & 0xFF000000) |( (a[2]<<16) & 0x00FF0000 )|( (a[1]<<8) & 0x0000FF00 )|( a[0] & 0x000000FF);

}

void CANreadElement(circularByteBuffer_t *cb, uint32_t *num)
{

  uint8_t a[4]; 
  
  circularByteBuffer_Dequeue(cb, &a[0]);
  circularByteBuffer_Dequeue(cb, &a[1]);
  circularByteBuffer_Dequeue(cb, &a[2]);
  circularByteBuffer_Dequeue(cb, &a[3]);
  
  circularByteBuffer_Bytes2Int(a, num);

}

void CANreadPayload(circularByteBuffer_t *cb, uint8_t *a, uint32_t len)
{

  uint32_t i=0;

  for (i=0; i<len; i++) { 
    circularByteBuffer_Dequeue(cb, a+i);
  }
  
}

int CAN1_TX(void)
{
	/*
	 * Frame payload length DLC
	 */
	txHeader.DLC=8;  			// give message size of 8 byte FOR NOW
	/*
	 * Frame ID
	 */
	txHeader.StdId=0x123;

	txHeader.IDE = CAN_ID_STD;
	txHeader.RTR = CAN_RTR_DATA;
	txHeader.TransmitGlobalTime = DISABLE;

	if ((CAN1_TX_MAILBOX_FREE_level()) != HAL_OK)
	{
		return 1;
	}

	uint8_t txData[8] = {0xAA,0x01,0x02,0x03,0x04,0x05,0x06,0x07};

	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, (uint32_t*)&TxMailbox) != HAL_OK) {
		printf("\n\rHAL_CAN_AddTxMessage() Failed \n\r");
		Error_Handler("\n\rCAN1_TX\n\r");

	}
	 printf("\n\rHAL_CAN_AddTxMessage() Passed \n\r");

	 return 0;

//#ifndef DISABLE_CAN1_TX_PENDING_WAIT
//	CAN1_TX_pending_wait();
//#endif
}

int CAN1_TX_MAILBOX_FREE_level(void)
{
	int i=0;
	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0)
	{
		i++;
		if(i>0xfffe)
			return 1;
	}
	HAL_Delay(500);
	return 0;
}

void CAN1_TX_pending_wait(void) {

	while (HAL_CAN_IsTxMessagePending(&hcan1, (long int)&TxMailbox) != HAL_OK) {
		printf("\n\rHAL_CAN_IsTxMessagePending() failed\n\r");
		HAL_Delay(10);
	}

}

static void Error_Handler(char * str)
{
	while (1) {
		printf("\n\r%s Failed \n\r", str);
		HAL_Delay(1000);
	}
}

