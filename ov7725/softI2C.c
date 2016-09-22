#include "softI2C.h"


//SDA PC5
//SCL PC4

#define soft_i2c_SCL_H         	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET)
#define soft_i2c_SCL_L        	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET)

#define soft_i2c_SDA_H         	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET)
#define soft_i2c_SDA_L       	 	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET)

#define SCL_read      					HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_4)
#define SDA_read     						HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_5)



#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define  I2C_Direction_Receiver         ((uint8_t)0x01)


volatile bool soft_i2c_bus_idle = false; 

static void I2C_delay(void)//实测181kHz
{
    volatile int i =	70;
    while (i)
        i--;
}


//static uint8_t I2C_recovery(void)//恢复IIC总线
//{

//	
//}

static bool I2C_Start(void)
{
    if(soft_i2c_bus_idle == false)
			return false;//总线被占用
		
		
		soft_i2c_SDA_H;
    soft_i2c_SCL_H;
    I2C_delay();
    if (!SDA_read)
		{
			//恢复IIC总线
			for(uint8_t i=0;i<9;i++)
			{
				soft_i2c_SCL_H;
				I2C_delay();
				soft_i2c_SCL_L;
				I2C_delay();
			}
			soft_i2c_SCL_H;
			I2C_delay();
			if(!SDA_read)//恢复失败
				return false;
		}
    soft_i2c_SDA_L;
    I2C_delay();
    if (SDA_read)
        return false;
    soft_i2c_SDA_L;
    I2C_delay();
    return true;
}

static void I2C_Stop(void)
{
    soft_i2c_SCL_L;
    I2C_delay();
    soft_i2c_SDA_L;
    I2C_delay();
    soft_i2c_SCL_H;
    I2C_delay();
    soft_i2c_SDA_H;
    I2C_delay();
		soft_i2c_bus_idle = true;//释放总线
}

static void I2C_Ack(void)
{
    soft_i2c_SCL_L;
    I2C_delay();
    soft_i2c_SDA_L;
    I2C_delay();
    soft_i2c_SCL_H;
    I2C_delay();
    soft_i2c_SCL_L;
    I2C_delay();
}

static void I2C_NoAck(void)
{
    soft_i2c_SCL_L;
    I2C_delay();
    soft_i2c_SDA_H;
    I2C_delay();
    soft_i2c_SCL_H;
    I2C_delay();
    soft_i2c_SCL_L;
    I2C_delay();
}

static bool I2C_WaitAck(void)
{
    soft_i2c_SCL_L;
    I2C_delay();
    soft_i2c_SDA_H;
    I2C_delay();
    soft_i2c_SCL_H;
    I2C_delay();
    if (SDA_read) {
        soft_i2c_SCL_L;
        return false;
    }
    soft_i2c_SCL_L;
    return true;
}

static void I2C_SendByte(uint8_t byte)
{
    uint8_t i = 8;
    while (i--) {
        soft_i2c_SCL_L;
        I2C_delay();
        if (byte & 0x80)
            soft_i2c_SDA_H;
        else
            soft_i2c_SDA_L;
        byte <<= 1;
        I2C_delay();
        soft_i2c_SCL_H;
        I2C_delay();
    }
    soft_i2c_SCL_L;
}

static uint8_t I2C_ReceiveByte(void)
{
    uint8_t i = 8;
    uint8_t byte = 0;

    soft_i2c_SDA_H;
    while (i--) {
        byte <<= 1;
        soft_i2c_SCL_L;
        I2C_delay();
        soft_i2c_SCL_H;
        I2C_delay();
        if (SDA_read) {
            byte |= 0x01;
        }
    }
    soft_i2c_SCL_L;
    return byte;
}

void i2cInit(void)
{
//移到cube中设置
	soft_i2c_SDA_H;
	soft_i2c_SCL_H;
	I2C_delay();
	//I2C的恢复函数
	for(uint8_t i=0;i<9;i++)
	{
		soft_i2c_SCL_H;
		I2C_delay();
		soft_i2c_SCL_L;
		I2C_delay();
	}
	soft_i2c_SCL_H;
	I2C_delay();//不管有没有问题，都先初始化一遍
	soft_i2c_bus_idle = true;
}

bool i2cWriteBuffer(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
{
    int i;
    if (!I2C_Start())
        return false;
    I2C_SendByte(addr << 1 | I2C_Direction_Transmitter);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return false;
    }
    I2C_SendByte(reg);
    I2C_WaitAck();
    for (i = 0; i < len; i++) {
        I2C_SendByte(data[i]);
        if (!I2C_WaitAck()) {
            I2C_Stop();
            return false;
        }
    }
    I2C_Stop();
    return true;
}

bool i2cWrite(uint8_t addr, uint8_t reg, uint8_t data)
{
	if (!I2C_Start())
        return false;
    I2C_SendByte(addr << 1 | I2C_Direction_Transmitter);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return false;
    }
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_SendByte(data);
    I2C_WaitAck();
    I2C_Stop();
    return true;
}

bool i2cRead(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	if (!I2C_Start())
        return false;
    I2C_SendByte(addr << 1 | I2C_Direction_Transmitter);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return false;
    }
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(addr << 1 | I2C_Direction_Receiver);
    I2C_WaitAck();
    while (len) {
        *buf = I2C_ReceiveByte();
        if (len == 1)
            I2C_NoAck();
        else
            I2C_Ack();
        buf++;
        len--;
    }
    I2C_Stop();
    return true;
}

uint16_t i2cGetErrorCounter(void)
{
    // TODO maybe fix this, but since this is test code, doesn't matter.
    return 0;
}

