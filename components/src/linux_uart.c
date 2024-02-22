
#include "stdlib.h"
#include "stdint.h"
#include <sys/time.h>
#include <sys/select.h>

/*******************************************************************************
 * @fn:
 * @brief:      计算校验和
 * @para:
 * @return:
 ******************************************************************************/
uint8_t CheckSum8(uint8_t seed, const uint8_t *input, uint16_t nbrOfBytes)
{
    nbrOfBytes = nbrOfBytes > 0xFFFE ? 0xFFFE : nbrOfBytes;

    uint8_t sum = seed;

    uint8_t iX = 0;

    for (; iX < nbrOfBytes && input; iX++)
    {
        sum += input[iX];
    }

    return sum;
}


/*******************************************************************************
 * @fn:
 * @brief:
 * @param:
 * 				polynomial：	生成多项式，常见的生成多项式:
 * 								CRC-ITU(0x1021)*shi yong zhe ge
 * 								CRC16(0x8005)
 *
 * 				seed: CRC16校验码的初始值， 可以设为0
 * @return:
 ******************************************************************************/

uint16_t CRC16(uint16_t polynomial, uint16_t seed, const uint8_t *input,
               uint16_t nbrOfBytes)
{

    // 100-byte，需要花费824us @ 12MHz

    uint16_t crc = seed, bit = 0, byteCtr = 0;

    for (byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
    {
        crc ^= input[byteCtr] << 8;

        for (bit = 8; bit > 0; bit--)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ polynomial;
            }
            else
            {
                crc <<= 1;
            }
        }

    }

    return crc;
}

#include "linux_uart.h"

struct _ts_
{
    struct timeval timeout;
    int dev_ttyS;
    fd_set readfd;
    struct pack_t
    {
        uint16_t work, size;
        uint8_t data[255];
    } pack_data;
    struct _jpg_
    {
        uint32_t pos;
        uint32_t len;
        uint8_t buf[100*1024];
    } jpg_data;
} ts, *self=&ts;

int linux_uart_load()
{
    uart_t uart_dev_parm = {
        .baud = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'n'};

    self->dev_ttyS = linux_uart_init((char *)"/dev/ttyUSB0", &uart_dev_parm);
    if (self->dev_ttyS < 0)
    {
      perror(" uart /dev/ttyUSB0 open err!");
      abort();
    }

    // tmp = 3E CA D9 0A 43 41 4E 47 4C 55 00 00 57 69 6C 64 50 72 33 39 42 4D 30 35 30 00 00 00 54 23 07 26 16 37 39 0A 13 05 E2 3B 60 01 B8 B3 DA 0F D4 16 89 12 34 56 78 00 00 24 91 08 30 30 30 32 2E 4A 50 47
    // uint8_t tmp[] = "\x3E\xCA\xD9\x0A\x43\x41\x4E\x47\x4C\x55\x00\x00\x57\x69\x6C\x64\x50\x72\x33\x39\x42\x4D\x30\x35\x30\x00\x00\x00\x54\x23\x07\x26\x16\x37\x39\x0A\x13\x05\xE2\x3B\x60\x01\xB8\xB3\xDA\x0F\xD4\x16\x89\x12\x34\x56\x78\x00\x00\x24\x91\x08\x30\x30\x30\x32\x2E\x4A\x50\x47";
    // write(self->dev_ttyS, tmp, sizeof(tmp));
    
    self->timeout.tv_sec = 0;
    self->timeout.tv_usec = 0;
    
    FD_ZERO(&self->readfd);
}

int linux_uart_loop()
{
    if (self->pack_data.work == 0)
    {
        memset(self->pack_data.data, 0, sizeof(self->pack_data.data));
        self->pack_data.size = 0;
        self->pack_data.work = 1;
    }

    if (self->pack_data.work == 1)
    {
        FD_SET(self->dev_ttyS, &self->readfd);
        int ret = select(self->dev_ttyS + 1, &self->readfd, NULL, NULL, &self->timeout);
        if (ret != -1 && FD_ISSET(self->dev_ttyS, &self->readfd))
        {
            uint8_t tmp = 0;
            int readByte = read(self->dev_ttyS, &tmp, 1);
            if (readByte != -1)
            {
                self->pack_data.data[self->pack_data.size] = tmp;
                self->pack_data.size += 1;
            }
            if (self->pack_data.size == (self->pack_data.data[0] + 4))
            {
                self->pack_data.work = 2;
            }
            if (self->pack_data.size > 255)
            {
                self->pack_data.work = 0;
            }
        }
    }

    if (self->pack_data.work == 2)
    {
        self->pack_data.work = 3;

        // do something
        printf("pack len %d\n", self->pack_data.size);
        for (int i = 0; i < self->pack_data.size; i++)
        {
            printf(" [%d](%02x) ", i, self->pack_data.data[i]);
        }
        printf("\n");

        uint8_t crc8 = CheckSum8(0, self->pack_data.data + 3, self->pack_data.size - 3);

        printf("pack crc:%x result:%d\n", crc8, crc8 == self->pack_data.data[2]);

        if (crc8 == self->pack_data.data[2])
        {
            if (self->pack_data.data[3] == 0x0A)
            {
                uint32_t file_size = 0;
                uint8_t *file_pos = &file_size;
                file_pos[3] = self->pack_data.data[53];
                file_pos[2] = self->pack_data.data[54];
                file_pos[1] = self->pack_data.data[55];
                file_pos[0] = self->pack_data.data[56];
                printf("file_size %d \r\n", file_size);

                uint8_t tmp[] = "\x05\xAC\x0A\x0A\x00\x00\x00\x00\x00";
                write(self->dev_ttyS, tmp, sizeof(tmp));
                
                self->jpg_data.pos = 0;
                self->jpg_data.len = file_size;
                
                puts("work!");
            }
            else if(self->pack_data.data[3] == 0x0B)
            {
                memcpy(self->jpg_data.buf + self->jpg_data.pos, self->pack_data.data + 4, (self->pack_data.size - 4));
                self->jpg_data.pos += (self->pack_data.size - 4);
                printf("jpg_data.pos %d len %d\r\n", self->jpg_data.pos, self->jpg_data.len);
                
                uint8_t tmp[] = "\x01\xAC\x0B\x0B\x00";
                write(self->dev_ttyS, tmp, sizeof(tmp));
                puts("data!");
            }
            else if(self->pack_data.data[3] == 0x0C)
            {
                if (self->jpg_data.pos == self->jpg_data.len)
                {
                    // crc16
                    // tmpbuff = CRC16(0x1021, tmpbuff, comtmp, 200);
                }
                
                // uint8_t tmp[] = "\x01\xAC\x0B\x0B\x00";
                // write(self->dev_ttyS, tmp, sizeof(tmp));

                puts("0x0C!");
            }
            else if(self->pack_data.data[3] == 0x04)
            {
                uint8_t tmp[] = "\x08\xAC\x3C\x04\x01\x18\x04\x02\x00\x19\x00\x00";
                write(self->dev_ttyS, tmp, sizeof(tmp));

                puts("0x04!");
            }
            else if(self->pack_data.data[3] == 0x0F)
            {
                uint8_t tmp[] = "\x0D\xAC\x1B\x0F\x00\x00\x7F\x00\x80\x00\x71\x00\x0F\x00\x0E\x00\x7F";
                write(self->dev_ttyS, tmp, sizeof(tmp));

                puts("0x0F!");
            }
            else
            {
                puts("other!");
            }

            self->pack_data.work = 0; // 复位
        }
        else
        {
            self->pack_data.work = 0; // 复位
        }
    }

    // if (self->pack_data.work == 4)
    // {
    //     self->pack_data.work = 0;
    //     // exit(0);
    // }

    // timeout need to self->pack_data.work = 0;

}

int linux_uart_main()
{
    puts("begin!");
    linux_uart_load();
    while (1)
    {
        linux_uart_loop();
    }
    return 0;
}