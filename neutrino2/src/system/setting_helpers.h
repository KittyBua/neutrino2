#ifndef __setting_helpers__
#define __setting_helpers__

/*
	Neutrino-GUI  -   DBoxII-Project
	
	$id: setting_helpers.h 2015.12.22 15:24:30 mohousch $

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <gui/widget/widget.h>
#include <gui/widget/listbox.h>

// zapit includes
#include <zapit/zapittypes.h>

#include <string>


// sectionsd config notifier
class CSectionsdConfigNotifier : public CChangeObserver
{
        public:
                bool changeNotify(const std::string&, void * );
};

// color setup notifier
class CColorSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(const std::string&, void *);
};

// subtitle change notifier
class CSubtitleChangeExec : public CMenuTarget
{
	public:
		CSubtitleChangeExec(){};
		~CSubtitleChangeExec(){};
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

// NVOD change notifier
class CNVODChangeExec : public CMenuTarget
{
	public:
		CNVODChangeExec(){};
		~CNVODChangeExec(){};
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

// tuxtxt notifier
class CTuxtxtChangeExec : public CMenuTarget
{
	public:
		CTuxtxtChangeExec(){};
		~CTuxtxtChangeExec(){};
		int exec(CMenuTarget* parent, const std::string& actionKey);
};

// USERMENU
class CUserMenuMenu : public CMenuTarget
{
        private:
                int button;
                std::string local;
        public:
                CUserMenuMenu(const char* const _local, int _button){local = _local; button = _button;};
                ~CUserMenuMenu(){};
                int exec(CMenuTarget* parent, const std::string& actionKey);
};

#endif
