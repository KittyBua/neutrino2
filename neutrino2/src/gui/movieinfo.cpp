 /***************************************************************************
	Neutrino-GUI  -   DBoxII-Project

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

	***********************************************************

	Module Name: movieinfo.cpp .

	Description: Implementation of the CMovieInfo class
	             This class loads, saves and shows the movie Information from the any .xml File on HD

	Date:	  Nov 2005

	Author: Günther@tuxbox.berlios.org

	Revision History:
	Date			Author		Change Description
	Nov 2005		Günther	initial start

****************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include <unistd.h>

#include <gui/widget/infobox.h>
#include <gui/movieinfo.h>

//#define XMLTREE_LIB
#ifdef XMLTREE_LIB
#include <xmltree/xmltree.h>
#include <xmltree/xmltok.h>
#endif

#include <system/debug.h>
#include <system/helpers.h>
#include <system/tmdbparser.h>
#include <system/settings.h>

#include <driver/encoding.h>

#include <driver/audioplay.h>
#include <gui/movieplayer.h>


CMovieInfo::CMovieInfo()
{

}

CMovieInfo::~CMovieInfo()
{
	;
}

bool CMovieInfo::convertTs2XmlName(char *char_filename, int size)
{
	bool result = false;
	std::string filename = char_filename;
	
	if (convertTs2XmlName(&filename) == true) 
	{
		strncpy(char_filename, filename.c_str(), size);
		char_filename[size - 1] = 0;
		result = true;
	}
	
	return (result);
}

bool CMovieInfo::convertTs2XmlName(std::string * filename)
{
	int bytes = -1;
	int ext_pos = 0;
	ext_pos = filename->rfind('.');
	
	if( ext_pos > 0)
	{
		std::string extension;
		extension = filename->substr(ext_pos + 1, filename->length() - ext_pos);
		extension = "." + extension;
			
		bytes = filename->find( extension.c_str() );
	}
	
	bool result = false;

	if (bytes != -1) 
	{
		if (bytes > 3) 
		{
			if ((*filename)[bytes - 4] == '.') 
			{
				bytes = bytes - 4;
			}
		}
		*filename = filename->substr(0, bytes) + ".xml";
		result = true;
	} 
	else			// not a TS file, return!!!!! 
	{
		dprintf(DEBUG_INFO, "    not a TS file ");
	}

	return (result);
}

static void XML_ADD_TAG_STRING(std::string &_xml_text_, const char *_tag_name_, std::string _tag_content_)
{
	_xml_text_ += "\t\t<";
	_xml_text_ += _tag_name_;
	_xml_text_ += ">";
	_xml_text_ += ::UTF8_to_UTF8XML(_tag_content_.c_str());
	_xml_text_ += "</";
	_xml_text_ += _tag_name_;
	_xml_text_ += ">\n";
}

static void XML_ADD_TAG_UNSIGNED(std::string &_xml_text_, const char *_tag_name_, unsigned int _tag_content_)
{
	_xml_text_ += "\t\t<";
	_xml_text_ += _tag_name_;
	_xml_text_ += ">";
	_xml_text_ += to_string(_tag_content_);
	_xml_text_ += "</";
	_xml_text_ += _tag_name_;
	_xml_text_ += ">\n";
}

static void XML_ADD_TAG_LONG(std::string &_xml_text_, const char *_tag_name_, uint64_t _tag_content_)
{
	_xml_text_ += "\t\t<";
	_xml_text_ += _tag_name_;
	_xml_text_ += ">";\
	_xml_text_ += to_string(_tag_content_);
	_xml_text_ += "</";
	_xml_text_ += _tag_name_;
	_xml_text_ += ">\n";
}

bool CMovieInfo::encodeMovieInfoXml(std::string * extMessage, MI_MOVIE_INFO * movie_info)
{
	char tmp[40];

	*extMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n";
	*extMessage += "<" MI_XML_TAG_NEUTRINO " commandversion=\"1\">\n";
	*extMessage += "\t<" MI_XML_TAG_RECORD " command=\"";
	*extMessage += "record";
	*extMessage += "\">\n";
	XML_ADD_TAG_STRING(*extMessage, MI_XML_TAG_CHANNELNAME, movie_info->epgChannel);
	XML_ADD_TAG_STRING(*extMessage, MI_XML_TAG_EPGTITLE, movie_info->epgTitle);
	XML_ADD_TAG_LONG(*extMessage, MI_XML_TAG_ID, movie_info->epgId);
	XML_ADD_TAG_STRING(*extMessage, MI_XML_TAG_INFO1, movie_info->epgInfo1);
	XML_ADD_TAG_STRING(*extMessage, MI_XML_TAG_INFO2, movie_info->epgInfo2);
	XML_ADD_TAG_LONG(*extMessage, MI_XML_TAG_EPGID, movie_info->epgEpgId);			// %llu
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_MODE, movie_info->epgMode);		//%d
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_VIDEOPID, movie_info->epgVideoPid);	//%u
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_VIDEOTYPE, movie_info->VideoType);		//%u
	
	if (movie_info->audioPids.size() > 0) 
	{
		*extMessage += "\t\t<" MI_XML_TAG_AUDIOPIDS ">\n";

		for (unsigned int i = 0; i < movie_info->audioPids.size(); i++)	// pids.APIDs.size()
		{
			*extMessage += "\t\t\t<" MI_XML_TAG_AUDIO " " MI_XML_TAG_PID "=\"";
			sprintf(tmp, "%u", movie_info->audioPids[i].epgAudioPid);	//pids.APIDs[i].pid);
			*extMessage += tmp;
			*extMessage += "\" " MI_XML_TAG_ATYPE "=\"";
			sprintf(tmp, "%u", movie_info->audioPids[i].atype);	//pids.APIDs[i].pid);
			*extMessage += tmp;
			*extMessage += "\" " MI_XML_TAG_SELECTED "=\"";
			sprintf(tmp, "%u", movie_info->audioPids[i].selected);	//pids.APIDs[i].pid);
			*extMessage += tmp;
			*extMessage += "\" " MI_XML_TAG_NAME "=\"";
			*extMessage += movie_info->audioPids[i].epgAudioPidName;
			*extMessage += "\"/>\n";
		}
		*extMessage += "\t\t</" MI_XML_TAG_AUDIOPIDS ">\n";
	}

	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_VTXTPID, movie_info->epgVTXPID);	//%u
	/* new tags */
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_GENRE_MAJOR, movie_info->genreMajor);
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_GENRE_MINOR, movie_info->genreMinor);
	XML_ADD_TAG_STRING(*extMessage, MI_XML_TAG_SERIE_NAME, movie_info->serieName);
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_LENGTH, movie_info->length);
	XML_ADD_TAG_STRING(*extMessage, MI_XML_TAG_PRODUCT_COUNTRY, movie_info->productionCountry);
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_PRODUCT_DATE, movie_info->productionDate);
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_QUALITY, movie_info->quality);
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_PARENTAL_LOCKAGE, movie_info->parentalLockAge);
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_DATE_OF_LAST_PLAY, movie_info->dateOfLastPlay);
	*extMessage += "\t\t<" MI_XML_TAG_BOOKMARK ">\n";
	*extMessage += "\t";
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_BOOKMARK_START, movie_info->bookmarks.start);
	*extMessage += "\t";
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_BOOKMARK_END, movie_info->bookmarks.end);
	*extMessage += "\t";
	XML_ADD_TAG_UNSIGNED(*extMessage, MI_XML_TAG_BOOKMARK_LAST, movie_info->bookmarks.lastPlayStop);
	
	for (int i = 0; i < MI_MOVIE_BOOK_USER_MAX; i++) 
	{
		if (movie_info->bookmarks.user[i].pos != 0 || i == 0) 
		{
			// encode any valid book, at least 1
			*extMessage += "\t\t\t<" MI_XML_TAG_BOOKMARK_USER " " MI_XML_TAG_BOOKMARK_USER_POS "=\"";
			sprintf(tmp, "%d", movie_info->bookmarks.user[i].pos);		//pids.APIDs[i].pid);
			*extMessage += tmp;
			*extMessage += "\" " MI_XML_TAG_BOOKMARK_USER_TYPE "=\"";
			sprintf(tmp, "%d", movie_info->bookmarks.user[i].length);	//pids.APIDs[i].pid);
			*extMessage += tmp;
			*extMessage += "\" " MI_XML_TAG_BOOKMARK_USER_NAME "=\"";
			*extMessage += movie_info->bookmarks.user[i].name;
			*extMessage += "\"/>\n";
		}
	}

	*extMessage += "\t\t</" MI_XML_TAG_BOOKMARK ">\n";

	// vote_average
	XML_ADD_TAG_UNSIGNED(*extMessage, "vote_average", movie_info->vote_average);
	
	// genres
	XML_ADD_TAG_STRING(*extMessage, "genres", movie_info->genres);

	*extMessage += "\t</" MI_XML_TAG_RECORD ">\n";
	*extMessage += "</" MI_XML_TAG_NEUTRINO ">\n";
	
	return true;
}

bool CMovieInfo::saveMovieInfo(MI_MOVIE_INFO & movie_info, CFile * file)
{
	dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo\n");

	bool result = true;
	std::string text;
	CFile file_xml;

	if (file == NULL) 
	{
		file_xml.Name = movie_info.file.Name;
		result = convertTs2XmlName(&file_xml.Name);
	} 
	else 
	{
		file_xml.Name = file->Name;
	}
	
	dprintf(DEBUG_INFO, "CMovieInfo::saveMovieInfo: %s\r\n", file_xml.Name.c_str());

	if (result == true) 
	{
		result = encodeMovieInfoXml(&text, &movie_info);

		if (result == true)
		{
			result = saveFile(file_xml, text.c_str(), text.size());	// save

			if (result == false) 
			{
				dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: save error\r\n");
			}
		} 
		else 
		{
			dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: encoding error\r\n");
		}
	} 
	else 
	{
		dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: error\r\n");
	}
	
	return (result);
}

//
bool CMovieInfo::saveMovieInfo(const char* fileName, std::string title, std::string info1, std::string info2, CFile* file)
{
	dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo\n");
	
	MI_MOVIE_INFO movie_info;
	clearMovieInfo(&movie_info);

	movie_info.file.Name = fileName;
	movie_info.epgTitle = title;
	movie_info.epgInfo1 = info1;
	movie_info.epgInfo2 = info2;

	bool result = true;
	std::string text;
	CFile file_xml;

	if (file == NULL) 
	{
		file_xml.Name = movie_info.file.Name;
		result = convertTs2XmlName(&file_xml.Name);
	} 
	else 
	{
		file_xml.Name = file->Name;
	}
	
	dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: %s\r\n", file_xml.Name.c_str());

	if (result == true) 
	{
		result = encodeMovieInfoXml(&text, &movie_info);

		if (result == true)
		{
			result = saveFile(file_xml, text.c_str(), text.size());	// save

			if (result == false) 
			{
				dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: save error\r\n");
			}
		} 
		else 
		{
			dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: encoding error\r\n");
		}
	} 
	else 
	{
		dprintf(DEBUG_NORMAL, "CMovieInfo::saveMovieInfo: error\r\n");
	}
	
	return (result);
}

bool CMovieInfo::loadMovieInfo(MI_MOVIE_INFO * movie_info, CFile * file)
{
	dprintf(DEBUG_INFO, "CMovieInfo::loadMovieInfo\n");

	bool result = true;
	CFile file_xml;

	if (file == NULL) 
	{
		// if there is no give file, we use the file name from movie info but we have to convert the ts name to xml name first
		file_xml.Name = movie_info->file.Name;
	} 
	else 
	{
		file_xml.Name = file->Name;
	}

	result = convertTs2XmlName(&file_xml.Name);

	if (result == true) 
	{
		// load xml file in buffer
		char text[6000];
		result = loadFile(file_xml, text, 6000);

		if (result == true) 
		{
#ifdef XMLTREE_LIB
			result = parseXmlTree(text, movie_info);
#else /* XMLTREE_LIB */
			result = parseXmlQuickFix(text, movie_info);
#endif /* XMLTREE_LIB */
		}
	}
	
	//epgTitle
	if (movie_info->epgTitle.empty())
	{
		std::string tmp_str = movie_info->file.getFileName();

		removeExtension(tmp_str);

		movie_info->epgTitle = htmlEntityDecode(tmp_str);
	}
	
	// production date
	if ((movie_info->productionDate == 0) && g_settings.enable_tmdb_infos)
	{
		if(movie_info->file.getType() == CFile::FILE_VIDEO)
		{
			CTmdb * tmdb = new CTmdb();

			if(tmdb->getMovieInfo(movie_info->epgTitle))
			{
				if (!tmdb->getReleaseDate().empty())
				{
					movie_info->productionDate = atoi(tmdb->getReleaseDate().substr(0,4));
				}
			}

			delete tmdb;
			tmdb = NULL;
		}
	}
	
	if (movie_info->productionDate > 50 && movie_info->productionDate < 200)	// backwardcompaibility
		movie_info->productionDate += 1900;

	//epgInfo1
	if(movie_info->file.getType() == CFile::FILE_VIDEO)
	{
		if (movie_info->epgInfo1.empty() && g_settings.enable_tmdb_infos)
		{
			std::string epgInfo1 = "";
			CTmdb * tmdb = new CTmdb();

			if(tmdb->getMovieInfo(movie_info->epgTitle))
			{
				if ((!tmdb->getDescription().empty())) 
				{
					//movie_info->epgInfo1 = htmlEntityDecode(tmdb->getDescription().c_str());
					epgInfo1 = tmdb->getDescription();
				}
			}

			delete tmdb;
			tmdb = NULL;
			
			movie_info->epgInfo1 = htmlEntityDecode(epgInfo1);
		}
	}
	else if(movie_info->file.getType() == CFile::FILE_AUDIO)
	{
		char duration[9] = "";
		
		CAudiofile audiofile(movie_info->file.Name, movie_info->file.getExtension());

		CAudioPlayer::getInstance()->init();
		int ret = CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

		if (!ret || (audiofile.MetaData.artist.empty() && audiofile.MetaData.title.empty() ))
		{
			//remove extension (.mp3)
			std::string tmp = movie_info->file.getFileName().substr(movie_info->file.getFileName().rfind('/') + 1);
			tmp = tmp.substr(0, tmp.length() - 4);	//remove extension (.mp3)

			std::string::size_type i = tmp.rfind(" - ");
		
			if(i != std::string::npos)
			{ 
				audiofile.MetaData.title = tmp.substr(0, i);
				audiofile.MetaData.artist = tmp.substr(i + 3);
			}
			else
			{
				i = tmp.rfind('-');
				if(i != std::string::npos)
				{
					audiofile.MetaData.title = tmp.substr(0, i);
					audiofile.MetaData.artist = tmp.substr(i + 1);
				}
				else
					audiofile.MetaData.title = tmp;
			}
		}
			
		snprintf(duration, 8, "(%ld:%02ld)", audiofile.MetaData.total_time / 60, audiofile.MetaData.total_time % 60);
			
		movie_info->epgInfo1 = audiofile.MetaData.title;
		movie_info->epgInfo1 += "\n";
		movie_info->epgInfo1 += audiofile.MetaData.artist;
		movie_info->epgInfo1 += "\n";
		movie_info->epgInfo1 += audiofile.MetaData.genre;
		movie_info->epgInfo1 += "\n";
		movie_info->epgInfo1 += audiofile.MetaData.date;
		movie_info->epgInfo1 += "\n";
		movie_info->epgInfo1 += duration;
	}

	// vote_average
	if (movie_info->vote_average == 0)
	{
		if (g_settings.enable_tmdb_infos) //grab from tmdb
		{
			if(movie_info->file.getType() == CFile::FILE_VIDEO)
			{
				CTmdb * tmdb = new CTmdb();

				if(tmdb->getMovieInfo(movie_info->epgTitle))
				{
					std::vector<tmdbinfo>& minfo_list = tmdb->getMInfos();

					movie_info->vote_average = minfo_list[0].vote_average;
				}

				delete tmdb;
				tmdb = NULL;
			}
		}
	}
	
	// genres
	if (movie_info->genres.empty())
	{
		if (g_settings.enable_tmdb_infos) //grab from tmdb
		{
			if(movie_info->file.getType() == CFile::FILE_VIDEO)
			{
				CTmdb * tmdb = new CTmdb();

				if(tmdb->getMovieInfo(movie_info->epgTitle))
				{
					std::vector<tmdbinfo>& minfo_list = tmdb->getMInfos();

					movie_info->genres = minfo_list[0].genres;
				}

				delete tmdb;
				tmdb = NULL;
			}
		}
	}
	
	// preview
	if (movie_info->tfile.empty())
	{
		// audio files
		if(movie_info->file.getType() == CFile::FILE_AUDIO)
		{
			movie_info->tfile = DATADIR "/icons/no_coverArt.png";
			
			// mp3
			if (getFileExt(movie_info->file.Name) == "mp3")
			{
				CAudiofile audiofile(movie_info->file.Name, CFile::EXTENSION_MP3);

				CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

				if (!audiofile.MetaData.cover.empty())
					movie_info->tfile = audiofile.MetaData.cover;
			}
		}
		else if(movie_info->file.getType() == CFile::FILE_VIDEO)
		{
			movie_info->tfile = DATADIR "/icons/nopreview.jpg";
			
			std::string fname = "";
			fname = movie_info->file.Name;
			changeFileNameExt(fname, ".jpg");
					
			if (::file_exists(fname.c_str()))
				movie_info->tfile = fname.c_str();
			else
			{
				fname.clear();
				fname = movie_info->file.getPath();
				fname += movie_info->epgTitle;
				fname += ".jpg";

				if (::file_exists(fname.c_str()))
					movie_info->tfile = fname.c_str();
				else if (g_settings.enable_tmdb_preview) //grab from tmdb
				{
					CTmdb * tmdb = new CTmdb();

					if(tmdb->getMovieInfo(movie_info->epgTitle))
					{
						if ((!tmdb->getDescription().empty())) 
						{
							std::string tname = movie_info->file.getPath();
							tname += movie_info->epgTitle;
							tname += ".jpg";

							tmdb->getSmallCover(tmdb->getPosterPath(), tname);

							if(!tname.empty())
								movie_info->tfile = tname;
						}
					}

					delete tmdb;
					tmdb = NULL;
				}
			}
		}
	}

	return (result);
}

//
MI_MOVIE_INFO CMovieInfo::loadMovieInfo(const char *file)
{
	dprintf(DEBUG_INFO, "CMovieInfo::loadMovieInfo\n");

	MI_MOVIE_INFO movie_info;
	clearMovieInfo(&movie_info);

	bool result = true;
	CFile file_xml;

	if (file != NULL) 
	{
		file_xml.Name = file;
		movie_info.file.Name = file; 

		result = convertTs2XmlName(&file_xml.Name);

		if (result == true) 
		{
			// load xml file in buffer
			char text[6000];
			result = loadFile(file_xml, text, 6000);

			if (result == true) 
			{
#ifdef XMLTREE_LIB
				result = parseXmlTree(text, &movie_info);
#else /* XMLTREE_LIB */
				result = parseXmlQuickFix(text, &movie_info);
#endif /* XMLTREE_LIB */
			}
		}
		
		//epgTitle
		if (movie_info.epgTitle.empty())
		{
			std::string tmp_str = movie_info.file.getFileName();

			removeExtension(tmp_str);

			movie_info.epgTitle = htmlEntityDecode(tmp_str);
		}
	
		// production date
		if ((movie_info.productionDate == 0) && g_settings.enable_tmdb_infos)
		{
			if(movie_info.file.getType() == CFile::FILE_VIDEO)
			{
				CTmdb * tmdb = new CTmdb();

				if(tmdb->getMovieInfo(movie_info.epgTitle))
				{
					if (!tmdb->getReleaseDate().empty())
					{
						movie_info.productionDate = atoi(tmdb->getReleaseDate().substr(0,4));
					}
				}

				delete tmdb;
				tmdb = NULL;
			}
		}
	
		if (movie_info.productionDate > 50 && movie_info.productionDate < 200)	// backwardcompaibility
			movie_info.productionDate += 1900;

		//epgInfo1
		if(movie_info.file.getType() == CFile::FILE_VIDEO)
		{
			if (movie_info.epgInfo1.empty() && g_settings.enable_tmdb_infos)
			{
				std::string epgInfo1= "";
				CTmdb * tmdb = new CTmdb();

				if(tmdb->getMovieInfo(movie_info.epgTitle))
				{
					if ((!tmdb->getDescription().empty())) 
					{
						epgInfo1 = tmdb->getDescription();
						//movie_info.epgInfo1 = htmlEntityDecode(tmdb->getDescription().c_str());
					}
				}

				delete tmdb;
				tmdb = NULL;
				
				movie_info.epgInfo1 = htmlEntityDecode(epgInfo1);
			}
		}
		else if(movie_info.file.getType() == CFile::FILE_AUDIO)
		{
			char duration[9] = "";
		
			CAudiofile audiofile(movie_info.file.Name, movie_info.file.getExtension());

			CAudioPlayer::getInstance()->init();
			int ret = CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

			if (!ret || (audiofile.MetaData.artist.empty() && audiofile.MetaData.title.empty() ))
			{
				//remove extension (.mp3)
				std::string tmp = movie_info.file.getFileName().substr(movie_info.file.getFileName().rfind('/') + 1);
				tmp = tmp.substr(0, tmp.length() - 4);

				std::string::size_type i = tmp.rfind(" - ");
		
				if(i != std::string::npos)
				{ 
					audiofile.MetaData.title = tmp.substr(0, i);
					audiofile.MetaData.artist = tmp.substr(i + 3);
				}
				else
				{
					i = tmp.rfind('-');
					if(i != std::string::npos)
					{
						audiofile.MetaData.title = tmp.substr(0, i);
						audiofile.MetaData.artist = tmp.substr(i + 1);
					}
					else
						audiofile.MetaData.title = tmp;
				}
			}
			
			snprintf(duration, 8, "(%ld:%02ld)", audiofile.MetaData.total_time / 60, audiofile.MetaData.total_time % 60);
			
			movie_info.epgInfo1 = audiofile.MetaData.title;
			movie_info.epgInfo1 += "\n";
			movie_info.epgInfo1 += audiofile.MetaData.artist;
			movie_info.epgInfo1 += "\n";
			movie_info.epgInfo1 += audiofile.MetaData.genre;
			movie_info.epgInfo1 += "\n";
			movie_info.epgInfo1 += audiofile.MetaData.date;
			movie_info.epgInfo1 += "\n";
			movie_info.epgInfo1 += duration;
		}

		// vote_average
		if (movie_info.vote_average == 0)
		{
			if (g_settings.enable_tmdb_infos) //grab from tmdb
			{
				if(movie_info.file.getType() == CFile::FILE_VIDEO)
				{
					CTmdb * tmdb = new CTmdb();

					if(tmdb->getMovieInfo(movie_info.epgTitle))
					{
						std::vector<tmdbinfo>& minfo_list = tmdb->getMInfos();

						movie_info.vote_average = minfo_list[0].vote_average;
					}

					delete tmdb;
					tmdb = NULL;
				}
			}
		}
		
		// genres
		if (movie_info.genres.empty())
		{
			if (g_settings.enable_tmdb_infos) //grab from tmdb
			{
				if(movie_info.file.getType() == CFile::FILE_VIDEO)
				{
					CTmdb * tmdb = new CTmdb();

					if(tmdb->getMovieInfo(movie_info.epgTitle))
					{
						std::vector<tmdbinfo>& minfo_list = tmdb->getMInfos();

						movie_info.genres = minfo_list[0].genres;
					}

					delete tmdb;
					tmdb = NULL;
				}
			}
		}
		
		// preview
		if (movie_info.tfile.empty())
		{
			// audio files
			if(movie_info.file.getType() == CFile::FILE_AUDIO)
			{
				movie_info.tfile = DATADIR "/icons/no_coverArt.png";
				
				// mp3
				if (getFileExt(movie_info.file.Name) == "mp3")
				{
					CAudiofile audiofile(movie_info.file.Name, CFile::EXTENSION_MP3);

					CAudioPlayer::getInstance()->init();
					CAudioPlayer::getInstance()->readMetaData(&audiofile, true);

					if (!audiofile.MetaData.cover.empty())
						movie_info.tfile = audiofile.MetaData.cover;
				}
			}
			else if(movie_info.file.getType() == CFile::FILE_VIDEO)
			{
				movie_info.tfile = DATADIR "/icons/nopreview.jpg";
				
				std::string fname = "";
				fname = movie_info.file.Name;
				changeFileNameExt(fname, ".jpg");
					
				if (::file_exists(fname.c_str()))
					movie_info.tfile = fname.c_str();
				else
				{
					fname.clear();
					fname = movie_info.file.getPath();
					fname += movie_info.epgTitle;
					fname += ".jpg";

					if (::file_exists(fname.c_str()))
						movie_info.tfile = fname.c_str();
					else if (g_settings.enable_tmdb_preview) //grab from tmdb
					{
						CTmdb * tmdb = new CTmdb();

						if(tmdb->getMovieInfo(movie_info.epgTitle))
						{
							if ((!tmdb->getDescription().empty())) 
							{
								std::string tname = movie_info.file.getPath();
								tname += movie_info.epgTitle;
								tname += ".jpg";

								tmdb->getSmallCover(tmdb->getPosterPath(), tname);

								if(!tname.empty())
									movie_info.tfile = tname;
							}
						}

						delete tmdb;
						tmdb = NULL;
					}
				}
			}
		}
	}

	return movie_info;
}

bool CMovieInfo::parseXmlTree(char */*text*/, MI_MOVIE_INFO */*movie_info*/)
{
#ifndef XMLTREE_LIB
	return (false);		// no XML lib available return false
#else /* XMLTREE_LIB */

	XMLTreeParser *parser = new XMLTreeParser(NULL);

	if (!parser->Parse(text, strlen(text), 1)) 
	{
		dprintf(DEBUG_INFO, "parse error: %s at line %d \r\n", parser->ErrorString(parser->GetErrorCode()), parser->GetCurrentLineNumber());
		//fclose(in);
		delete parser;
		return (false);
	}

	XMLTreeNode *root = parser->RootNode();
	if (!root) 
	{
		dprintf(DEBUG_NORMAL, " root error \r\n");
		return (false);
	}

	if (strcmp(root->GetType(), MI_XML_TAG_NEUTRINO)) 
	{
		dprintf(DEBUG_NORMAL, "not neutrino file. %s", root->GetType());
		return (false);
	}

	XMLTreeNode *node = parser->RootNode();

	for (node = node->GetChild(); node; node = node->GetNext()) 
	{
		if (!strcmp(node->GetType(), MI_XML_TAG_RECORD)) 
		{
			for (XMLTreeNode * xam1 = node->GetChild(); xam1; xam1 = xam1->GetNext()) 
			{
				XML_GET_DATA_STRING(xam1, MI_XML_TAG_CHANNELNAME, movie_info->epgChannel);
				XML_GET_DATA_STRING(xam1, MI_XML_TAG_EPGTITLE, movie_info->epgTitle);
				XML_GET_DATA_LONG(xam1, MI_XML_TAG_ID, movie_info->epgId);
				XML_GET_DATA_STRING(xam1, MI_XML_TAG_INFO1, movie_info->epgInfo1);
				XML_GET_DATA_STRING(xam1, MI_XML_TAG_INFO2, movie_info->epgInfo2);
				XML_GET_DATA_LONG(xam1, MI_XML_TAG_EPGID, movie_info->epgEpgId);	// %llu
				XML_GET_DATA_INT(xam1, MI_XML_TAG_MODE, movie_info->epgMode);		//%d
				XML_GET_DATA_INT(xam1, MI_XML_TAG_VIDEOPID, movie_info->epgVideoPid);	//%u
				XML_GET_DATA_INT(xam1, MI_XML_TAG_VIDEOTYPE, movie_info->VideoType);	//%u
				
				if (!strcmp(xam1->GetType(), MI_XML_TAG_AUDIOPIDS)) 
				{
					for (XMLTreeNode * xam2 = xam1->GetChild(); xam2; xam2 = xam2->GetNext()) 
					{
						if (!strcmp(xam2->GetType(), MI_XML_TAG_AUDIO)) 
						{
							EPG_AUDIO_PIDS pids;
							pids.epgAudioPid = atoi(xam2->GetAttributeValue((char *)MI_XML_TAG_PID));
							pids.atype = atoi(xam2->GetAttributeValue((char *)MI_XML_TAG_ATYPE));
							pids.selected = atoi(xam2->GetAttributeValue((char *)MI_XML_TAG_SELECTED));
							pids.epgAudioPidName = xam2->GetAttributeValue((char *)MI_XML_TAG_NAME);
							//printf("MOVIE INFO: apid %d type %d name %s selected %d\n", pids.epgAudioPid, pids.atype, pids.epgAudioPidName.c_str(), pids.selected);
							movie_info->audioPids.push_back(pids);
						}
					}
				}
				XML_GET_DATA_INT(xam1, MI_XML_TAG_VTXTPID, movie_info->epgVTXPID);	//%u
				/*new tags */
				XML_GET_DATA_INT(xam1, MI_XML_TAG_GENRE_MAJOR, movie_info->genreMajor);
				XML_GET_DATA_INT(xam1, MI_XML_TAG_GENRE_MINOR, movie_info->genreMinor);
				XML_GET_DATA_STRING(xam1, MI_XML_TAG_SERIE_NAME, movie_info->serieName);
				XML_GET_DATA_INT(xam1, MI_XML_TAG_LENGTH, movie_info->length);
				XML_GET_DATA_STRING(xam1, MI_XML_TAG_PRODUCT_COUNTRY, movie_info->productionCountry);
				//if(!strcmp(xam1->GetType(), MI_XML_TAG_PRODUCT_COUNTRY)) if(xam1->GetData() != NULL)strncpy(movie_info->productionCountry, xam1->GetData(),4);        
				XML_GET_DATA_INT(xam1, MI_XML_TAG_PRODUCT_DATE, movie_info->productionDate);
				XML_GET_DATA_INT(xam1, MI_XML_TAG_QUALITY, movie_info->quality);
				XML_GET_DATA_INT(xam1, MI_XML_TAG_PARENTAL_LOCKAGE, movie_info->parentalLockAge);
				XML_GET_DATA_INT(xam1, MI_XML_TAG_DATE_OF_LAST_PLAY, movie_info->dateOfLastPlay);

				if (!strcmp(xam1->GetType(), MI_XML_TAG_BOOKMARK)) 
				{
					for (XMLTreeNode * xam2 = xam1->GetChild(); xam2; xam2 = xam2->GetNext()) 
					{
						XML_GET_DATA_INT(xam2, MI_XML_TAG_BOOKMARK_START, movie_info->bookmarks.start);
						XML_GET_DATA_INT(xam2, MI_XML_TAG_BOOKMARK_END, movie_info->bookmarks.end);
						XML_GET_DATA_INT(xam2, MI_XML_TAG_BOOKMARK_LAST, movie_info->bookmarks.lastPlayStop);
					}
				}
				
				//
				XML_GET_DATA_INT(xam1, "vote_average", movie_info->vote_average);
				XML_GET_DATA_STRING(xam1, "genres", movie_info->genres);
			}
		}
	}

	delete parser;
	
	//
	strReplace(movie_info->epgTitle, "&quot;", "\"");
	strReplace(movie_info->epgInfo1, "&quot;", "\"");
	strReplace(movie_info->epgTitle, "&apos;", "'");
	strReplace(movie_info->epgInfo1, "&apos;", "'");
	strReplace(movie_info->epgInfo1, "&amp;", "&");
	
	htmlEntityDecode(movie_info->epgInfo1, true);
	
	//
	strReplace(movie_info->epgInfo2, "&quot;", "\"");
	strReplace(movie_info->epgInfo2, "&apos;", "'");
	strReplace(movie_info->epgInfo2, "&amp;", "&");
		
	htmlEntityDecode(movie_info->epgInfo2, true);
#endif /* XMLTREE_LIB */

	return (true);
}

void CMovieInfo::showMovieInfo(MI_MOVIE_INFO &movie_info)
{
	dprintf(DEBUG_NORMAL, "CMovieInfo::showMovieInfo:\n");

	std::string print_buffer;
	tm *date_tm;
	char date_char[100];

	// prepare print buffer 
	if(movie_info.vote_count != 0)
	{
		print_buffer = "Vote: " + to_string(movie_info.vote_average) + "/10 Votecount: " + to_string(movie_info.vote_count);

		print_buffer += "\n";
	}

	// epgInfo1
	if(!movie_info.epgInfo1.empty())
	{
		print_buffer += "\n";
		print_buffer += movie_info.epgInfo1;
		print_buffer += "\n";
	}

	// genre // genre major|minor ???
	if(!movie_info.genres.empty())
	{
		print_buffer += (std::string)_("Genre") + ": " + movie_info.genres;
		print_buffer += "\n";
	}

	// orig title
	if(!movie_info.original_title.empty())
	{
		print_buffer += (std::string)_("Original Title") + " : " + movie_info.original_title;
		print_buffer += "\n";
	}

	// release date
	/*
	if(!movie_info.release_date.empty())
	{
		print_buffer += (std::string)_("Year of profuction") + " : " + movie_info.release_date.substr(0,4);
		print_buffer += "\n";
	}
	*/

	// cast
	if (!movie_info.cast.empty())
	{
		print_buffer += "\n";
		print_buffer += (std::string)_("Actors") + ":\n" + movie_info.cast;
		print_buffer += "\n";
	}
	
	// epgInfo2
	if(!movie_info.epgInfo2.empty())
	{
		print_buffer += "\n";
		print_buffer += movie_info.epgInfo2;
		print_buffer += "\n";
	}

	// production country
	if (movie_info.productionCountry.size() != 0) 
	{
		print_buffer += "\n";
		print_buffer += (std::string)_("Country") + " : ";
		print_buffer += movie_info.productionCountry;

		print_buffer += "\n";
	}
	
	// production date
	if (movie_info.productionDate != 0) 
	{
		print_buffer += "\n";
		print_buffer += (std::string)_("Year of production") + " : ";
		//snprintf(date_char, 12, "%4d", movie_info.productionDate + 1900);
		//print_buffer += date_char;
		print_buffer += to_string(movie_info.productionDate);

		print_buffer += "\n";
	}

	// serie name
	if (!movie_info.serieName.empty()) 
	{
		print_buffer += "\n";
		print_buffer += _("Serie");
		print_buffer += ": ";
		print_buffer += movie_info.serieName;

		print_buffer += "\n";
	}
	
	// epgChannel
	if (!movie_info.epgChannel.empty()) 
	{
		print_buffer += "\n";
		print_buffer += _("Channel");
		print_buffer += ": ";
		print_buffer += movie_info.epgChannel;

		print_buffer += "\n";
	}
	
	// quality
	if (movie_info.quality != 0) 
	{
		print_buffer += "\n";
		print_buffer += _("Quality");
		print_buffer += ": ";
		snprintf(date_char, 12, "%2d", movie_info.quality);
		print_buffer += date_char;

		print_buffer += "\n";
	}
	
	// parental
	if (movie_info.parentalLockAge != 0) 
	{
		print_buffer += "\n";
		print_buffer += _("Age");
		print_buffer += ": ";
		snprintf(date_char, 12, "%2d", movie_info.parentalLockAge);
		print_buffer += date_char;
		print_buffer += " Jahre";

		print_buffer += "\n";
	}
	
	// lenght
	if (movie_info.length != 0) 
	{
		print_buffer += "\n";
		print_buffer += _("Length (Min)");
		print_buffer += ": ";
		snprintf(date_char, 12, "%3d", movie_info.length);
		print_buffer += date_char;

		print_buffer += "\n";
	}
	
	// audio pids
	if (movie_info.audioPids.size() != 0) 
	{
		print_buffer += "\n";
		print_buffer += _("Audio");
		print_buffer += ": ";
		for (unsigned int i = 0; i < movie_info.audioPids.size(); i++) 
		{
			print_buffer += movie_info.audioPids[i].epgAudioPidName;
			print_buffer += ", ";
		}

		print_buffer += "\n";
	}

	// ytdate
	if(movie_info.ytdate.empty())
	{
		print_buffer += "\n";
		print_buffer += _("Last play date");
		print_buffer += ": ";
		date_tm = localtime(&movie_info.dateOfLastPlay);
		snprintf(date_char, 12, "%02d.%02d.%04d", date_tm->tm_mday, date_tm->tm_mon + 1, date_tm->tm_year + 1900);
		print_buffer += date_char;
		print_buffer += "\n";
		print_buffer += _("Record date");
		print_buffer += ": ";
		date_tm = localtime(&movie_info.file.Time);
		snprintf(date_char, 12, "%02d.%02d.%04d", date_tm->tm_mday, date_tm->tm_mon + 1, date_tm->tm_year + 1900);
		print_buffer += date_char;

		print_buffer += "\n";
	}
	
	// file size
	if (movie_info.file.Size != 0) 
	{
		print_buffer += "\n";
		print_buffer += _("Size");
		print_buffer += ": ";
		//snprintf(date_char, 12,"%4llu",movie_info.file.Size>>20);
		sprintf(date_char, "%llu", movie_info.file.Size >> 20);
		print_buffer += date_char;

		print_buffer += "\n"; 
	}
	
	// file path
	if(movie_info.ytdate.empty())
	{
		print_buffer += "\n";
		print_buffer += _("Path");
		print_buffer += ": ";
		print_buffer += movie_info.file.Name;

		print_buffer += "\n";
	}
	
	// thumbnail
	if(access(movie_info.tfile.c_str(), F_OK))
		movie_info.tfile = "";
	
	// infoBox
	CBox position(g_settings.screen_StartX + 50, g_settings.screen_StartY + 50, g_settings.screen_EndX - g_settings.screen_StartX - 100, g_settings.screen_EndY - g_settings.screen_StartY - 100); 
	
	CInfoBox * infoBox = new CInfoBox(&position, movie_info.epgTitle.empty()? movie_info.file.getFileName().c_str() : movie_info.epgTitle.c_str(), NEUTRINO_ICON_MOVIE);

	// scale pic
	int p_w = 0;
	int p_h = 0;

	::scaleImage(movie_info.tfile, &p_w, &p_h);

	infoBox->setFont(SNeutrinoSettings::FONT_TYPE_EPG_INFO1);
	infoBox->setMode(SCROLL);
	infoBox->setText(print_buffer.c_str(), movie_info.tfile.c_str(), p_w, p_h);
	infoBox->exec();
	delete infoBox;
}

void CMovieInfo::printDebugMovieInfo(MI_MOVIE_INFO & movie_info)
{
	dprintf(DEBUG_DEBUG, " FileName: %s", movie_info.file.Name.c_str());
	//dprintf(DEBUG_DEBUG, " FilePath: %s", movie_info.file.GetFilePath ); 
	//dprintf(DEBUG_DEBUG, " FileLength: %d", movie_info.file.GetLength ); 
	//dprintf(DEBUG_DEBUG, " FileStatus: %d", movie_info.file.GetStatus ); 

	dprintf(DEBUG_DEBUG, " ********** Movie Data ***********\r\n");				// (date, month, year)
	dprintf(DEBUG_DEBUG, " dateOfLastPlay: \t%d\r\n", (int)movie_info.dateOfLastPlay);	// (date, month, year)
	dprintf(DEBUG_DEBUG, " dirItNr: \t\t%d\r\n", movie_info.dirItNr);			// 
	dprintf(DEBUG_DEBUG, " genreMajor: \t\t%d\r\n", movie_info.genreMajor);			//genreMajor;                           
	dprintf(DEBUG_DEBUG, " genreMinor: \t\t%d\r\n", movie_info.genreMinor);			//genreMinor;                           
	dprintf(DEBUG_DEBUG, " length: \t\t%d\r\n", movie_info.length);				// (minutes)
	dprintf(DEBUG_DEBUG, " quality: \t\t%d\r\n", movie_info.quality);			// (3 stars: classics, 2 stars: very good, 1 star: good, 0 stars: OK)
	dprintf(DEBUG_DEBUG, " productionCount:\t>%s<\r\n", movie_info.productionCountry.c_str());
	dprintf(DEBUG_DEBUG, " productionDate: \t%d\r\n", movie_info.productionDate);		// (Year)  years since 1900
	dprintf(DEBUG_DEBUG, " parentalLockAge: \t\t\t%d\r\n", movie_info.parentalLockAge);	// MI_PARENTAL_LOCKAGE (0,6,12,16,18)
	dprintf(DEBUG_DEBUG, " format: \t\t%d\r\n", movie_info.format);				// MI_VIDEO_FORMAT(16:9, 4:3)
	dprintf(DEBUG_DEBUG, " audio: \t\t%d\r\n", movie_info.audio);				// MI_AUDIO (AC3, Deutsch, Englisch)
	dprintf(DEBUG_DEBUG, " epgId: \t\t%d\r\n", (int)movie_info.epgId);
	dprintf(DEBUG_DEBUG, " epgEpgId: \t\t%llu\r\n", movie_info.epgEpgId);
	dprintf(DEBUG_DEBUG, " epgMode: \t\t%d\r\n", movie_info.epgMode);
	dprintf(DEBUG_DEBUG, " epgVideoPid: \t\t%d\r\n", movie_info.epgVideoPid);
	dprintf(DEBUG_DEBUG, " epgVTXPID: \t\t%d\r\n", movie_info.epgVTXPID);
	dprintf(DEBUG_DEBUG, " Size: \t\t%d\r\n", (int)movie_info.file.Size >> 20);
	dprintf(DEBUG_DEBUG, " Date: \t\t%d\r\n", (int)movie_info.file.Time);

	for (unsigned int i = 0; i < movie_info.audioPids.size(); i++) 
	{
		dprintf(DEBUG_DEBUG, " audioPid (%d): \t\t%d\r\n", i, movie_info.audioPids[i].epgAudioPid);
		dprintf(DEBUG_DEBUG, " audioName(%d): \t\t>%s<\r\n", i, movie_info.audioPids[i].epgAudioPidName.c_str());
	}

	dprintf(DEBUG_DEBUG, " epgTitle: \t\t>%s<\r\n", movie_info.epgTitle.c_str());
	dprintf(DEBUG_DEBUG, " epgInfo1:\t\t>%s<\r\n", movie_info.epgInfo1.c_str());	//epgInfo1              
	dprintf(DEBUG_DEBUG, " epgInfo2:\t\t\t>%s<\r\n", movie_info.epgInfo2.c_str());	//epgInfo2
	dprintf(DEBUG_DEBUG, " epgChannel:\t\t>%s<\r\n", movie_info.epgChannel.c_str());
	dprintf(DEBUG_DEBUG, " serieName:\t\t>%s<\r\n", movie_info.serieName.c_str());	// (name e.g. 'StarWars)

	dprintf(DEBUG_DEBUG, " bookmarks start: \t%d\r\n", movie_info.bookmarks.start);
	dprintf(DEBUG_DEBUG, " bookmarks end: \t%d\r\n", movie_info.bookmarks.end);
	dprintf(DEBUG_DEBUG, " bookmarks lastPlayStop: %d\r\n", movie_info.bookmarks.lastPlayStop);

	for (int i = 0; i < MI_MOVIE_BOOK_USER_MAX; i++) 
	{
		if (movie_info.bookmarks.user[i].pos != 0 || i == 0) 
		{
			dprintf(DEBUG_DEBUG, " bookmarks user, pos:%d, type:%d, name: >%s<\r\n", movie_info.bookmarks.user[i].pos, movie_info.bookmarks.user[i].length, movie_info.bookmarks.user[i].name.c_str());
		}
	}
}

int find_next_char(char to_find, char *text, int start_pos, int end_pos)
{
	while (start_pos < end_pos) 
	{
		if (text[start_pos] == to_find) 
		{
			return (start_pos);
		}
		start_pos++;
	}
	return (-1);
}

#define GET_XML_DATA_STRING(_text_,_pos_,_tag_,_dest_)\
	if(strncmp(&_text_[_pos_],_tag_,sizeof(_tag_)-1) == 0)\
	{\
		_pos_ += sizeof(_tag_) ;\
		int pos_prev = _pos_;\
		while(_pos_ < bytes && _text_[_pos_] != '<' ) _pos_++;\
		_dest_ = "";\
		_dest_.append(&_text_[pos_prev],_pos_ - pos_prev );\
		_pos_ += sizeof(_tag_);\
		continue;\
	}

#define GET_XML_DATA_INT(_text_,_pos_,_tag_,_dest_)\
	if(strncmp(&_text_[pos],_tag_,sizeof(_tag_)-1) == 0)\
	{\
		_pos_ += sizeof(_tag_) ;\
		int pos_prev = _pos_;\
		while(_pos_ < bytes && _text_[_pos_] != '<' ) pos++;\
		_dest_ = atoi(&_text_[pos_prev]);\
		continue;\
	}
	
#define GET_XML_DATA_LONG(_text_,_pos_,_tag_,_dest_)\
	if(strncmp(&_text_[pos],_tag_,sizeof(_tag_)-1) == 0)\
	{\
		_pos_ += sizeof(_tag_) ;\
		int pos_prev = _pos_;\
		while(_pos_ < bytes && _text_[_pos_] != '<' ) pos++;\
		_dest_ = atoll(&_text_[pos_prev]);\
		continue;\
	}

bool CMovieInfo::parseXmlQuickFix(char *text, MI_MOVIE_INFO * movie_info)
{
#ifndef XMLTREE_LIB
	int bookmark_nr = 0;
	movie_info->dateOfLastPlay = 0;	//100*366*24*60*60;              // (date, month, year)

	int bytes = strlen(text);
	/** search ****/
	int pos = 0;

	EPG_AUDIO_PIDS audio_pids;

	while ((pos = find_next_char('<', text, pos, bytes)) != -1) 
	{
		pos++;
		GET_XML_DATA_STRING(text, pos, MI_XML_TAG_CHANNELNAME, movie_info->epgChannel)
		    GET_XML_DATA_STRING(text, pos, MI_XML_TAG_EPGTITLE, movie_info->epgTitle)
		    GET_XML_DATA_LONG(text, pos, MI_XML_TAG_ID, movie_info->epgId)
		    GET_XML_DATA_STRING(text, pos, MI_XML_TAG_INFO1, movie_info->epgInfo1)
		    GET_XML_DATA_STRING(text, pos, MI_XML_TAG_INFO2, movie_info->epgInfo2)
		    GET_XML_DATA_LONG(text, pos, MI_XML_TAG_EPGID, movie_info->epgEpgId)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_MODE, movie_info->epgMode)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_VIDEOPID, movie_info->epgVideoPid)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_VIDEOTYPE, movie_info->VideoType)
		    GET_XML_DATA_STRING(text, pos, MI_XML_TAG_NAME, movie_info->epgChannel)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_VTXTPID, movie_info->epgVTXPID)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_GENRE_MAJOR, movie_info->genreMajor)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_GENRE_MINOR, movie_info->genreMinor)
		    GET_XML_DATA_STRING(text, pos, MI_XML_TAG_SERIE_NAME, movie_info->serieName)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_LENGTH, movie_info->length)
		    GET_XML_DATA_STRING(text, pos, MI_XML_TAG_PRODUCT_COUNTRY, movie_info->productionCountry)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_PRODUCT_DATE, movie_info->productionDate)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_PARENTAL_LOCKAGE, movie_info->parentalLockAge)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_QUALITY, movie_info->quality)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_DATE_OF_LAST_PLAY, movie_info->dateOfLastPlay)
		    GET_XML_DATA_STRING(text, pos, "genres", movie_info->genres)
		    GET_XML_DATA_INT(text, pos, "vote_average", movie_info->vote_average)
		    if (strncmp(&text[pos], MI_XML_TAG_AUDIOPIDS, sizeof(MI_XML_TAG_AUDIOPIDS) - 1) == 0)
			pos += sizeof(MI_XML_TAG_AUDIOPIDS);

		/* parse audio pids */
		if (strncmp(&text[pos], MI_XML_TAG_AUDIO, sizeof(MI_XML_TAG_AUDIO) - 1) == 0) 
		{
			pos += sizeof(MI_XML_TAG_AUDIO);

			size_t pos2;
			char *ptr;

			pos2 = -1;
			ptr = strstr(&text[pos], MI_XML_TAG_PID);
			if (ptr)
				pos2 = (size_t)ptr - (size_t)&text[pos];
			//pos2 = strcspn(&text[pos],MI_XML_TAG_PID);
			if (pos2 >= 0) 
			{
				pos2 += sizeof(MI_XML_TAG_PID);
				while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
					pos2++;
				if (text[pos + pos2] == '\"')
					audio_pids.epgAudioPid = atoi(&text[pos + pos2 + 1]);
			} else
				audio_pids.epgAudioPid = 0;

			audio_pids.atype = 0;
			pos2 = -1;
			ptr = strstr(&text[pos], MI_XML_TAG_ATYPE);
			if (ptr)
				pos2 = (size_t)ptr - (size_t)&text[pos];
			//pos2 = strcspn(&text[pos],MI_XML_TAG_ATYPE);
			if (pos2 >= 0) 
			{
				pos2 += sizeof(MI_XML_TAG_ATYPE);
				while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
					pos2++;
				if (text[pos + pos2] == '\"')
					audio_pids.atype = atoi(&text[pos + pos2 + 1]);
			}

			audio_pids.selected = 0;
			pos2 = -1;
			ptr = strstr(&text[pos], MI_XML_TAG_SELECTED);
			if (ptr)
				pos2 = (size_t)ptr - (size_t)&text[pos];
			//pos2 = strcspn(&text[pos],MI_XML_TAG_SELECTED);
			if (pos2 >= 0) 
			{
				pos2 += sizeof(MI_XML_TAG_SELECTED);
				while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
					pos2++;
				if (text[pos + pos2] == '\"')
					audio_pids.selected = atoi(&text[pos + pos2 + 1]);
			}

			audio_pids.epgAudioPidName = "";
			//pos2 = strcspn(&text[pos],MI_XML_TAG_NAME);
			pos2 = -1;
			ptr = strstr(&text[pos], MI_XML_TAG_NAME);
			if (ptr)
				pos2 = (size_t)ptr - (size_t)&text[pos];
			if (pos2 >= 0) 
			{
				pos2 += sizeof(MI_XML_TAG_PID);
				while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
					pos2++;
				if (text[pos + pos2] == '\"') 
				{
					size_t pos3 = pos2 + 1;
					while (text[pos + pos3] != '\"' && text[pos + pos3] != 0 && text[pos + pos3] != '/')
						pos3++;
					if (text[pos + pos3] == '\"')
						audio_pids.epgAudioPidName.append(&text[pos + pos2 + 1], pos3 - pos2 - 1);
				}
			}
			//printf("MOVIE INFO: apid %d type %d name %s selected %d\n", audio_pids.epgAudioPid, audio_pids.atype, audio_pids.epgAudioPidName.c_str(), audio_pids.selected);
			movie_info->audioPids.push_back(audio_pids);
		}
		
		/* parse bookmarks */
		GET_XML_DATA_INT(text, pos, MI_XML_TAG_BOOKMARK_START, movie_info->bookmarks.start)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_BOOKMARK_END, movie_info->bookmarks.end)
		    GET_XML_DATA_INT(text, pos, MI_XML_TAG_BOOKMARK_LAST, movie_info->bookmarks.lastPlayStop)

		    if (bookmark_nr < MI_MOVIE_BOOK_USER_MAX) 
		    {
			if (strncmp(&text[pos], MI_XML_TAG_BOOKMARK_USER, sizeof(MI_XML_TAG_BOOKMARK_USER) - 1) == 0) 
			{
				pos += sizeof(MI_XML_TAG_BOOKMARK_USER);
				//int pos2 = strcspn(&text[pos],MI_XML_TAG_BOOKMARK_USER_POS);
				if (strcspn(&text[pos], MI_XML_TAG_BOOKMARK_USER_POS) == 0) 
				{
					size_t pos2 = 0;
					pos2 += sizeof(MI_XML_TAG_BOOKMARK_USER_POS);
					while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
						pos2++;
					if (text[pos + pos2] == '\"') 
					{
						movie_info->bookmarks.user[bookmark_nr].pos = atoi(&text[pos + pos2 + 1]);

						//pos2 = strcspn(&text[pos],MI_XML_TAG_BOOKMARK_USER_TYPE);
						pos++;
						while (text[pos + pos2] == ' ')
							pos++;
						if (strcspn(&text[pos], MI_XML_TAG_BOOKMARK_USER_TYPE) == 0) 
						{
							pos2 += sizeof(MI_XML_TAG_BOOKMARK_USER_TYPE);
							while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
								pos2++;
							if (text[pos + pos2] == '\"') 
							{
								movie_info->bookmarks.user[bookmark_nr].length = atoi(&text[pos + pos2 + 1]);

								movie_info->bookmarks.user[bookmark_nr].name = "";
								//pos2 = ;
								if (strcspn(&text[pos], MI_XML_TAG_BOOKMARK_USER_NAME) == 0) 
								{
									pos2 += sizeof(MI_XML_TAG_BOOKMARK_USER_NAME);
									while (text[pos + pos2] != '\"' && text[pos + pos2] != 0 && text[pos + pos2] != '/')
										pos2++;
									if (text[pos + pos2] == '\"') 
									{
										size_t pos3 = pos2 + 1;
										while (text[pos + pos3] != '\"' && text[pos + pos3] != 0 && text[pos + pos3] != '/')
											pos3++;
										if (text[pos + pos3] == '\"')
											movie_info->bookmarks.user[bookmark_nr].name.append(&text[pos + pos2 + 1], pos3 - pos2 - 1);
									}
								}
							}
						} 
						else
							movie_info->bookmarks.user[bookmark_nr].length = 0;
					}
					bookmark_nr++;
				} 
				else
					movie_info->bookmarks.user[bookmark_nr].pos = 0;
			}
		}
	}

	strReplace(movie_info->epgTitle, "&quot;", "\"");
	strReplace(movie_info->epgInfo1, "&quot;", "\"");
	strReplace(movie_info->epgTitle, "&apos;", "'");
	strReplace(movie_info->epgInfo1, "&apos;", "'");
	strReplace(movie_info->epgInfo1, "&amp;", "&");
	
	htmlEntityDecode(movie_info->epgInfo1, true);
	
	//
	strReplace(movie_info->epgInfo2, "&quot;", "\"");
	strReplace(movie_info->epgInfo2, "&apos;", "'");
	strReplace(movie_info->epgInfo2, "&amp;", "&");
		
	htmlEntityDecode(movie_info->epgInfo2, true);

	return (true);
#endif
	return (false);
}

bool CMovieInfo::addNewBookmark(MI_MOVIE_INFO * movie_info, MI_BOOKMARK & new_bookmark)
{
	dprintf(DEBUG_INFO, "CMovieInfo::addNewBookmark:\n");
	
	bool result = false;
	
	if (movie_info != NULL) 
	{
		// search for free entry 
		bool loop = true;
		for (int i = 0; i < MI_MOVIE_BOOK_USER_MAX && loop == true; i++) 
		{
			if (movie_info->bookmarks.user[i].pos == 0) 
			{
				// empty entry found
				result = true;
				loop = false;
				movie_info->bookmarks.user[i].pos = new_bookmark.pos;
				movie_info->bookmarks.user[i].length = new_bookmark.length;
				//if(movie_info->bookmarks.user[i].name.empty())
				if (movie_info->bookmarks.user[i].name.size() == 0) 
				{
					if (new_bookmark.length == 0)
						movie_info->bookmarks.user[i].name = _("New Bookmark");
					if (new_bookmark.length < 0)
						movie_info->bookmarks.user[i].name = _("Repeat");
					if (new_bookmark.length > 0)
						movie_info->bookmarks.user[i].name = _("Jump over");
				} 
				else 
				{
					movie_info->bookmarks.user[i].name = new_bookmark.name;
				}
			}
		}
	}
	
	return (result);
}

void CMovieInfo::clearMovieInfo(MI_MOVIE_INFO * movie_info)
{
	dprintf(DEBUG_INFO, "CMovieInfo::clearMovieInfo:\n");

	tm timePlay;
	timePlay.tm_hour = 0;
	timePlay.tm_min = 0;
	timePlay.tm_sec = 0;
	timePlay.tm_year = 100;
	timePlay.tm_mday = 0;
	timePlay.tm_mon = 1;

	movie_info->file.Name = "";
	movie_info->file.Size = 0;			// Megabytes
	movie_info->file.Time = mktime(&timePlay);
	movie_info->dateOfLastPlay = mktime(&timePlay);	// (date, month, year)
	movie_info->dirItNr = 0;			// 
	movie_info->genreMajor = 0;			//genreMajor;                           
	movie_info->genreMinor = 0;			//genreMinor;                           
	movie_info->length = 0;			// (minutes)
	movie_info->quality = 0;			// (3 stars: classics, 2 stars: very good, 1 star: good, 0 stars: OK)
	movie_info->productionDate = 0;		// (Year)  years since 1900
	movie_info->parentalLockAge = 0;		// MI_PARENTAL_LOCKAGE (0,6,12,16,18)
	movie_info->format = 0;			// MI_VIDEO_FORMAT(16:9, 4:3)
	movie_info->audio = 0;				// MI_AUDIO (AC3, Deutsch, Englisch)

	movie_info->epgId = 0;
	movie_info->epgEpgId = 0;
	movie_info->epgMode = 0;
	movie_info->epgVideoPid = 0;
	movie_info->VideoType = 0;
	movie_info->epgVTXPID = 0;

	movie_info->audioPids.clear();

	movie_info->productionCountry.clear();
	movie_info->epgTitle.clear();
	movie_info->epgInfo1.clear();			//epgInfo1              
	movie_info->epgInfo2.clear();			//epgInfo2
	movie_info->epgChannel = "";
	movie_info->serieName.clear();			// (name e.g. 'StarWars)

	movie_info->bookmarks.end = 0;
	movie_info->bookmarks.start = 0;
	movie_info->bookmarks.lastPlayStop = 0;
	
	for (int i = 0; i < MI_MOVIE_BOOK_USER_MAX; i++) 
	{
		movie_info->bookmarks.user[i].pos = 0;
		movie_info->bookmarks.user[i].length = 0;
		movie_info->bookmarks.user[i].name = "";
	}
	
	movie_info->tfile = "";
	movie_info->ytdate = "";
	movie_info->ytid = "";

	movie_info->original_title = "";
	movie_info->genres = "";
	movie_info->media_type = "";
	movie_info->vote_count = 0;
	movie_info->vote_average = 0.0;
	movie_info->runtimes = "";
	movie_info->seasons = 0;
	movie_info->episodes = 0;
	movie_info->cast = "";
	movie_info->vid = "";
	movie_info->vkey = "";
	movie_info->vname = "";
		
}

bool CMovieInfo::loadFile(CFile& file, char *buffer, int buffer_size)
{
	bool result = true;

	//dprintf(DEBUG_INFO, "CMovieInfo::laodFile: %s\n", file.getFileName().c_str());

	// open file
	int fd = open(file.Name.c_str(), O_RDONLY);
	if (fd == -1)		// cannot open file, return!!!!! 
	{
		//dprintf(DEBUG_NORMAL, "CMovieInfo::laodFile: cannot open (%s)\r\n", file.getFileName().c_str());
		return false;
	}
	
	// read file content to buffer 
	int bytes = read(fd, buffer, buffer_size - 1);
	if (bytes <= 0)		// cannot read file into buffer, return!!!! 
	{
		//dprintf(DEBUG_NORMAL, "CMovieInfo::laodFile: cannot read (%s)\r\n", file.getFileName().c_str());
		return false;
	}

	close(fd);
	buffer[bytes] = 0;	// terminate string
	
	return (result);
}

bool CMovieInfo::saveFile(const CFile & file, const char *text, const int text_size)
{
	dprintf(DEBUG_INFO, "CMovieInfo::saveFile: %s\n", file.getName().c_str());

	bool result = false;
	int fd;

	if ((fd = open(file.Name.c_str(), O_SYNC | O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) >= 0) 
	{
		int nr;
		nr = write(fd, text, text_size);
		//fdatasync(fd);
		close(fd);
		result = true;
	} 
	else 
	{
		dprintf(DEBUG_NORMAL, "CMovieInfo::saveFile: cannot open\r\n");
	}

	return (result);
}

void CMovieInfo::copy(MI_MOVIE_INFO * src, MI_MOVIE_INFO * dst)
{
	dst->file.Name = src->file.Name;
	dst->file.Size = src->file.Size;
	dst->file.Time = src->file.Time;
	dst->dateOfLastPlay = src->dateOfLastPlay;
	dst->dirItNr = src->dirItNr;
	dst->genreMajor = src->genreMajor;
	dst->genreMinor = src->genreMinor;
	dst->length = src->length;
	dst->quality = src->quality;
	dst->productionDate = src->productionDate;
	dst->parentalLockAge = src->parentalLockAge;
	dst->format = src->format;
	dst->audio = src->audio;

	dst->epgId = src->epgId;
	dst->epgEpgId = src->epgEpgId;
	dst->epgMode = src->epgMode;
	dst->epgVideoPid = src->epgVideoPid;
	dst->VideoType = src->VideoType;
	dst->epgVTXPID = src->epgVTXPID;

	dst->productionCountry = src->productionCountry;
	dst->epgTitle = src->epgTitle;
	dst->epgInfo1 = src->epgInfo1;
	dst->epgInfo2 = src->epgInfo2;
	dst->epgChannel = src->epgChannel;
	dst->serieName = src->serieName;
	dst->bookmarks.end = src->bookmarks.end;
	dst->bookmarks.start = src->bookmarks.start;
	dst->bookmarks.lastPlayStop = src->bookmarks.lastPlayStop;

	for (int i = 0; i < MI_MOVIE_BOOK_USER_MAX; i++) 
	{
		dst->bookmarks.user[i].pos = src->bookmarks.user[i].pos;
		dst->bookmarks.user[i].length = src->bookmarks.user[i].length;
		dst->bookmarks.user[i].name = src->bookmarks.user[i].name;
	}

	for (unsigned int i = 0; i < src->audioPids.size(); i++) 
	{
		EPG_AUDIO_PIDS audio_pids;
		audio_pids.epgAudioPid = src->audioPids[i].epgAudioPid;
		audio_pids.epgAudioPidName = src->audioPids[i].epgAudioPidName;
		audio_pids.atype = src->audioPids[i].atype;
		dst->audioPids.push_back(audio_pids);
	}
	
	//
	dst->vote_average = src->vote_average;
	dst->genres = src->genres;
}

// CMovieInfoWidget
CMovieInfoWidget::CMovieInfoWidget()
{
	m_movieInfo.clearMovieInfo(&movieFile);
}

CMovieInfoWidget::~CMovieInfoWidget()
{
	m_movieInfo.clearMovieInfo(&movieFile);
}

void CMovieInfoWidget::hide()
{
	CFrameBuffer::getInstance()->paintBackground();
	CFrameBuffer::getInstance()->blit();
}

void CMovieInfoWidget::setMovie(MI_MOVIE_INFO& file)
{
	m_movieInfo.clearMovieInfo(&movieFile);

	movieFile = file;
}

void CMovieInfoWidget::setMovie(const CFile& file, std::string title, std::string info1, std::string info2, std::string tfile, std::string duration, std::string rating)
{
	m_movieInfo.clearMovieInfo(&movieFile);

	movieFile.file.Name = file.Name;
	movieFile.epgTitle = title;
	movieFile.epgInfo1 = info1;
	movieFile.epgInfo2 = info2;
	movieFile.tfile = tfile;
	movieFile.length = atol(duration.c_str());
	movieFile.vote_average = (float)atol(rating.c_str());
}

void CMovieInfoWidget::setMovie(const char* fileName, std::string title, std::string info1, std::string info2, std::string tfile, std::string duration, std::string rating)
{
	m_movieInfo.clearMovieInfo(&movieFile);

	movieFile.file.Name = fileName;
	movieFile.epgTitle = title;
	movieFile.epgInfo1 = info1;
	movieFile.epgInfo2 = info2;
	movieFile.tfile = tfile;
	movieFile.length = atol(duration.c_str());
	movieFile.vote_average = (float)atol(rating.c_str());
	
	printf("setMovie:%d\n", movieFile.length);
}

void CMovieInfoWidget::funArt()
{
	// mainBox
	CBox box;
	box.iX = CFrameBuffer::getInstance()->getScreenX();
	box.iY = CFrameBuffer::getInstance()->getScreenY();
	box.iWidth = CFrameBuffer::getInstance()->getScreenWidth();
	box.iHeight = CFrameBuffer::getInstance()->getScreenHeight();

	// titleBox
	CBox titleBox;
	titleBox.iX = box.iX + 10;
	titleBox.iY = box.iY + 10;
	titleBox.iWidth = box.iWidth;
	titleBox.iHeight = 40;

	// starBox
	CBox starBox;
	starBox.iX = box.iX +10;
	starBox.iY = box.iY + titleBox.iHeight + 10;
	starBox.iWidth = 25;
	starBox.iHeight = 25;

	// playBox
	CBox playBox;
	playBox.iWidth = 300;
	playBox.iHeight = 60;
	playBox.iX = box.iX + 10;
	playBox.iY = box.iY + box.iHeight - 10 - playBox.iHeight;
	
	// infoBox
	CBox infoBox;
	infoBox.iWidth = 300;
	infoBox.iHeight = 60;
	infoBox.iX = box.iX + 10 + playBox.iWidth + 10;
	infoBox.iY = box.iY + box.iHeight - 10 - infoBox.iHeight;

	// textBox
	CBox textBox;
	textBox.iWidth = box.iWidth/2 - 20;
	textBox.iHeight = box.iHeight - playBox.iHeight - starBox.iHeight - titleBox.iHeight - 3*10 - 100;
	textBox.iX = box.iX + 10;
	textBox.iY = starBox.iY + 10 + 60;
	
	// artBox
	CBox artBox;
	artBox.iWidth = box.iWidth/2 - 20;
	artBox.iHeight = box.iHeight - 20;
	artBox.iX = box.iX + box.iWidth/2 + 10;
	artBox.iY = box.iY + 10;

	CFrameBox * testFrameBox = new CFrameBox(&box);
	CWidget * widget = new CWidget(&box);

	// artFrame
	CFrame * artFrame = new CFrame();
	artFrame->setMode(FRAME_PICTURE);
	artFrame->setPosition(&artBox);
	artFrame->setIconName(movieFile.tfile.c_str());
	artFrame->setActive(false);

	testFrameBox->addFrame(artFrame);

	// title
	CFrame *titleFrame = new CFrame();
	titleFrame->setMode(FRAME_LABEL);
	int t_w = g_Font[SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE]->getRenderWidth(movieFile.epgTitle);
	if (t_w > box.iWidth)
		t_w = box.iWidth;
	titleFrame->setPosition(titleBox.iX, titleBox.iY, t_w, titleBox.iHeight);
	titleFrame->paintMainFrame(false);
	titleFrame->setTitle(movieFile.epgTitle.c_str());
	titleFrame->setCaptionFont(SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE);
	titleFrame->setActive(false);

	testFrameBox->addFrame(titleFrame);

	// vote
	for (int i = 0; i < 5; i++)
	{
		CFrame *starOffFrame = new CFrame();
		starOffFrame->setMode(FRAME_ICON);
		starOffFrame->setPosition(starBox.iX + i*25, starBox.iY, starBox.iWidth, starBox.iHeight);
		starOffFrame->setIconName(NEUTRINO_ICON_STAR_OFF);
		starOffFrame->paintMainFrame(false);
		starOffFrame->setActive(false);

		testFrameBox->addFrame(starOffFrame);
	}

	int average = movieFile.vote_average/2 + 1;

	for (int i = 0; i < average; i++)
	{
		CFrame *starOnFrame = new CFrame();
		starOnFrame->setMode(FRAME_ICON);
		starOnFrame->setPosition(starBox.iX + i*25, starBox.iY, starBox.iWidth, starBox.iHeight);
		starOnFrame->setIconName(NEUTRINO_ICON_STAR_ON);
		starOnFrame->paintMainFrame(false);
		starOnFrame->setActive(false);

		testFrameBox->addFrame(starOnFrame);
	}
	
	// duration
	std::string l_buffer = "";
	if (movieFile.length != 0)
	{
		l_buffer = to_string(movieFile.length);
		l_buffer += " Min";
	}
	
	if (movieFile.productionDate)
	{
		l_buffer += " " + to_string(movieFile.productionDate);
	}
	
	if (!movieFile.genres.empty())
	{
		l_buffer += " (" + movieFile.genres + ")";
	}
	
	CFrame *lengthFrame = new CFrame();
	lengthFrame->setMode(FRAME_LABEL);
		
	int l_w = g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getRenderWidth(l_buffer);
			
	lengthFrame->setPosition(titleBox.iX, starBox.iY + starBox.iHeight + 10, l_w, g_Font[SNeutrinoSettings::FONT_TYPE_EPG_INFO1]->getHeight());
	lengthFrame->paintMainFrame(false);
		
	lengthFrame->setTitle(l_buffer.c_str());
	lengthFrame->setCaptionFont(SNeutrinoSettings::FONT_TYPE_EPG_INFO1);
	lengthFrame->setActive(false);

	testFrameBox->addFrame(lengthFrame);

	// text
	CFrame *textFrame = new CFrame();
	textFrame->setMode(FRAME_TEXT);
	textFrame->setPosition(&textBox);
	std::string buffer;
	buffer = movieFile.epgInfo1;
	buffer += "\n";
	buffer += movieFile.epgInfo2;

	textFrame->setTitle(buffer.c_str());
	textFrame->paintMainFrame(false);
	textFrame->setActive(false);
	
	testFrameBox->addFrame(textFrame);

	// play
	CFrame *playFrame = new CFrame();
	playFrame->setPosition(&playBox);
	playFrame->setCaptionFont(SNeutrinoSettings::FONT_TYPE_EPG_INFO1);
	playFrame->setTitle("Movie abspielen");
	playFrame->setIconName(NEUTRINO_ICON_PLAY);
	playFrame->setActionKey(this, "playMovie");
	playFrame->enableBorder();

	testFrameBox->addFrame(playFrame);

	// infoFrame
	CFrame * infoFrame = new CFrame();
	infoFrame->setPosition(&infoBox);
	infoFrame->setCaptionFont(SNeutrinoSettings::FONT_TYPE_EPG_INFO1);
	infoFrame->setTitle("Movie Details");
	infoFrame->setIconName(NEUTRINO_ICON_INFO);
	infoFrame->setActionKey(this, "MovieInfo");
	infoFrame->enableBorder();

	testFrameBox->addFrame(infoFrame, true);

	widget->addWidgetItem(testFrameBox);
	widget->exec(NULL, "");

	delete widget;
	widget = NULL;

	delete testFrameBox;
	testFrameBox = NULL;
}

int CMovieInfoWidget::exec(CMenuTarget* parent, const std::string& actionKey)
{
	dprintf(DEBUG_NORMAL, "CMovieInfoWidget::exec: actionKey:%s\n", actionKey.c_str());
	
	if(parent)
		hide();
	
	if(actionKey == "playMovie")
	{
		CMoviePlayerGui tmpMoviePlayerGui;
		tmpMoviePlayerGui.addToPlaylist(movieFile);

		tmpMoviePlayerGui.exec(NULL, "");

		return RETURN_REPAINT;
	}
	else if(actionKey == "MovieInfo")
	{
		m_movieInfo.showMovieInfo(movieFile);

		return RETURN_REPAINT;
	}

	funArt();
	
	return RETURN_EXIT;
}



