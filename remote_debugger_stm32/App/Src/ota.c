#include "ota.h"
#include "ring_buffer.h"

extern ring_buffer_typedef ota_buffer;

FLASH_EraseInitTypeDef erase_init;

/* ota运行相关 */
uint8_t ota_start_flag = 0;
uint32_t last_time = 0;
uint32_t vtor = 0;
/* ota版本信息相关 */
uint32_t version = 0;
uint16_t size = 0;
/* ota写入相关 */
uint16_t ota_write_data = 0;
uint8_t msb_flag = 0;
uint32_t ota_write_addr = 0;

/* 内部函数 */
void updateFirmwareInfo(uint32_t version, uint8_t state, uint16_t size);

void otaInit(void) {
    vtor = SCB->VTOR;
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.NbPages = 1;
    erase_init.Banks = FLASH_BANK_1;
}

/* ota缓冲区结构 */
/* ----------------------------------- */
/* |ota数据类型| 数据长度 |帧编号|数据| */
/* ----------------------------------- */
/* |    0xXX   |0xXX 0xXX| 0xXX |    | */
/* ----------------------------------- */
void otaTask(uint16_t *notification) {
    uint8_t ota_data[600] = {0};
    
    if (bufferRead(&ota_buffer, ota_data, 0)) {
        uint8_t ota_type = ota_data[0];
        uint16_t ota_len = ota_data[1];
        ota_len = (ota_len << 8) + ota_data[2];
        uint8_t ota_seq = ota_data[3];
        (void)ota_seq;
        
        uint32_t page_error = 0;
        uint16_t page_count = 0;
        
        switch (ota_type) {
            /* 第一帧结构，告诉芯片要传输程序 */
            /* ------------------------------- */
            /* |        版本       | 固件大小 | */
            /* ------------------------------- */
            /* |0xXX 0xXX 0xXX 0xXX|0xXX 0xXX| */
            /* ------------------------------- */
            case OTA_MSG_START:
                version = ota_data[4];
                version = (version << 8) + ota_data[5];
                version = (version << 8) + ota_data[6];
                version = (version << 8) + ota_data[7];
                size = ota_data[8];
                size = (size << 8) + ota_data[9];
                if (size > APP_PROGRAM_MAX_SIZE) {     // 固件太大
                    *notification |= 0x4000;
                    break;
                }
                
                ota_start_flag = 1;
                
                /* 将固件信息标记为正在升级 */
                updateFirmwareInfo(version, IOS_UPDATING, size);
                
                /* 擦除app2区的数据 */
                HAL_FLASH_Unlock();
                
                page_count = (uint16_t)((size + FLASH_PAGE_SIZE - 1U) / FLASH_PAGE_SIZE);
                for (uint16_t i = 0; i < page_count; i ++) {
                    for (uint16_t j = 0; j < FLASH_PAGE_SIZE; j ++) {
                        /* 如果原本有数据则擦除 */
                        if (*(volatile uint8_t *)(APP2_PROGRAM_ADDR + i * FLASH_PAGE_SIZE + j) != 0xFF) {
                            
                            erase_init.PageAddress = APP2_PROGRAM_ADDR + i * FLASH_PAGE_SIZE;
                            HAL_FLASHEx_Erase(&erase_init, &page_error);
                                
                            break;
                        }
                    }
                }
                
                HAL_FLASH_Lock();
                
                /* 清空标志位 */
                msb_flag = 0;
                ota_write_addr = 0;
                ota_write_data = 0;
                
                /* 通知上位机芯片做好ota准备 */
                *notification |= 0x8000;
                
                /* 启动计时器，长时间收不到数据则标记固件损坏 */
                last_time = HAL_GetTick();
                
                break;
                
            /* 数据帧结构，告诉芯片传输的数据是什么 */
            /* ------ */
            /* |数据| */
            /* ------ */
            /* |    | */
            /* ------ */
            case OTA_MSG_DATA:
                if (ota_start_flag == 1) {
                    HAL_FLASH_Unlock();
                    for (uint16_t i = 0; i < ota_len; i ++) {
                        if (msb_flag == 0) {
                            ota_write_data = ota_data[4 + i];
                            msb_flag = 1;
                        }
                        else {
                            ota_write_data = ((uint16_t)ota_data[4 + i] << 8) + ota_write_data;
                            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP2_PROGRAM_ADDR + ota_write_addr, ota_write_data);
                            ota_write_addr  = ota_write_addr + 2;
                            msb_flag = 0;
                        }
                    }
                    HAL_FLASH_Lock();
                    
                    /* 通知上位机芯片处理好这一帧 */
                    *notification |= 0x8000;
                    
                    /* 更新计时器 */
                    last_time = HAL_GetTick();
                }
                else {
                    /* 通知数据错误 */
                    *notification |= 0x4000;
                }
                break;
                
            case OTA_MSG_END:
                if (ota_start_flag == 1) {
                    /* 如果有单独字节，先写入 */
                    if (msb_flag == 1) {
                        ota_write_data = ota_write_data + 0xFF00;
                        HAL_FLASH_Unlock();
                        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP2_PROGRAM_ADDR + ota_write_addr, ota_write_data);
                        HAL_FLASH_Lock();
                        ota_write_addr  = ota_write_addr + 1;
                    }
                    
                    /* 校验实际写入数据长度是否等于固件长度 */
                    if (ota_write_addr == size) {
                        /* 数据正确，保存固件信息，标记就绪状态 */
                        updateFirmwareInfo(version, IOS_READY, size);
                        
                        /* 通知上位机芯片处理好这一帧 */
                        *notification |= 0x8000;
                    }
                    else {
                        /* 通知数据错误 */
                        *notification |= 0x4000;
                        /* 数据有误，保存固件信息，标记错误状态 */
                        updateFirmwareInfo(version, IOS_BROKEN, size);
                    }
                    /* 退出ota进程 */
                    ota_start_flag = 0;
                    
                }
                else {
                    /* 通知数据错误 */
                    *notification |= 0x4000;
                }
                break;
            default:
                ;
        }
    }
    
    /* 传输超时处理 */
    if (ota_start_flag == 1) {
        uint32_t now_time = HAL_GetTick();
        if (now_time > last_time + 1000) {
            /* 通知错误 */
            *notification |= 0x4000;
            /* 数据有误，保存固件信息，标记错误状态 */
            updateFirmwareInfo(version, IOS_BROKEN, size);
            /* 退出ota进程 */
            ota_start_flag = 0;
        }
    }
}

void updateFirmwareInfo(uint32_t version, uint8_t state, uint16_t size) {
    /* 首先读取旧固件信息 */
    uint32_t app1_version = *(volatile uint32_t *)APP1_VERSION_ADDR;
    uint32_t app2_version = *(volatile uint32_t *)APP2_VERSION_ADDR;
    uint32_t app1_state   = *(volatile uint32_t *)APP1_IOS_STATE;
    uint32_t app2_state   = *(volatile uint32_t *)APP2_IOS_STATE;
    uint32_t app1_size    = *(volatile uint32_t *)APP1_IOS_SIZE;
    uint32_t app2_size    = *(volatile uint32_t *)APP2_IOS_SIZE;
    
    /* 修改app2固件信息 */
    app2_version = version;
    app2_state = state;
    app2_size = size;
    
    HAL_FLASH_Unlock();
    erase_init.PageAddress = APP1_VERSION_ADDR;
    uint32_t page_error = 0;
    HAL_FLASHEx_Erase(&erase_init, &page_error);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_VERSION_ADDR, app1_version);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_STATE, app1_state);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP1_IOS_SIZE, app1_size);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_VERSION_ADDR, app2_version);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_IOS_STATE, app2_state);
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP2_IOS_SIZE, app2_size);
    HAL_FLASH_Lock();
}
