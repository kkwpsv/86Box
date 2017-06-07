/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		EGA renderers.
 *
 * Version:	@(#)vid_ega_render.c	1.0.1	2017/06/05
 *
 * Author:	Sarah Walker, <http://pcem-emulator.co.uk/>
 *		Miran Grca, <mgrca8@gmail.com>
 *		Copyright 2008-2017 Sarah Walker.
 *		Copyright 2016-2017 Miran Grca.
 */

#include <stdio.h>
#include "../ibm.h"
#include "../device.h"
#include "../mem.h"
#include "../rom.h"
#include "video.h"
#include "vid_ega.h"
#include "vid_ega_render.h"


int ega_display_line(ega_t *ega)
{
	int y_add = (enable_overscan) ? (overscan_y >> 1) : 0;
	unsigned int dl = ega->displine;
	if (ega->crtc[9] & 0x1f)
	{
		dl -= (ega->crtc[8] & 0x1f);
	}
	dl += y_add;
	dl &= 0x7ff;
	return dl;
}

void ega_render_blank(ega_t *ega)
{
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        int x, xx;

	for (x = 0; x < ega->hdisp; x++)
	{
		switch (ega->seqregs[1] & 9)
		{
			case 0:
				for (xx = 0; xx < 9; xx++)  ((uint32_t *)buffer32->line[dl])[(x * 9) + xx + 32 + x_add] = 0;
				break;
			case 1:
				for (xx = 0; xx < 8; xx++)  ((uint32_t *)buffer32->line[dl])[(x * 8) + xx + 32 + x_add] = 0;
				break;
			case 8:
				for (xx = 0; xx < 18; xx++) ((uint32_t *)buffer32->line[dl])[(x * 18) + xx + 32 + x_add] = 0;
				break;
			case 9:
				for (xx = 0; xx < 16; xx++) ((uint32_t *)buffer32->line[dl])[(x * 16) + xx + 32 + x_add] = 0;
				break;
		}
	}
}

void ega_render_text_standard(ega_t *ega, int drawcursor)
{     
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        uint8_t chr, dat, attr;
        uint32_t charaddr;
        int x, xx;
        uint32_t fg, bg;

	if (fullchange)
	{
		for (x = 0; x < ega->hdisp; x++)
		{
			drawcursor = ((ega->ma == ega->ca) && ega->con && ega->cursoron);
			chr  = ega->vram[(ega->ma << 1) & ega->vrammask];
			attr = ega->vram[((ega->ma << 1) + 1) & ega->vrammask];

			if (attr & 8) charaddr = ega->charsetb + (chr * 128);
			else          charaddr = ega->charseta + (chr * 128);

			if (drawcursor) 
			{ 
				bg = ega->pallook[ega->egapal[attr & 15]]; 
				fg = ega->pallook[ega->egapal[attr >> 4]]; 
			}
			else
			{
				fg = ega->pallook[ega->egapal[attr & 15]];
				bg = ega->pallook[ega->egapal[attr >> 4]];
				if (attr & 0x80 && ega->attrregs[0x10] & 8)
				{
					bg = ega->pallook[ega->egapal[(attr >> 4) & 7]];
					if (ega->blink & 16) 
						fg = bg;
				}
			}

			dat = ega->vram[charaddr + (ega->sc << 2)];
			if (ega->seqregs[1] & 8)
			{
				if (ega->seqregs[1] & 1) 
				{
					for (xx = 0; xx < 8; xx++) 
						((uint32_t *)buffer32->line[dl])[(((x << 4) + 32 + (xx << 1)) & 2047) + x_add] =
						((uint32_t *)buffer32->line[dl])[(((x << 4) + 33 + (xx << 1)) & 2047) + x_add] = (dat & (0x80 >> xx)) ? fg : bg; 
				}
				else
				{
					for (xx = 0; xx < 8; xx++) 
						((uint32_t *)buffer32->line[dl])[(((x * 18) + 32 + (xx << 1)) & 2047) + x_add] = 
						((uint32_t *)buffer32->line[dl])[(((x * 18) + 33 + (xx << 1)) & 2047) + x_add] = (dat & (0x80 >> xx)) ? fg : bg;
					if ((chr & ~0x1f) != 0xc0 || !(ega->attrregs[0x10] & 4)) 
						((uint32_t *)buffer32->line[dl])[(((x * 18) + 32 + 16) & 2047) + x_add] = 
						((uint32_t *)buffer32->line[dl])[(((x * 18) + 32 + 17) & 2047) + x_add] = bg;
					else
						((uint32_t *)buffer32->line[dl])[(((x * 18) + 32 + 16) & 2047) + x_add] = 
						((uint32_t *)buffer32->line[dl])[(((x * 18) + 32 + 17) & 2047) + x_add] = (dat & 1) ? fg : bg;
				}
			}
			else
			{
				if (ega->seqregs[1] & 1) 
				{ 
					for (xx = 0; xx < 8; xx++) 
						((uint32_t *)buffer32->line[dl])[(((x << 3) + 32 + xx) & 2047) + x_add] = (dat & (0x80 >> xx)) ? fg : bg; 
				}
				else
				{
					for (xx = 0; xx < 8; xx++) 
						((uint32_t *)buffer32->line[dl])[(((x * 9) + 32 + xx) & 2047) + x_add] = (dat & (0x80 >> xx)) ? fg : bg;
					if ((chr & ~0x1f) != 0xc0 || !(ega->attrregs[0x10] & 4))
						((uint32_t *)buffer32->line[dl])[(((x * 9) + 32 + 8) & 2047) + x_add] = bg;
					else
						((uint32_t *)buffer32->line[dl])[(((x * 9) + 32 + 8) & 2047) + x_add] = (dat & 1) ? fg : bg;
				}
			}
			ega->ma += 4; 
			ega->ma &= ega->vrammask;
		}
	}
}

static __inline int is_kanji1(uint8_t chr)
{
	return (chr >= 0x81 && chr <= 0x9f) || (chr >= 0xe0 && chr <= 0xfc);
}

static __inline int is_kanji2(uint8_t chr)
{
	return (chr >= 0x40 && chr <= 0x7e) || (chr >= 0x80 && chr <= 0xfc);
}

void ega_jega_render_blit_text(ega_t *ega, int x, int dl, int start, int width, uint16_t dat, int cw, uint32_t fg, uint32_t bg)
{
	int x_add = (enable_overscan) ? 8 : 0;

	int xx = 0;
	int xxx = 0;

	if (ega->seqregs[1] & 8)
	{
		for (xx = start; xx < (start + width); xx++) 
			for (xxx = 0; xxx < cw; xxx++)
				((uint32_t *)buffer32->line[dl])[(((x * width) + 32 + (xxx << 1) + ((xx << 1) * cw)) & 2047) + x_add] = 
				((uint32_t *)buffer32->line[dl])[(((x * width) + 33 + (xxx << 1) + ((xx << 1) * cw)) & 2047) + x_add] = (dat & (0x80 >> xx)) ? fg : bg;
	}
	else
	{
		for (xx = start; xx < (start + width); xx++) 
			((uint32_t *)buffer32->line[dl])[(((x * width) + 32 + xxx + (xx * cw)) & 2047) + x_add] = (dat & (0x80 >> xx)) ? fg : bg;
	}
}

void ega_render_text_jega(ega_t *ega, int drawcursor)
{     
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        uint8_t chr, attr;
	uint16_t dat, dat2;
        uint32_t charaddr;
        int x, xx;
        uint32_t fg, bg;

	/* Temporary for DBCS. */
	unsigned int chr_left;
	unsigned int bsattr;
	int chr_wide = 0;
	uint32_t bg_ex = 0;
	uint32_t fg_ex = 0;

	int blocks = ega->hdisp;
	int fline;

	unsigned int pad_y, exattr;

	if (fullchange)
	{
		for (x = 0; x < ega->hdisp; x++)
		{
			drawcursor = ((ega->ma == ega->ca) && ega->con && ega->cursoron);
			chr  = ega->vram[(ega->ma << 1) & ega->vrammask];
			attr = ega->vram[((ega->ma << 1) + 1) & ega->vrammask];

			if (chr_wide = 0)
			{
				if (ega->RMOD2 & 0x80)
				{
					fg_ex = ega->pallook[ega->egapal[attr & 15]];

					if (attr & 0x80 && ega->attrregs[0x10] & 8)
					{
						bg_ex = ega->pallook[ega->egapal[(attr >> 4) & 7]];
					}
					else
					{
						bg_ex = ega->pallook[ega->egapal[attr >> 4]];
					}
				}
				else
				{
					if (attr & 0x40)
					{
						/* Reversed in JEGA mode */
						bg_ex = ega->pallook[ega->egapal[attr & 15]];
						fg_ex = ega->pallook[0];
					}
					else
					{
						/* Reversed in JEGA mode */
						fg_ex = ega->pallook[ega->egapal[attr & 15]];
						bg_ex = ega->pallook[0];
					}
				}

				if (drawcursor) 
				{ 
					bg = fg_ex;
					fg = bg_ex;
				}
				else
				{
					fg = fg_ex;
					bg = bg_ex;
				}
	
				if (attr & 0x80 && ega->attrregs[0x10] & 8)
				{
					if (ega->blink & 16) 
						fg = bg;
				}

				/* Stay drawing if the char code is DBCS and not at last column. */
				if (is_kanji1(dat) && (blocks > 1))
				{
					/* Set the present char/attr code to the next loop. */
					chr_left = chr;
					chr_wide = 1;
				}
				else
				{
					/* The char code is ANK (8 dots width). */
					dat = jfont_sbcs_19[chr*19+(ega->sc)];	/* w8xh19 font */
					ega_jega_render_blit_text(ega, x, dl, 0, 8, dat, 1, fg, bg);
					if (bsattr & 0x20)
					{
						/* Vertical line. */
						dat = 0x18;
						ega_jega_render_blit_text(ega, x, fline, 0, 8, dat, 1, fg, bg);
					}
					if (ega->sc == 18 && bsattr & 0x10)
					{
						/* Underline. */
						dat = 0xff;
						ega_jega_render_blit_text(ega, x, fline, 0, 8, dat, 1, fg, bg);
					}
					chr_wide = 0;
					blocks--;
				}
			}
			else
			{
				/* The char code may be in DBCS. */
				pad_y = ega->RPSSC;
				exattr = 0;

				/* Note: The second column should be applied its basic attribute. */
				if (ega->RMOD2 & 0x40)
				{
					/* If JEGA Extended Attribute is enabled. */
					exattr = attr;
					if ((exattr & 0x30) == 0x30)  pad_y = ega->RPSSL;	/* Set top padding of lower 2x character. */
					else if (exattr & 0x30)  pad_y = ega->RPSSU;	/* Set top padding of upper 2x character. */
				}

				if (ega->sc >= pad_y && ega->sc < 16 + pad_y)
				{
					/* Check the char code is in Wide charset of Shift-JIS. */
					if (is_kanji2(chr))
					{
						fline = ega->sc - pad_y;
						chr_left <<= 8;
						/* Fix vertical position. */
						chr |= chr_left;
						/* Horizontal wide font (Extended Attribute). */
						if (exattr & 0x20)
						{
							if (exattr & 0x10)  fline = (fline >> 1) + 8;
							else  fline = fline >> 1;
						}
						/* Vertical wide font (Extended Attribute). */
						if (exattr & 0x40)
						{
							dat = jfont_dbcs_16[chr * 32 + fline * 2];
							if (!(exattr & 0x08))
								dat = jfont_dbcs_16[chr * 32 + fline * 2 + 1];
							/* Draw 8 dots. */
							ega_jega_render_blit_text(ega, x, dl, 0, 8, dat, 2, fg, bg);
						}
						else
						{
							/* Get the font pattern. */
							dat = jfont_dbcs_16[chr * 32 + fline * 2];
							dat <<= 8;
							dat |= jfont_dbcs_16[chr * 32 + fline * 2 + 1];
							/* Bold (Extended Attribute). */
							if (exattr &= 0x80)
							{
								dat2 = dat;
								dat2 >>= 1;
								dat |= dat2;
								/* Original JEGA colours the last row with the next column's attribute. */
							}
							/* Draw 16 dots */
							ega_jega_render_blit_text(ega, x, dl, 0, 16, dat, 1, fg, bg);
						}
					}
					else
					{
						/* Ignore wide char mode, put blank. */
						dat = 0;
						ega_jega_render_blit_text(ega, x, dl, 0, 16, dat, 1, fg, bg);
					}
				}
				else if (ega->sc == (17 + pad_y) && (bsattr & 0x10))
				{
					/* Underline. */
					dat = 0xffff;
					ega_jega_render_blit_text(ega, x, dl, 0, 16, dat, 1, fg, bg);
				}
				else
				{
					/* Draw blank */
					dat = 0;
					ega_jega_render_blit_text(ega, x, dl, 0, 16, dat, 1, fg, bg);
				}

				if (bsattr & 0x20)
				{
					/* Vertical line draw at last. */
					dat = 0x0180;
					ega_jega_render_blit_text(ega, x, dl, 0, 16, dat, 1, fg, bg);
				}

				chr_wide = 0;
				blocks -= 2;	/* Move by 2 columns. */
			}

			ega->ma += 4; 
			ega->ma &= ega->vrammask;
		}
	}
}

void ega_render_2bpp_lowres(ega_t *ega)
{
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        int x;
        int offset;
        uint8_t edat[4];

	offset = ((8 - ega->scrollcache) << 1) + 16;
	for (x = 0; x <= ega->hdisp; x++)
	{
		if (ega->sc & 1 && !(ega->crtc[0x17] & 1))
		{
			edat[0] = ega->vram[(ega->ma << 1) + 0x8000];
			edat[1] = ega->vram[(ega->ma << 1) + 0x8001];
		}
		else
		{
			edat[0] = ega->vram[(ega->ma << 1)];
			edat[1] = ega->vram[(ega->ma << 1) + 1];
		}
		ega->ma += 4; 
		ega->ma &= ega->vrammask;

		((uint32_t *)buffer32->line[dl])[(x << 4) + 14 + offset + x_add]=  ((uint32_t *)buffer32->line[dl])[(x << 4) + 15 + offset + x_add] = ega->pallook[ega->egapal[edat[1] & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 12 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) + 13 + offset + x_add] = ega->pallook[ega->egapal[(edat[1] >> 2) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 10 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) + 11 + offset + x_add] = ega->pallook[ega->egapal[(edat[1] >> 4) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +  8 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  9 + offset + x_add] = ega->pallook[ega->egapal[(edat[1] >> 6) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +  6 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  7 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 0) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +  4 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  5 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 2) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +  2 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  3 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 4) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +      offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  1 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 6) & 3]];
	}
}

void ega_render_2bpp_highres(ega_t *ega)
{
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        int x;
        int offset;
        uint8_t edat[4];

	offset = ((8 - ega->scrollcache) << 1) + 16;
	for (x = 0; x <= ega->hdisp; x++)
	{
		if (ega->sc & 1 && !(ega->crtc[0x17] & 1))
		{
			edat[0] = ega->vram[(ega->ma << 1) + 0x8000];
			edat[1] = ega->vram[(ega->ma << 1) + 0x8001];
		}
		else
		{
			edat[0] = ega->vram[(ega->ma << 1)];
			edat[1] = ega->vram[(ega->ma << 1) + 1];
		}
		ega->ma += 4; 
		ega->ma &= ega->vrammask;

		((uint32_t *)buffer32->line[dl])[(x << 4) + 7 + offset + x_add] = ega->pallook[ega->egapal[edat[1] & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 6 + offset + x_add] = ega->pallook[ega->egapal[(edat[1] >> 2) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 5 + offset + x_add] = ega->pallook[ega->egapal[(edat[1] >> 4) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 4 + offset + x_add] = ega->pallook[ega->egapal[(edat[1] >> 6) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 3 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 0) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 2 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 2) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 1 + offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 4) & 3]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +     offset + x_add] = ega->pallook[ega->egapal[(edat[0] >> 6) & 3]];
	}
}

void ega_render_4bpp_lowres(ega_t *ega)
{
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        uint8_t dat;
        int x;
        int offset;
        uint8_t edat[4];

	offset = ((8 - ega->scrollcache) << 1) + 16;
	for (x = 0; x <= ega->hdisp; x++)
	{
		if (ega->sc & 1 && !(ega->crtc[0x17] & 1))
		{
			edat[0] = ega->vram[ega->ma | 0x8000];
			edat[1] = ega->vram[ega->ma | 0x8001];
			edat[2] = ega->vram[ega->ma | 0x8002];
			edat[3] = ega->vram[ega->ma | 0x8003];
		}
		else
		{
			edat[0] = ega->vram[ega->ma];
			edat[1] = ega->vram[ega->ma | 0x1];
			edat[2] = ega->vram[ega->ma | 0x2];
			edat[3] = ega->vram[ega->ma | 0x3];
		}
		ega->ma += 4; 
		ega->ma &= ega->vrammask;
		dat = edatlookup[edat[0] & 3][edat[1] & 3] | (edatlookup[edat[2] & 3][edat[3] & 3] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 4) + 14 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) + 15 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 4) + 12 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) + 13 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
		dat = edatlookup[(edat[0] >> 2) & 3][(edat[1] >> 2) & 3] | (edatlookup[(edat[2] >> 2) & 3][(edat[3] >> 2) & 3] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 4) + 10 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) + 11 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +  8 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  9 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
		dat = edatlookup[(edat[0] >> 4) & 3][(edat[1] >> 4) & 3] | (edatlookup[(edat[2] >> 4) & 3][(edat[3] >> 4) & 3] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 4) +  6 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  7 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +  4 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  5 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
		dat = edatlookup[edat[0] >> 6][edat[1] >> 6] | (edatlookup[edat[2] >> 6][edat[3] >> 6] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 4) +  2 + offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  3 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 4) +      offset + x_add] = ((uint32_t *)buffer32->line[dl])[(x << 4) +  1 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
	}
}

void ega_render_4bpp_highres(ega_t *ega)
{
	int x_add = (enable_overscan) ? 8 : 0;
	int dl = ega_display_line(ega);
        uint8_t dat;
        int x;
        int offset;
        uint8_t edat[4];

	offset = (8 - ega->scrollcache) + 24;
	for (x = 0; x <= ega->hdisp; x++)
	{
		if (ega->sc & 1 && !(ega->crtc[0x17] & 1))
		{
			edat[0] = ega->vram[ega->ma | 0x8000];
			edat[1] = ega->vram[ega->ma | 0x8001];
			edat[2] = ega->vram[ega->ma | 0x8002];
			edat[3] = ega->vram[ega->ma | 0x8003];
		}
		else
		{
			edat[0] = ega->vram[ega->ma];
			edat[1] = ega->vram[ega->ma | 0x1];
			edat[2] = ega->vram[ega->ma | 0x2];
			edat[3] = ega->vram[ega->ma | 0x3];
		}
		ega->ma += 4; 
		ega->ma &= ega->vrammask;

		dat = edatlookup[edat[0] & 3][edat[1] & 3] | (edatlookup[edat[2] & 3][edat[3] & 3] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 3) + 7 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 3) + 6 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
		dat = edatlookup[(edat[0] >> 2) & 3][(edat[1] >> 2) & 3] | (edatlookup[(edat[2] >> 2) & 3][(edat[3] >> 2) & 3] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 3) + 5 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 3) + 4 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
		dat = edatlookup[(edat[0] >> 4) & 3][(edat[1] >> 4) & 3] | (edatlookup[(edat[2] >> 4) & 3][(edat[3] >> 4) & 3] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 3) + 3 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 3) + 2 + offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
		dat = edatlookup[edat[0] >> 6][edat[1] >> 6] | (edatlookup[edat[2] >> 6][edat[3] >> 6] << 2);
		((uint32_t *)buffer32->line[dl])[(x << 3) + 1 + offset + x_add] = ega->pallook[ega->egapal[(dat & 0xf) & ega->attrregs[0x12]]];
		((uint32_t *)buffer32->line[dl])[(x << 3) +     offset + x_add] = ega->pallook[ega->egapal[(dat >> 4)  & ega->attrregs[0x12]]];
	}
}
