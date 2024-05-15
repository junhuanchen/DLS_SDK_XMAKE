#include "stdio.h"
// #include "log.hpp"
// #include "sqlite3.h"
// #include <stdlib.h>
// #include "unit_test_sql.h"
// #include "UartFinger.h"
// #include "XQDB.h"
// #include "cJSON.h"
// #include "finger.h"
// #include "linux_uart.h"
// #include "yefiot.h"
// #include "event.h"
// #include "net.h"

// yf_param *g_yf = NULL;

#include "UartFinger.h"

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#include <list>
#include <vector>

class rc522:public UartFinger
{
public:
    enum MFRC522
    {
        NRSTPD = 22,
        
        MAX_LEN = 16,
        
        PCD_IDLE       = 0x00,
        PCD_MEM        = 0x01,
        PCD_RNDID      = 0x02,
        PCD_CALCCRC    = 0x03,
        PCD_TRANSMIT   = 0x04,
        PCD_AUTHENT    = 0x0E,
        PCD_RECEIVE    = 0x08,
        PCD_TRANSCEIVE = 0x0C,
        PCD_RESETPHASE = 0x0F,
        
        
        PICC_REQA      = 0x26,
        PICC_WUPA      = 0x52,
        PICC_ANTICOLL  = 0x93,
        PICC_SElECTTAG = 0x93,
        PICC_AUTHENT1A = 0x60,
        PICC_AUTHENT1B = 0x61,
        PICC_READ      = 0x30,
        PICC_WRITE     = 0xA0,
        PICC_DECREMENT = 0xC0,
        PICC_INCREMENT = 0xC1,
        PICC_RESTORE   = 0xC2,
        PICC_TRANSFER  = 0xB0,
        PICC_HALT      = 0x50,
        
        MI_OK       = 0,
        MI_NOTAGERR = 1,
        MI_ERR      = 2,
        

        CommandReg     = 0x01,
        CommIEnReg     = 0x02,
        DivlEnReg      = 0x03,
        CommIrqReg     = 0x04,
        DivIrqReg      = 0x05,
        ErrorReg       = 0x06,
        Status1Reg     = 0x07,
        Status2Reg     = 0x08,
        FIFODataReg    = 0x09,
        FIFOLevelReg   = 0x0A,
        WaterLevelReg  = 0x0B,
        ControlReg     = 0x0C,
        BitFramingReg  = 0x0D,
        CollReg        = 0x0E,
            
        ModeReg        = 0x11,
        TxModeReg      = 0x12,
        RxModeReg      = 0x13,
        TxControlReg   = 0x14,
        TxASKReg       = 0x15,
        TxSelReg       = 0x16,
        RxSelReg       = 0x17,
        RxThresholdReg = 0x18,
        DemodReg       = 0x19,

        MifareTxReg    = 0x1C,
        MifarerxReg    = 0x1D,

        SerialSpeedReg = 0x1F,

        CRCResultRegM     = 0x21,
        CRCResultRegL     = 0x22,

        ModWidthReg       = 0x24,

        RFCfgReg          = 0x26,
        GsNReg            = 0x27,
        CWGsPReg          = 0x28,
        ModGsPReg         = 0x29,
        TModeReg          = 0x2A,
        TPrescalerReg     = 0x2B,
        TReloadRegH       = 0x2C,
        TReloadRegL       = 0x2D,
        TCounterValueRegH = 0x2E,
        TCounterValueRegL = 0x2F,
        
        TestSel1Reg     = 0x31,
        TestSel2Reg     = 0x32,
        TestPinEnReg    = 0x33,
        TestPinValueReg = 0x34,
        TestBusReg      = 0x35,
        AutoTestReg     = 0x36,
        VersionReg      = 0x37,
        AnalogTestReg   = 0x38,
        TestDAC1Reg     = 0x39,
        TestDAC2Reg     = 0x3A,
        TestADCReg      = 0x3B,
    };

    bool m_bOpen;
    rc522():UartFinger()
    {
        m_bOpen = OpenRc522();
    }

    ~rc522()
    {
        CloseFinger();
    }

    bool OpenRc522(int braud = 115200)
    {
        CloseFinger();
        bool b = OpenUartFinger(2,braud,8,'N',1);
        if(b) 
            printf("open rc522 success!\n");
        else 
            printf("open rc522 fail!\n");
        return b;
    }

    int writeRegister(int addr, int val)
    {
        int count = 0;
        while (true)
        {
            // printf("w: 0x%02x, 0x%02x\n", addr, val);
            uart_flush();
            Send((uint8_t*)&addr, 1);
            Send((uint8_t*)&val, 1);
            int tmp = 0;
            int ret = Recv((uint8_t*)&tmp, 1);
            // uint8_t tmp = 0;
            // int ret = uart_read_bytes((uint8_t*)&tmp, 1, 1);
            // printf("tmp:%d\n",tmp);
            // printf("ret:%d\n",ret);
            if(ret == 1 && tmp == addr)
                return 1;
            count++;
            if(count > 2)
            {
                // printf("Error de escritura en: %02x\n", addr);
                return 0;
            }
        }
        return 1;
    }

    void setBitMask(int reg, int mask)
    {
        int tmp = readRegister(reg);
        writeRegister(reg, tmp | mask);
    }

    void clearBitMask(int reg, int mask)
    {
        int tmp = readRegister(reg);
        writeRegister(reg, tmp & (~mask));
    }

    void antennaOn()
    {
        int temp = readRegister(TxControlReg);
        if(~(temp & 0x03))
            setBitMask(TxControlReg, 0x03);
    }
  
    void antennaOff()
    {
        clearBitMask(TxControlReg, 0x03);
    }

    int readRegister(int addr)
    {
        uint8_t tmp_addr = addr|0x80;
        uart_flush();
        Send((uint8_t*)&tmp_addr, 1);
        uint8_t val = 0;
        // usleep(5000);
        // sleep(1);
        // int ret = uart_read_bytes((uint8_t*)&val, 1, 1);
        int ret = Recv((uint8_t*)&val, 1);
        if (ret == 1)
            printf("r: 0x%02x, 0x%02x\n", addr, val);
        // else
        //     printf("Error de lectura en: %02x\n", addr);
        if (ret != 1)
            return 0;
        return val;
    }

    int getAntennaGain()
    {
        return readRegister((RFCfgReg) & (0x07<<4));
    }

    void reset()
    {
        printf("reset rc522\n");
        if (!writeRegister(CommandReg, PCD_RESETPHASE))
        {
            OpenRc522(115200);
            writeRegister(CommandReg, PCD_RESETPHASE);
            OpenRc522(9600);
            usleep(50000);
            writeRegister(SerialSpeedReg, 0x7A);
            OpenRc522(115200);
            usleep(50000);
        }
        
        writeRegister(TModeReg, 0x80);
        writeRegister(TPrescalerReg, 0xA9);
        writeRegister(TReloadRegH, 0x03);
        writeRegister(TReloadRegL, 0xE8);
        writeRegister(TxASKReg, 0x40);
        writeRegister(ModeReg, 0x3D);
        writeRegister(TestPinEnReg, 0x00);
        antennaOn();
    }

    int MFRC522_ToCard(std::vector<uint8_t> &backData, int &backBits, uint8_t command, std::vector<uint8_t> &sendData)
    {
        backData.clear();
        backBits = 0;
        int status = MI_ERR;
        int irqEn = 0x00;
        int waitIRq = 0x00;
        int lastBits = 0x00;
        int n = 0;
        int i = 0;

        if (command == PCD_AUTHENT)
        {
            irqEn = 0x12;
            waitIRq = 0x10;
        }
        if (command == PCD_TRANSCEIVE)
        {
            irqEn = 0x77;
            waitIRq = 0x30;
        }
        writeRegister(CommIEnReg, irqEn|0x80);
        clearBitMask(CommIrqReg, 0x80);
        setBitMask(FIFOLevelReg, 0x80);

        writeRegister(CommandReg, PCD_IDLE);
        
        while(i<sendData.size())
        {
            writeRegister(FIFODataReg, sendData[i]);
            i = i + 1;
        }
        writeRegister(CommandReg, command);

        if (command == PCD_TRANSCEIVE)
        {
            setBitMask(BitFramingReg, 0x80);
        }

        i = 100;
        while (true)
        {
            n = readRegister(CommIrqReg);
            i = i - 1;
            if (~((i!=0) && ~(n&0x01) && ~(n&waitIRq)))
            {
                break;
            }
        }

        clearBitMask(BitFramingReg, 0x80);

        if (i != 0)
        {
            if ((readRegister(ErrorReg) & 0x1B) == 0x00)
            {
                status = MI_OK;
                if (n & irqEn & 0x01)
                {
                    status = MI_NOTAGERR;
                }
                if (command == PCD_TRANSCEIVE)
                {
                    n = readRegister(FIFOLevelReg);
                    lastBits = readRegister(ControlReg) & 0x07;
                    if (lastBits != 0)
                    {
                        backBits = (n-1)*8 + lastBits;
                    }
                    else
                    {
                        backBits = n*8;
                    }
                    if (n == 0)
                    {
                        n = 1;
                    }
                    if (n > MAX_LEN)
                    {
                        n = MAX_LEN;
                    }
                    i = 0;
                    while (i<n)
                    {
                        backData.push_back(readRegister(FIFODataReg));
                        i = i + 1;
                    }
                }
            }
            else
            {
                status = MI_ERR;
            }
        }
        // printf("backData: %d\n", backData.size());
        // for (int i = 0; i < backData.size(); i++)
        // {
        //     printf("%02X ", backData[i]);
        // }
        // printf("\n");
        return status;
    }
    
    int MFRC522_Request(uint8_t reqMode, int &backBits)
    {
        int status = MI_ERR;
        std::vector<uint8_t> backData;
        std::vector<uint8_t> TagType;
        writeRegister(BitFramingReg, 0x07);
        TagType.push_back(reqMode);
        status = MFRC522_ToCard(backData, backBits, PCD_TRANSCEIVE, TagType);
        if ((status != MI_OK) | (backBits != 0x10))
            status = MI_ERR;
        return status;
    }
    
    int MFRC522_Anticoll(std::vector<uint8_t> &backData)
    {
        int status = MI_ERR;
        int backBits = 0;
        int serNumCheck = 0;
        std::vector<uint8_t> serNum;
        writeRegister(BitFramingReg, 0x00);
        serNum.push_back(PICC_ANTICOLL);
        serNum.push_back(0x20);
        status = MFRC522_ToCard(backData, backBits, PCD_TRANSCEIVE, serNum);
        if (status == MI_OK)
        {
            int i = 0;
            if (backData.size() == 5)
            {
                while(i<4)
                {
                    serNumCheck = serNumCheck ^ backData[i];
                    i = i + 1;
                }
                if (serNumCheck != backData[i])
                {
                    status = MI_ERR;
                }
            }
            else
            {
                status = MI_ERR;
            }
        }
        return status;
    }

    std::vector<uint8_t> MFRC522_CalulateCRC(std::vector<uint8_t> pIndata)
    {
        std::vector<uint8_t> pOutData;
        clearBitMask(DivIrqReg, 0x04);
        setBitMask(FIFOLevelReg, 0x80);
        int i = 0;
        while (i < pIndata.size())
        {
            writeRegister(FIFODataReg, pIndata[i]);
            i = i + 1;
        }
        writeRegister(CommandReg, PCD_CALCCRC);
        i = 0xFF;
        while (true)
        {
            int n = readRegister(DivIrqReg);
            i = i - 1;
            if (!((i != 0) && !(n & 0x04)))
            {
                break;
            }
        }
        writeRegister(CommandReg, PCD_IDLE);
        pOutData.push_back(readRegister(CRCResultRegL));
        pOutData.push_back(readRegister(CRCResultRegM));
        return pOutData;
    }

    int MFRC522_SelectTag(std::vector<uint8_t> serNum)
    {
        std::vector<uint8_t> backData;
        std::vector<uint8_t> buf;
        int backBits = 0;
        buf.push_back(PICC_SElECTTAG);
        buf.push_back(0x70);
        int i = 0;
        while (i < 5)
        {
            buf.push_back(serNum[i]);
            i = i + 1;
        }
        std::vector<uint8_t> pOut = MFRC522_CalulateCRC(buf);
        buf.push_back(pOut[0]);
        buf.push_back(pOut[1]);
        int status = MFRC522_ToCard(backData, backBits, PCD_TRANSCEIVE, buf);
        if ((status == MI_OK) && (backBits == 0x18))
        {
            return backData[0];
        }
        else
        {
            return 0;
        }
    }

    int MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, std::vector<uint8_t> Sectorkey, std::vector<uint8_t> serNum)
    {
        std::vector<uint8_t> buff;
        buff.push_back(authMode);
        buff.push_back(BlockAddr);
        int i = 0;
        while(i < Sectorkey.size())
        {
            buff.push_back(Sectorkey[i]);
            i = i + 1;
        }
        i = 0;
        while(i < 4)
        {
            buff.push_back(serNum[i]);
            i = i + 1;
        }
        std::vector<uint8_t> backData;
        int backBits = 0;
        int status = MFRC522_ToCard(backData, backBits, PCD_AUTHENT, buff);
        
        if (status != MI_OK)
        {
            printf("auth failed!\n");
        }
        
        if (!((readRegister(Status2Reg) & 0x08) != 0))
        {
            printf("auth failed!(status2reg & 0x08) != 0\n");
        }

        return status;
    }

    void MFRC522_StopCrypto1()
    {
        clearBitMask(Status2Reg, 0x08);
    }

    std::vector<uint8_t> MFRC522_Read(int blockAddr)
    {
        std::vector<uint8_t> recvData;
        recvData.push_back(PICC_READ);
        recvData.push_back(blockAddr);
        std::vector<uint8_t> pOut = MFRC522_CalulateCRC(recvData);
        recvData.push_back(pOut[0]);
        recvData.push_back(pOut[1]);
        std::vector<uint8_t> backData;
        int backBits = 0;
        int status = MFRC522_ToCard(backData, backBits, PCD_TRANSCEIVE, recvData);
        if (status != MI_OK)
        {
            printf("Error while reading!\r\n");
        }
        if (backData.size() == 16)
        {
            std::vector<uint8_t> msg;
            msg.push_back(blockAddr);
            msg.insert(msg.end(), backData.begin(), backData.end());
            return msg;
        }
        return backData;
    }

    int MFRC522_Write(int blockAddr, std::vector<uint8_t> writeData)
    {
        std::vector<uint8_t> buff;
        buff.push_back(PICC_WRITE);
        buff.push_back(blockAddr);
        int i = 0;
        while(i < writeData.size())
        {
            buff.push_back(writeData[i]);
            i = i + 1;
        }
        std::vector<uint8_t> pOut = MFRC522_CalulateCRC(buff);
        buff.push_back(pOut[0]);
        buff.push_back(pOut[1]);
        std::vector<uint8_t> backData;
        int backBits = 0;
        int status = MFRC522_ToCard(backData, backBits, PCD_TRANSCEIVE, buff);
        if (status != MI_OK || backBits != 4 || ((backData[0] & 0x0F) != 0x0A))
        {
            printf("Error while writing!\r\n");
        }
        if (status == MI_OK)
        {
            i = 0;
            buff.clear();
            while(i < 16)
            {
                if (writeData[i] != backData[i + 2])
                {
                    status = MI_ERR;
                    break;
                }
                i = i + 1;
            }
            std::vector<uint8_t> pOut = MFRC522_CalulateCRC(buff);
            buff.push_back(pOut[0]);
            buff.push_back(pOut[1]);
            status = MFRC522_ToCard(backData, backBits, PCD_TRANSCEIVE, buff);
            if (status != MI_OK || backBits != 4 || ((backData[0] & 0x0F) != 0x0A))
            {
                printf("Error while writing!\r\n");
            }
            if (status == MI_OK)
            {
                printf("Data written\r\n");
            }
        }
        
        return status;
    } 

    void MFRC522_DumpClassic1K(std::vector<uint8_t> key, std::vector<uint8_t> uid)
    {
        int i = 0;
        while(i < 64)
        {
            int status = MFRC522_Auth(PICC_AUTHENT1A, i, key, uid);
            if (status == MI_OK)
            {
                MFRC522_Read(i);
            }
            else
            {
                printf("Authentication error\r\n");
            }
            i = i+1;
        }
    }



    void run()
    {
        m_bRun = true;
        reset();
        while (m_bRun)
        {
            int sector_now = 8;
            printf("sector_now\r\n");

            int status = 0;
            int TagType = 0;
            status = MFRC522_Request(PICC_REQA, TagType);
            printf("status:%d TagType:%d\n",status,TagType);

            if (status == MI_OK)
            {
                printf("Card detected\r\n");
            }

            std::vector<uint8_t> uid;
            status = MFRC522_Anticoll(uid);
            if (status != MI_OK)
            {
                printf("Anticoll error\r\n");
                reset();
                continue;
            }
            else
            {
                printf("Anticoll ok\r\n");
                printf("uid.size:%d\n",uid.size());
                for (int i = 0; i < uid.size(); i++)
                {
                    printf("%02X ", uid[i]);
                }
                printf("\r\n");

                printf("MFRC522 init ok\r\n");
                std::vector<uint8_t> key;
                key.push_back(0xFF);
                key.push_back(0xFF);
                key.push_back(0xFF);
                key.push_back(0xFF);
                key.push_back(0xFF);
                key.push_back(0xFF);

                int ret = MFRC522_SelectTag(uid);
                printf("MFRC522_SelectTag ret:%d\n",ret);

                MFRC522_Auth(PICC_AUTHENT1A, sector_now, key, uid);
                if (status != MI_OK)
                {
                    printf("Authentication error\r\n");
                }
                else
                {
                    printf("Authentication ok\r\n");

                    printf("sector_now:%d\n",sector_now);

                    std::vector<uint8_t> data;

                    data = MFRC522_Read(sector_now);
                    if (data.size() > 0)
                    {
                        printf("Sector[%d] ", data[0]);
                        for (int i = 1; i < data.size(); i++)
                        {
                            printf("%02X ", data[i]);
                        }
                        printf("\r\n");
                    }

                    data.clear();
                    for (int i = 0; i < 16; i++)
                    {
                        data.push_back(0xFF);
                    }
                    MFRC522_Write(sector_now, data);

                    data = MFRC522_Read(sector_now);
                    if (data.size() > 0)
                    {
                        printf("Sector[%d] ", data[0]);
                        for (int i = 1; i < data.size(); i++)
                        {
                            printf("%02X ", data[i]);
                        }
                        printf("\r\n");
                    }

                    MFRC522_StopCrypto1();
                }

                break;
            }

        }
        return;
    }
};

rc522 *g_rc522 = NULL;

int main(void)
{
    // test_log();
    // // getdevip();
    // g_yf = new yf_param();
    // printf("DLS Hello World!\n");
    // puts(SQLITE_VERSION);
    // g_yf->getparameter();
    
    g_rc522 = new rc522;
    if(g_rc522)
    {
        g_rc522->start();
    }
    
    while(1)
    {
        printf("-------------\n");
        sleep(5);
    }
    
    return 0;
}

// static int callback(void *NotUsed, int argc, char **argv, char **azColName){
//    int i;
//    for(i=0; i<argc; i++){
//       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
//    }
//    printf("\n");
//    return 0;
// }

// int main(void)
// {
    // test_log();
    // printf("DLS Hello World!\n");
    // puts(SQLITE_VERSION);


    // unit_test_sql();


    // return 0;
// }