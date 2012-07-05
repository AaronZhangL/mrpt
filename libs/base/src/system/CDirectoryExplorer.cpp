/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2012, MAPIR group, University of Malaga                |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */

#include <mrpt/base.h>  // Precompiled headers



#include <mrpt/config.h>

#ifdef MRPT_OS_WINDOWS
        #ifdef _MSC_VER
        	#include <sys/utime.h>
        #endif
	#include <io.h>
	#include <windows.h>
	#include <direct.h>
#else
	#include <sys/types.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <time.h>
	#include <utime.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include <queue>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>


#include <mrpt/system/CDirectoryExplorer.h>
#include <mrpt/system/filesystem.h>

using namespace mrpt::system;
using namespace std;

/*---------------------------------------------------------------
		explore
 ---------------------------------------------------------------*/
void CDirectoryExplorer::explore(
	const string 		&path,
	const unsigned long 	in_mask,
	TFileInfoList  		&outList )
{
	MRPT_START

	unsigned long mask = in_mask;

	outList.clear();

	// The path terminated in "/" or "\\"
	string 			searchPath( path );
	if (searchPath.size())
		if ( searchPath[searchPath.size()-1]!='\\' && searchPath[searchPath.size()-1]!='/')
		{
#ifdef MRPT_OS_WINDOWS
			searchPath += '\\';
#else
			searchPath.push_back('/');
#endif
		}

	//cout << "searchPath:"<<searchPath<<endl;

#ifdef MRPT_OS_WINDOWS
	// ====================
	// WINDOWS VERSION
	// ====================
	WIN32_FIND_DATAA		f;
	TFileInfo		newEntry;

	string searchPath_mask=searchPath + string("*.*");

	HANDLE h = FindFirstFileA( searchPath_mask.c_str(), &f);
	if (h==INVALID_HANDLE_VALUE)
		THROW_EXCEPTION("Error starting exploration! (does path exist?)");

	// Include the FILE_ATTRIB_ARCHIVE flag for files:
	if (mask & FILE_ATTRIB_ARCHIVE) mask |= FILE_ATTRIBUTE_NORMAL;
	do
	{
		if ( (mask & f.dwFileAttributes)!=0  )	// Passes the user masks:
		{
			// File name:
			newEntry.name = string(f.cFileName);

			// Complete file path:
			newEntry.wholePath = searchPath;
			newEntry.wholePath += newEntry.name;

			// File size:
			newEntry.fileSize = ((uint64_t)f.nFileSizeLow) + (((uint64_t)f.nFileSizeHigh) << 32);

			// File times:
			struct stat statDat;
			if (stat( newEntry.wholePath.c_str(),&statDat ))
			{
				FindClose(h);
				THROW_EXCEPTION_CUSTOM_MSG1("Cannot get stat for file: '%s'",newEntry.wholePath.c_str())
			}

			newEntry.modTime    = statDat.st_mtime;
			newEntry.accessTime = statDat.st_atime;

			// Flags:
			newEntry.isDir = 0!=(statDat.st_mode &_S_IFDIR);
			newEntry.isSymLink = false; // (We donnot look for this in Windows, by now...)


			// Save:
			outList.push_back( newEntry );
		}
	} while(FindNextFileA(h, &f));

	FindClose(h); // Ignore possible errors..

	// Done
#else
	// ====================
	// LINUX VERSION
	// ====================
	TFileInfo		newEntry;
	struct dirent 		*ent;

	DIR *dir = opendir( searchPath.c_str() );
	if (!dir)
		THROW_EXCEPTION("Error starting exploration! (does path exist?)");


	while((ent = readdir(dir)) != NULL)
	{
		if ( strcmp(ent->d_name,".") && strcmp(ent->d_name,"..") )
		{
			// File name:
			newEntry.name = string(ent->d_name);

			// Complete file path:
			newEntry.wholePath = searchPath;
			newEntry.wholePath += newEntry.name;

			// File times:
			struct stat statDat, lstatDat;
			if (stat( newEntry.wholePath.c_str(),&statDat ))
			{
				closedir(dir);
				THROW_EXCEPTION_CUSTOM_MSG1("Cannot get stat for file: '%s'",newEntry.wholePath.c_str())
			}

			newEntry.modTime    = statDat.st_mtime;
			newEntry.accessTime = statDat.st_atime;

			// Flags:
			newEntry.isDir = S_ISDIR(statDat.st_mode);

			if ( ( (mask & FILE_ATTRIB_ARCHIVE)!=0 && !newEntry.isDir ) ||
			     ( (mask & FILE_ATTRIB_DIRECTORY)!=0 && newEntry.isDir ) )
			{
				// File size:
				newEntry.fileSize = (intmax_t)statDat.st_size;

				// Is it a symbolic link?? Need to call "lstat":
				if (!lstat( newEntry.wholePath.c_str(),&lstatDat ))
				{
					newEntry.isSymLink = S_ISLNK(lstatDat.st_mode);
				}
				else	newEntry.isSymLink = false;

				// Save:
				outList.push_back( newEntry );
			}
		}
	}

	closedir(dir);

	// Done
#endif

	MRPT_END
}


// Auxiliary function to order by name, ascending
bool cmpFileEntriesName_Asc(const CDirectoryExplorer::TFileInfo &a, const CDirectoryExplorer::TFileInfo &b)
{
    return a.wholePath < b.wholePath;
}
bool cmpFileEntriesName_Desc(const CDirectoryExplorer::TFileInfo &a, const CDirectoryExplorer::TFileInfo &b)
{
    return a.wholePath < b.wholePath;
}

/*---------------------------------------------------------------
		sortByName
 ---------------------------------------------------------------*/
void CDirectoryExplorer::sortByName( TFileInfoList &lstFiles, bool ascendingOrder )
{
    std::sort(lstFiles.begin(),lstFiles.end(), ascendingOrder ? cmpFileEntriesName_Asc : cmpFileEntriesName_Desc );
}


/*---------------------------------------------------------------
		filterByExtension
 ---------------------------------------------------------------*/
void CDirectoryExplorer::filterByExtension( TFileInfoList &lstFiles, const std::string &extension  )
{
	int	i,n=(int)lstFiles.size();
	for (i=n-1;i>=0;i--)
	{
		if ( 0!=os::_strcmpi(mrpt::system::extractFileExtension(lstFiles[i].name).c_str(),extension.c_str() ) )
		{
			// Does not match:
			lstFiles.erase( lstFiles.begin()+i );
		}
	}
}

