/*
	Neutrino-GUI  -   DBoxII-Project
	
	$Id: vcrcontrol.cpp 2013/10/12 mohousch Exp $

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
#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <errno.h>
#include <math.h>

#include <global.h>
#include <neutrino2.h>

#include <gui/movieinfo.h>

#include <driver/encoding.h>
#include <driver/stream2file.h>

#include <gui/widget/messagebox.h>

#include <daemonc/remotecontrol.h>

//
#include <zapit/zapit.h>
#include <zapit/zapittypes.h>
#include <zapit/frontend_c.h>
#include <zapit/channel.h>

#include <system/helpers.h>
#include <system/debug.h>

#include <driver/vcrcontrol.h>


extern bool autoshift;
extern bool autoshift_delete;

#define SA struct sockaddr
#define SAI struct sockaddr_in
extern "C" {
#include <driver/genpsi.h>
}


CMovieInfo * g_cMovieInfo;
MI_MOVIE_INFO * g_movieInfo;

extern CZapitChannel * live_channel;			// defined in zapit.cpp

static CVCRControl vcrControl;

CVCRControl * CVCRControl::getInstance()
{
	return &vcrControl;
}

CVCRControl::CVCRControl()
{
	Device = NULL;
}

CVCRControl::~CVCRControl()
{
	unregisterDevice();
}

void CVCRControl::unregisterDevice()
{
	if (Device)
	{
		delete Device;
		Device = NULL;
	}
}

void CVCRControl::registerDevice(CDevice * const device)
{
	unregisterDevice();
	
	Device = device;
}

bool CVCRControl::Record(const CTimerd::RecordingInfo * const eventinfo)
{
	int mode = CNeutrinoApp::getInstance()->getMode();

	return Device->Record(eventinfo->channel_id, mode, eventinfo->epgID, eventinfo->epgTitle, eventinfo->apids, eventinfo->epg_starttime); 
}

void CVCRControl::CDevice::getAPIDs(const unsigned char ap, APIDList & apid_list)
{
        unsigned char apids = ap;

        if (apids == TIMERD_APIDS_CONF)
                apids = g_settings.recording_audio_pids_default;

        apid_list.clear();
        CZapit::responseGetPIDs allpids;
	
	CZapit::getInstance()->getRecordPIDS(allpids);

        // assume smallest apid ist std apid
        if (apids & TIMERD_APIDS_STD)
        {
                uint32_t apid_min = UINT_MAX;
                uint32_t apid_min_idx = 0;
		
                for(unsigned int i = 0; i < allpids.APIDs.size(); i++)
                {
                        if (allpids.APIDs[i].pid < apid_min && !allpids.APIDs[i].is_ac3)
                        {
                                apid_min = allpids.APIDs[i].pid;
                                apid_min_idx = i;
                        }
                }
                
                if (apid_min != UINT_MAX)
                {
                        APIDDesc a = {apid_min, apid_min_idx, false};
                        apid_list.push_back(a);
                }
        }

        if (apids & TIMERD_APIDS_ALT)
        {
                uint32_t apid_min=UINT_MAX;
                uint32_t apid_min_idx = 0;
                for(unsigned int i = 0; i < allpids.APIDs.size(); i++)
                {
                        if (allpids.APIDs[i].pid < apid_min && !allpids.APIDs[i].is_ac3)
                        {
                                apid_min = allpids.APIDs[i].pid;
                                apid_min_idx = i;
                        }
                }
                
                for(unsigned int i = 0; i < allpids.APIDs.size(); i++)
                {
                        if (allpids.APIDs[i].pid != apid_min && !allpids.APIDs[i].is_ac3)
                        {
                                APIDDesc a = {allpids.APIDs[i].pid, i, false};
                                apid_list.push_back(a);
                        }
                }
        }

        if (apids & TIMERD_APIDS_AC3)
        {
                bool ac3_found = false;
                for(unsigned int i = 0; i < allpids.APIDs.size(); i++)
                {
                        if (allpids.APIDs[i].is_ac3)
                        {
                                APIDDesc a = {allpids.APIDs[i].pid, i, true};
                                apid_list.push_back(a);
                                ac3_found=true;
                        }
                }
                
                // add non ac3 apid if ac3 not found
                if (!(apids & TIMERD_APIDS_STD) && !ac3_found)
                {
                        uint32_t apid_min = UINT_MAX;
                        uint32_t apid_min_idx = 0;
			
                        for(unsigned int i = 0; i < allpids.APIDs.size(); i++)
                        {
                                if (allpids.APIDs[i].pid < apid_min && !allpids.APIDs[i].is_ac3)
                                {
                                        apid_min = allpids.APIDs[i].pid;
                                        apid_min_idx = i;
                                }
                        }

                        if (apid_min != UINT_MAX)
                        {
                                APIDDesc a = {apid_min, apid_min_idx, false};
                                apid_list.push_back(a);
                        }
                }
        }

        // no apid selected use standard
        if (apid_list.empty() && !allpids.APIDs.empty())
        {
                uint32_t apid_min = UINT_MAX;
                uint32_t apid_min_idx = 0;
		
                for(unsigned int i = 0; i < allpids.APIDs.size(); i++)
                {
                        if (allpids.APIDs[i].pid < apid_min && !allpids.APIDs[i].is_ac3)
                        {
                                apid_min = allpids.APIDs[i].pid;
                                apid_min_idx = i;
                        }
                }
                
                if (apid_min != UINT_MAX)
                {
                        APIDDesc a = {apid_min, apid_min_idx, false};
                        apid_list.push_back(a);
                }
                
                //for(APIDList::iterator it = apid_list.begin(); it != apid_list.end(); it++)
                //      printf("[CVCRControl] Record APID 0x%X %d\n",it->apid, it->ac3);
        }
}

bool CVCRControl::CVCRDevice::Stop()
{
	deviceState = CMD_VCR_STOP;

	if(last_mode != NeutrinoMessages::mode_scart)
	{
		g_RCInput->postMsg( NeutrinoMessages::VCR_OFF, 0 );
		g_RCInput->postMsg( NeutrinoMessages::CHANGEMODE , last_mode);
	}

	return true;
}

bool CVCRControl::CVCRDevice::Record(const t_channel_id channel_id, int mode, const event_id_t epgid, const std::string& epgTitle, unsigned char apids, const time_t epg_time)
{
	dprintf(DEBUG_NORMAL, "[CVCRControl] Record channel_id:%llx epg: %llx, apids %x mode\n", channel_id, epgid, apids);
	
	// leave menu (if in any)
	g_RCInput->postMsg(RC_timeout, 0);
	
	last_mode = CNeutrinoApp::getInstance()->getMode();

	if(mode != last_mode) 
	{
		CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , mode | NeutrinoMessages::norezap );
	}
	
	//
	if (!IS_WEBTV(channel_id))
	{
		// zapit
		if(channel_id != 0)	// wenn ein channel angegeben ist
		{
			// zap for record
			CZapit::getInstance()->zapTo_record(channel_id);			// for recording
		}

		// apids
		if(! (apids & TIMERD_APIDS_STD)) // nicht std apid
		{
		        APIDList apid_list;
		        getAPIDs(apids, apid_list);

		        if(!apid_list.empty())
		        {
		                if(!apid_list.begin()->ac3)
		                        CZapit::getInstance()->setAudioChannel(apid_list.begin()->index);
		                else
		                        CZapit::getInstance()->setAudioChannel(0); //sonst apid 0, also auf jeden fall ac3 aus !
		        }
		        else
		                CZapit::getInstance()->setAudioChannel(0); //sonst apid 0, also auf jeden fall ac3 aus !
		}
		else
		        CZapit::getInstance()->setAudioChannel(0); //sonst apid 0, also auf jeden fall ac3 aus !
	}

	// switch to scart
	if(SwitchToScart)
	{
		// Auf Scart schalten
		CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::VCR_ON, 0 );
		// Das ganze nochmal in die queue, da obiges RC_timeout erst in der naechsten ev. loop ausgef�hrt wird
		// und dann das menu widget das display falsch r�cksetzt
		g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
	}

	deviceState = CMD_VCR_RECORD;

	return true;
}

bool CVCRControl::CVCRDevice::Pause()
{
	return true;
}

bool CVCRControl::CVCRDevice::Resume()
{
	return true;
}

void CVCRControl::CFileAndServerDevice::RestoreNeutrino(void)
{
	CZapit::getInstance()->setRecordMode( false );

	// start playback
	if (!CZapit::getInstance()->isPlayBackActive() && (CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_standby))
		CZapit::getInstance()->startPlayBack(live_channel);

	// alten mode wieder herstellen (ausser wen zwischenzeitlich auf oder aus sb geschalten wurde)
	if(CNeutrinoApp::getInstance()->getMode() != last_mode && CNeutrinoApp::getInstance()->getMode() != NeutrinoMessages::mode_standby && last_mode != NeutrinoMessages::mode_standby)
	{
		if(!autoshift) 
			g_RCInput->postMsg( NeutrinoMessages::CHANGEMODE , last_mode);
	}

	if(last_mode == NeutrinoMessages::mode_standby && CNeutrinoApp::getInstance()->getMode() == NeutrinoMessages::mode_standby )
	{
		//Wenn vorher und jetzt standby, dann die zapit wieder auf sb schalten
		CZapit::getInstance()->setStandby(true);
	}	
}

void CVCRControl::CFileAndServerDevice::CutBackNeutrino(const t_channel_id channel_id, const int mode)
{
	last_mode = CNeutrinoApp::getInstance()->getMode();

	if (!IS_WEBTV(channel_id))
	{
		if(last_mode == NeutrinoMessages::mode_standby)
		{
			CZapit::getInstance()->setStandby(false);
		}
	
		if (channel_id != 0) 
		{
			if (mode != last_mode && (last_mode != NeutrinoMessages::mode_standby || mode != CNeutrinoApp::getInstance()->getLastMode())) 
			{
				CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , mode | NeutrinoMessages::norezap );
				// Wenn wir im Standby waren, dann brauchen wir f�rs streamen nicht aufwachen...
				if(last_mode == NeutrinoMessages::mode_standby)
					CNeutrinoApp::getInstance()->handleMsg( NeutrinoMessages::CHANGEMODE , NeutrinoMessages::mode_standby);
			}
		
			// zap to record
			CZapit::getInstance()->zapTo_record(channel_id);
		}

		// after this zapit send EVT_RECORDMODE_ACTIVATED, so neutrino getting NeutrinoMessages::EVT_RECORDMODE
		CZapit::getInstance()->setRecordMode( true );

		// stop playback im standby
		if( last_mode == NeutrinoMessages::mode_standby )
			CZapit::getInstance()->stopPlayBack();
	}
}

//
std::string CVCRControl::CFileAndServerDevice::getCommandString(const CVCRCommand command, const t_channel_id channel_id, const event_id_t epgid, const std::string& epgTitle, unsigned char apids)
{
	char tmp[40];
	std::string apids_selected;
	const char * extCommand;
	std::string info1, info2;

	std::string extMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n<neutrino commandversion=\"1\">\n\t<record command=\"";
	switch(command)
	{
		case CMD_VCR_RECORD:
			extCommand = "record";
			break;
		case CMD_VCR_STOP:
			extCommand = "stop";
			break;
		case CMD_VCR_PAUSE:
			extCommand = "pause";
			break;
		case CMD_VCR_RESUME:
			extCommand = "resume";
			break;
		case CMD_VCR_AVAILABLE:
			extCommand = "available";
			break;
		case CMD_VCR_UNKNOWN:
		default:
			extCommand = "unknown";
			printf("[CVCRControl] Unknown Command\n");
	}

	extMessage += extCommand;
	extMessage += 
		"\">\n"
		"\t\t<channelname>";
	
	//
	CZapit::responseGetPIDs pids;

	CZapit::getInstance()->getRecordPIDS(pids);

	CZapit::CCurrentServiceInfo si;

	si = CZapit::getInstance()->getRecordServiceInfo();

        APIDList apid_list;
        getAPIDs(apids, apid_list);
        apids_selected = "";
	
        for(APIDList::iterator it = apid_list.begin(); it != apid_list.end(); it++)
        {
                if(it != apid_list.begin())
                        apids_selected += " ";
                sprintf(tmp, "%u", it->apid);
                apids_selected += tmp;
        }

	std::string tmpstring;
	tmpstring = CZapit::getInstance()->getChannelName(channel_id);

	if (tmpstring.empty())
		extMessage += "unknown";
	else
		extMessage += UTF8_to_UTF8XML(tmpstring.c_str());
	
	extMessage += "</channelname>\n\t\t<epgtitle>";
	
	tmpstring = "not available";
	if (epgid != 0)
	{
		CShortEPGData epgdata;

		if(CSectionsd::getInstance()->getEPGidShort(epgid, &epgdata)) 
		{
			//#warning fixme sectionsd should deliver data in UTF-8 format
			tmpstring = epgdata.title;
			info1 = epgdata.info1;
			info2 = epgdata.info2;
		}
	} 
	else if (!epgTitle.empty()) 
	{
		tmpstring = epgTitle;
	}

	extMessage += UTF8_to_UTF8XML(tmpstring.c_str());
	
	extMessage += "</epgtitle>\n\t\t<id>";
	
	sprintf(tmp, "%llx", channel_id);
	extMessage += tmp;
	
	extMessage += "</id>\n\t\t<info1>";
	extMessage += UTF8_to_UTF8XML(info1.c_str());
	extMessage += "</info1>\n\t\t<info2>";
	extMessage += UTF8_to_UTF8XML(info2.c_str());
	extMessage += "</info2>\n\t\t<epgid>";
	sprintf(tmp, "%llu", epgid);
	extMessage += tmp;
	extMessage += "</epgid>\n\t\t<mode>";

	sprintf(tmp, "%d", CZapit::getInstance()->getMode());
	extMessage += tmp;
	extMessage += "</mode>\n\t\t<videopid>";
	sprintf(tmp, "%u", si.vpid);
	extMessage += tmp;
	extMessage += "</videopid>\n\t\t<audiopids selected=\"";
	extMessage += apids_selected;

	extMessage += "\">\n";
	
	// audio desc
	processAPIDnames();

	for(unsigned int i= 0; i< pids.APIDs.size(); i++)
	{
		extMessage += "\t\t\t<audio pid=\"";
		sprintf(tmp, "%u", pids.APIDs[i].pid);
		extMessage += tmp;
		extMessage += "\" name=\"";
		extMessage += UTF8_to_UTF8XML(pids.APIDs[i].desc);
		extMessage += "\"/>\n";
	}
	extMessage += 
		"\t\t</audiopids>\n"
		"\t\t<vtxtpid>";
	sprintf(tmp, "%u", si.vtxtpid);
	extMessage += tmp;
	extMessage +=
		"</vtxtpid>\n"
		"\t</record>\n"
		"</neutrino>\n";

	return extMessage;
}

bool CVCRControl::CFileDevice::Stop()
{
	std::string extMessage = " ";
	time_t end_time = time(0);
		
	g_movieInfo->length = (int) round((double) (end_time - start_time) / (double) 60);
	g_cMovieInfo->encodeMovieInfoXml(&extMessage, g_movieInfo);	

	bool return_value = (::stop_recording(extMessage.c_str()) == STREAM2FILE_OK);

	//
	RestoreNeutrino();

	deviceState = CMD_VCR_STOP;

	g_movieInfo->audioPids.clear();
	delete g_movieInfo;
	g_movieInfo = NULL;
	delete g_cMovieInfo;
	g_cMovieInfo = NULL;

	return return_value;
}

std::string ext_channel_name;
bool CVCRControl::CFileDevice::Record(const t_channel_id channel_id, int mode, const event_id_t epgid, const std::string& epgTitle, unsigned char apids, const time_t epg_time) 
{
	#define MAXPIDS		64
	unsigned short pids[MAXPIDS];
	unsigned int numpids = 0;
	unsigned int pos = 0;

	// cut neutrino
	CutBackNeutrino(channel_id, mode);

	CZapit::CCurrentServiceInfo si;

	si = CZapit::getInstance()->getRecordServiceInfo();

	numpids = 0;

	// vpid
	if (si.vpid != 0)
		transfer_pids(si.vpid, si.vtype ? EN_TYPE_AVC : EN_TYPE_VIDEO, 0);

	// apids
        APIDList apid_list;
        getAPIDs(apids, apid_list);

        for(APIDList::iterator it = apid_list.begin(); it != apid_list.end(); it++) 
	{
                pids[numpids++] = it->apid;
		transfer_pids(it->apid, EN_TYPE_AUDIO, it->ac3 ? 1 : 0);
        }
        
        CZapit::responseGetPIDs allpids;

	CZapit::getInstance()->getRecordPIDS(allpids);

	//record file name format
	char filename[512]; // UTF-8

	// Create filename for recording
	if (!IS_WEBTV(channel_id))
	{
		pos = Directory.size();
		strcpy(filename, Directory.c_str());
	
		if ((pos == 0) || (filename[pos - 1] != '/')) 
		{
			filename[pos] = '/';
			pos++;
			filename[pos] = '\0';
		}

		pos = strlen(filename);
	}

	ext_channel_name = CZapit::getInstance()->getChannelName(channel_id);

	if (!(ext_channel_name.empty()))
	{
		strcpy(&(filename[pos]), UTF8_TO_FILESYSTEM_ENCODING(ext_channel_name.c_str()));
		char * p_act = &(filename[pos]);
		do {
			p_act += strcspn(p_act, "/ \"%&-\t`'�!,:;");
			if (*p_act) 
			{
				*p_act++ = '_';
			}
		} while (*p_act);

		// save channel name dir
		if (!autoshift && g_settings.recording_save_in_channeldir)
		{
			struct stat statInfo;
			int res = stat(filename, &statInfo);

			if (res == -1) 
			{
				if (errno == ENOENT) 
				{
					res = safe_mkdir(filename);

					if (res == 0) 
					{
						strcat(filename, "/");
					} 
					else 
					{
						perror("[vcrcontrol] mkdir");
					}
						
				} 
				else 
				{
					perror("[vcrcontrol] stat");
				}
			} 
			else 
			{
				// directory exists
				strcat(filename, "/");
			}	
				
		} 
		else
		{
			strcat(filename, "_");
		}
	}

	pos = strlen(filename);

	// epg for filename
	if (g_settings.recording_epg_for_filename) 
	{ 
		if(epgid != 0) 
		{
			CShortEPGData epgdata;

			if(CSectionsd::getInstance()->getEPGidShort(epgid, &epgdata))
			{
				if (!(epgdata.title.empty()))
				{
					strcpy(&(filename[pos]), epgdata.title.c_str());
					char * p_act = &(filename[pos]);
					do {
						p_act +=  strcspn(p_act, "/ \"%&-\t`'~<>!,:;?^�$\\=*#@�|");

						if (*p_act) 
						{
							*p_act++ = '_';
						}
					} while (*p_act);
				}
			}
		} 
		else if (!epgTitle.empty()) 
		{
			strcpy(&(filename[pos]), epgTitle.c_str());
			char * p_act = &(filename[pos]);
			do {
				p_act +=  strcspn(p_act, "/ \"%&-\t`'~<>!,:;?^�$\\=*#@�|");
				if (*p_act) 
				{
					*p_act++ = '_';
				}
			} while (*p_act);
		}
	}

	pos = strlen(filename);
	time_t t = time(NULL);
	strftime(&(filename[pos]), sizeof(filename) - pos - 1, "%Y%m%d_%H%M%S", localtime(&t));

	start_time = time(0);

	stream2file_error_msg_t error_msg = STREAM2FILE_BUSY;

	if (IS_WEBTV(channel_id))
	{
		error_msg = ::start_file_recording(filename,
			      getMovieInfoString(CMD_VCR_RECORD, channel_id, epgid, epgTitle, apid_list, epg_time).c_str(), CZapit::getInstance()->getChannelURL(channel_id));
	}
	else
	{
		error_msg = ::start_recording(filename,
			      getMovieInfoString(CMD_VCR_RECORD, channel_id, epgid, epgTitle, apid_list, epg_time).c_str(), 
			      si.vpid, 
			      pids, 
			      numpids);
	}

	if (error_msg == STREAM2FILE_OK) 
	{
		deviceState = CMD_VCR_RECORD;
		return true;
	}
	else 
	{
		RestoreNeutrino();

		MessageBox(_("Error"), error_msg == STREAM2FILE_BUSY ? _("One or several recording processes are active.\nIf you encounter this message and no recording is active, please restart Neutrino.") : error_msg == STREAM2FILE_INVALID_DIRECTORY ? _("The recording directory is not writable.\nRecording will not work.") : _("The recording was aborted,\nbecause the target file could not be opened."), mbrCancel, mbCancel, NEUTRINO_ICON_ERROR);

		return false;
	}
}

bool CVCRControl::Screenshot(const t_channel_id channel_id, char * fname) 
{
	//FIXME:
	
	char filename[512]; // UTF-8
	char cmd[512];
	std::string channel_name;
	CEPGData epgData;
	event_id_t epgid = 0;
	unsigned int pos = 0;

	if(!fname) // live stream
	{
		std::string str = "sda1";
		
		struct statfs s;
		
		if (::statfs(g_settings.network_nfs_recordingdir, &s) == 0) 
		{
			//std::string str1 = g_settings.network_nfs_recordingdir;
			//str = str1.substr(7, 4);
			//str = str1.substr(str1.length() - 4, str1.length());
		}
		char str2[100];	
		sprintf(str2, "/media/%s/screenshots/", str.c_str() );
		
		if(safe_mkdir(str2))
			return false;

		strcpy(filename, str2);
	
		pos = strlen(filename);

		channel_name = CZapit::getInstance()->getChannelName(channel_id);

		if (!(channel_name.empty())) 
		{
			strcpy(&(filename[pos]), UTF8_TO_FILESYSTEM_ENCODING(channel_name.c_str()));
			char * p_act = &(filename[pos]);

			do {
				p_act += strcspn(p_act, "/ \"%&-\t`'�!,:;");
				if (*p_act) {
					*p_act++ = '_';
				}
			} while (*p_act);
			strcat(filename, "_");
		}

		pos = strlen(filename);

		if(CSectionsd::getInstance()->getActualEPGServiceKey(channel_id&0xFFFFFFFFFFFFULL, &epgData));
			epgid = epgData.eventID;

		if(epgid != 0) 
		{
			CShortEPGData epgdata;

			if(CSectionsd::getInstance()->getEPGidShort(epgid, &epgdata)) 
			{
				if (!(epgdata.title.empty())) 
				{
					strcpy(&(filename[pos]), epgdata.title.c_str());
					char * p_act = &(filename[pos]);
					do {
						p_act +=  strcspn(p_act, "/ \"%&-\t`'~<>!,:;?^�$\\=*#@�|");
						if (*p_act) {
							*p_act++ = '_';
						}
					} while (*p_act);
				}
			}
		}
		
		pos = strlen(filename);
		time_t t = time(NULL);
		strftime(&(filename[pos]), sizeof(filename) - pos - 1, "%Y%m%d_%H%M%S", localtime(&t));
		
		strcat(filename, ".jpg");
		
		sprintf(cmd, "grab -v -r320 %s", filename);
	} 
	else
	{
		//from tsbrowser
		strcpy(filename, fname);

		std::string file_name = fname; // UTF-8
		
		changeFileNameExt(file_name, ".jpg");
		
		sprintf(cmd, "grab -v -r320 %s", (char *)file_name.c_str());
	}
	
	printf("Executing %s\n", cmd);
	
	if(system(cmd))
	{
		return false;
	}
	
	return true;
}

std::string CVCRControl::CFileAndServerDevice::getMovieInfoString(const CVCRCommand command, const t_channel_id channel_id, const event_id_t epgid, const std::string& epgTitle, APIDList apid_list, const time_t epg_time)
{
	std::string extMessage;
	std::string apids10;
	std::string info1, info2;

	if(!g_cMovieInfo)
		g_cMovieInfo = new CMovieInfo();
	if(!g_movieInfo)
		g_movieInfo = new MI_MOVIE_INFO();

	g_cMovieInfo->clearMovieInfo(g_movieInfo);

	CZapit::getInstance()->getRecordPIDS(pids);


	CZapit::CCurrentServiceInfo si;

	si = CZapit::getInstance()->getRecordServiceInfo();

	std::string tmpstring;

	tmpstring = CZapit::getInstance()->getChannelName(channel_id);

	if (tmpstring.empty())
		g_movieInfo->epgChannel = "unknown";
	else
		g_movieInfo->epgChannel = UTF8_to_UTF8XML(tmpstring.c_str());

	tmpstring = "not available";
	if (epgid != 0) 
	{
		CEPGData epgdata;
		
		if (CSectionsd::getInstance()->getEPGid(epgid, epg_time, &epgdata)) 
		{
			tmpstring = epgdata.title;
			info1 = epgdata.info1;
			info2 = epgdata.info2;
			
			g_movieInfo->parentalLockAge = epgdata.fsk;
			
			if(epgdata.contentClassification.size() > 0 )
				g_movieInfo->genreMajor = epgdata.contentClassification[0];
				
			g_movieInfo->length = epgdata.epg_times.dauer	/ 60;
				
			printf("[CVCRControl] fsk:%d, Genre:%d, Dauer: %d min\r\n",g_movieInfo->parentalLockAge,g_movieInfo->genreMajor,g_movieInfo->length);	
		}
	} 
	else if (!epgTitle.empty()) 
	{
		tmpstring = epgTitle;
	}
	g_movieInfo->epgTitle		= UTF8_to_UTF8XML(tmpstring.c_str());
	g_movieInfo->epgId		= channel_id;
	g_movieInfo->epgInfo1		= UTF8_to_UTF8XML(info1.c_str());
	g_movieInfo->epgInfo2		= UTF8_to_UTF8XML(info2.c_str());
	g_movieInfo->epgEpgId		= epgid ;

	g_movieInfo->epgMode		= CZapit::getInstance()->getMode();
	g_movieInfo->epgVideoPid	= si.vpid;
	g_movieInfo->VideoType		= si.vtype;

	g_vpid = si.vpid;
	g_vtype = si.vtype;
	g_currentapid = si.apid;
	memset(g_apids, 0, sizeof(unsigned short)*10);
	memset(g_ac3flags, 0, sizeof(unsigned short)*10);
	g_numpida = 0;

	EPG_AUDIO_PIDS audio_pids;
	
	// get apids desc
	processAPIDnames();

	APIDList::iterator it;
	for(unsigned int i = 0; i < pids.APIDs.size(); i++) 
	{
		for(it = apid_list.begin(); it != apid_list.end(); it++) 
		{
			if(pids.APIDs[i].pid == it->apid) 
			{
				audio_pids.epgAudioPid = pids.APIDs[i].pid;
				audio_pids.epgAudioPidName = UTF8_to_UTF8XML(pids.APIDs[i].desc);
				audio_pids.atype = pids.APIDs[i].is_ac3;
				audio_pids.selected = (audio_pids.epgAudioPid == (int) g_currentapid) ? 1 : 0;

				g_movieInfo->audioPids.push_back(audio_pids);

				if(pids.APIDs[i].is_ac3)
					g_ac3flags[i] = 1;

				g_apids[i] = pids.APIDs[i].pid;
				if(g_apids[i] == g_currentapid)
					g_currentac3 = pids.APIDs[i].is_ac3;
				g_numpida++;
			}
		}
	}

	//FIXME sometimes no apid in xml ??
	if(g_movieInfo->audioPids.empty() && pids.APIDs.size()) 
	{
		int i = 0;
		audio_pids.epgAudioPid = pids.APIDs[i].pid;
		audio_pids.epgAudioPidName = UTF8_to_UTF8XML(pids.APIDs[i].desc);
		audio_pids.atype = pids.APIDs[i].is_ac3;
		audio_pids.selected = 1;

		g_movieInfo->audioPids.push_back(audio_pids);
	}
	g_movieInfo->epgVTXPID = si.vtxtpid;

	g_cMovieInfo->encodeMovieInfoXml(&extMessage, g_movieInfo);

	return extMessage;
}

void CVCRControl::CFileAndServerDevice::processAPIDnames()
{
	bool has_unresolved_ctags= false;
	bool has_ac3 = false; //FIXME what this variable suppoused to do ?? seems unused
	int ac3_found = -1;

	for(unsigned int count = 0; count < pids.APIDs.size(); count++)
	{
		printf("CVCRControl::CFileAndServerDevice::processAPIDnames: apid name= %s (%s) pid= 0x%x\n", pids.APIDs[count].desc, getISO639Description( pids.APIDs[count].desc ), pids.APIDs[count].pid);
		
		if ( pids.APIDs[count].component_tag != 0xFF )
		{
			has_unresolved_ctags= true;
		}
		
		if ( strlen( pids.APIDs[count].desc ) == 3 )
		{
			// unaufgeloeste Sprache...
			strcpy( pids.APIDs[count].desc, getISO639Description( pids.APIDs[count].desc ) );
		}

		if ( pids.APIDs[count].is_ac3 )
		{
			//strncat(pids.APIDs[count].desc, " (Dolby Digital)", 25);
			has_ac3 = true;
			if((strlen( pids.APIDs[count].desc ) == 3) && (ac3_found < 0))
				ac3_found = count;
		}
	}

	if ( has_unresolved_ctags )
	{
		if ( record_EPGid != 0 )
		{
			CSectionsd::ComponentTagList tags;

			if ( CSectionsd::getInstance()->getComponentTagsUniqueKey( record_EPGid, tags ) )
			{
				has_unresolved_ctags = false;

				for (unsigned int i=0; i< tags.size(); i++)
				{
					for (unsigned int j=0; j< pids.APIDs.size(); j++)
					{
						if ( pids.APIDs[j].component_tag == tags[i].componentTag )
						{
							// workaround for buggy ZDF ctags / or buggy sectionsd/drivers , who knows...
							if(!tags[i].component.empty())
							{
								strncpy(pids.APIDs[j].desc, tags[i].component.c_str(), 25);
								
								//if ( pids.APIDs[j].is_ac3)
								//	strncat( pids.APIDs[j].desc, " (Dolby Digital)", 25);
							}
							pids.APIDs[j].component_tag = -1;
							break;
						}
					}
				}
			}
		}
	}
}


