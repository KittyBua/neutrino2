# neutrinoNG python sample plugin
# mohousch 25112024

# License: GPL

# This program is free software; you can redistribute it and/or modify	it under the terms of the GNU General Public License as published # by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

from _neutrino2 import *

class messageBox(CMessageBox):
	title = "python: CMessageBox"
	msg = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA."
	def __init__(self):
		CMessageBox.__init__(self, self.title, self.msg)
		self._exec(-1)

class helpBox(CHelpBox):
	line1 = "python: CHelpBox"
	line2 = "Huhu"
	line3 = "alles gut"
	def __init__(self):
		CHelpBox.__init__(self)
		self.addLine(self.line1)
		self.addLine(self.line2)
		self.addLine(self.line3)
		self.show("python: CHelpBox")

class hintBox(CHintBox):
	title = "python: CHintBox:"
	msg = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA."
	def __init__(self):
		CHintBox.__init__(self, self.title, self.msg)
		self._exec()

class infoBox(CInfoBox):
	msgTitle = "pythonTest: CInfoBox"
	msgText = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA."
	def __init__(self):
		CInfoBox.__init__(self)
		self.setTitle(self.msgTitle)
		self.setText(self.msgTitle + "\n" + self.msgText)
		self._exec(-1)

class stringInput(CStringInputSMS):
	title = "pythonTest: CStringInputSMS"
	value = ''
	def __init__(self):
		CStringInputSMS.__init__(self, self.title, self.value)
		self._exec(None, "")

class audioPlayer(CFileBrowser):
	config = CConfigFile('\t')

	config.loadConfig(CONFIGDIR + "/neutrino2.conf")

	PATH = config.getString("network_nfs_audioplayerdir")

	def __init__(self):
		CFileBrowser.__init__(self)
		fileFilter = CFileFilter()

		fileFilter.addFilter("cdr")
		fileFilter.addFilter("mp3")
		fileFilter.addFilter("m2a")
		fileFilter.addFilter("mpa")
		fileFilter.addFilter("mp2")
		fileFilter.addFilter("ogg")
		fileFilter.addFilter("wav")
		fileFilter.addFilter("flac")
		fileFilter.addFilter("aac")
		fileFilter.addFilter("dts")
		fileFilter.addFilter("m4a")

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter

		self._exec(self.PATH)

		self.PATH = self.getCurrentDir()

		player = CAudioPlayerGui()

		if self.getSelectedFile() is not None:
			player.addToPlaylist(self.getSelectedFile())
			player._exec(None, "")

		if self.getExitPressed() is not True:
			self.__init__()

class pictureViewer(CFileBrowser):
	config = CConfigFile('\t')

	config.loadConfig(CONFIGDIR + "/neutrino2.conf")

	PATH = config.getString("network_nfs_picturedir")

	def __init__(self):
		CFileBrowser.__init__(self)
		fileFilter = CFileFilter()

		fileFilter.addFilter("jpeg")
		fileFilter.addFilter("jpg")
		fileFilter.addFilter("png")
		fileFilter.addFilter("bmp")

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter
		
		self._exec(self.PATH)

		self.PATH = self.getCurrentDir()

		player = CPictureViewerGui()
	
		if self.getSelectedFile() is not None:
			player.addToPlaylist(self.getSelectedFile())
			player._exec(None, "")
			
		if self.getExitPressed() is not True:
			self.__init__()

class moviePlayer(CFileBrowser):
	config = CConfigFile('\t')

	config.loadConfig(CONFIGDIR + "/neutrino2.conf")

	PATH = config.getString("network_nfs_recordingdir")

	def __init__(self):
		CFileBrowser.__init__(self)
		fileFilter = CFileFilter()

		fileFilter.addFilter("ts")
		fileFilter.addFilter("mpg");
		fileFilter.addFilter("mpeg");
		fileFilter.addFilter("divx");
		fileFilter.addFilter("avi");
		fileFilter.addFilter("mkv");
		fileFilter.addFilter("asf");
		fileFilter.addFilter("aiff");
		fileFilter.addFilter("m2p");
		fileFilter.addFilter("mpv");
		fileFilter.addFilter("m2ts");
		fileFilter.addFilter("vob");
		fileFilter.addFilter("mp4");
		fileFilter.addFilter("mov");	
		fileFilter.addFilter("flv");	
		fileFilter.addFilter("dat");
		fileFilter.addFilter("trp");
		fileFilter.addFilter("vdr");
		fileFilter.addFilter("mts");
		fileFilter.addFilter("wmv");
		fileFilter.addFilter("wav");
		fileFilter.addFilter("flac");
		fileFilter.addFilter("mp3");
		fileFilter.addFilter("wma");
		fileFilter.addFilter("ogg");

		self.Multi_Select = False
		self.Dirs_Selectable = False
		self.Filter = fileFilter

		self._exec(self.PATH)

		self.PATH = self.getCurrentDir()

		player = CMoviePlayerGui()
	
		if self.getSelectedFile() is not None:
			player.addToPlaylist(self.getSelectedFile())
			player._exec(None, "")
			
		if self.getExitPressed() is not True:
			self.__init__()

if __name__ == "__main__":
	moviePlayer()


















