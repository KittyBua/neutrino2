/*
 * $Id: widget_helpers.cpp 27.02.2019 mohousch Exp $
 *
 * (C) 2003 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <global.h>
#include <neutrino2.h>

#include <driver/color.h>
#include <system/settings.h>
#include <system/debug.h>

#include <gui/widget/widget_helpers.h>
#include <gui/widget/textbox.h>
#include <gui/widget/window.h>
#include <gui/widget/framebox.h>

#include <video_cs.h>


extern cVideo * videoDecoder;

// CComponent
CComponent::CComponent()
{
	//
	cCBox.iX = 0;
	cCBox.iY = 0;
	cCBox.iWidth = 0;
	cCBox.iHeight = 0;
	
	rePaint = false; 
	halign = CC_ALIGN_LEFT;
	
	cc_type = -1;
}

// CCIcon
CCIcon::CCIcon(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCIcon::CCIcon: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	iconName = ""; 
	iWidth = 0; 
	iHeight = 0; 
	
	cc_type = CC_ICON;
}

//
void CCIcon::setIcon(const char* const icon)
{
	iconName = icon? icon : ""; 
	
	if (!iconName.empty()) frameBuffer->getIconSize(iconName.c_str(), &iWidth, &iHeight);
}

//
void CCIcon::paint()
{
	dprintf(DEBUG_INFO, "CCIcon::paint\n");
	
	if (!iconName.empty()) frameBuffer->paintIcon(iconName.c_str(), cCBox.iX + (cCBox.iWidth - iWidth)/2, cCBox.iY + (cCBox.iHeight - iHeight)/2);
};

// CCImage
CCImage::CCImage(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCImage::CCImage: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	 
	imageName = ""; 
	iWidth = 0; 
	iHeight = 0; 
	iNbp = 0; 
	scale = false;
	color = COL_MENUCONTENT_PLUS_0;
	paintframe = false; 
	
	cc_type = CC_IMAGE;
}

//
void CCImage::setImage(const char* const image)
{
	imageName = image? image : "";
	 
	if (!imageName.empty()) frameBuffer->getSize(imageName, &iWidth, &iHeight, &iNbp);
}

//
void CCImage::paint()
{
	dprintf(DEBUG_INFO, "CCImage::paint\n");
	
	if (iWidth > cCBox.iWidth) iWidth = cCBox.iWidth;
	if (iHeight > cCBox.iHeight) iHeight = cCBox.iHeight;
			
	int startPosX = cCBox.iX + (cCBox.iWidth - iWidth)/2;
			
	if (scale)
	{
		// bg
		if (paintframe) frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, color);
				
		// image
		if (!imageName.empty()) frameBuffer->displayImage(imageName.c_str(), cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);
	}
	else
	{
		// bg
		if (paintframe) frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, color);
				
		// image
		if (!imageName.empty()) frameBuffer->displayImage(imageName.c_str(), startPosX, cCBox.iY + (cCBox.iHeight - iHeight)/2, iWidth, iHeight);
	}
}

// progressbar
CProgressBar::CProgressBar(int x, int y, int w, int h, int r, int g, int b, bool inv)
{
	dprintf(DEBUG_INFO, "CProgressBar::CProgressBar: x:%d y;%d dx:%d dy:%d\n", x, y, w, h);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = w;
	cCBox.iHeight = h;
	inverse = inv;
	
	div = (double) cCBox.iWidth / (double) 100;
	
	red = (double) r * (double) div ;
	green = (double) g * (double) div;
	yellow = (double) g * (double) div;
	
	while ((red + yellow + green) < cCBox.iWidth) 
	{
		if (green)
			green++;
		if (yellow && ((red + yellow + green) < cCBox.iWidth))
			yellow++;
		if (red && ((red + yellow + green) < cCBox.iWidth))
			red++;
	}
	
	rgb = 0xFF0000;
	
	percent = 255;
	
	cc_type = CC_PROGRESSBAR;
}

CProgressBar::CProgressBar(const CBox* position, int r, int g, int b, bool inv)
{
	dprintf(DEBUG_INFO, "CProgressBar::CProgressBar: x:%d y:%d dx:%d dy:%d\n", position->iX, position->iY, position->iWidth, position->iHeight);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox = *position;
	inverse = inv;
	
	div = (double) cCBox.iWidth / (double) 100;
	
	red = (double) r * (double) div ;
	green = (double) g * (double) div;
	yellow = (double) g * (double) div;
	
	while ((red + yellow + green) < cCBox.iWidth) 
	{
		if (green)
			green++;
		if (yellow && ((red + yellow + green) < cCBox.iWidth))
			yellow++;
		if (red && ((red + yellow + green) < cCBox.iWidth))
			red++;
	}
	
	rgb = 0xFF0000;
	
	percent = 255;
	
	cc_type = CC_PROGRESSBAR;
}

void CProgressBar::paint(unsigned char pcr, bool paintBG)
{
	dprintf(DEBUG_DEBUG, "CProgressBar::paint: pcr:%d\n", pcr);
	
	int i = 0;
	int b = 0;
	int siglen = 0;
	
	// body
	if (paintBG)
		frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, COL_MENUCONTENT_PLUS_2, NO_RADIUS, CORNER_ALL, g_settings.progressbar_color? g_settings.progressbar_gradient : NOGRADIENT);	//fill passive
	
	if (pcr != percent) 
	{
		if(percent == 255) 
			percent = 0;

		siglen = (double) pcr * (double) div;
		int step = 0;
		uint32_t diff = 0;

		if(g_settings.progressbar_color)
		{
			//red
			for (i = 0; (i < red) && (i < siglen); i++) 
			{
				diff = i * 255 / red;

				if(inverse) 
					rgb = 0x00FF00 + (diff << 16); // adding red
				else
					rgb = 0xFF0000 + (diff << 8); // adding green
				
				frameBuffer->paintBoxRel(cCBox.iX + i, cCBox.iY, 1, cCBox.iHeight, make16color(rgb), NO_RADIUS, CORNER_ALL, g_settings.progressbar_gradient);
			}
			
			//yellow
			step = yellow - red - 1;
			if (step < 1)
				step = 1;
		
			for (; (i < yellow) && (i < siglen); i++) 
			{
				diff = b++ * 255 / step / 2;

				if(inverse) 
					rgb = 0xFFFF00 - (diff << 8); // removing green
				else
					rgb = 0xFFFF00 - (diff << 16); // removing red
	
				frameBuffer->paintBoxRel(cCBox.iX + i, cCBox.iY, 1, cCBox.iHeight, make16color(rgb), NO_RADIUS, CORNER_ALL, g_settings.progressbar_gradient);
			}

			//green
			int off = diff;
			b = 0;
			step = green - yellow - 1;
			if (step < 1)
				step = 1;
			for (; (i < green) && (i < siglen); i++) 
			{
				diff = b++ * 255 / step / 2 + off;

				if(inverse) 
					rgb = 0xFFFF00 - (diff << 8); // removing green
				else
					rgb = 0xFFFF00 - (diff << 16); // removing red
				
				frameBuffer->paintBoxRel(cCBox.iX + i, cCBox.iY, 1, cCBox.iHeight, make16color(rgb), NO_RADIUS, CORNER_ALL, g_settings.progressbar_gradient);
			}
		}
		else
		{
			frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, siglen, cCBox.iHeight, COL_MENUCONTENT_PLUS_6, NO_RADIUS, CORNER_ALL, g_settings.progressbar_gradient);
		}
		
		percent = pcr;
	}
}

void CProgressBar::reset()
{
  	percent = 255;
}

// CCButtons
CCButtons::CCButtons(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCButtons::CCButtons: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy; 
	
	buttons.clear(); 
	count = 0;
	head = false;
	mode = BUTTON_BUTTON;
	
	cc_type = CC_BUTTON;
}

void CCButtons::setButtons(const struct button_label *button_label, const int button_count, bool _head)
{
	if (button_count)
	{
		for (unsigned int i = 0; i < (unsigned int)button_count; i++)
		{
			buttons.push_back(button_label[i]);
		}
	}

	count = buttons.size();
	head = _head;
}

void CCButtons::paint()
{
	dprintf(DEBUG_INFO, "CCButtons::CCButtons:paint:\n");

	count = buttons.size();
	
	dprintf(DEBUG_INFO, "CCButtons::CCButtons:paint: count: %d\n", count);
	
	//
	int buttonWidth = 0;
	int maxButtonTextWidth = buttonWidth;
	int iw[count];
	int ih[count];
	int f_w[count];
	
	if (head)
	{
		int startx = cCBox.iX + cCBox.iWidth - BORDER_LEFT;
		
		if(count)
		{
			// get max width
			for (unsigned int i = 0; i < count; i++)
			{
				if (!buttons[i].localename.empty())
				{
					f_w[i] = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(_(buttons[i].localename.c_str()));
					if (i > 0)
						maxButtonTextWidth = std::max(f_w[i], f_w[i - 1]) + 10;
					else
						maxButtonTextWidth = f_w[i] + 10;
				}
			}
			
			for (unsigned int i = 0; i < count; i++)
			{
				if(!buttons[i].button.empty())
				{
					if (mode == BUTTON_BUTTON)
					{
						frameBuffer->getIconSize(buttons[i].button.c_str(), &iw[i], &ih[i]);
						
						// scale icon
						if(ih[i] >= cCBox.iHeight)
						{
							ih[i] = cCBox.iHeight - 2;
						}
					
						startx -= (iw[i] + ICON_TO_ICON_OFFSET);

						frameBuffer->paintIcon(buttons[i].button, startx, cCBox.iY + (cCBox.iHeight - ih[i])/2, 0, true, iw[i], ih[i]);
					}
					else if (mode == BUTTON_FRAME_BORDER)
					{
						startx -= (maxButtonTextWidth + 10 + ICON_TO_ICON_OFFSET);
							
						//
						CFrame frame;
						
						frame.setPosition(startx, cCBox.iY - 1, maxButtonTextWidth + 10, cCBox.iHeight -2);
						frame.enableBorder();
						frame.paintMainFrame(true);
						//frame.setColor(buttons[i].color);
						frame.setCaptionFont(SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL);
						frame.setTitle(_(buttons[i].localename.c_str()));
						frame.setHAlign(CC_ALIGN_CENTER);
						frame.paint();
					}
					else if (mode == BUTTON_FRAME_COLORED)
					{
						startx -= (maxButtonTextWidth + 10 + ICON_TO_ICON_OFFSET);
							
						//
						CFrame frame;
						
						frame.setPosition(startx, cCBox.iY - 1, maxButtonTextWidth + 10, cCBox.iHeight -2);
						frame.enableBorder();
						frame.paintMainFrame(true);
						frame.setColor(buttons[i].color);
						frame.setCaptionFont(SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL);
						frame.setTitle(_(buttons[i].localename.c_str()));
						frame.setHAlign(CC_ALIGN_CENTER);
						frame.paint();
					}
				}
			}
		}
	}
	else
	{
		if(count)
		{
			buttonWidth = (cCBox.iWidth - BORDER_LEFT - BORDER_RIGHT)/count;
			
			// get max width
			for (unsigned int i = 0; i < count; i++)
			{
				if (!buttons[i].localename.empty())
				{
					f_w[i] = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getRenderWidth(_(buttons[i].localename.c_str()));
					
					if (i > 0)
						maxButtonTextWidth = std::max(f_w[i], f_w[i - 1]) + 10;
					else
						maxButtonTextWidth = f_w[i] + 10;
				}
			}
			
			if (maxButtonTextWidth > buttonWidth)
				maxButtonTextWidth = buttonWidth;
		
			for (unsigned int i = 0; i < count; i++)
			{
				if (!buttons[i].button.empty())
				{
					if (mode == BUTTON_BUTTON)
					{
						iw[i] = 0;
						ih[i] = 0;

						CFrameBuffer::getInstance()->getIconSize(buttons[i].button.c_str(), &iw[i], &ih[i]);
						
						// scale icon
						if(ih[i] >= cCBox.iHeight)
						{
							ih[i] = cCBox.iHeight - 2;
						}
						
						int f_h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
				
						CFrameBuffer::getInstance()->paintIcon(buttons[i].button, cCBox.iX + BORDER_LEFT + i*buttonWidth, cCBox.iY + (cCBox.iHeight - ih[i])/2, 0, true, iw[i], ih[i]);

						// FIXME: i18n
						g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(cCBox.iX + BORDER_LEFT + iw[i] + ICON_OFFSET + i*buttonWidth, cCBox.iY + f_h + (cCBox.iHeight - f_h)/2, buttonWidth - iw[i] - ICON_OFFSET, _(buttons[i].localename.c_str()), COL_MENUFOOT, 0, true); // UTF-8
					}
					else if (mode == BUTTON_FRAME_BORDER)
					{
						//
						CFrame frame;
						
						frame.setPosition(cCBox.iX + BORDER_LEFT + i*buttonWidth, cCBox.iY - 1, maxButtonTextWidth + 10, cCBox.iHeight - 2);
						frame.enableBorder();
						frame.paintMainFrame(true);
						//frame.setColor(buttons[i].color);
						frame.setCaptionFont(SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL);
						frame.setTitle(_(buttons[i].localename.c_str()));
						frame.setHAlign(CC_ALIGN_CENTER);
						frame.paint();
					}
					else if (mode == BUTTON_FRAME_COLORED)
					{
						//
						CFrame frame;
						frame.setPosition(cCBox.iX + BORDER_LEFT + i*buttonWidth, cCBox.iY - 1, maxButtonTextWidth + 10, cCBox.iHeight - 2);
						frame.enableBorder();
						frame.paintMainFrame(true);
						frame.setColor(buttons[i].color);
						frame.setCaptionFont(SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL);
						frame.setTitle(_(buttons[i].localename.c_str()));
						frame.setHAlign(CC_ALIGN_CENTER);
						frame.paint();
					}
				}
			}
		}
	}
}

// scrollBar
void CScrollBar::paint(const int x, const int y, const int dy, const int NrOfPages, const int CurrentPage)
{
	// scrollBar
	CBox cFrameScrollBar;
	CWindow cScrollBarWindow;

	cFrameScrollBar.iX = x;
	cFrameScrollBar.iY = y;
	cFrameScrollBar.iWidth = SCROLLBAR_WIDTH;
	cFrameScrollBar.iHeight = dy;


	cScrollBarWindow.setPosition(&cFrameScrollBar);
	cScrollBarWindow.setColor(COL_MENUCONTENT_PLUS_1);
	cScrollBarWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cScrollBarWindow.paint();
		
	// scrollBar slider
	CBox cFrameSlider;
	CWindow cSliderWindow;	

	cFrameSlider.iX = cFrameScrollBar.iX + 2;
	cFrameSlider.iY = cFrameScrollBar.iY + CurrentPage*(cFrameScrollBar.iHeight/NrOfPages);
	cFrameSlider.iWidth = cFrameScrollBar.iWidth - 4;
	cFrameSlider.iHeight = cFrameScrollBar.iHeight/NrOfPages;

	cSliderWindow.setPosition(&cFrameSlider);
	cSliderWindow.setColor(COL_MENUCONTENT_PLUS_3);
	cSliderWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cSliderWindow.paint();
}

void CScrollBar::paint(CBox* position, const int NrOfPages, const int CurrentPage)
{
	// scrollBar
	CBox cFrameScrollBar;
	CWindow cScrollBarWindow;

	cFrameScrollBar = *position;

	cScrollBarWindow.setPosition(&cFrameScrollBar);
	cScrollBarWindow.setColor(COL_MENUCONTENT_PLUS_1);
	cScrollBarWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cScrollBarWindow.paint();
		
	// scrollBar slider
	CBox cFrameSlider;
	CWindow cSliderWindow;	

	cFrameSlider.iX = cFrameScrollBar.iX + 2;
	cFrameSlider.iY = cFrameScrollBar.iY + CurrentPage*(cFrameScrollBar.iHeight/NrOfPages);
	cFrameSlider.iWidth = cFrameScrollBar.iWidth - 4;
	cFrameSlider.iHeight = cFrameScrollBar.iHeight/NrOfPages;

	cSliderWindow.setPosition(&cFrameSlider);
	cSliderWindow.setColor(COL_MENUCONTENT_PLUS_3);
	cSliderWindow.setCorner(NO_RADIUS, CORNER_ALL);
	cSliderWindow.paint();
}

// detailsLine
CItems2DetailsLine::CItems2DetailsLine()
{
	frameBuffer = CFrameBuffer::getInstance(); 
	
	mode = DL_INFO; 
	info1 = "";
	option_info1 = "";
	info2 = "";
	option_info2 = "";
	hint = "";
	icon = "";
	
	// hintitem / hinticon
	tFont = SNeutrinoSettings::FONT_TYPE_EPG_INFO2;
	borderMode = BORDER_NO;
	savescreen = false;
	paintframe = true;
	color = COL_MENUCONTENT_PLUS_0;
	scale = false;
	
	cc_type = CC_DETAILSLINE;
}

CItems2DetailsLine::~CItems2DetailsLine()
{
	info1.clear();
	option_info1.clear();
	info2.clear();
	option_info2.clear();
	hint.clear();
	icon.clear();
}

void CItems2DetailsLine::paint(int x, int y, int width, int height, int info_height, int iheight, int iy)
{
	dprintf(DEBUG_INFO, "CItems2DetailsLine::paint: x:%d y:%d width:%d height:%d\n", x, y, width, height);
	
	//
	int ypos2 = y + height;

	// border / frame
	if ( (mode == DL_INFO) || (mode == DL_HINT) )
	{
		// border
		if (g_settings.Hint_border) frameBuffer->paintBoxRel(x, ypos2, width, info_height, COL_MENUCONTENT_PLUS_6, g_settings.Hint_radius, g_settings.Hint_corner);
		
		// infoBox
		frameBuffer->paintBoxRel(g_settings.Hint_border? x + 2 : x, g_settings.Hint_border? ypos2 + 2 : ypos2, g_settings.Hint_border? width - 4 : width, g_settings.Hint_border? info_height - 4 : info_height, COL_MENUHINT_PLUS_0, g_settings.Hint_radius, g_settings.Hint_corner, g_settings.Hint_gradient);
	}
	
	//
	int DLx = x + 2;
	int DLy = ypos2 + 2;
	int DLwidth = width - 4;
	int DLheight = info_height - 4;
	
	if (mode == DL_INFO)
	{
		// option_info1
		int l_ow1 = 0;
		if(!option_info1.empty())
		{
			l_ow1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(option_info1.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString(DLx + DLwidth - BORDER_RIGHT - l_ow1, DLy + (DLheight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), DLwidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, option_info1.c_str(), COL_MENUHINT, 0, true);
		}

		// info1
		int l_w1 = 0;
		if(!info1.empty())
		{
			l_w1 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getRenderWidth(info1.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->RenderString(DLx + BORDER_LEFT, DLy + (DLheight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST]->getHeight(), DLwidth - BORDER_LEFT - BORDER_RIGHT - l_ow1, info1.c_str(), COL_MENUHINT, 0, true);
		}

		// option_info2
		int l_ow2 = 0;
		if(!option_info2.empty())
		{
			l_ow2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getRenderWidth(option_info2.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->RenderString(DLx + DLwidth - BORDER_RIGHT - l_ow2, DLy + DLheight/2 + (DLheight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER]->getHeight(), DLwidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, option_info2.c_str(), COL_MENUHINT, 0, true);
		}

		// info2
		int l_w2 = 0;
		if(!info2.empty())
		{
			l_w2 = g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getRenderWidth(info2.c_str());

			g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->RenderString (DLx + BORDER_LEFT, DLy + DLheight/2 + (DLheight/2 - g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR]->getHeight(), DLwidth - BORDER_LEFT - BORDER_RIGHT - l_ow2, info2.c_str(), COL_MENUHINT, 0, true); // UTF-8
		}
	}
	else if (mode == DL_HINT)
	{	
		CTextBox Dline(DLx, DLy, DLwidth, DLheight);
		Dline.paintMainFrame(paintframe);
		//Dline.setBorderMode(borderMode);
		Dline.setCorner(g_settings.Hint_radius, g_settings.Hint_corner);
		//Dline.setRadius(g_settings.Hint_radius);
		Dline.setMode(AUTO_WIDTH);
		
		int iw = 100;
		int ih = DLheight - 4;
		int bpp = 0;
		
		frameBuffer->getSize(icon.c_str(), &iw, &iw, &bpp);
		
		if (iw > 100)
			iw = 100;
			
		if (ih > (DLheight - 4))
			ih = DLheight - 4;

		// Hint
		if(!hint.empty())
		{
			Dline.setText(hint.c_str(), !icon.empty()? icon.c_str() : NEUTRINO_ICON_MENUITEM_NOPREVIEW, iw, ih, PIC_LEFT);
		}
					
		Dline.paint();
	}
	else if (mode == DL_HINTITEM)
	{
		//
		CTextBox Dline(x, y, width, height);
		Dline.paintMainFrame(paintframe);
		Dline.setMode(AUTO_WIDTH);
		Dline.setFont(tFont);
		Dline.setBorderMode(borderMode);
		if (savescreen) Dline.enableSaveScreen();
		Dline.setBackgroundColor(color);
		//Dline.setCorner(g_settings.Hint_corner);
		//Dline.setRadius(g_settings.Hint_radius);
		
		// scale icon
		int pw = 0;
		int ph = 0;

		::scaleImage(icon, &pw, &ph);

		// Hint
		Dline.setText(hint.c_str(), icon.c_str(), pw, ph, PIC_CENTER);
					
		Dline.paint();
	}
	else if (mode == DL_HINTICON)	
	{
		CCImage DImage(x, y, width, height);
		DImage.setImage(icon.c_str());
		DImage.setScaling(scale);
		DImage.paintMainFrame(true);
		DImage.setColor(color);
		DImage.paint();
	}
	else if (mode == DL_HINTHINT)
	{
		CTextBox Dline(x, y, width, height);
		Dline.paintMainFrame(paintframe);
		Dline.setMode(AUTO_WIDTH);
		Dline.setFont(tFont);
		Dline.setBorderMode(borderMode);
		if (savescreen) Dline.enableSaveScreen();
		Dline.setBackgroundColor(color);
		//Dline.setCorner(g_settings.Hint_corner);
		//Dline.setRadius(g_settings.Hint_radius);

		// Hint
		Dline.setText(hint.c_str());
					
		Dline.paint();
	}
}

CCSlider::CCSlider(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_NORMAL, "CCSlider::CCSlider\n");
	
	//
	frameBuffer = CFrameBuffer::getInstance();
	
	//
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy; 
	
	//
	cc_type = CC_SLIDER;
}

void CCSlider::paint(const int spos, const char * const iconname, const bool selected)
{
	dprintf(DEBUG_NORMAL, "CCSlider::paint:\n");
	
	// volumebox box
	int icon_w = 120;
	int icon_h = 11;
	
	// volumebody
	frameBuffer->getIconSize(NEUTRINO_ICON_VOLUMEBODY, &icon_w, &icon_h);
	frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, COL_MENUCONTENT_PLUS_0);
	frameBuffer->paintIcon(NEUTRINO_ICON_VOLUMEBODY, cCBox.iX, cCBox.iY);

	// slider icon
	frameBuffer->paintIcon(selected ? iconname : NEUTRINO_ICON_VOLUMESLIDER2, cCBox.iX + spos, cCBox.iY);
}

//
void CItems2DetailsLine::clear(int x, int y, int width, int height, int info_height)
{
	if ( (mode == DL_INFO) ||(mode == DL_HINT) )
	{ 
		// info box
		frameBuffer->paintBackgroundBoxRel(x, y + height, width, info_height);
	}
	else if ( (mode == DL_HINTITEM) || (mode == DL_HINTICON) || (mode == DL_HINTHINT))
	{
		frameBuffer->paintBackgroundBoxRel(x, y, width, height);
	}
}

// Hline
CCHline::CCHline(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCHline::CCHline: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	if (cCBox.iHeight > 2)
		cCBox.iHeight = 2;
	
	color = COL_MENUCONTENT_PLUS_5;
	gradient = NOGRADIENT;
	 
	cc_type = CC_HLINE;
}

void CCHline::paint()
{
	dprintf(DEBUG_INFO, "CCHline::paint\n");
	
	if (cCBox.iHeight > 2)
		cCBox.iHeight = 2;
	
	//
	frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, color, 0, CORNER_NONE, gradient, GRADIENT_HORIZONTAL);
}

// Vline
CCVline::CCVline(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCVline::CCVline: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	if (cCBox.iWidth > 2)
		cCBox.iWidth = 2;
	
	color = COL_MENUCONTENT_PLUS_5;
	gradient = NOGRADIENT;
	
	cc_type = CC_VLINE;
}

void CCVline::paint()
{
	dprintf(DEBUG_INFO, "CCVline::paint\n");
	
	if (cCBox.iWidth > 2)
		cCBox.iWidth = 2;
	
	//
	frameBuffer->paintBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, color, 0, CORNER_NONE, gradient, GRADIENT_VERTICAL);
}

// CFrameLine
CCFrameLine::CCFrameLine(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCFrameLine::CCFrameLine: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	color = COL_WHITE_PLUS_0; 
	cc_type = CC_FRAMELINE;
}

//
void CCFrameLine::paint()
{
	dprintf(DEBUG_INFO, "CCFrameLine::paint\n");
	
	frameBuffer->paintFrameBox(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, color);
}

// CLabel
CCLabel::CCLabel(const int x, const int y, const int dx, const int dy, bool save)
{
	dprintf(DEBUG_INFO, "CCLabel::CCLabel: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance();
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	background = NULL;
	
	//
	savescreen = save;
	
	if (savescreen)
	{
		background = new fb_pixel_t[dx*dy];
		
		if (background)
		{
			frameBuffer->saveScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
		}
	}
	
	
	//
	color = COL_MENUCONTENT;
	paintBG = false; 
	font = SNeutrinoSettings::FONT_TYPE_MENU_TITLE;
	
	halign = CC_ALIGN_LEFT;
	
	label = "";
	
	cc_type = CC_LABEL;
}

CCLabel::~CCLabel()
{
	if (savescreen)
	{
		if (background)
		{
			delete [] background;
			background = NULL;
		}
	}
}

void CCLabel::paint()
{
	dprintf(DEBUG_INFO, "CCLabel::paint\n");
	
	//
	if (savescreen)
	{
		if (background)
		{
			frameBuffer->restoreScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
		}
	}
	
	//
	int stringWidth = 0;
	int height = 0;
	
	height = g_Font[font]->getHeight();
	
	if (!label.empty()) stringWidth = g_Font[font]->getRenderWidth(label.c_str());
	
	if (stringWidth > cCBox.iWidth)
		stringWidth = cCBox.iWidth;
		
	int startPosX = cCBox.iX;
	
	if (halign == CC_ALIGN_CENTER)
		startPosX = cCBox.iX + (cCBox.iWidth - stringWidth)/2;
	else if (halign == CC_ALIGN_RIGHT)
		startPosX = cCBox.iX + cCBox.iWidth - stringWidth;
	
	g_Font[font]->RenderString(startPosX, cCBox.iY + height + (cCBox.iHeight - height)/2, cCBox.iWidth, label.c_str(), color, 0, true, paintBG);
}

//
CCText::CCText(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCText::CCText: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	font = SNeutrinoSettings::FONT_TYPE_EPG_INFO1;
	mode = AUTO_WIDTH;
	color = COL_MENUCONTENT;
	useBG = false;
	
	Text = " ";
	
	cc_type = CC_TEXT;
}

void CCText::paint()
{
	dprintf(DEBUG_INFO, "CCText::paint\n");
	
	CTextBox textBox(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);

	textBox.paintMainFrame(false);
	textBox.enableSaveScreen();
	textBox.setMode(mode);
	textBox.setFont(font);
	textBox.setTextColor(color);

	// caption
	if(!Text.empty())
	{
		textBox.setText(Text.c_str(), NULL, 0, 0, PIC_RIGHT, false, useBG);
	}
	
	textBox.paint();
}

// grid
CCGrid::CCGrid(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCGrid::CCGrid: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;

	init();
}

CCGrid::CCGrid(CBox* position)
{
	dprintf(DEBUG_INFO, "CCGrid::CCGrid:\n");
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox = *position;

	init();
}

void CCGrid::init()
{
	rgb = COL_NOBEL_PLUS_0;
	inter_frame = 15;
	
	//
	cc_type = CC_GRID;
}

void CCGrid::setPosition(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_DEBUG, "CGrid::%s\n", __FUNCTION__);
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
}

void CCGrid::setPosition(CBox* position)
{
	dprintf(DEBUG_DEBUG, "CGrid::%s\n", __FUNCTION__);
	
	cCBox = *position;
}

void CCGrid::paint()
{
	dprintf(DEBUG_INFO, "CCGrid::paint\n");
	
	// hlines grid
	for(int count = 0; count < cCBox.iHeight; count += inter_frame)
		frameBuffer->paintHLine(cCBox.iX, cCBox.iX + cCBox.iWidth, cCBox.iY + count, rgb );

	// vlines grid
	for(int count = 0; count < cCBox.iWidth; count += inter_frame)
		frameBuffer->paintVLine(cCBox.iX + count, cCBox.iY, cCBox.iY + cCBox.iHeight, rgb );
}

void CCGrid::hide()
{
	frameBuffer->paintBackgroundBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);
	
	CFrameBuffer::getInstance()->blit();
}

// pig
CCPig::CCPig(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCPig::CCPig: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	//
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;

	init();
}

CCPig::CCPig(CBox* position)
{
	dprintf(DEBUG_INFO, "CCPig::CCPig:\n");
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	//
	cCBox = *position;

	init();
}

void CCPig::init()
{
	//
	cc_type = CC_PIG;
}

void CCPig::setPosition(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_DEBUG, "CPig::%s\n", __FUNCTION__);
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
}

void CCPig::setPosition(CBox* position)
{
	dprintf(DEBUG_DEBUG, "CPig::%s\n", __FUNCTION__);
	
	cCBox = *position;
}

void CCPig::paint()
{
	dprintf(DEBUG_INFO, "CCPig::paint\n");
	
	frameBuffer->paintBackgroundBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);	
		

	if(videoDecoder)
		videoDecoder->Pig(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);	
}

void CCPig::hide()
{
	if(videoDecoder)  
		videoDecoder->Pig(-1, -1, -1, -1);

	frameBuffer->paintBackgroundBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);
	
	CFrameBuffer::getInstance()->blit();
}

// CCTime
CCTime::CCTime(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCTime::CCTime: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	font = SNeutrinoSettings::FONT_TYPE_MENU_TITLE;
	color = COL_MENUHEAD;
	
	background = NULL;
	
	format = "%d.%m.%Y %H:%M";
	
	enableRepaint();
	
	cc_type = CC_TIME;
}

void CCTime::setFormat(const char* const f)
{
	format = f? _(f) : "";
}

void CCTime::paint()
{
	dprintf(DEBUG_INFO, "CCTime::paint\n");
	
	//
	background = new fb_pixel_t[cCBox.iWidth*cCBox.iHeight];
	
	if (background)
	{
		frameBuffer->saveScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
	}
	
	//
	std::string timestr = getNowTimeStr(format.c_str());
		
	int timestr_len = g_Font[font]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
	if (timestr_len > cCBox.iWidth)
		timestr_len = cCBox.iWidth;
		
	int startPosX = cCBox.iX + (cCBox.iWidth - timestr_len)/2;
	
	g_Font[font]->RenderString(startPosX, cCBox.iY + (cCBox.iHeight - g_Font[font]->getHeight())/2 + g_Font[font]->getHeight(), timestr_len, timestr.c_str(), color, 0, true);
}

void CCTime::refresh()
{
	if (background)
	{
		frameBuffer->restoreScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
	}
	
	//
	std::string timestr = getNowTimeStr(format.c_str());
		
	int timestr_len = g_Font[font]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
	if (timestr_len > cCBox.iWidth)
		timestr_len = cCBox.iWidth;
		
	int startPosX = cCBox.iX + (cCBox.iWidth - timestr_len)/2;
	
	g_Font[font]->RenderString(startPosX, cCBox.iY + (cCBox.iHeight - g_Font[font]->getHeight())/2 + g_Font[font]->getHeight(), timestr_len, timestr.c_str(), color, 0, true);
}

// CCCounter
CCCounter::CCCounter(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CCCounter::CCCounter: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	font = SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO;
	color = COL_INFOBAR;
	
	background = NULL;
	
	total_time = 0;
	play_time = 0;
	
	//
	cCBox.iWidth = g_Font[font]->getRenderWidth("00:00:00 / 00:00:00");
	cCBox.iHeight = g_Font[font]->getHeight();
	
	//
	enableRepaint();
	
	cc_type = CC_COUNTER;
}

void CCCounter::paint()
{
	dprintf(DEBUG_INFO, "CCCounter::paint\n");
	
	//
	background = new fb_pixel_t[cCBox.iWidth*cCBox.iHeight];
	
	if (background)
	{
		frameBuffer->saveScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
	}
	
	// play_time
	char playTime[11];
	strftime(playTime, 11, "%T/", gmtime(&play_time));//FIXME
	
	g_Font[font]->RenderString(cCBox.iX, cCBox.iY + (cCBox.iHeight - g_Font[font]->getHeight())/2 + g_Font[font]->getHeight(), cCBox.iWidth/2, playTime, color, 0, true);
	
	// total_time
	char totalTime[10];
	strftime(totalTime, 10, "%T", gmtime(&total_time));//FIXME
	g_Font[font]->RenderString(cCBox.iX + cCBox.iWidth/2, cCBox.iY + (cCBox.iHeight - g_Font[font]->getHeight())/2 + g_Font[font]->getHeight(), cCBox.iWidth/2, totalTime, color, 0, true);
}

void CCCounter::refresh()
{
	if (background)
	{
		frameBuffer->restoreScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
	}
	
	// play_time
	char playTime[11];
	strftime(playTime, 11, "%T/", gmtime(&play_time));//FIXME
	g_Font[font]->RenderString(cCBox.iX, cCBox.iY + (cCBox.iHeight - g_Font[font]->getHeight())/2 + g_Font[font]->getHeight(), cCBox.iWidth/2, playTime, color, 0, true);
	
	// total_time
	char totalTime[10];
	strftime(totalTime, 10, "%T", gmtime(&total_time));//FIXME
	g_Font[font]->RenderString(cCBox.iX + cCBox.iWidth/2, cCBox.iY + (cCBox.iHeight - g_Font[font]->getHeight())/2 + g_Font[font]->getHeight(), cCBox.iWidth/2, totalTime, color, 0, true);
}

// CCSpinner
CCSpinner::CCSpinner(const int x, const int y, const int dx, const int dy)
{
	frameBuffer = CFrameBuffer::getInstance();
	
	//
	cCBox.iX = x;
	cCBox.iY = y;
	cCBox.iWidth = dx;
	cCBox.iHeight = dy;
	
	int iw, ih;
	frameBuffer->getIconSize("hourglass0", &iw, &ih);
	
	if (iw > dx)
		cCBox.iWidth = iw;
		
	if (ih > dy)
		cCBox.iHeight = ih;
	
	//
	filename = "hourglass";
	
	//
	count = 0;
	background = NULL;
	
	enableRepaint();
	
	//
	cc_type = CC_SPINNER;
}

void CCSpinner::paint()
{
	dprintf(DEBUG_DEBUG, "CCSpinner::paint\n");
	
	//
	if(background)
	{
		delete[] background;
		background = NULL;
	}

	background = new fb_pixel_t[cCBox.iWidth*cCBox.iHeight];
		
	if(background)
	{
		frameBuffer->saveScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
	}
	//
	
	filename += to_string(count);
		
	//count = (count + 1) % 9;
	
	frameBuffer->paintIcon(filename, cCBox.iX, cCBox.iY);
}

void CCSpinner::hide()
{
	dprintf(DEBUG_DEBUG, "CCSpinner::hide\n");
	
	if(background) 
	{
		frameBuffer->restoreScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
		
		delete[] background;
		background = NULL;
	}
	else //FIXME:
		frameBuffer->paintBackgroundBoxRel(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight);
}

void CCSpinner::refresh()
{
	dprintf(DEBUG_DEBUG, "CCSpinner::refresh\n");
	
	filename.clear();
	
	filename = "hourglass";
	
	if (background)
	{
		frameBuffer->restoreScreen(cCBox.iX, cCBox.iY, cCBox.iWidth, cCBox.iHeight, background);
	}
		
	count = (count + 1) % 9;
	
	filename += to_string(count);
	
	frameBuffer->paintIcon(filename, cCBox.iX, cCBox.iY);
}

// CWidgetItem
CWidgetItem::CWidgetItem()
{
	itemBox.iX = 0;
	itemBox.iY = 0;
	itemBox.iWidth = 0;
	itemBox.iHeight = 0; 
	
	inFocus = true; 
	rePaint = false; 
	
	painted = false;
	
	actionKey = ""; 
	parent = NULL; 
	
	sec_timer_id = 0;
	
	widgetItem_type = -1;
	widgetItem_name = "";
}

//
void CWidgetItem::addKey(neutrino_msg_t key, CMenuTarget *menue, const std::string & action)
{
	dprintf(DEBUG_DEBUG, "CWidgetItem::addKey: %s\n", action.c_str());
	
	keyActionMap[key].menue = menue;
	keyActionMap[key].action = action;
}

//
bool CWidgetItem::onButtonPress(neutrino_msg_t msg, neutrino_msg_data_t data)
{
	dprintf(DEBUG_DEBUG, "CWidgetItem::onButtonPress: (msg:%ld) (data:%ld)\n", msg, data);
	
	bool ret = true;
	bool handled = false;
	
	//
	if ( msg <= RC_MaxRC ) 
	{
		std::map<neutrino_msg_t, keyAction>::iterator it = keyActionMap.find(msg);
					
		if (it != keyActionMap.end()) 
		{
			actionKey = it->second.action;

			if (it->second.menue != NULL)
			{
				int rv = it->second.menue->exec(parent, it->second.action);

				//FIXME:review this
				switch ( rv ) 
				{
					case RETURN_EXIT_ALL:
						ret = false; //fall through
					case RETURN_EXIT:
						ret = false;
						break;
					case RETURN_REPAINT:
						ret = true;
						paint();
						break;
				}
			}
			else
				handled = true;
		}
		
		//
		directKeyPressed(msg);
	}
	
	if (!handled) 
	{
		if (msg == RC_up)
		{
			scrollLineUp();
		}
		else if (msg == RC_down)
		{
			scrollLineDown();
		}
		else if (msg == RC_left)
		{
			swipLeft();
		}
		else if (msg == RC_right)
		{
			swipRight();
		}
		else if (msg == RC_page_up)
		{
			scrollPageUp();
		}
		else if (msg == RC_page_down)
		{
			scrollPageDown();
		}
		else if (msg == RC_ok)
		{
			int rv = oKKeyPressed(parent);
				
			switch ( rv ) 
			{
				case RETURN_EXIT_ALL:
					ret = false;
				case RETURN_EXIT:
					ret = false;
					break;
				case RETURN_REPAINT:
					ret = true;
					paint();
					break;
			}
		}
		else if (msg == RC_home || msg == RC_timeout) 
		{
			ret = false;
		}
		else if ( (msg == NeutrinoMessages::EVT_TIMER) && (data == sec_timer_id) )
		{
			if (update())
			{
				refresh();
			}
		}
		else if ( CNeutrinoApp::getInstance()->handleMsg( msg, data ) & messages_return::cancel_all ) 
		{
			ret = false;
		}
	}
	
	return ret;
}

void CWidgetItem::exec(int timeout)
{
	dprintf(DEBUG_NORMAL, "CWidgetItem::exec: timeout:%d\n", timeout);
	
	// loop
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	bool loop = true;
	
	if ( timeout == -1 )
		timeout = 0xFFFF;
		
	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd(timeout);

	while(loop)
	{
		g_RCInput->getMsgAbsoluteTimeout(&msg, &data, &timeoutEnd);		
		
		loop = onButtonPress(msg, data); //

		CFrameBuffer::getInstance()->blit();
	}		
}

// headers
CHeaders::CHeaders(const int x, const int y, const int dx, const int dy, const char * const title, const char * const icon)
{
	dprintf(DEBUG_INFO, "CHeaders::CHeaders: x:%d y:%d dx:%d dy:%d title:%s icon:%s\n", x, y, dx, dy, title, icon);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	itemBox.iX = x;
	itemBox.iY = y;
	itemBox.iWidth = dx;
	itemBox.iHeight = dy;

	htitle = title? title : "";
	hicon = icon? icon : "";

	bgcolor = COL_MENUHEAD_PLUS_0;
	radius = g_settings.Head_radius;
	corner = g_settings.Head_corner;
	gradient = g_settings.Head_gradient;
	head_line = g_settings.Head_line;
	head_line_gradient = false;

	paintFrame = true;
	paintDate = false;
	format = "%d.%m.%Y %H:%M";
	timer = NULL;
	
	hbutton_count	= 0;
	hbutton_labels.clear();
	
	thalign = CC_ALIGN_LEFT;

	widgetItem_type = WIDGETITEM_HEAD;
}

CHeaders::CHeaders(CBox* position, const char * const title, const char * const icon)
{
	dprintf(DEBUG_INFO, "CHeaders::CHeaders: x:%d y:%d dx:%d dy:%d title:%s icon:%s\n", position->iX, position->iY, position->iWidth, position->iHeight, title, icon);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	itemBox = *position;

	htitle = title? title : "";
	hicon = icon? icon : "";

	bgcolor = COL_MENUHEAD_PLUS_0;
	radius = g_settings.Head_radius;
	corner = g_settings.Head_corner;
	gradient = g_settings.Head_gradient;
	head_line = g_settings.Head_line;
	head_line_gradient = false;

	paintFrame = true;
	paintDate = false;
	format = "%d.%m.%Y %H:%M";
	timer = NULL;
	
	hbutton_count	= 0;
	hbutton_labels.clear();
	
	thalign = CC_ALIGN_LEFT;

	widgetItem_type = WIDGETITEM_HEAD;
}

void CHeaders::setButtons(const struct button_label* _hbutton_labels, const int _hbutton_count)		
{
	if (_hbutton_count)
	{
		for (unsigned int i = 0; i < (unsigned int)_hbutton_count; i++)
		{
			hbutton_labels.push_back(_hbutton_labels[i]);
		}
	}

	hbutton_count = hbutton_labels.size();
}		

void CHeaders::paint()
{
	dprintf(DEBUG_INFO, "CHeaders::paint: (%s) (%s)\n", htitle.c_str(), hicon.c_str());
	
	// box
	if (paintFrame)
		CFrameBuffer::getInstance()->paintBoxRel(itemBox.iX, itemBox.iY, itemBox.iWidth, itemBox.iHeight, bgcolor, radius, corner, gradient);
	
	if (head_line)
		//CFrameBuffer::getInstance()->paintHLineRel(itemBox.iX + BORDER_LEFT, itemBox.iWidth - BORDER_LEFT - BORDER_RIGHT, itemBox.iY + itemBox.iHeight - 2, COL_MENUCONTENT_PLUS_5);
		frameBuffer->paintBoxRel(itemBox.iX + BORDER_LEFT, itemBox.iY + itemBox.iHeight - 2, itemBox.iWidth - BORDER_LEFT - BORDER_RIGHT, 2, COL_MENUCONTENT_PLUS_5, 0, CORNER_NONE, head_line_gradient? DARK2LIGHT2DARK : NOGRADIENT, GRADIENT_HORIZONTAL);

	// left icon
	int i_w = 0;
	int i_h = 0;

	if(!hicon.empty())
	{
		CFrameBuffer::getInstance()->getIconSize(hicon.c_str(), &i_w, &i_h);

		// scale icon
		if(i_h >= itemBox.iHeight)
		{
			i_h = itemBox.iHeight - 4;
			i_w = i_h*1.67;
		}

		CFrameBuffer::getInstance()->paintIcon(hicon.c_str(), itemBox.iX + BORDER_LEFT, itemBox.iY + (itemBox.iHeight - i_h)/2, 0, true, i_w, i_h);
	}

	// right buttons
	hbutton_count = hbutton_labels.size();

	int iw[hbutton_count], ih[hbutton_count];
	int startx = itemBox.iX + itemBox.iWidth - BORDER_RIGHT;
	int buttonWidth = 0;

	if(hbutton_count)
	{
		for (unsigned int i = 0; i < (unsigned int)hbutton_count; i++)
		{
			if (!hbutton_labels[i].button.empty())
			{
				CFrameBuffer::getInstance()->getIconSize(hbutton_labels[i].button.c_str(), &iw[i], &ih[i]);
				
				// scale icon
				if(ih[i] >= itemBox.iHeight)
				{
					ih[i] = itemBox.iHeight - 4;
				}
		
				startx -= (iw[i] + ICON_TO_ICON_OFFSET);
				buttonWidth += iw[i];

				CFrameBuffer::getInstance()->paintIcon(hbutton_labels[i].button, startx, itemBox.iY + (itemBox.iHeight - ih[i])/2, 0, true, iw[i], ih[i]);
			}
		}
	}

	// paint time/date
	int timestr_len = 0;
	if(paintDate)
	{
		std::string timestr = getNowTimeStr(format.c_str());
		
		timestr_len = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(timestr.c_str(), true); // UTF-8
	
		timer = new CCTime();
		timer->setPosition(startx - timestr_len, itemBox.iY, timestr_len, itemBox.iHeight);
		timer->setFont(SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE);
		timer->setFormat(format.c_str());
		timer->paint();
	}
	
	int startPosX = itemBox.iX + BORDER_LEFT + i_w + ICON_OFFSET;
	int stringWidth = g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getRenderWidth(htitle);
	
	if (thalign == CC_ALIGN_CENTER)
		startPosX = itemBox.iX + (itemBox.iWidth >> 1) - (stringWidth >> 1);

	// title
	g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->RenderString(startPosX, itemBox.iY + (itemBox.iHeight - g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight())/2 + g_Font[SNeutrinoSettings::FONT_TYPE_MENU_TITLE]->getHeight(), itemBox.iWidth - BORDER_LEFT - BORDER_RIGHT - i_w - 2*ICON_OFFSET - buttonWidth - (hbutton_count - 1)*ICON_TO_ICON_OFFSET - timestr_len, htitle, COL_MENUHEAD);
}

void CHeaders::hide()
{
	dprintf(DEBUG_INFO, "CHeaders::hide:\n");
	
	if (paintFrame)
		CFrameBuffer::getInstance()->paintBackgroundBoxRel(itemBox.iX, itemBox.iY, itemBox.iWidth, itemBox.iHeight);
		
	if (timer)
	{
		delete timer;
		timer = NULL;
	}
}

// footers
CFooters::CFooters(const int x, const int y, const int dx, const int dy)
{
	dprintf(DEBUG_INFO, "CFooters::CFooters: x:%d y:%d dx:%d dy:%d\n", x, y, dx, dy);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	itemBox.iX = x;
	itemBox.iY = y;
	itemBox.iWidth = dx;
	itemBox.iHeight = dy;

	fbuttons.clear();
	fcount = 0;
	
	paintFrame = true;

	fbgcolor = COL_MENUFOOT_PLUS_0;
	fradius = g_settings.Foot_radius;
	fcorner = g_settings.Foot_corner;
	fgradient = g_settings.Foot_gradient;
	foot_line = g_settings.Foot_line;
	foot_line_gradient = false;

	widgetItem_type = WIDGETITEM_FOOT;
}

CFooters::CFooters(CBox* position)
{
	dprintf(DEBUG_INFO, "CFooters::CFooters: x:%d y:%d dx:%d dy:%d\n", position->iX, position->iY, position->iWidth, position->iHeight);
	
	frameBuffer = CFrameBuffer::getInstance(); 
	
	itemBox = *position;

	fbuttons.clear();
	fcount = 0;
	fbutton_width = itemBox.iWidth;
	
	paintFrame = true;

	fbgcolor = COL_MENUFOOT_PLUS_0;
	fradius = g_settings.Foot_radius;
	fcorner = g_settings.Foot_corner;
	fgradient = g_settings.Foot_gradient;
	foot_line = g_settings.Foot_line;
	foot_line_gradient = false;

	widgetItem_type = WIDGETITEM_FOOT;
}

void CFooters::setButtons(const struct button_label *button_label, const int button_count, const int _fbutton_width)
{
	if (button_count)
	{
		for (unsigned int i = 0; i < (unsigned int)button_count; i++)
		{
			fbuttons.push_back(button_label[i]);
		}
	}

	fcount = fbuttons.size();
	fbutton_width = (_fbutton_width == 0)? itemBox.iWidth : _fbutton_width;	
}

void CFooters::paint()
{
	dprintf(DEBUG_INFO, "CFooters::paint:\n");
	
	// box
	if (paintFrame)
		CFrameBuffer::getInstance()->paintBoxRel(itemBox.iX, itemBox.iY, itemBox.iWidth, itemBox.iHeight, fbgcolor, fradius, fcorner, fgradient);
	
	// paint horizontal line buttom
	if (foot_line)
		//CFrameBuffer::getInstance()->paintHLineRel(itemBox.iX + BORDER_LEFT, itemBox.iWidth - BORDER_LEFT - BORDER_RIGHT, itemBox.iY + itemBox.iHeight - itemBox.iHeight + 2, COL_MENUCONTENT_PLUS_5);
		frameBuffer->paintBoxRel(itemBox.iX + BORDER_LEFT, itemBox.iY + 2, itemBox.iWidth - BORDER_LEFT - BORDER_RIGHT, 2, COL_MENUCONTENT_PLUS_5, 0, CORNER_NONE, foot_line_gradient? DARK2LIGHT2DARK : NOGRADIENT, GRADIENT_HORIZONTAL);

	int buttonWidth = 0;

	fcount = fbuttons.size();

	if(fcount)
	{
		buttonWidth = (fbutton_width)/fcount;
	
		for (unsigned int i = 0; i < fcount; i++)
		{
			if (!fbuttons[i].button.empty())
			{
				int iw = 0;
				int ih = 0;

				CFrameBuffer::getInstance()->getIconSize(fbuttons[i].button.c_str(), &iw, &ih);
				
				// scale icon
				if(ih >= itemBox.iHeight)
				{
					ih = itemBox.iHeight - 4;
				}
				
				int f_h = g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->getHeight();
		
				CFrameBuffer::getInstance()->paintIcon(fbuttons[i].button, itemBox.iX + BORDER_LEFT + i*buttonWidth, itemBox.iY + (itemBox.iHeight - ih)/2, 0, true, iw, ih);

				// FIXME: i18n
				g_Font[SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL]->RenderString(itemBox.iX + BORDER_LEFT + iw + ICON_OFFSET + i*buttonWidth, itemBox.iY + f_h + (itemBox.iHeight - f_h)/2, buttonWidth - iw - ICON_OFFSET, _(fbuttons[i].localename.c_str()), COL_MENUFOOT, 0, true); // UTF-8
			}
		}
	}
}

void CFooters::hide()
{
	dprintf(DEBUG_INFO, "CFooters::hide:\n");
	
	if (paintFrame)
		CFrameBuffer::getInstance()->paintBackgroundBoxRel(itemBox.iX, itemBox.iY, itemBox.iWidth, itemBox.iHeight);
}

