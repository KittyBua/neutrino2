/*
 * $Id: channel.h 16.11.2020 mohousch Exp $
 *
 * (C) 2002 Steffen Hehn <mcclean@berlios.de>
 * (C) 2002-2003 Andreas Oberritter <obi@tuxbox.org>
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

#ifndef __zapit_channel_h__
#define __zapit_channel_h__

#include <algorithm>
#include <cstdio>
#include <functional>
#include <map>
#include <vector>
#include <string.h>
#include <ctype.h>

// system 
#include <string>
#include <inttypes.h>
#include <cstdlib>

#include <sectionsd/sectionsd.h>

// zapit 
#include <zapit/ci.h>
#include <zapit/zapittypes.h>


// audio map struct
typedef struct audio_map_set 
{
        unsigned short apid;
        int mode;
        unsigned int volume;
        int subpid;
	int ttxpid;
	int ttxpage;
} audio_map_set_t;

// subtitling support
class CZapitAbsSub 
{
 	public:
    		unsigned short pId;
    		std::string ISO639_language_code;
		
    		enum ZapitSubtitleType 
    		{
        		TTX,
        		DVB
    		};
		
    		ZapitSubtitleType thisSubType;
};

class CZapitDVBSub : public CZapitAbsSub 
{
	public:
		unsigned short subtitling_type;
		/*
		 * possible values:
		 * 0x01 EBU Teletex subtitles
		 * 0x10 DVB subtitles (normal) with no monitor aspect ratio criticality
		 * 0x11 DVB subtitles (normal) for display on 4:3 aspect ratio monitor
		 * 0x12 DVB subtitles (normal) for display on 16:9 aspect ratio monitor
		 * 0x13 DVB subtitles (normal) for display on 2.21:1 aspect ratio monitor
		 * 0x20 DVB subtitles (for the hard of hearing) with no monitor aspect ratio criticality
		 * 0x21 DVB subtitles (for the hard of hearing) for display on 4:3 aspect ratio monitor
		 * 0x22 DVB subtitles (for the hard of hearing) for display on 16:9 aspect ratio monitor
		 * 0x23 DVB subtitles (for the hard of hearing) for display on 2.21:1 aspect ratio monitor
		 */
		unsigned int composition_page_id;
		unsigned int ancillary_page_id;

		CZapitDVBSub(){thisSubType = DVB;};
};

class CZapitTTXSub : public CZapitAbsSub
{
	public:
		unsigned short teletext_magazine_number;
		unsigned short teletext_page_number; // <- the actual important stuff here
		bool hearingImpaired;

		CZapitTTXSub(){thisSubType = TTX;};
};

class CZapitAudioChannel
{
	public:
		unsigned short		pid;

		std::string		description;
		unsigned char		componentTag;
		
		enum ZapitAudioChannelType 
		{
	                 MPEG,
	                 AC3,
	                 AAC,
	                 AACPLUS, //?
	                 DTS,
			 DTSHD,
	                 EAC3,
	                 LPCM,
	                 UNKNOWN,
	       };
	       
	       ZapitAudioChannelType audioChannelType;
};

//
class CCaPmt;

class CZapitChannel
{
	private:
		// pids of this channel
		std::vector <CZapitAbsSub* > channelSubs;
		std::vector <CZapitAudioChannel *> audioChannels;
		
		unsigned short pcrPid;
		unsigned short pmtPid;
		unsigned short teletextPid;
		unsigned short videoPid;
		unsigned short audioPid;
		unsigned short privatePid;
		unsigned short aitPid;

		// set true when pids are set up
		bool pidsFlag;

		// last selected audio channel
		unsigned char currentAudioChannel;

		// chosen subtitle stream
		unsigned char currentSub;

		// read only properties, set by constructor
		t_service_id			service_id;
		t_transport_stream_id		transport_stream_id;
		t_original_network_id		original_network_id;
		t_satellite_position		satellitePosition;
		freq_id_t			freq;

		// read/write properties (write possibility needed by scan)
		unsigned char serviceType;

		// the conditional access program map table of this channel
		CCaPmt* caPmt;
		unsigned char* rawPmt;
		int pmtLen;

		//
		std::string ttx_language_code;

	public:
		// channel name
		std::string name;

		bool bAlwaysLocked;

		int number;
		CChannelEvent currentEvent, nextEvent;
		int videoType;
		t_channel_id channel_id;
		t_channel_id epgid;
		t_channel_id logoid;
		unsigned char scrambled;
		char * pname; //makeRemainingChannelsBouquet
		
		uint8_t	polarization;

		// webtv
		std::string url;
		std::string description;
		bool isWebTV;
		std::string logourl;
		std::string epgidname;
		std::string epgurl;
		
		//
		uint64_t last_unlocked_EPGid;
		time_t last_unlocked_time;

		// constructor, desctructor
		CZapitChannel(const std::string& p_name, t_service_id p_sid, t_transport_stream_id p_tsid, t_original_network_id p_onid, unsigned char p_service_type, t_satellite_position p_satellite_position, freq_id_t p_freq);
		CZapitChannel(const std::string& p_name, t_channel_id p_chid, const std::string& p_url, const std::string& p_description);

		~CZapitChannel(void);

		// get methods - read only variables
		t_service_id		getServiceId(void)         	const { return service_id; }
		t_transport_stream_id	getTransportStreamId(void) 	const { return transport_stream_id; }
		t_original_network_id	getOriginalNetworkId(void) 	const { return original_network_id; }
		unsigned char        	getServiceType(bool real = false);
		int 			getVideoType()			{ return videoType;};

		bool			isHD();
		bool			is3DTV();
		bool 			isUHD();
		bool 			isWEBTV(){return isWebTV;};

		t_channel_id         	getChannelID(void)         	const { return channel_id; }
		transponder_id_t       	getTransponderId(void)		const { return CREATE_TRANSPONDER_ID(freq, satellitePosition, original_network_id, transport_stream_id); }
		freq_id_t		getFreqId()			const { return freq; }

		// get methods - read and write variables
		const std::string&	getName(void)			const { return name; }
		t_satellite_position	getSatellitePosition(void)	const { return satellitePosition; }
		unsigned char 		getAudioChannelCount(void)	{ return audioChannels.size(); }
		unsigned short		getPcrPid(void)			{ return pcrPid; }
		unsigned short		getPmtPid(void)			{ return pmtPid; }
		unsigned short		getTeletextPid(void)		{ return teletextPid; }
		const char *		getTeletextLang(void)		{ return ttx_language_code.c_str(); }
		unsigned short		getVideoPid(void)		{ return videoPid; }
		unsigned short		getPrivatePid(void)		{ return privatePid; }
		unsigned short		getPreAudioPid(void)		{ return audioPid; }
		bool			getPidsFlag(void)		{ return pidsFlag; }
		CCaPmt *		getCaPmt(void)			{ return caPmt; }
		unsigned char *		getRawPmt(int &len)		{ len = pmtLen; return rawPmt; };
		unsigned short		getaitPid(void)			{return aitPid;};
		//
		std::string		getUrl(void)			{ return url;};
		std::string		getDescription(void)		{ return description;};
		std::string		getLogoUrl(void)		{ return logourl;};
		std::string		getEPGUrl(void)			{ return epgurl;};
		t_channel_id		getEPGID(void)	 		const { return epgid;};
		t_channel_id		getLogoID(void)			const {return logoid;};
		std::string		getEPGIDName(void)		{return epgidname;};

		CZapitAudioChannel * 	getAudioChannel(unsigned char index = 0xFF);
		unsigned short 		getAudioPid(unsigned char index = 0xFF);
		unsigned char  		getAudioChannelIndex(void)	{ return currentAudioChannel; }

		int addAudioChannel(const unsigned short pid, const CZapitAudioChannel::ZapitAudioChannelType audioChannelType, const std::string & description, const unsigned char componentTag);

		// set methods
		void setServiceType(const unsigned char pserviceType)	{ serviceType = pserviceType; }
		inline void setName(const std::string pName)            { name = pName; }
		void setDescription(const std::string pDescr)		{ description = pDescr;};
		void setAudioChannel(unsigned char pAudioChannel)	{ if (pAudioChannel < audioChannels.size()) currentAudioChannel = pAudioChannel; }
		void setPcrPid(unsigned short pPcrPid)			{ pcrPid = pPcrPid; }
		void setPmtPid(unsigned short pPmtPid)			{ pmtPid = pPmtPid; }
		void setTeletextPid(unsigned short pTeletextPid)	{ teletextPid = pTeletextPid; }
		void setTeletextLang(char * lang)			{ ttx_language_code = lang; };
		void setVideoPid(unsigned short pVideoPid)		{ videoPid = pVideoPid; }
		void setAudioPid(unsigned short pAudioPid)		{ audioPid = pAudioPid; }
		void setPrivatePid(unsigned short pPrivatePid)		{ privatePid = pPrivatePid; }
		void setPidsFlag(void)					{ pidsFlag = true; }
		void setCaPmt(CCaPmt * pCaPmt);
		void setRawPmt(unsigned char * pmt, int len = 0);
		void setaitPid(unsigned short aitPID)			{ aitPid = aitPID; };
		void setNumber(unsigned int num)			{ number = num; };
		//
		void setLogoUrl(const std::string url)			{ logourl = url; };
		void setEPGUrl(const std::string url)			{ epgurl = url; };
		void setEPGID(const t_channel_id id)			{ epgid = id; };
		void setLogoID(const t_channel_id id)			{ logoid = id; };
		void setEPGIDName(const std::string name)		{ epgidname = name; };
		
		// cleanup methods
		void resetPids(void);
		
		// subtitling related methods
		void addTTXSubtitle(const unsigned int pid, const std::string langCode, const unsigned char magazine_number, const unsigned char page_number, const bool impaired = false);

		void addDVBSubtitle(const unsigned int pid, const std::string langCode, const unsigned char subtitling_type, const unsigned short composition_page_id, const unsigned short ancillary_page_id);

		unsigned getSubtitleCount() 				const { return channelSubs.size(); };
		CZapitAbsSub * getChannelSub(int index = -1);
		int getChannelSubIndex(void);
		void setChannelSub(int subIdx);
};

typedef std::vector<CZapitChannel*> ZapitChannelList;

typedef std::map<t_channel_id, CZapitChannel> tallchans;
typedef tallchans::iterator tallchans_iterator;

#endif /* __zapit_channel_h__ */
