/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fatfs_sd.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <stdbool.h>



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


struct {
  float voltage;
  float current;
  float power;
	float energy;
  float frequecy;
  float power_factor; // 24th11
  uint16_t alarms;
} currentValues;

struct {
  float voltage;
  float current;
  float power;
	float energy;
  float frequecy;
  float power_factor;
  uint16_t alarms;
} check;


uint8_t buf[8] = {0xF8, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x64, 0x64 }; // CRC 0x6464
uint8_t rst_buf[4] = {0xF8, 0x42, 0xC2, 0x41}; // CRC 0x41C2
uint8_t res_buf[25];
uint8_t flag =0;       // ko dung
uint8_t rst_flag =0;   // ko dung
uint8_t scr_flag = 0;  // ko dung

// nhan du lieu dien nang trong main
float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;
float power_factor = 0; 

float current_energy = 0;  
float previous_energy = 0;  
float previous_month_energy = 0;
float tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc = 0;
float dien_nang_tieu_thu_trong_ngay_hom_nay = 0;
float dien_nang_tieu_thu_trong_ngay_hom_truoc = 0;
char thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua[7];

char du_lieu_nhan_duoc_tu_esp[2];

void clear_res_buf (void) // chuc nang xoa bo dem
{
	for (int i=0; i<25; i++)
	{
		res_buf[i] = '\0';
	}
}

void clear_thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua (void) 
{
	for (int i=0; i<7; i++)
	{
		thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua[i] = '\0';
	}
}


// begin - CRC16
static const uint16_t wCRCTable[] = {
0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

uint16_t MB_CRC16 (const uint8_t *nData, uint16_t wLength)
{
uint8_t nTemp;
uint16_t wCRCWord = 0xFFFF;

   while (wLength--)
   {
      nTemp = *nData++ ^ wCRCWord;
      wCRCWord >>= 8;
      wCRCWord ^= wCRCTable[nTemp];
   }
   return wCRCWord;

}
// end - CRC16

/*
 * checkCRC
 *
 * Performs CRC check of the buffer up to len-2 and compares check sum to last two bytes
 *
 * @param[in] data Memory buffer containing the frame to check
 * @param[in] len  Length of the respBuffer including 2 bytes for CRC
 *
 * @return is the buffer check sum valid
*/

bool checkCRC(const uint8_t *buf, uint16_t len){
    if(len <= 2) // Sanity check
        return false;

    uint16_t crc = MB_CRC16(buf, len - 2); // Compute CRC of data
    return ((uint16_t)buf[len-2]  | (uint16_t)buf[len-1] << 8) == crc;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART5_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C2_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t BCD2DEC(uint8_t data);
uint8_t DEC2BCD(uint8_t data);
#define DS3231_ADD 0x68 // 01101000, 0x68<<1 = 11010000
uint8_t BCD2DEC(uint8_t data)
{
  return(data>>4)*10+(data&0x0f);
}

uint8_t DEC2BCD(uint8_t data)
{
 return(data/10)<<4|(data%10);
}

uint8_t receive_data_i2c[7], send_data_i2c[7];
uint8_t second, minute, hour, day, date, month, year;
uint8_t data_buffer_checktime[60]; // 
//uint8_t data_buffer_print_ds3231[60]; //  chua du lieu chuoi ds3231 trong ham Get_Time()
uint8_t data_buffer_print_energy_and_current_energy[70]; // du dai de chua du lieu chuoi pzem
uint8_t data_buffer_print[170]; // du dai de chua du lieu chuoi pzem + ds3232 .
uint8_t du_lieu_gui_toi_lora[170]; // chuoi du lieu lora max la 124
uint8_t do_dai_du_lieu_gui_toi_lora[170];

uint8_t today, current_month, current_year;
uint8_t previous_date, previous_month, previous_year;


void clear_send_data_i2c (void) 
{
	for (int i=0; i<7; i++)
	{
		send_data_i2c[i] = '\0';
	}
}

void clear_receive_data_i2c (void) 
{
	for (int i=0; i<7; i++)
	{
		receive_data_i2c[i] = '\0';
	}
}

// begin - HAL_GPIO_EXTI_Callback - Dung de set thoi gian
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{	
	if(GPIO_Pin == GPIO_PIN_0)
	{
		// 12h:12m:12s Monday 12/12/12
		send_data_i2c[0] = DEC2BCD(30);  // second
		send_data_i2c[1] = DEC2BCD(48);  // minute
		send_data_i2c[2] = DEC2BCD(16);  // hour
		
		send_data_i2c[3] = DEC2BCD(6);   // day  , 1->7, CN->T7
		send_data_i2c[4] = DEC2BCD(15);  // date 30/10/2023
		send_data_i2c[5] = DEC2BCD(12);  // month 
		send_data_i2c[6] = DEC2BCD(23);  // year
		
		HAL_I2C_Mem_Write(&hi2c2, DS3231_ADD <<1,0x00, I2C_MEMADD_SIZE_8BIT, send_data_i2c,7,2000);
    uint8_t check_gpio_exti_callback[] ="GPIO_EXTI_Callback\n";
    HAL_UART_Transmit(&huart5,check_gpio_exti_callback,strlen((char*)check_gpio_exti_callback), 500);		
		clear_send_data_i2c();		
	}
}
// end - HAL_GPIO_EXTI_Callback - Dung de set thoi gian

void send_uart (char *string)
{
 uint8_t len=strlen(string);
 HAL_UART_Transmit( &huart5, (uint8_t *) string, len, 2000); // transmit in blocking mode 
}

/* // begin - HAL_UART_RxCpltCallback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//if(huart->Instance == huart4.Instance)
	if(huart->Instance == UART4)	
  {	
		send_uart("Phat hien ngat uart4.\n");	
		if (du_lieu_nhan_duoc_tu_esp[0] == 's')	
		{
      // Thuc hien lenh neu hai chuoi giong nhau
			send_uart("Da nhan duoc yeu cau gui lai du lieu lora tu esp.\n");
			uint8_t TxBuffer[170] = "21:13:05 02/12/2023 223.4 V 0.2700 A 57.7 W 0.0500 kWh 0.96 PF 0.0090 kWh 0.0000 kWh 01/12 0.3270 kWh";
			HAL_UART_Transmit(&huart4, TxBuffer, strlen((char *)TxBuffer), 2000);
			send_uart("Da gui du lieu sang esp.\n");
		} 
		else 
		{
			// Neu 2 chuoi khong giong nhau
			send_uart("Du lieu phan hoi tu esp khong khop.\n");
		}
		HAL_UART_Receive_IT(&huart4,(uint8_t *)du_lieu_nhan_duoc_tu_esp, 1); // Kich hoat lai ham ngat
		send_uart("Kich hoat lai ham ngat.\n");
  }
}
*/ // end - HAL_UART_RxCpltCallback

void Get_Time(void)
{ 
	HAL_I2C_Mem_Read(&hi2c2, DS3231_ADD << 1,0x00, I2C_MEMADD_SIZE_8BIT, receive_data_i2c, 7,2000); 
	second =BCD2DEC(receive_data_i2c[0]);
	minute =BCD2DEC(receive_data_i2c[1]);
  hour =BCD2DEC(receive_data_i2c[2]);

  day =BCD2DEC(receive_data_i2c[3]);
  date =BCD2DEC(receive_data_i2c[4]);
  month =BCD2DEC(receive_data_i2c[5]);
  year =BCD2DEC(receive_data_i2c[6]);	
	
	uint8_t data_buffer_print_ds3231[60]; //  chua du lieu chuoi ds3231 trong ham Get_Time()
	sprintf((char *)data_buffer_print_ds3231, "%02d:%02d:%02d %02d/%02d/20%02d\n", hour, minute, second, date, month, year); // chi can them \n la co the xuong dong
	HAL_UART_Transmit(&huart5, data_buffer_print_ds3231, strlen((char *)data_buffer_print_ds3231), 2000);
	clear_receive_data_i2c();	 	
}

void Get_Time_DS3231(void)
{ 
	HAL_I2C_Mem_Read(&hi2c2, DS3231_ADD << 1,0x00, I2C_MEMADD_SIZE_8BIT, receive_data_i2c, 7,1000); 
	second =BCD2DEC(receive_data_i2c[0]);
	minute =BCD2DEC(receive_data_i2c[1]);
  hour =BCD2DEC(receive_data_i2c[2]);

  day =BCD2DEC(receive_data_i2c[3]);
  date =BCD2DEC(receive_data_i2c[4]);
  month =BCD2DEC(receive_data_i2c[5]);
  year =BCD2DEC(receive_data_i2c[6]);	
	
	//sprintf((char *)data_buffer_print_ds3231, "%02d:%02d:%02d %02d/%02d/20%02d", hour, minute, second, date, month, year);
	//HAL_UART_Transmit(&huart5, data_buffer_print_ds3231, strlen((char *)data_buffer_print_ds3231), 1000);
		 
}

void clear_data_buffer_print (void) // Ham xoa du lieu cu cua chuoi pzem + ds3231 
{
	for (int i=0; i<170; i++) 
	{
		data_buffer_print[i] = '\0'; 
	}
} 

//
char string_test_hour[100]; //  test hour bang  send_uart
char string_test_hour_HAL[100]; //  test hour bang HAL uart
char string_test_hour_send_02d[100]; // test hour bang 02d

void clear_string_test_hour (void) // chuc nang xoa bo dem cua hour
{
	for (int i=0; i<100; i++)
	{
		string_test_hour[i] = '\0';
	}
}

void clear_string_test_hour_HAL (void) // clear buffer // chuc nang xoa bo dem
{
	for (int i=0; i<100; i++)
	{
		string_test_hour_HAL[i] = '\0';
	}
}

void clear_string_test_hour_send_02d (void) 
{
	for (int i=0; i<100; i++)
	{
		string_test_hour_send_02d[i] = '\0';
	}
}
//
void clear_data_buffer_print_energy_and_current_energy (void) 
{
	for (int i=0; i<70; i++) 
	{
		data_buffer_print_energy_and_current_energy[i] = '\0';
	}
}

int bufsize_uint8_t (uint8_t *buf)
{
	int i=0;
	while (*buf++ != '\0') i++;
	return i;
}


// Ham con check CRC , TH CRC sai
bool Check_CRC_Lan1(bool result)
{ 
	
	if (result == false)
	{	
		int i=1;
		while(result==false) 
		{	
				
			if (i>3)
			{
				result = false; // That bai
				send_uart("ERROR CRC\n");
				break;
			}
			clear_res_buf();
			HAL_UART_Transmit(&huart2, (uint8_t*) &buf, 8, 1000); 
			HAL_UART_Receive(&huart2, res_buf, 25, 1000); 
			Get_Time_DS3231(); // Nhan du lieu thoi gian
			result = checkCRC(res_buf, sizeof(res_buf) / sizeof(res_buf[0]));
			if (result == true) // Thanh Cong
			{
				break;
			}
			else {i=i+1;}				
		}	
	}
	
	return result;  // Thanh Cong or That Bai
}

//

bool Check_Gia_Tri_Pzem(bool result)
{
		// Ham con check giatri , TH giatri sai
		if(result == true)
		{
			currentValues.voltage = ((uint32_t) res_buf[3] << 8 | ( uint32_t) res_buf[4])/10.0 ; // neu thuc te la 220.0V, decimal la 2200(raw voltage in 0.1V), muon hien thi thuc te thi /10.0

			currentValues.current = ((uint32_t) res_buf[5] << 8 | (uint32_t) res_buf[6] | (uint32_t) res_buf[7] <<24 | (uint32_t) res_buf[8] << 16) / 1000.0 ; //neu thuc te la 1.000A, decimal la 1000 (raw voltage in 0.001A), muon hien thi thuc te thi /1000.0
	
			currentValues.power = ((uint32_t) res_buf[9] <<8 | (uint32_t) res_buf[10] | (uint32_t) res_buf[11] <<24 | (uint32_t) res_buf[12] << 16) / 10.0; // neu thuc te la 220.0W, decimal la 2200(raw voltage in 0.1W), muon hien thi thuc te thi /10.0

			currentValues.energy = ((uint32_t) res_buf [13] << 8| (uint32_t) res_buf[14] | (uint32_t) res_buf[15] << 24 | (uint32_t) res_buf[16] << 16) /1000.0; // decimal la don vi Wh, neu muon thanh KWh thì /1000.0, (//raw energy in 1Wh ( moi so don vi cua decimal))
		
			currentValues.power_factor = ((uint32_t) res_buf [19] << 8 | (uint32_t)res_buf [20])/100.0; 
					
			if ((currentValues.voltage > 260) || (currentValues.current >100) || (currentValues.power >23000) || (currentValues.energy > 9999 ) || (currentValues.power_factor >1))
			{
				int i=0;
				while(((currentValues.voltage > 260) || (currentValues.current >100) || (currentValues.power >23000) || (currentValues.energy > 9999 ) || (currentValues.power_factor >1)) == true)
				{	
					if (i>3)
					{
						result = false;
						send_uart("ERROR Gia Tri Pzem\n");
						break;
					}
					clear_res_buf();
					HAL_UART_Transmit(&huart2, (uint8_t*) &buf, 8, 1000); // yeu cau doc lai du lieu tu pzem
					HAL_UART_Receive(&huart2, res_buf, 25, 1000); // nhan du lieu phan hoi tu pzem
					Get_Time_DS3231(); // Nhan du lieu thoi gian
					result = checkCRC(res_buf, sizeof(res_buf) / sizeof(res_buf[0]));
					if (result == true)
					{ 
							
						currentValues.voltage = ((uint32_t) res_buf[3] << 8 | ( uint32_t) res_buf[4])/10.0 ; 

						currentValues.current = ((uint32_t) res_buf[5] << 8 | (uint32_t) res_buf[6] | (uint32_t) res_buf[7] <<24 | (uint32_t) res_buf[8] << 16) / 1000.0 ; 
	
						currentValues.power = ((uint32_t) res_buf[9] <<8 | (uint32_t) res_buf[10] | (uint32_t) res_buf[11] <<24 | (uint32_t) res_buf[12] << 16) / 10.0; 

						currentValues.energy = ((uint32_t) res_buf [13] << 8| (uint32_t) res_buf[14] | (uint32_t) res_buf[15] << 24 | (uint32_t) res_buf[16] << 16) /1000.0; 
		
						currentValues.power_factor = ((uint32_t) res_buf [19] << 8 | (uint32_t)res_buf [20])/100.0; 
						
						if (((currentValues.voltage > 260) || (currentValues.current >100) || (currentValues.power >23000) || (currentValues.energy > 9999 ) || (currentValues.power_factor >1))==false)
						{	
							result = true;
							break;//
						}
						i=i+1;
					}
					
					else {i=i+1;}		
				}
			}
			
		}
		return result;
}
//

// Begin- Ham Check 25 byte phan hoi tu Pzem
bool Gui_Lai_Lenh_Doc_Du_Lieu_Pzem(void)
{
	clear_res_buf();
	HAL_UART_Transmit(&huart2, (uint8_t*) &buf, 8, 1000); 
	HAL_UART_Receive(&huart2, res_buf, 25, 1000); 
	int res_buf_len = bufsize_uint8_t(res_buf);
	uint8_t res_buf_position_1 = 0;
	uint8_t ket_qua_res_buf_position_1[20];
	
	res_buf_position_1 = res_buf[1];
	sprintf((char *) ket_qua_res_buf_position_1, "%d\n", res_buf_position_1);
	HAL_UART_Transmit(&huart5, ket_qua_res_buf_position_1, strlen((char *)ket_qua_res_buf_position_1), 200);
	
	int i = 0;
	while(res_buf_position_1 != 0x04)
	{	
		
		if(i>2)
		{
			return 0; // 0 = false
			break;
		}
		if (res_buf_position_1 == 0x84) 
		{
			send_uart("Lenh doc lieu stm32 den pzem co van de, 0x84 phan hoi.\n");			
		}
		
		else
		{
			send_uart("Lenh doc lieu stm32 den pzem co van de, Truong Hop Khac.\n");
		}
		i=i+1;
		
		clear_res_buf();
		HAL_UART_Transmit(&huart2, (uint8_t*) &buf, 8, 1000); // yeu cau doc du lieu tu pzem
		HAL_UART_Receive(&huart2, res_buf, 25, 1000); // nhan du lieu phan hoi tu pzem
		res_buf_position_1 = res_buf[1];
	}
	
	if(res_buf_position_1 == 0x04)
	{
     // Tiep tuc xu ly du lieu phan hoi CRC, check gia tri pzem
    bool result = checkCRC(res_buf, sizeof(res_buf) / sizeof(res_buf[0]));
		if (result == false)
		{
			result = Check_CRC_Lan1(result); // Ham con check CRC , TH CRC sai
		}	
		if (result == true)
		{	
			result = Check_Gia_Tri_Pzem(result); // Ham con check gia tri pzem , TH CRC dung
		}	
			return result; // Thanh cong or that bai
  }
	
	//
	return 0; // 0 = ERROR, 1 = OKE
}
//  End- Ham Check 25 byte phan hoi tu Pzem

// Begin - Ham check  25 Byte , CRC va Gia tri Pzem sau khi check CRC
bool Check_Data_Pzem() 
{		
	int res_buf_len = bufsize_uint8_t(res_buf);
	uint8_t res_buf_position_1 = 0;
	uint8_t ket_qua_do_dai_res_buf[20];
	uint8_t ket_qua_res_buf_position_1[20];
	
	res_buf_position_1 = res_buf[1];
	if(res_buf_position_1 == 0x04)
	{
		bool result = checkCRC(res_buf, sizeof(res_buf) / sizeof(res_buf[0]));
		if (result == false)
		{
			result = Check_CRC_Lan1(result); // Ham con check CRC , TH CRC sai
		}	
		if (result == true)
		{	
			result = Check_Gia_Tri_Pzem(result); // Ham con check gia tri pzem , TH CRC dung
		}	
			return result; // Thanh cong or that bai
	}
	
	else if (res_buf_position_1 == 0x84)
	{
		send_uart("Lenh doc lieu stm32 den pzem co van de, 0x84 phan hoi.\n");
		Gui_Lai_Lenh_Doc_Du_Lieu_Pzem();		
	}
	
	else
	{
		send_uart("Lenh doc lieu stm32 den pzem co van de, Truong Hop Khac.\n");
		Gui_Lai_Lenh_Doc_Du_Lieu_Pzem();
	}
	
	return 1; 
}
// End - Ham check  25 Byte , CRC va Gia tri Pzem sau khi check CRC


//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
// ------------ BEGIN CODE CUA SD CARD -------------------

FATFS fs; // file system 
FIL fil; // file
FRESULT fresult;  // luu tru ket qua cua tung thao tac 
char buffer[1024]; //  bo dem de luu tru du lieu ma chung ta co the doc,ghi
//
char buffer_doc_tat_ca_du_lieu_sdcard[60];  //  de test - luu tru du lieu ma chung ta co the doc,ghi// de test doc tat ca du lieu 

char buffer_doc_tat_ca_du_lieu_eeproom[20]; // De doc tat ca du lieu eeproom

char data_buffer_to_sdcard[170]; //Dung de ghi du lieu chuoi pzem +ds3231 vao file chinh

char data_buffer_to_eeproom[50]; //Dung de ghi du lieu eeproom vao file1

char data_buffer_dmy_to_eeproom[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)

uint8_t ket_qua_fresult[100];
static int ghi_lien_tuc_vao_the =10;

UINT br, bw ;  // luu bo dems ghi va doc

// cac bien lien quan de kiem tra dung luong cua the
FATFS *pfs; 		 // Dung de doc dung luong
DWORD fre_clust; // unsigned long - Dung de doc dung luong
uint32_t total, free_space;

// dung de tim kich thuoc du lieu trong bo dem
int bufsize (char *buf)
{
	int i=0;
	while (*buf++ != '\0') i++;
	return i;
}

void bufclear (void) // clear buffer 
{
	for (int i=0; i<1024; i++)
	{
		buffer[i] = '\0';
	}
}

void clear_buffer_doc_tat_ca_du_lieu_sdcard (void) 
{
	for (int i=0; i<60; i++)
	{
		buffer_doc_tat_ca_du_lieu_sdcard[i] = '\0';
	}
}

void clear_buffer_doc_tat_ca_du_lieu_eeproom (void) 
{
	for (int i=0; i<20; i++)
	{
		buffer_doc_tat_ca_du_lieu_eeproom[i] = '\0';
	}
}

// void  clear_ket_qua_fresult , ket_qua_fresult thuong in ra ket qua cua fresult - uart 
void clear_ket_qua_fresult (void) // chuc nang xoa bo dem khi in ra ket qua cua fresult - uart 
{
	for (int i=0; i<100; i++)
	{
		ket_qua_fresult[i] = '\0';
	}
}

void clear_data_buffer_to_sdcard (void)  // chuc nang xoa bo dem khi ghi du lieu vao SD
{
	for (int i=0; i<170; i++)
	{
		data_buffer_to_sdcard[i] = '\0';
	}
}

void clear_data_buffer_to_eeproom (void)  // chuc nang xoa bo dem khi ghi du lieu eeproom (SD card)
{
	for (int i=0; i<50; i++)
	{
		data_buffer_to_eeproom[i] = '\0';
	}
}

void clear_data_buffer_dmy_to_eeproom (void)  // chuc nang xoa bo dem khi ghi du lieu date, month, year vao eeproom (file1)
{
	for (int i=0; i<5; i++)
	{
		data_buffer_dmy_to_eeproom[i] = '\0';
	}
}

////-------------------
void Write_Data_To_Sdcard()
{ 
	HAL_Delay(100);
	fresult = f_open(&fil, "file4.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
  {
		sprintf((char *) ket_qua_fresult, "f_open file4 thanh cong de update - write_data_to_sdcard(), gia tri cua fresult: %d\n", fresult); //85 ky tu
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
		
		//- Move to offset to the end of the file -/
		fresult = f_lseek(&fil,f_size(&fil));
		
		sprintf(data_buffer_to_sdcard, "%02d:%02d:%02d %02d/%02d/20%02d %.1f V %.4f A %.1f W %.4f kWh %.2f PF %.4f kWh %.4f kWh %s %.4f kWh\n", hour, minute, second, date, month, year, voltage, current, power, energy, power_factor, dien_nang_tieu_thu_trong_ngay_hom_nay, dien_nang_tieu_thu_trong_ngay_hom_truoc, thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, previous_month_energy );
		
		fresult = f_write(&fil, data_buffer_to_sdcard, bufsize(data_buffer_to_sdcard), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong vao file4 de update - write_data_to_sdcard(), fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu vua ghi vao write_data_to_sdcard() la :\n"); // 
			send_uart(data_buffer_to_sdcard); // de hien thi len PC du lieu vua ghi vao la gi
    }
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_write du lieu update vao file4 - write_data_to_sdcard(), gia tri cua fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close file4 thanh cong, sau khi ghi du lieu update - write_data_to_sdcard(), fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close file4, sau khi ghi du lieu update - write_data_to_sdcard(), fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		clear_data_buffer_to_sdcard(); // Xoa du lieu cua data_buffer_to_sdcard (du lieu nay ghi vao sd card)
		
	}	// if cua f_open
	
	else // f_open (&fil, "file4.txt", FA_OPEN_ALWAYS | FA_WRITE);
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open file4 de update, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
  }

}

void Write_Current_Energy_To_Eeproom()
{ 
	HAL_Delay(100);
	char data_buffer_to_eeproom_cuc_bo[20]; //Dung de ghi du lieu Current_Energy vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	fresult = f_open(&fil, "file1.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
  {
		sprintf((char *) ket_qua_fresult, "f_open file1 thanh cong, de ghi Current_Energy vao eeproom, fresult: %d\n", fresult); //85 ky tu
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
		
		sprintf(data_buffer_to_eeproom_cuc_bo , "%.4f\n", current_energy);
	
		fresult = f_write(&fil, data_buffer_to_eeproom_cuc_bo , bufsize(data_buffer_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi Current_Energy vao file1, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu Current_Energy vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
    }
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_write Current_Energy vao file1, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close file1 thanh cong, sau khi ghi Current_Energy vao file1, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close file1, sau khi ghi Current_Energy vao file1, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		
	}	// if cua f_open
	
	else 
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open file1, de ghi Current_Energy vao file1, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
  }
}

// Begin - Write_Previous_Date_To_Eeproom_2()
void Write_Previous_Date_To_Eeproom_2()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%d\n", previous_date);
	fresult = f_open(&fil, "previous_date.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open previous_date.txt thanh cong, de doc du lieu previous_date, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi previous_date vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu previous_date vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close previous_date.txt thanh cong, sau khi ghi du lieu previous_date vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close previous_date.txt, sau khi ghi du lieu vao previous_date vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		
	}

}
// End - Write_Previous_Date_To_Eeproom_2() 

// Begin - Write_Today_To_Eeproom_2
void Write_Today_To_Eeproom_2()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%d\n", today);
	fresult = f_open(&fil, "today.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open today.txt thanh cong, de doc du lieu today, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write today thanh cong vao eeproom, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu today vua ghi vao Eeproom() la :\n"); // 
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_write today vao eeproom, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close today.txt thanh cong, sau khi ghi du lieu today vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close today.txt, sau khi ghi du lieu vao today vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		
	}

}

// End - Write_Today_To_Eeproom_2

// Begin - Write_Previous_Month_To_Eeproom()
void Write_Previous_Month_To_Eeproom()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%d\n", previous_month);
	fresult = f_open(&fil, "previous_month.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open previous_month.txt thanh cong, de doc du lieu previous_month, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi previous_month vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu previous_month vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close previous_month.txt thanh cong, sau khi ghi du lieu previous_month vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close previous_month.txt, sau khi ghi du lieu vao previous_month vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		//
	}

}
// End - Write_Previous_Month_To_Eeproom() 

// Begin - Write_Current_Month_To_Eeproom()
void Write_Current_Month_To_Eeproom()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%d\n", current_month);
	fresult = f_open(&fil, "current_month.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open current_month.txt thanh cong, de doc du lieu current_month, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi current_month vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu current_month vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close current_month.txt thanh cong, sau khi ghi du lieu current_month vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close current_month.txt, sau khi ghi du lieu vao current_month vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		//
	}

}
// End - Write_Current_Month_To_Eeproom() 

// Begin - Write_Current_Year_To_Eeproom()
void Write_Current_Year_To_Eeproom()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%d\n", current_year);
	fresult = f_open(&fil, "current_year.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open current_year.txt thanh cong, de doc du lieu current_year, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi current_year vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu current_year vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close current_year.txt thanh cong, sau khi ghi du lieu current_year vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close current_year.txt, sau khi ghi du lieu vao current_year vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		//
	}

}
// End - Write_Current_Year_To_Eeproom() 

// Begin - Write_Previous_Year_To_Eeproom()
void Write_Previous_Year_To_Eeproom()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[5]; //Dung de ghi du lieu date, month, year vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%d\n", previous_year);
	fresult = f_open(&fil, "previous_year.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open previous_year.txt thanh cong, de doc du lieu previous_year, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi previous_year vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu previous_year vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close previous_year.txt thanh cong, sau khi ghi du lieu previous_year vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close previous_year.txt, sau khi ghi du lieu vao previous_year vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		//
	}

}
// End - Write_Previous_Year_To_Eeproom() 

// Begin - Write_Previous_Year_To_Eeproom()
void Write_Previous_Month_Energy_To_Eeproom()
{ 
	
	char data_buffer_to_eeproom_cuc_bo[20]; //Dung de ghi du lieu Current_Energy vao eeproom (file1)
	uint8_t ket_qua_fresult[120];
	sprintf(data_buffer_to_eeproom_cuc_bo , "%.4f\n", previous_month_energy);
	fresult = f_open(&fil, "previous_month_energy.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open previous_month_energy.txt thanh cong, de doc du lieu previous_month_energy, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_to_eeproom_cuc_bo , bufsize(data_buffer_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi previous_month_energy vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu previous_month_energy vua ghi vao Eeproom la :\n"); // 
			send_uart(data_buffer_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close previous_year.txt thanh cong, sau khi ghi du lieu previous_month_energy vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close previous_month_energy.txt, sau khi ghi du lieu vao previous_month_energy vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		//
	}

}
// End - Write_Previous_Year_To_Eeproom() 

// Begin - Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Truoc_To_Eeproom()
void Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Truoc_To_Eeproom()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[20]; //Dung de ghi du lieu vao eeproom 
	uint8_t ket_qua_fresult[130];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%.4f\n", tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc);
	fresult = f_open(&fil, "tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc.txt thanh cong, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc vua ghi vao Eeproom la :\n"); // // de hien thi len PC du lieu vua ghi vao la gi
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);

	}
	else 
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc.txt, de ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
	}

}
// End - Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Truoc_To_Eeproom()

// Begin - Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Truoc_To_Eeproom()
void Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Truoc_To_Eeproom()
{ 
	char data_buffer_dmy_to_eeproom_cuc_bo[20]; //Dung de ghi du lieu vao eeproom 
	uint8_t ket_qua_fresult[130];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%.4f\n", dien_nang_tieu_thu_trong_ngay_hom_truoc);
	fresult = f_open(&fil, "dien_nang_tieu_thu_trong_ngay_hom_truoc.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open dien_nang_tieu_thu_trong_ngay_hom_truoc.txt thanh cong, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi dien_nang_tieu_thu_trong_ngay_hom_truoc vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu dien_nang_tieu_thu_trong_ngay_hom_truoc vua ghi vao Eeproom la :\n"); // // de hien thi len PC du lieu vua ghi vao la gi
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		/* 
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close dien_nang_tieu_thu_trong_ngay_hom_truoc.txt thanh cong, sau khi ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close dien_nang_tieu_thu_trong_ngay_hom_truoc.txt, sau khi ghi du lieu vao  vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		*/
		//
	}
	else 
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open dien_nang_tieu_thu_trong_ngay_hom_truoc.txt, de ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
	}

}
// End - Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Truoc_To_Eeproom()

// Begin - Write_Thoi_Diem_Chot_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Qua()
void Write_Thoi_Diem_Chot_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Qua()
{ 
	////char data_buffer_dmy_to_eeproom_cuc_bo[20]; //Dung de ghi du lieu vao eeproom 
	uint8_t ket_qua_fresult[130];
	////sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%.4f\n", dien_nang_tieu_thu_trong_ngay_hom_truoc);
	fresult = f_open(&fil, "thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt thanh cong, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua , bufsize(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua vua ghi vao Eeproom la :\n"); // // de hien thi len PC du lieu vua ghi vao la gi
			send_uart(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		/* 
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt thanh cong, sau khi ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt, sau khi ghi du lieu vao  vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		*/
		//
	}
	else 
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt, de ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
	}

}
// End - Write_Thoi_Diem_Chot_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Qua()

// Begin - Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Nay_To_Eeproom()
void Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Nay_To_Eeproom()
{ 
	HAL_Delay(100);
	char data_buffer_dmy_to_eeproom_cuc_bo[20]; //Dung de ghi du lieu vao eeproom 
	uint8_t ket_qua_fresult[130];
	sprintf(data_buffer_dmy_to_eeproom_cuc_bo , "%.4f\n", dien_nang_tieu_thu_trong_ngay_hom_nay);
	fresult = f_open(&fil, "dien_nang_tieu_thu_trong_ngay_hom_nay.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
	{
		sprintf((char *) ket_qua_fresult, "f_open dien_nang_tieu_thu_trong_ngay_hom_nay.txt thanh cong, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		fresult = f_write(&fil, data_buffer_dmy_to_eeproom_cuc_bo , bufsize(data_buffer_dmy_to_eeproom_cuc_bo), &bw);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_write thanh cong, de ghi dien_nang_tieu_thu_trong_ngay_hom_nay vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000);
			send_uart("Du lieu dien_nang_tieu_thu_trong_ngay_hom_nay vua ghi vao Eeproom la :\n"); // // de hien thi len PC du lieu vua ghi vao la gi
			send_uart(data_buffer_dmy_to_eeproom_cuc_bo); // de hien thi len PC du lieu vua ghi vao la gi
		}
		
		fresult = f_close(&fil);
		/* 
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close dien_nang_tieu_thu_trong_ngay_hom_nay.txt thanh cong, sau khi ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close dien_nang_tieu_thu_trong_ngay_hom_nay.txt, sau khi ghi du lieu vao  vao EEPROOM, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		*/
		//
	}
	else 
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open dien_nang_tieu_thu_trong_ngay_hom_nay.txt, de ghi du lieu vao EEPROOM, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
	}

}
// End - Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Nay_To_Eeproom()


// BEGIN -  void Doc_Du_Lieu_Eeproom (void)
// Cac gia tri duoc doc: current_energy, previous_date, today, previous_month,current_month, previous_year, current_year 
// Cac gia tri duoc doc: tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc, dien_nang_tieu_thu_trong_ngay_hom_truoc, dien_nang_tieu_thu_trong_ngay_hom_nay,previous_month_energy
void Doc_Du_Lieu_Eeproom (void)
{	
		// BEGIN DOC TAT CA DU LIEU BANG f_gets //
		// Begin Doc du lieu current_energy tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "file1.txt", FA_READ);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_open file1 thanh cong, de doc all data, gia tri cua fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);

			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			current_energy = atof(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				//send_uart("f_eof End of file reached.\n");
				send_uart("f_eof Da den cuoi tap tin file1.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep file1.\n");
			}
		
			f_close(&fil);
			if (fresult == FR_OK) 
			{
				sprintf((char *) ket_qua_fresult, "f_close file1 thanh cong - doc all data tu eeproom, fresult: %d\n", fresult);
				HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
			}
			else 
			{
				sprintf((char *) ket_qua_fresult, "ERROR f_close file1 - doc all data tu eeproom, fresult: %d\n", fresult);
				HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
			}
			
			clear_buffer_doc_tat_ca_du_lieu_sdcard();	// xoa bo dem buffer_doc_tat_ca_du_lieu_sdcard sau khi close file
			
		} // if f_open
		
		else // else f_open 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_open file1 - doc all data tu eeproom, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		// End Doc du lieu current_energy tu eeproom
		
		// Begin Doc du lieu previous_month_energy tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "previous_month_energy.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			previous_month_energy = atof(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin previous_month_energy.txt.\n");
			}
			if (f_error(&fil)) 
			{
				send_uart("f_error Da xay ra loi khi doc tep previous_month_energy.txt.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu previous_month_energy tu eeproom
		
		// Begin Doc du lieu previous_date tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "previous_date.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			previous_date = atoi(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu previous_date len PC
			

			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin previous_date.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu previous_date tu eeproom
		
		// Begin Doc du lieu today tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "today.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			today = atoi(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin today.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu today tu eeproom
		
		// Begin Doc du lieu previous_month tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "previous_month.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			previous_month = atoi(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu previous_month len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin previous_month.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep previous_month.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu previous_month tu eeproom
		
		// Begin Doc du lieu current_month tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "current_month.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			current_month = atoi(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu previous_month len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin current_month.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep current_month.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu current_month tu eeproom
		
		// Begin Doc du lieu previous_year tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "previous_year.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			previous_year = atoi(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu previous_month len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin previous_year.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep previous_year.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu previous_year tu eeproom
	
		// Begin Doc du lieu current_year tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "current_year.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			current_year = atoi(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin current_year.\n");
			}
			if (f_error(&fil)) 
			{
				//send_uart("f_error An error occurred while reading the file.\n");
				send_uart("f_error Da xay ra loi khi doc tep current_year.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu current_year tu eeproom
		
		// Begin Doc du lieu tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc = atof(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc.txt.\n");
			}
			if (f_error(&fil)) 
			{
				send_uart("f_error Da xay ra loi khi doc tep tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc.txt.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc tu eeproom
		
		// Begin Doc du lieu dien_nang_tieu_thu_trong_ngay_hom_truoc tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "dien_nang_tieu_thu_trong_ngay_hom_truoc.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			dien_nang_tieu_thu_trong_ngay_hom_truoc = atof(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin dien_nang_tieu_thu_trong_ngay_hom_truoc.txt.\n");
			}
			if (f_error(&fil)) 
			{
				send_uart("f_error Da xay ra loi khi doc tep dien_nang_tieu_thu_trong_ngay_hom_truoc.txt.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu dien_nang_tieu_thu_trong_ngay_hom_truoc tu eeproom
		
		// Begin Doc du lieu thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			strcpy(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, buffer_doc_tat_ca_du_lieu_eeproom);
			thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua[5] = '\0';
			thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua[6] = '\0';
			
			
			send_uart(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt.\n");
			}
			if (f_error(&fil)) 
			{
				send_uart("f_error Da xay ra loi khi doc tep thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua.txt.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua tu eeproom
		
		// Begin Doc du lieu dien_nang_tieu_thu_trong_ngay_hom_nay tu eeproom
		HAL_Delay(100);
		fresult = f_open(&fil, "dien_nang_tieu_thu_trong_ngay_hom_nay.txt", FA_READ);
		if (fresult == FR_OK)
		{ 
			clear_buffer_doc_tat_ca_du_lieu_eeproom();
			f_gets(buffer_doc_tat_ca_du_lieu_eeproom, sizeof(buffer_doc_tat_ca_du_lieu_eeproom), &fil);
			dien_nang_tieu_thu_trong_ngay_hom_nay = atof(buffer_doc_tat_ca_du_lieu_eeproom);
			send_uart(buffer_doc_tat_ca_du_lieu_eeproom); // gui du lieu len PC
			
			// Kiem tra trang thai EOF và loi
			if (f_eof(&fil))
			{
				send_uart("f_eof Da den cuoi tap tin dien_nang_tieu_thu_trong_ngay_hom_nay.txt.\n");
			}
			if (f_error(&fil)) 
			{
				send_uart("f_error Da xay ra loi khi doc tep dien_nang_tieu_thu_trong_ngay_hom_nay.txt.\n");
			}
			fresult = f_close(&fil);
			clear_buffer_doc_tat_ca_du_lieu_eeproom();  // xoa bo dem cua buffer_doc_tat_ca_du_lieu_eeproom, khi doc lien tuc du lieu
	  }
		// End Doc du lieu dien_nang_tieu_thu_trong_ngay_hom_nay tu eeproom
		
		
		
} // END - void Doc_Du_Lieu_Eeproom (void)

// BEGIN - void Reset_Pzem(void)
void Reset_Pzem(void)
{
		// LENH : reset dien nang pzem 
		clear_res_buf(); // xoa res_buf truoc khi nhan du lieu phan hoi tu pzem
		HAL_UART_Transmit(&huart2, (uint8_t*) &rst_buf, 4, 1000); // reset du lieu tu pzem
		HAL_UART_Receive(&huart2, res_buf, 25, 1000); // nhan du lieu phan hoi tu pzem
		int res_buf_len = bufsize_uint8_t(res_buf);
		uint8_t res_buf_position_1 = 0;
		uint8_t ket_qua_res_buf_position_1[20];
		res_buf_position_1 = res_buf[1];
		//if (res_buf_len == 4)  // Neu la 4 byte thi lenh reset da thanh cong
		if(res_buf_position_1 == 0x42) // 0x42 Correct reply
		{
      // Xu ly du lieu phan hoi khi do dai la 4
      send_uart("Reset du lieu dien nang tieu thu PZEM thanh cong.\n");
    }
		//else if (res_buf_len == 5) // Neu la 5 byte thi lenh reset khong thanh cong
		else if (res_buf_position_1 == 0xC2)
		{
			// Xu ly du lieu phan hoi khi do dai la 5
			send_uart("Reset du lieu dien nang tieu thu PZEM that bai, 0xC2.\n");
			int i = 0;
			while(res_buf_position_1 != 0x42)
			{	
				
				if(i>9)
				{ 
					break;
				}
				clear_res_buf(); // xoa res_buf truoc khi nhan du lieu phan hoi tu pzem
				HAL_UART_Transmit(&huart2, (uint8_t*) &rst_buf, 4, 1000); // yeu cau doc du lieu tu pzem
				HAL_UART_Receive(&huart2, res_buf, 25, 1000); // nhan du lieu phan hoi tu pzem
				res_buf_len = bufsize_uint8_t(res_buf);
				//res_buf_position_1 = 0;
				//ket_qua_res_buf_position_1[20];
				res_buf_position_1 = res_buf[1];
				
				
				if (res_buf_position_1 == 0x42)  // Neu la 0x42 thi lenh reset da thanh cong
				{
					// Xu ly du lieu phan hoi khi do dai la 4
					send_uart("Reset du lieu dien nang tieu thu PZEM thanh cong.\n");
					break;
				}
				else if (res_buf_position_1 == 0xC2) // Neu la 0xC2 thi lenh reset that bai
				{
					// Xu ly du lieu phan hoi khi do dai la 5
					send_uart("Reset du lieu dien nang tieu thu PZEM that bai, 0xC2.\n");
				}
				else 
				{
					// Xu ly du lieu phan hoi khi do dai la Truong Hop Khac
					send_uart("Reset du lieu dien nang tieu thu PZEM that bai, Truong Hop Khac.\n");  
				}
				i = i+1;
			}
		}
		else 
		{
      // Xu ly du lieu phan hoi khi do dai la Truong Hop Khac
      send_uart("Reset du lieu dien nang tieu thu PZEM that bai, Truong Hop Khac.\n");  					
			int i = 0;
			while(res_buf_position_1 != 0x42)
			{	
				
				if(i>9)
				{
					break;
				}
				clear_res_buf(); // xoa res_buf truoc khi nhan du lieu phan hoi tu pzem
				HAL_UART_Transmit(&huart2, (uint8_t*) &rst_buf, 4, 1000); // reset doc du lieu tu pzem
				HAL_UART_Receive(&huart2, res_buf, 25, 1000); // nhan du lieu phan hoi tu pzem
				res_buf_len = bufsize_uint8_t(res_buf);
				//res_buf_position_1 = 0;
				//ket_qua_res_buf_position_1[20];
				res_buf_position_1 = res_buf[1];
				if (res_buf_position_1 == 0x42)  // Neu la 0x42 thi lenh reset da thanh cong
				{
					// Xu ly du lieu phan hoi khi do dai la 25
					send_uart("Reset du lieu dien nang tieu thu PZEM thanh cong.\n");
					break;
				}
				else if (res_buf_position_1 == 0xC2) // Neu la 0xC2 thi lenh reset khong thanh cong
				{
					// Xu ly du lieu phan hoi khi do dai la 5
					send_uart("Reset du lieu dien nang tieu thu PZEM that bai, 0xC2.\n");
				}
				else 
				{
					// Xu ly du lieu phan hoi khi do dai la Truong Hop Khac
					send_uart("Reset du lieu dien nang tieu thu PZEM that bai, Truong Hop Khac.\n");  
				}
				i = i+1;
			}
			
    }
} 
// BEGIN - void Reset_Pzem(void)

// BEGIN void Check_Time(void)
void Check_Time(void)
{	
	Get_Time_DS3231(); 
	
	if((date != today) && (date != 0) && (year >= 23))
	{
		previous_date = today;
		Write_Previous_Date_To_Eeproom_2();
		
		today = date;
		Write_Today_To_Eeproom_2();
		if(month == current_month  && (month != 0))
		{
			tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc = current_energy;
			Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Truoc_To_Eeproom();
		
			dien_nang_tieu_thu_trong_ngay_hom_truoc  = dien_nang_tieu_thu_trong_ngay_hom_nay;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Truoc_To_Eeproom();
			
			// Viet lenh: thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua = previous_date/current_month
			sprintf(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, "%02d/%02d\n", previous_date, current_month);
			Write_Thoi_Diem_Chot_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Qua();
			//clear_thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua(); // Ko nen xoa thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua vi o duoi con su dung de gui di
			
			thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua[5] ='\0';
			thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua[6] ='\0';
			// Viet lenh: dien nang tieu thu trong ngay hom nay 
			dien_nang_tieu_thu_trong_ngay_hom_nay = current_energy - tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Nay_To_Eeproom();
		
		}
		
		else if ( month != current_month && (month != 0))
		{
			// begin 1
			tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc = 0; // dung de tinh dien_nang_tieu_thu_trong_ngay_hom_nay
			Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Truoc_To_Eeproom();
			
			dien_nang_tieu_thu_trong_ngay_hom_truoc  = dien_nang_tieu_thu_trong_ngay_hom_nay;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Truoc_To_Eeproom();	
			// end 1
			
			// begin 2
			// VIET LENH : dien tieu thu thang trc =  dien tieu thu thang hien tai hien tai
			previous_month_energy = current_energy;
			Write_Previous_Month_Energy_To_Eeproom();		
			previous_month = current_month;
			Write_Previous_Month_To_Eeproom();
			current_month = month;
			Write_Current_Month_To_Eeproom();
		
			// Viet lenh: thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua = previous_date/previous_month
			sprintf(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, "%02d/%02d\n", previous_date, previous_month);
			Write_Thoi_Diem_Chot_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Qua();
			//clear_thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua(); // KO NEN CLEAR
			
			// LENH : reset dien nang pzem 
			Reset_Pzem();
			// LENH : dien tieu thu thang hien tai = 0;
			current_energy = 0;
			Write_Current_Energy_To_Eeproom(); // Reset du lieu Current_Energy trong Eeproom de luu du lieu thang moi
			// end 2
			
			// begin 3
			// Viet lenh: reset dien_nang_tieu_thu_trong_ngay_hom_nay			
			dien_nang_tieu_thu_trong_ngay_hom_nay = current_energy - tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Nay_To_Eeproom();
			// end 3
			
		}
	}
	//-------------//
	
	if((date == today) && (date != 0) && (year >= 23))
	{
		if(month != current_month && (month != 0))
		{ 
			// begin 
			previous_date = today;
			Write_Previous_Date_To_Eeproom_2();
			
			today = date;
			Write_Today_To_Eeproom_2();
			// end
			
			// begin 1
			tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc = 0; // dung de tinh dien_nang_tieu_thu_trong_ngay_hom_nay
			Write_Tong_Dien_Nang_Tieu_Thu_Thang_Nay_Do_Vao_Ngay_Hom_Truoc_To_Eeproom();
			
			dien_nang_tieu_thu_trong_ngay_hom_truoc  = dien_nang_tieu_thu_trong_ngay_hom_nay;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Truoc_To_Eeproom();
			// end 1
			
			// begin 2
			// VIET LENH : dien tieu thu thang trc =  dien tieu thu thang hien tai hien tai
			previous_month_energy = current_energy;
			Write_Previous_Month_Energy_To_Eeproom();
			previous_month = current_month;
			Write_Previous_Month_To_Eeproom();
			current_month = month;
			Write_Current_Month_To_Eeproom();
			
			// Viet lenh: thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua = previous_date/previous_month
			sprintf(thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, "%02d/%02d\n", previous_date, previous_month);
			Write_Thoi_Diem_Chot_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Qua();
			//clear_thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua(); // KO NEN CLEAR
			
			// LENH : reset dien nang pzem 
			Reset_Pzem();
			// LENH : dien tieu thu thang hien tai = 0;
			current_energy = 0;
			Write_Current_Energy_To_Eeproom(); // Reset du lieu Current_Energy trong Eeproom de luu du lieu thang moi
			// end 2
			
			// begin 3
			// Viet lenh: reset dien_nang_tieu_thu_trong_ngay_hom_nay			
			dien_nang_tieu_thu_trong_ngay_hom_nay = current_energy - tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Nay_To_Eeproom();
			// end 3
				
		}	
	}
	
	//--------------//
	
	if((year != current_year) && (year != 0) && (year >= 23))
	{
		previous_year = current_year;
		Write_Previous_Year_To_Eeproom();
		current_year = year;
		Write_Current_Year_To_Eeproom();
	}
	
	clear_receive_data_i2c();	
}		

// END void Check_Time(void)


// ------------ END CODE CUA SD CARD -------------------
//HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_UART5_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_I2C2_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
	
	
	
	// ----------------------- begin spi - sd card -----------------------

	Get_Time(); // Kiem tra do chinh xac cua thoi gian luc khoi dong
	send_uart("Kiem tra do chinh xac cua thoi gian ds3231 luc khoi dong.\n");
	send_uart("Khoi dong chuong trinh - Delay 5s.\n");
	HAL_Delay(5000); // delay 5s trc khi khoi dong, de kip doc uart
	
	/* Mount SD Card */
	fresult = f_mount(&fs, "", 0);
	if(fresult != FR_OK) send_uart ("error in mouting SD card...\n");
	else send_uart("SD CARD mounted successfully...\n");
	
	/********** Card capacity details ************/
	/* Check free space */
	f_getfree("", &fre_clust, &pfs);
	total = (uint32_t)((pfs->n_fatent -2) * pfs->csize*0.5) ; 
	sprintf(buffer, "SD CARD Total Size: \t%u\n", total); 
	send_uart(buffer);
	bufclear(); 
	free_space = (uint32_t) (fre_clust * pfs->csize *0.5);
	sprintf(buffer, "SD CARD Free Space: \t%u\n",free_space);
	send_uart(buffer);
	bufclear(); 
	

	uint8_t key = 0;
	
	Doc_Du_Lieu_Eeproom();
	
	//- Open the file with write access -/
	fresult = f_open(&fil, "file4.txt", FA_OPEN_ALWAYS | FA_WRITE);
	if (fresult == FR_OK) 
  {
		sprintf((char *) ket_qua_fresult, "f_open file4 thanh cong, de update, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		
		//- Move to offset to the end of the file -/
		fresult = f_lseek(&fil,f_size(&fil));
		
		fresult = f_close(&fil);
		if (fresult == FR_OK) 
		{
			sprintf((char *) ket_qua_fresult, "f_close file4 thanh cong, sau khi ghi du lieu update, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		else 
		{
			sprintf((char *) ket_qua_fresult, "ERROR f_close file4, sau khi ghi du lieu update, fresult: %d\n", fresult);
			HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
		}
		bufclear();	
		
	} // if (fresult == FR_OK) cua f_open(&fil, "file4.txt", FA_OPEN_ALWAYS | FA_WRITE); 

	else // f_open(&fil, "file4.txt", FA_OPEN_ALWAYS | FA_WRITE);
	{
		sprintf((char *) ket_qua_fresult, "ERROR f_open file4 de update, fresult: %d\n", fresult);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 200);
  }
			
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		uint8_t batdau[] ="Bat Dau while(1)\n";
    HAL_UART_Transmit(&huart5,batdau,strlen((char*)batdau), 500);
    
		Get_Time();	 
		Check_Time(); 

		clear_res_buf(); 
		HAL_UART_Transmit(&huart2, (uint8_t*) &buf, 8, 1000); // yeu cau doc du lieu tu pzem
		HAL_UART_Receive(&huart2, res_buf, 25, 1000); 				// nhan du lieu phan hoi tu pzem
		
		Get_Time_DS3231();  // Chi nhan du lieu, chu chua in ra
		Check_Data_Pzem(); 	// Check CRC , Gia tri Pzem. 
		
		HAL_Delay(10); 
		
		
		// begin , cac gia tri nay da dc lay khi check CRC, gia tri Pzem, 24th11 t nghi ko con can thiet nua // se xoa
		/*
		currentValues.voltage = ((uint32_t) res_buf[3] << 8 | ( uint32_t) res_buf[4])/10.0 ; // raw voltage in 1b //0.1V // neu thuc te la 220.0V, decimal la 2200(raw voltage in 0.1V), muon hien thi thuc te thi /10.0

		currentValues.current = ((uint32_t) res_buf[5] << 8 | (uint32_t) res_buf[6] | (uint32_t) res_buf[7] <<24 | (uint32_t) res_buf[8] << 16) / 1000.0 ; //neu thuc te la 1.000A, decimal la 1000 (raw voltage in 0.001A), muon hien thi thuc te thi /1000.0
	
		currentValues.power = ((uint32_t) res_buf[9] <<8 | (uint32_t) res_buf[10] | (uint32_t) res_buf[11] <<24 | (uint32_t) res_buf[12] << 16) / 10.0; // neu thuc te la 220.0W, decimal la 2200(raw voltage in 0.1W), muon hien thi thuc te thi /10.0

		currentValues.energy = ((uint32_t) res_buf [13] << 8| (uint32_t) res_buf[14] | (uint32_t) res_buf[15] << 24 | (uint32_t) res_buf[16] << 16) /1000.0; // decimal la don vi Wh, neu muon thanh KWh thì /1000.0, (//raw energy in 1Wh ( moi so don vi cua decimal))
		
		currentValues.power_factor = ((uint32_t) res_buf [19] << 8 | (uint32_t)res_buf [20])/1000.0; //them vao 23th11
		*/
		// end , cac gia tri nay da dc lay khi check CRC, gia tri Pzem, 24th11 t nghi ko con can thiet nua // se xoa
		
		voltage = currentValues.voltage;
		current = currentValues.current;
		power =  currentValues.power;
		energy = currentValues.energy;
		power_factor = currentValues.power_factor; 
		
		// begin - Hien thi Ghep du lieu pzem + ds3231 len PC
		sprintf((char *)data_buffer_print, "%02d:%02d:%02d %02d/%02d/20%02d Voltage: %.1f V, Current: %.4f A, Power: %.1f W, Energy: %.4f kWh, Power_factor: %.2f\n", hour, minute, second, date, month, year, voltage, current, power, energy, power_factor);
		HAL_UART_Transmit(&huart5, data_buffer_print, strlen((char *)data_buffer_print), 2000);
		clear_data_buffer_print(); 
		// end - Hien thi Ghep du lieu pzem + ds3231 len PC
	
		// Hien thi gia tri cua energy, current_energy len PC
		sprintf((char *)data_buffer_print_energy_and_current_energy, "energy: %.4f kWh, current_energy: %.4f kWh,\n", energy, current_energy);
		HAL_UART_Transmit(&huart5, data_buffer_print_energy_and_current_energy, strlen((char *)data_buffer_print_energy_and_current_energy), 2000);
		clear_data_buffer_print_energy_and_current_energy();
		
		// Begin - Chuan bi du lieu sang san cho Lora
		sprintf((char *)data_buffer_print, "%02d:%02d:%02d %02d/%02d/20%02d %.1f V, %.4f A, %.1f W, %.4f kWh, %.2f PF, %.4f kWh, %.4f kWh, %s, %.4f kWh\n", hour, minute, second, date, month, year, voltage, current, power, energy, power_factor, dien_nang_tieu_thu_trong_ngay_hom_nay, dien_nang_tieu_thu_trong_ngay_hom_truoc, thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, previous_month_energy );
		HAL_UART_Transmit(&huart5, data_buffer_print, strlen((char *)data_buffer_print), 2000);
		clear_data_buffer_print(); 
		// End - Chuan bi du lieu sang san cho Lora
		
		
		//So sanh gia tri energy (tuc thoi) voi gia tri current_energy (hien tai dang luu)	
		if ((energy!= current_energy) && (voltage>0) && (year >= 23))
    { 
			dien_nang_tieu_thu_trong_ngay_hom_nay = energy - tong_dien_nang_tieu_thu_thang_nay_do_vao_ngay_hom_truoc;
			Write_Dien_Nang_Tieu_Thu_Trong_Ngay_Hom_Nay_To_Eeproom();
    	Write_Data_To_Sdcard(); // Ghi du lieu pzem + ds3231 vao SD card
    	current_energy = energy; // Luu gia tri 'energy vua doc' vao bien 'current_energy'
			
			Write_Current_Energy_To_Eeproom();
			
			key = 9;
			sprintf((char *)do_dai_du_lieu_gui_toi_lora, "%d %02d:%02d:%02d %02d/%02d/20%02d %.1f %.4f %.1f %.4f %.2f %.4f %.4f %s %.4f %dC ",key, hour, minute, second, date, month, year, voltage, current, power, energy, power_factor, dien_nang_tieu_thu_trong_ngay_hom_nay, dien_nang_tieu_thu_trong_ngay_hom_truoc, thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, previous_month_energy, previous_month);
			uint16_t gia_tri_crc = MB_CRC16(do_dai_du_lieu_gui_toi_lora, strlen((char *)do_dai_du_lieu_gui_toi_lora));
			sprintf((char *)du_lieu_gui_toi_lora, "%d %02d:%02d:%02d %02d/%02d/20%02d %.1f %.4f %.1f %.4f %.2f %.4f %.4f %s %.4f %dC %04X",key, hour, minute, second, date, month, year, voltage, current, power, energy, power_factor, dien_nang_tieu_thu_trong_ngay_hom_nay, dien_nang_tieu_thu_trong_ngay_hom_truoc, thoi_diem_chot_dien_nang_tieu_thu_trong_ngay_hom_qua, previous_month_energy, previous_month,gia_tri_crc );
			
			// begin - gui du lieu cho esp
			HAL_UART_Transmit(&huart4, du_lieu_gui_toi_lora, strlen((char *)du_lieu_gui_toi_lora), 2000); // Gui toi esp
			send_uart("Du lieu stm vua gui qua esp lora la: \n");
			memcpy(data_buffer_print, du_lieu_gui_toi_lora, strlen((char *) du_lieu_gui_toi_lora));
			HAL_UART_Transmit(&huart5, data_buffer_print, strlen((char *)data_buffer_print), 2000); // Hien thi len PC
			clear_data_buffer_print();
			send_uart("\n");
		  // end - gui du lieu cho esp
			
		}
				
		// begin - Doc tong dung luong
		f_getfree("", &fre_clust, &pfs);
		total = (uint32_t)((pfs->n_fatent -2) * pfs->csize*0.5) ; 
		sprintf(buffer, "SD CARD Total Size: \t%u\n", total); 
		send_uart(buffer);
		bufclear(); 
		free_space = (uint32_t) (fre_clust * pfs->csize *0.5);
		sprintf(buffer, "SD CARD Free Space: \t%u\n",free_space);
		send_uart(buffer);
		bufclear(); 
		// end - Doc tong dung luong
		
		// begin - Code xem dung luong file
		fresult = f_open(&fil, "file4.txt", FA_READ); 
		DWORD fileSize4 = f_size(&fil);

		sprintf((char *) ket_qua_fresult, "f_size file4, dung luong cua file: %lu Bytes\n", fileSize4);
		HAL_UART_Transmit(&huart5, ket_qua_fresult, strlen((char *)ket_qua_fresult), 1000); 
		f_close(&fil);
		clear_ket_qua_fresult();
		// end - Code xem dung luong file
		
		HAL_Delay(10000); 
		send_uart("Xong lenh HAL_Delay(10000).\n");
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
