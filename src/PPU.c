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
    ppu->secondaryAddr = 0;
    ppu->oddFrame = false;
    ppu->frameComple = false;
    ppu->nmi = false;
    ppu->nmiPending = false;
    ppu->nmiDelay = 0;
    ppu->ioBus = 0;
    
    memset(ppu->name_table, 0, sizeof(ppu->name_table));
    memset(ppu->palette, 0, sizeof(ppu->palette));
    memset(ppu->oam, 0xFF, sizeof(ppu->oam));
    memset(ppu->secondaryOam, 0xFF, sizeof(ppu->secondaryOam));
    memset(ppu->secondaryOamIndex, 0xFF, sizeof(ppu->secondaryOamIndex));
}

uint8_t PPU_read(PPU* ppu, uint16_t addr)
{
    addr &= 0x3FFF;
    uint8_t data = 0x00;

    if (addr <= 0x1FFF) {
        data = ppu->cart->mapper.chr_read(&ppu->cart->mapper, addr);
    } else if (addr < 0x3F00) {
        addr &= 0x0FFF;
        uint16_t tableAddr = ppu->cart->mapper.name_table_map[addr / 0x0400];
        data = ppu->name_table[(tableAddr - 0x2000) + (addr & 0x03FF)];
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint16_t paletteAddr = addr & 0x001F;
        if ((paletteAddr & 0x03) == 0 && (paletteAddr & 0x10))
            paletteAddr &= 0x000F;
        data = ppu->palette[paletteAddr & 0x001F] & 0x3F;
        data |= (ppu->ioBus & 0xC0);
    }
    ppu->ioBus = data;
    return data;
}

void PPU_write(PPU* ppu, uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;
    ppu->ioBus = data;

    if (addr <= 0x1FFF) {
        ppu->cart->mapper.chr_write(&ppu->cart->mapper, addr, data);
    } else if (addr < 0x3F00) {
        addr &= 0x0FFF;
        uint16_t tableAddr = ppu->cart->mapper.name_table_map[addr / 0x0400];
        ppu->name_table[(tableAddr - 0x2000) + (addr & 0x03FF)] = data;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        uint16_t paletteAddr = addr & 0x001F;
        if ((paletteAddr & 0x03) == 0 && (paletteAddr & 0x10))
            paletteAddr &= 0x000F;
        ppu->palette[paletteAddr & 0x001F] = data & 0x3F;
    }
}

// Helper functions
static void increment_coarse_x(PPU* ppu)
{
    if ((ppu->v & COARSE_X) == COARSE_X) {
        ppu->v &= ~COARSE_X;
        ppu->v ^= NAMETBL_X;
    } else {
        ppu->v++;
    }
}

static void increment_y(PPU* ppu)
{
    if ((ppu->v & FINE_Y) != FINE_Y) {
        ppu->v += 0x1000;
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

static void load_bg_shifters(PPU* ppu)
{
    ppu->bgShiftPtrLow = (ppu->bgShiftPtrLow & 0xFF00) | ppu->bgNextTileLow;
    ppu->bgShiftPtrHigh = (ppu->bgShiftPtrHigh & 0xFF00) | ppu->bgNextTileHigh;
    
    uint8_t attr_lo = (ppu->bgNextTileAttr & 0x01) ? 0xFF : 0x00;
    uint8_t attr_hi = (ppu->bgNextTileAttr & 0x02) ? 0xFF : 0x00;
    
    ppu->bgShiftAttrLow = (ppu->bgShiftAttrLow & 0xFF00) | attr_lo;
    ppu->bgShiftAttrHigh = (ppu->bgShiftAttrHigh & 0xFF00) | attr_hi;
}

static void update_shifters(PPU* ppu)
{
    if(ppu->mask & MSK_BG)
    {
        ppu->bgShiftPtrLow <<= 1;
        ppu->bgShiftPtrHigh <<= 1;
        ppu->bgShiftAttrLow <<= 1;
        ppu->bgShiftAttrHigh <<= 1;
    }
}

void PPU_clock(PPU* ppu)
{
    if (ppu->nmiPending)
    {
        if (!(ppu->ctrl & CTRL_VBLANK))
        {
            ppu->nmiPending = false;
        }
        else if (ppu->nmiDelay > 0)
        {
            ppu->nmiDelay--;
            if (ppu->nmiDelay == 0)
            {
                ppu->nmiPending = false;
                ppu->nmi = true;
            }
        }
        else
        {
            ppu->nmiPending = false;
            ppu->nmi = true;
        }
    }

    if(ppu->frameComple)
        return;

    uint16_t pixel = ppu->pixels;
    uint16_t scanline = ppu->scanLines;   
    
    uint8_t ptrAddr;
    uint8_t backPriority, len, attr;
    uint16_t tileAddr, attrAddr, addr;
    
    if(scanline < VISIBLE_SCANLINES || scanline == ppu->scanLinesPerFame - 1)
    { // 0 - 239 and 261
        if((pixel >= 2 && pixel < 258) || (pixel >= 321 && pixel < 338))
        {
            update_shifters(ppu);
            switch((pixel - 1) & 0x07)
            {
                case 0:
                    load_bg_shifters(ppu);
                    if(ppu->mask & MSK_SHOW_ALL)
                        ppu->bgNextTileId = PPU_read(ppu, 0x2000 | (ppu->v & 0x0FFF));
                    break;
                case 2:
                    if(ppu->mask & MSK_SHOW_ALL)
                    {
                        addr = 0x23C0 | (ppu->v & 0x0C00);
                        addr |= ((ppu->v >> 4) & 0x38);
                        addr |= ((ppu->v >> 2) & 0x07);
                        
                        attr = PPU_read(ppu, addr);
                        uint8_t shift = ((ppu->v >> 4) & 4) | (ppu->v & 2);
                        ppu->bgNextTileAttr = (attr >> shift) & 0x3;
                    }
                    break;
                case 4:
                    if(ppu->mask & MSK_SHOW_ALL)
                    {
                        addr = ppu->bgNextTileId << 4;
                        addr += (ppu->v >> 12) & 0x07;
                        addr |= (ppu->ctrl & CTRL_BG) << 8;
                        ppu->bgNextTileLow = PPU_read(ppu, addr);
                    }
                    break;
                case 6:
                    if(ppu->mask & MSK_SHOW_ALL)
                    {
                        addr = ppu->bgNextTileId << 4;
                        addr += (ppu->v >> 12) & 0x07;
                        addr |= (ppu->ctrl & CTRL_BG) << 8;
                        ppu->bgNextTileHigh = PPU_read(ppu, addr + 8);
                    }
                    break;
                    
                case 7:
                    if(ppu->mask & MSK_SHOW_ALL)
                        increment_coarse_x(ppu);
                    break;
            }
        }
    }

    if(scanline < VISIBLE_SCANLINES)
    { // 0 - 239
        if(pixel >= 1 && pixel <= 256)
        {// Visible pixel rendering
            uint8_t bg_pixel = 0;
            uint8_t bg_palette = 0;

            if(ppu->mask & MSK_BG)
            {
                if((ppu->mask & MSK_BG_8) || pixel >= 9) 
                {
                    uint16_t bit_mask = 0x8000 >> ppu->x;
                    
                    uint8_t pixel_lo = (ppu->bgShiftPtrLow & bit_mask) ? 1 : 0;
                    uint8_t pixel_hi = (ppu->bgShiftPtrHigh & bit_mask) ? 1 : 0;
                    bg_pixel = (pixel_hi << 1) | pixel_lo;
                    
                    uint8_t pal_lo = (ppu->bgShiftAttrLow & bit_mask) ? 1 : 0;
                    uint8_t pal_hi = (ppu->bgShiftAttrHigh & bit_mask) ? 1 : 0;
                    bg_palette = (pal_hi << 1) | pal_lo;
                }
            }

            uint8_t sprite_pixel = 0;
            uint8_t sprite_palette = 0;
            bool sprite_has_priority = false;

            if(ppu->mask & MSK_SPR)
            {
                if(ppu->mask & MSK_SPR_8 || pixel >= 9)
                {
                    uint8_t len = (ppu->ctrl & CTRL_SPR_SIZE) ? 16 : 8;
                    
                    for(int i = 0; i < ppu->secondaryAddr; i++)
                    {
                        uint8_t spriteIndex = ppu->secondaryOamIndex[i];
                        if(spriteIndex == 0xFF)
                            continue;

                        OAM sprite = ppu->secondaryOam[i];

                        int16_t y_offset = scanline - (sprite.y + 1);
                        int16_t x_offset = (pixel - 1) - sprite.x;
                        if(x_offset < 0 || x_offset >= 8)
                            continue;

                        uint8_t row = (uint8_t)y_offset;
                        uint8_t col = (uint8_t)x_offset;

                        if(sprite.attr & OAM_FLIP_Y)
                            row = (uint8_t)((len - 1) - row);

                        if(sprite.attr & OAM_FLIP_X)
                            col = (uint8_t)(7 - col);
                        
                        uint16_t pattern_addr;
                        
                        if(len == 16)
                        {
                            uint16_t bank = (sprite.idx & 0x01) ? 0x1000 : 0x0000;
                            uint8_t tile_num = sprite.idx & 0xFE;
                            if(row >= 8)
                            {
                                tile_num++;
                                row -= 8;
                            }
                            pattern_addr = bank | (tile_num << 4) | row;
                        }
                        else
                        {
                            uint16_t bank = (ppu->ctrl & CTRL_SP_TABLE) ? 0x1000 : 0x0000;
                            pattern_addr = bank | (sprite.idx << 4) | row;
                        }
                        
                        uint8_t pattern_lo = PPU_read(ppu, pattern_addr);
                        uint8_t pattern_hi = PPU_read(ppu, pattern_addr + 8);
                        
                        uint8_t bit = 7 - col;
                        uint8_t pixel_lo = (pattern_lo >> bit) & 0x01;
                        uint8_t pixel_hi = (pattern_hi >> bit) & 0x01;
                        sprite_pixel = (pixel_hi << 1) | pixel_lo;
                        
                        if(sprite_pixel == 0)
                            continue;
                        
                        sprite_palette = (sprite.attr & 0x03) + 4;
                        sprite_has_priority = (sprite.attr & OAM_PRIORITY) == 0;
                        
                        if((ppu->mask & MSK_BG) && !(ppu->status & STS_0_HIT) &&
                           bg_pixel && pixel < 256 && spriteIndex == 0)
                        {
                            ppu->status |= STS_0_HIT;
                        }
                        break; 
                    }
                }
            }
            uint8_t final_pixel = 0;
            uint8_t final_palette = 0;
            
            if(bg_pixel == 0 && sprite_pixel == 0)
            {
                final_pixel = 0;
                final_palette = 0;
            }
            else if(bg_pixel == 0 && sprite_pixel > 0)
            {
                final_pixel = sprite_pixel;
                final_palette = sprite_palette;
            }
            else if(bg_pixel > 0 && sprite_pixel == 0)
            {
                final_pixel = bg_pixel;
                final_palette = bg_palette;
            }
            else
            {
                if(sprite_has_priority)
                {
                    final_pixel = sprite_pixel;
                    final_palette = sprite_palette;
                }
                else
                {
                    final_pixel = bg_pixel;
                    final_palette = bg_palette;
                }
            }
            
            uint8_t palette_addr = (final_palette << 2) | final_pixel;
            uint8_t color = ppu->palette[palette_addr] & 0x3F;
            ppu->screen[(scanline * PPU_SCANLINE) + (pixel - 1)] = nes_palette[color];
        }

        if(pixel == 256)
            if(ppu->mask & MSK_SHOW_ALL)
                increment_y(ppu);

        if(pixel == 257)
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
                ppu->nmiPending = true;
                ppu->nmiDelay = 6;
            }
        }
    }
    else if(scanline == ppu->scanLinesPerFame-1)
    {
        if(pixel == 1)
        {
            ppu->status &= ~(STS_VBLANK | STS_0_HIT | STS_OVERFLOW);
            ppu->nmi = false;
            ppu->nmiPending = false;
            ppu->nmiDelay = 0;
        }
        if(pixel == 256)
            if(ppu->mask & MSK_SHOW_ALL)
                increment_y(ppu);

        if(pixel == 257)
            if(ppu->mask & MSK_SHOW_ALL)
                ppu->v = (ppu->v & 0x7BE0) | (ppu->t & 0x041F);

        if(pixel >= 280 && pixel <= 304)
            if (ppu->mask & MSK_SHOW_ALL)
                ppu->v = (ppu->v & 0x041F) | (ppu->t & 0x7BE0);
        
        if(pixel == 339 && ppu->oddFrame && (ppu->mask & MSK_SHOW_ALL)) 
        { 
            ppu->pixels++;
        }
    }

    if((scanline < VISIBLE_SCANLINES) || (scanline == ppu->scanLinesPerFame - 1))
    {
        if(pixel == 320)
        {
            if(ppu->mask & MSK_SHOW_ALL)
            {
                memset(ppu->secondaryOam, 0xFF, sizeof(ppu->secondaryOam));
                memset(ppu->secondaryOamIndex, 0xFF, sizeof(ppu->secondaryOamIndex));
                ppu->secondaryAddr = 0;
                uint8_t len = (ppu->ctrl & CTRL_SPR_SIZE) ? 16 : 8;
                uint16_t next_scanline = (scanline + 1) % ppu->scanLinesPerFame;
                for(int i = 0; i < 64; i++)
                {
                    if(ppu->secondaryAddr == 8) break;
                    int16_t diff = next_scanline - (ppu->oam[i].y + 1);
                    if(diff >= 0 && diff < len)
                    {
                        ppu->secondaryOam[ppu->secondaryAddr] = ppu->oam[i];
                        ppu->secondaryOamIndex[ppu->secondaryAddr] = i;
                        ppu->secondaryAddr++;
                    }
                }
            }
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
    switch(addr)
    {
        case PPUCTRL:
        {
            ppu->ctrl = data;
            ppu->t &= ~(NAMETBL_X | NAMETBL_Y);
            ppu->t |= (ppu->ctrl & CTRL_NAMETABLE) << 10;
            ppu->t &= 0x3FFF;
            if (!(ppu->ctrl & CTRL_VBLANK))
            {
                ppu->nmiPending = false;
                ppu->nmiDelay = 0;
            }
            else if ((ppu->ctrl & CTRL_VBLANK) && (ppu->status & STS_VBLANK))
            {
                ppu->nmiPending = true;
                ppu->nmiDelay = 6;
            }
            
            break;
        }
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
            
            switch(ppu->oamAddr++ & 0x3)
            {
            case 0: ppu->oam[oamIdx].y = data; break;
            case 1: ppu->oam[oamIdx].idx = data; break;
            case 2: ppu->oam[oamIdx].attr = data; break;
            case 3: ppu->oam[oamIdx].x = data; break;
            }

            break;
        case PPUSCROLL:
            if(!ppu->w)
            {
                ppu->t &= ~COARSE_X;
                ppu->t |= (data >> 3) & 0x1F;
                ppu->t &= 0x3FFF;
                ppu->x = data & 0x07;
                ppu->w = 1;
            }
            else
            {
                ppu->t &= ~(COARSE_Y | FINE_Y);
                ppu->t |= (data << 5) & COARSE_Y;
                ppu->t |= (data << 12) & FINE_Y;
                ppu->t &= 0x3FFF;
                ppu->w = 0;
            }
            break;
        case PPUADDR:
            if(!ppu->w)
            {
                ppu->t = (ppu->t & 0x00FF) | ((uint16_t)(data & 0x3F) << 8);
                ppu->t &= 0x3FFF;
                ppu->w = 1;
            }
            else
            {
                ppu->t = (ppu->t & 0xFF00) | data;
                ppu->t &= 0x3FFF;
                ppu->v = ppu->t;
                ppu->w = 0;
            }
            break;
        case PPUDATA:
            PPU_write(ppu, ppu->v, data);
            ppu->v += (ppu->ctrl & CTRL_INCREMENT) ? 32 : 1;
            break;
    }
    ppu->ioBus = data;
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
            ppu->nmiPending = false;
            ppu->nmiDelay = 0;
            ppu->w = 0;
            ppu->ioBus = data;
            return data;
        case OAMDATA:
        {
            uint8_t oamIdx = ppu->oamAddr >> 2;
            switch(ppu->oamAddr & 0x3)
            {
            case 0: data = ppu->oam[oamIdx].y; break;
            case 1: data = ppu->oam[oamIdx].idx; break;
            case 2: data = ppu->oam[oamIdx].attr; break;
            case 3: data = ppu->oam[oamIdx].x; break;
            }
            ppu->ioBus = data;
            return data;
        }
        case PPUDATA:
        {
            uint16_t addrBus = ppu->v;
            data = ppu->dataBus;
            uint8_t value = PPU_read(ppu, addrBus);
            if(addrBus >= 0x3F00)
            {
                uint8_t busBackup = ppu->ioBus;
                data = value;
                uint16_t bufferAddr = addrBus & 0x2FFF;
                ppu->dataBus = PPU_read(ppu, bufferAddr);
                ppu->ioBus = busBackup;
            }
            else
            {
                ppu->dataBus = value;
            }
            ppu->v += (ppu->ctrl & CTRL_INCREMENT) ? 32 : 1;
            ppu->ioBus = data;
            return data;
        }
    }
    ppu->ioBus = ppu->dataBus;
    return ppu->dataBus;
}

void PPU_free(PPU* ppu)
{
    free(ppu->screen);
}