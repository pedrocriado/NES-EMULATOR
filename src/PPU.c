#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PPU.h"
#include "CPU6502.h"

void PPU_init(PPU* ppu)
{
    ppu->screen = malloc(sizeof(uint32_t) * VISIBLE_SCANLINES * VISIBLE_DOTS);
    memset(ppu->screen, 0, sizeof(uint32_t) * VISIBLE_SCANLINES * VISIBLE_DOTS);

    ppu->scanLines = 0;
    ppu->pixels = 0;
    ppu->oamCacheLen = 0;
    ppu->oddFrame = false;
    ppu->frameComple = false;
    ppu->nmi = false;
}

uint8_t PPU_read(PPU* ppu, uint16_t addr)
{
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        return ppu->cart->mapper.chr_read(&ppu->cart->mapper, addr);
    } else if (addr < 0x3F00) {
        addr &= 0x0FFF;
        uint16_t tableAddr = ppu->cart->mapper.name_table_map[addr/0x0400];
        return ppu->name_table[(tableAddr - 0x2000) + (addr & 0x03FF)];
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x1F;
        return ppu->palette[addr] & 0x3F;
    }
    return 0;
}

void PPU_write(PPU* ppu, uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;

    if (addr <= 0x1FFF) {
        ppu->cart->mapper.chr_write(&ppu->cart->mapper, addr, data);
    } else if (addr <= 0x2FFF) {
        addr &= 0x0FFF;  
        uint16_t tableAddr = ppu->cart->mapper.name_table_map[addr/0x0400];
        ppu->name_table[(tableAddr - 0x2000) + (addr & 0x03FF)] = data;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x1F;
        if ((addr & 0x10) && (addr % 4 == 0))
        {
            ppu->palette[addr ^ 0x10] = data;
        }
        ppu->palette[addr] = data;
    }
}

// Helper functions
static void increment_coarse_x(PPU* ppu)
{
    if ((ppu->v & COARSE_X) == COARSE_X) {
        ppu->v &= ~COARSE_X;
        ppu->v ^= NAMETBL_X;
    } else {
        ppu->v += 1;
    }
}

static void increment_y(PPU* ppu)
{
    if ((ppu->v & FINE_Y) != FINE_Y) {
        ppu->v += 0x1000; // increment fine Y
    } else {
        ppu->v &= ~FINE_Y;
        uint16_t y = (ppu->v & COARSE_Y) >> 5;
        if (y == 29) {
            y = 0;
            ppu->v ^= NAMETBL_Y;
        } else if (y == 31) {
            y = 0;
        } else {
            y++;
        }
        ppu->v = (ppu->v & ~COARSE_Y) | (y << 5);
    }
}

static void evaluate_sprites(PPU* ppu, uint16_t scanline)
{
    ppu->oamCacheLen = 0;
    uint8_t len = (ppu->ctrl & CTRL_SPR_SIZE) ? 16 : 8;
    
    for(int i = 0; i < 64 && ppu->oamCacheLen < 8; i++)
    {
        int16_t diff = scanline - ppu->oam[i].y;
        if(diff >= 0 && diff < len) {
            ppu->cacheOam[ppu->oamCacheLen++] = i;
        }
    }
}

void PPU_clock(PPU* ppu)
{
    if(ppu->frameComple)
        return;

    uint16_t pixel = ppu->pixels;
    uint16_t scanline = ppu->scanLines;   
    
    if (scanline == 261 && pixel == 1) {
        ppu->status &= ~(STS_VBLANK | STS_0_HIT | STS_OVERFLOW); 
    }

    if(ppu->scanLines < VISIBLE_SCANLINES)
    { // 0 - 239
        if(pixel == 64) {
            evaluate_sprites(ppu, scanline);
        }

        if(pixel > 0 && pixel <= 256)
        {// Visible pixel rendering
            uint8_t fineX = ppu->x;
            uint8_t pltAddr = 0, pltAddrSp = 0, ptrAddr = 0;
            uint8_t backPriority = 0, len =  0, attr = 0;
            uint8_t lowPltAddr = 0, highPltAddr = 0;
            uint16_t tileAddr = 0, attrAddr = 0;

            if(ppu->mask & MSK_BG)
            {
                if((ppu->mask & MSK_BG_8) || pixel >= 8) 
                {
                    tileAddr = 0x2000 | (ppu->v & 0x0FFF);
                    
                    ptrAddr = PPU_read(ppu, tileAddr) * 16;
                    ptrAddr += (ppu->v & 0xE000) >> 9; 
                    ptrAddr |= (ppu->ctrl & CTRL_BG) << 8;

                    lowPltAddr = (PPU_read(ppu, ptrAddr) >> (7 - fineX)) & 1;
                    highPltAddr = (PPU_read(ppu, ptrAddr + 8) >> (7 - fineX)) & 1;

                    pltAddr = (highPltAddr << 1) | lowPltAddr;
                    
                    
                    attrAddr = 0x23C0 | (ppu->v & 0x0C00);
                    attrAddr |= ((ppu->v >> 4) & 0x38);
                    attrAddr |= ((ppu->v >> 2) & 0x07); 

                    attr = PPU_read(ppu, attrAddr);
                    uint8_t shift = ((ppu->v >> 4) & 4) | (ppu->v & 2);
                    pltAddr |= (((attr >> shift) & 0x3) << 2);
                }
                if((pixel % 8) == 0)
                {
                    increment_coarse_x(ppu);
                }
            }
            if(ppu->mask & MSK_SPR)
            {
                if(ppu->mask & MSK_SPR_8 || pixel >= 9)
                {
                    len = (ppu->ctrl & CTRL_SPR_SIZE) ? 16 : 8;
                    for(int i = 0; i < ppu->oamCacheLen; i++)
                    {
                        OAM sprite = ppu->oam[ppu->cacheOam[i]];

                        if(pixel -1 - sprite.x  < 0 || pixel - sprite.x >= 8) continue;

                        uint8_t xOff = (pixel -1 - sprite.x) % 8;
                        uint8_t yOff = (scanline - sprite.y) % len;
                        
                        if((sprite.attr & OAM_FLIP_X))
                            xOff ^= 7;
                        if((sprite.attr & OAM_FLIP_Y))
                            yOff ^= len - 1;

                        tileAddr = 0;

                        if(len == 16)
                        {
                            yOff = (yOff & 0x08) | (yOff & 0x07);
                            tileAddr = (sprite.idx >> 1) * 32 + yOff;
                            tileAddr |= (sprite.idx & 1) << 12;
                        }
                        else
                        {
                            tileAddr = sprite.idx*16 + yOff;
                            if(ppu->ctrl & CTRL_SP_TABLE)
                                tileAddr += 0x1000;
                        }
                        
                        pltAddrSp = (PPU_read(ppu, tileAddr) >> xOff) & 1;
                        pltAddrSp |= ((PPU_read(ppu, tileAddr + 8) >> xOff) & 1) << 1;

                        if(pltAddrSp == 0) continue;

                        pltAddrSp |= 0x10;
                        pltAddrSp |= (sprite.attr & CTRL_NAMETABLE) << 2;
                        backPriority = sprite.attr & OAM_PRIORITY;

                        if(!(ppu->status & STS_0_HIT) && (ppu->mask & MSK_BG)
                        &&  ppu->cacheOam[i] == 0    && pltAddrSp
                        &&  pltAddr                  && pixel < 256)
                        {
                            ppu->status |= STS_0_HIT;
                        } 
                    }
                }
                if(pltAddrSp && (!pltAddr || (pltAddr && backPriority)))
                {
                    pltAddr = pltAddrSp;
                }
            }
            ppu->screen[scanline * PPU_SCANLINE + pixel-1] = nes_palette[ppu->palette[pltAddr]];
        }

        else if(pixel == 256)
            if(ppu->mask & MSK_SHOW_ALL)
                increment_y(ppu);

        else if(pixel == 257)
            if(ppu->mask & MSK_SHOW_ALL)
                ppu->v = (ppu->v & 0x7BE0) | (ppu->t & 0x041F);

    }
    else if(scanline < ppu->scanLinesPerFame - 1)
    { // 241 - 260/310
        if(scanline == 241 && pixel == 1)
        {
            ppu->status |= STS_VBLANK;
            if(ppu->ctrl & CTRL_VBLANK)
            {
                ppu->nmi = true;
            }
        }
    }
    else if(scanline  < ppu->scanLinesPerFame)
    { // 261
        if(pixel == 1)
        {
            ppu->status &= ~(STS_VBLANK | STS_0_HIT | STS_OVERFLOW);
            ppu->nmi = false;
        }
        if(pixel == 256 && (scanline == ppu->scanLinesPerFame - 1))
            if(ppu->mask & MSK_SHOW_ALL)
                increment_y(ppu);

        if(pixel == 257 && (scanline == ppu->scanLinesPerFame - 1))
            if(ppu->mask & MSK_SHOW_ALL)
                ppu->v = (ppu->v & 0x7BE0) | (ppu->t & 0x041F);
            
        if(pixel >= 280 && pixel <= 304)
            if (ppu->mask & MSK_SHOW_ALL)
                ppu->v = (ppu->v & 0x041F) | (ppu->t & 0x7BE0);

        if(pixel == 339 && ppu->oddFrame && (ppu->mask & MSK_SHOW_ALL)) 
        { 
            ppu->pixels = 0;
            ppu->scanLines = 0;
            ppu->oddFrame = !ppu->oddFrame;
            ppu->frameComple = true;
            return;
        }
    }

    // Increment pixel
    if(++ppu->pixels >= ALL_DOTS)
    {
        ppu->pixels = 0;
        ppu->scanLines++;
        if(ppu->scanLines == ppu->scanLinesPerFame)
        {
            ppu->scanLines = 0;
            ppu->oddFrame = !ppu->oddFrame;
            ppu->frameComple = true;
        }
    }
}

void PPU_set_register(PPU* ppu, uint16_t addr, uint8_t data)
{
    //printf("PSR: %4x\n", addr);
    switch(addr)
    {
        case PPUCTRL:
            ppu->ctrl = data;
            ppu->t &= ~(NAMETBL_X | NAMETBL_Y);
            ppu->t |= (ppu->ctrl & CTRL_NAMETABLE) << 10;
            break;
        case PPUMASK:
            ppu->mask = data;
            break;
        case PPUSTATUS:
            ppu->w = 0;
            break;
        case OAMADDR:
            ppu->oamAddr = data;
            break;
        case OAMDATA:
            uint8_t oamIdx = ppu->oamAddr >> 2;
            
            switch(ppu->oamAddr & 0x3)
            {
            case 0:
                ppu->oam[oamIdx].y = data;
                break;
            case 1:
                ppu->oam[oamIdx].idx = data;
                break;
            case 2:
                ppu->oam[oamIdx].attr = data;
                break;
            case 3:
                ppu->oam[oamIdx].x = data;
                break;
            }

            ppu->oamAddr++;

            break;
        case PPUSCROLL:
            if(!ppu->w)
            {
                ppu->t &= ~COARSE_X;
                ppu->t |= (data >> 3);
                ppu->x = data & 0x07;
                ppu->w = 1;
            }
            else
            {
                ppu->t &= ~COARSE_Y;
                ppu->t |= (data << 5) & COARSE_Y;
                ppu->t &= ~FINE_Y;
                ppu->t |= (data << 12) & FINE_Y;
                ppu->w = 0;
            }
            break;
        case PPUADDR:
            if(!ppu->w)
            {
                ppu->t = ((data & 0x3F) << 8) | (ppu->t & 0x00FF);
                ppu->w = 1;
            }
            else
            {
                ppu->t = (ppu->t & 0xFF00) | data;
                ppu->v = ppu->t;
                ppu->w = 0;
            }
            break;
        case PPUDATA:
            PPU_write(ppu, ppu->v, data);
            ppu->v += (ppu->ctrl & CTRL_INCREMENT) ? 32 : 1;
            break;
    }
}

uint8_t PPU_get_register(PPU* ppu, uint16_t addr)
{
    uint8_t data = 0x00;

    switch(addr)
    {
        case PPUSTATUS:
            data = (ppu->status & 0xE0);
            data |= (ppu->dataBus & 0x1F);
            ppu->status &= ~STS_VBLANK;
            ppu->w = 0;
            return data;
        case OAMDATA:
            uint8_t oamIdx = ppu->oamAddr >> 2;
            
            switch(ppu->oamAddr & 0x3)
            {
            case 0: return ppu->oam[oamIdx].y;
            case 1: return ppu->oam[oamIdx].idx;
            case 2: return ppu->oam[oamIdx].attr;
            case 3: return ppu->oam[oamIdx].x;
            }
        case PPUDATA:
            data = ppu->dataBus;
            ppu->dataBus = PPU_read(ppu, ppu->v);
            if(ppu->v >= 0x3F00)
                data = ppu->dataBus;
            ppu->v += (ppu->ctrl & CTRL_INCREMENT) ? 32 : 1;
            return data;
    }
    return ppu->dataBus;
}

void PPU_free(PPU* ppu)
{
    free(ppu->screen);
}