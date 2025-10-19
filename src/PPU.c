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
    memset(ppu->name_table, 0, sizeof(ppu->name_table));
    memset(ppu->palette, 0, sizeof(ppu->palette));
    memset(ppu->oam, 0xFF, sizeof(ppu->oam));
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
    } else if (addr < 0x3F00) {
        addr &= 0x0FFF;  
        uint16_t tableAddr = ppu->cart->mapper.name_table_map[addr/0x0400];
        ppu->name_table[(tableAddr - 0x2000) + (addr & 0x03FF)] = data;
    } else if (addr >= 0x3F00 && addr <= 0x3FFF) {
        addr &= 0x1F;
        if (addr == 0x10) addr = 0x00;
        else if (addr == 0x14) addr = 0x04;
        else if (addr == 0x18) addr = 0x08;
        else if (addr == 0x1C) addr = 0x0C;
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

static void evaluate_sprites(PPU* ppu)
{
    uint16_t pixel = ppu->pixels;
    uint16_t scanline = ppu->scanLines;

    uint8_t sprHeight = (ppu->ctrl & CTRL_SPR_SIZE) ? 16 : 8;

    if(pixel >=1 && pixel <=64)
    {
        if(pixel == 1) 
        {
            ppu->secondaryAddr = 0;
        }
        else if(pixel & 0x01 == 0)
        {
            ppu->secondaryAddr++;
        }
        else
        {
            switch(ppu->secondaryAddr & 0x3)
            {
                case 0:
                    ppu->oam[ppu->secondaryAddr >> 2].y = 0xFF;
                    break;
                case 1:
                    ppu->oam[ppu->secondaryAddr >> 2].idx = 0xFF;
                    break;
                case 2:
                    ppu->oam[ppu->secondaryAddr >> 2].attr = 0xFF;
                    break;
                case 3:
                    ppu->oam[ppu->secondaryAddr >> 2].x = 0xFF;
                    break;
            }
        }
        
        if(pixel == 64)
        {
            ppu->secondaryAddr = 0;
            ppu->primaryCursor = 0;
            ppu->secondaryCursor = 0;
        }
    }
    else if(pixel >= 65 && pixel <=256)
    {
        if (pixel & 0x01) {
            ppu->tmpOAM = ppu->oam[ppu->primaryCursor];
            if (ppu->sprM == 0) {
                ppu->sprTemp = ppu->oam[ppu->sprN].y;
            }
        } 
        if (pixel >= 65 && pixel <= 256) 
        {
            if (pixel & 1) {
                if (ppu->sprN < 64) {
                    uint8_t* oam_ptr = (uint8_t*)&ppu->oam[ppu->sprN];
                    ppu->sprTemp = oam_ptr[ppu->sprM];
                } else {
                    ppu->sprTemp = 0xFF;
                }
            }
        }
        // On even cycles: Process the read data
        else 
        {
            // If we're done evaluating all sprites
            if (ppu->sprDone) {
                // Sprite overflow bug checking
                if (ppu->sprN < 64) {
                    // Check if current sprite is in range
                    int16_t diff = scanline - ppu->sprTemp;
                    
                    if (diff >= 0 && diff < sprHeight) {
                        // Sprite overflow detected
                        ppu->status |= STS_OVERFLOW;
                    }
                    
                    // Hardware bug: increment both n and m
                    ppu->sprM++;
                    if (ppu->sprM >= 4) {
                        ppu->sprM = 0;
                        ppu->sprN++;
                    }
                    
                    // Also increment n (bug behavior)
                    if (ppu->sprM == 0) {
                        ppu->sprN++;
                    }
                }
            }
            else if (ppu->secondaryAddr < 32) 
            {
                // Not currently copying a sprite
                if (!ppu->sprCopying) 
                {
                    // Check if sprite Y is in range (m should be 0 here)
                    int16_t diff = scanline - ppu->sprTemp;
                    
                    if (diff >= 0 && diff < sprHeight) 
                    {
                        // Sprite is in range, start copying
                        ppu->sprCopying = true;
                        
                        // Write Y coordinate to secondary OAM
                        ((uint8_t*)ppu->secondaryOam)[ppu->secondaryAddr] = ppu->sprTemp;
                        ppu->secondaryAddr++;
                        ppu->sprM = 1;
                    } 
                    else 
                    {
                        // Not in range, move to next sprite
                        ppu->sprN++;
                        ppu->sprM = 0;
                        
                        // Check if we've evaluated all 64 sprites
                        if (ppu->sprN >= 64) {
                            ppu->sprDone = true;
                        }
                    }
                }
                // Currently copying a sprite (m = 1, 2, or 3)
                else 
                {
                    // Write byte to secondary OAM
                    ((uint8_t*)ppu->secondaryOam)[ppu->secondaryAddr] = ppu->sprTemp;
                    ppu->secondaryAddr++;
                    ppu->sprM++;
                    
                    // Finished copying this sprite
                    if (ppu->sprM >= 4) {
                        ppu->sprCopying = false;
                        ppu->sprM = 0;
                        ppu->sprN++;
                        
                        // Check if we've evaluated all 64 sprites
                        if (ppu->sprN >= 64) {
                            ppu->sprDone = true;
                        }
                        
                        // Check if secondary OAM is full (8 sprites found)
                        if (ppu->secondaryAddr >= 32) {
                            // Switch to overflow checking mode
                            ppu->sprDone = true;
                            ppu->sprM = 0;
                        }
                    }
                }
            }
        }
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
    if (ppu->mask & MSK_BG) {
        ppu->bgShiftPtrLow <<= 1;
        ppu->bgShiftPtrHigh <<= 1;
        ppu->bgShiftAttrLow <<= 1;
        ppu->bgShiftAttrHigh <<= 1;
    }
}

void PPU_clock(PPU* ppu)
{
    if(ppu->frameComple)
        return;

    uint16_t pixel = ppu->pixels;
    uint16_t scanline = ppu->scanLines;   
    
    uint8_t ptrAddr;
    uint8_t backPriority, len, attr;
    uint16_t tileAddr, attrAddr, addr;
    
    if(scanline < VISIBLE_SCANLINES || scanline == ppu->scanLinesPerFame - 1)
    { // 0 - 239 and 261
        if((pixel >= 1 && pixel <=256) || (pixel >= 321 && pixel <336))
        {
            update_shifters(ppu);

            switch((pixel -1) & 0x07)
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
        evaluate_sprites(ppu);
        if(pixel >= 1 && pixel <= 256)
        {// Visible pixel rendering
            uint8_t bg_pixel = 0;
            uint8_t bg_palette = 0;

            if(ppu->mask & MSK_BG)
            {
                if((ppu->mask & MSK_BG_8) || pixel >= 9) 
                {
                    uint16_t bit_mask = 0x8000 >> ppu->x;
                    
                    // Extract 2-bit pixel value
                    uint8_t pixel_lo = (ppu->bgShiftPtrLow & bit_mask) ? 1 : 0;
                    uint8_t pixel_hi = (ppu->bgShiftPtrHigh & bit_mask) ? 1 : 0;
                    bg_pixel = (pixel_hi << 1) | pixel_lo;
                    
                    // Extract 2-bit palette value
                    uint8_t pal_lo = (ppu->bgShiftAttrLow & bit_mask) ? 1 : 0;
                    uint8_t pal_hi = (ppu->bgShiftAttrHigh & bit_mask) ? 1 : 0;
                    bg_palette = (pal_hi << 1) | pal_lo;
                }
            }

            uint8_t sprite_pixel = 0;
            uint8_t sprite_palette = 0;
            bool sprite_has_priority = false;
            bool is_sprite_0 = false;

            if(ppu->mask & MSK_SPR)
            {
                if(ppu->mask & MSK_SPR_8 || pixel >= 9)
                {
                    uint8_t sprite_height = (ppu->ctrl & CTRL_SPR_SIZE) ? 16 : 8;
                    
                    for(int i = 0; i < ppu->secondaryAddr >> 2; i++)
                    {
                        OAM* sprite = &ppu->secondaryOam[i];
                        
                        if (sprite->y == ppu->oam[0].y && 
                            sprite->idx == ppu->oam[0].idx &&
                            sprite->attr == ppu->oam[0].attr &&
                            sprite->x == ppu->oam[0].x) 
                        {
                            is_sprite_0 = true;
                        } 
                        else 
                        {
                            is_sprite_0 = false;
                        }


                        int16_t x_offset = (pixel - 1) - sprite->x;                        
                        if(x_offset < 0 || x_offset > 7)
                            continue;
                        
                        uint8_t y_offset = scanline - sprite->y;
                        if(sprite->attr & OAM_FLIP_Y)
                            y_offset = (sprite_height - 1) - y_offset;
                        
                        if(sprite->attr & OAM_FLIP_X)
                            x_offset = 7 - x_offset;
                        
                        uint16_t pattern_addr;
                        
                        if(sprite_height == 16)
                        {
                            // 8x16 sprites
                            uint16_t bank = (sprite->idx & 0x01) ? 0x1000 : 0x0000;
                            uint8_t tile_num = sprite->idx & 0xFE;
                            if(y_offset >= 8)
                            {
                                tile_num++;
                                y_offset -= 8;
                            }
                            pattern_addr = bank | (tile_num << 4) | y_offset;
                        }
                        else
                        {
                            // 8x8 sprites
                            uint16_t bank = (ppu->ctrl & CTRL_SP_TABLE) ? 0x1000 : 0x0000;
                            pattern_addr = bank | (sprite->idx << 4) | y_offset;
                        }
                        
                        uint8_t pattern_lo = PPU_read(ppu, pattern_addr);
                        uint8_t pattern_hi = PPU_read(ppu, pattern_addr + 8);
                        
                        uint8_t bit = 7 - x_offset;
                        uint8_t pixel_lo = (pattern_lo >> bit) & 0x01;
                        uint8_t pixel_hi = (pattern_hi >> bit) & 0x01;
                        sprite_pixel = (pixel_hi << 1) | pixel_lo;
                        
                        if(sprite_pixel == 0)
                            continue;
                        
                        sprite_palette = (sprite->attr & 0x03) + 4;
                        sprite_has_priority = (sprite->attr & OAM_PRIORITY) == 0;
                        
                        break; 
                    }
                    
                    if(is_sprite_0 && sprite_pixel != 0 && bg_pixel != 0 && 
                       pixel < 255 && !(ppu->status & STS_0_HIT))
                    {
                        ppu->status |= STS_0_HIT;
                    }
                }
            }
            uint8_t final_pixel = 0;
            uint8_t final_palette = 0;
            
            // Determine final pixel based on priority
            if(bg_pixel == 0 && sprite_pixel == 0)
            {
                // Both transparent - backdrop color
                final_pixel = 0;
                final_palette = 0;
            }
            else if(bg_pixel == 0 && sprite_pixel > 0)
            {
                // Only sprite visible
                final_pixel = sprite_pixel;
                final_palette = sprite_palette;
            }
            else if(bg_pixel > 0 && sprite_pixel == 0)
            {
                // Only background visible
                final_pixel = bg_pixel;
                final_palette = bg_palette;
            }
            else
            {
                // Both visible - check priority
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
            
            // Write pixel to screen
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
                ppu->nmi = true;
            }
        }
    }
    else if(scanline == ppu->scanLinesPerFame-1)
    {
        if(pixel == 1)
        {
            ppu->status &= ~(STS_VBLANK | STS_0_HIT | STS_OVERFLOW);
            ppu->nmi = false;
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
                ppu->t |= (data >> 3) & 0x1F;
                ppu->x = data & 0x07;
                ppu->w = 1;
            }
            else
            {
                ppu->t &= ~(COARSE_Y | FINE_Y);
                ppu->t |= (data << 5) & COARSE_Y;
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