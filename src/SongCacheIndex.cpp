#include "global.h"

#include "SongCacheIndex.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "RageFileManager.h"
#include "Song.h"
#include "SpecialFiles.h"
#include "Steps.h"
#include "NotesLoaderSSC.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

/*
 * A quick explanation of song cache hashes: Each song has two hashes; a hash of the
 * song path, and a hash of the song directory.  The former is Song::GetCacheFilePath;
 * it stays the same if the contents of the directory change.  The latter is 
 * GetHashForDirectory(m_sSongDir), and changes on each modification.
 *
 * The file hash is used as the cache filename.  We don't want to use the directory
 * hash: if we do that, then we'll write a new cache file every time the song changes,
 * and they'll accumulate or we'll have to be careful to delete them.
 *
 * The directory hash is stored in here, indexed by the song path, and used to determine
 * if a song has changed.
 *
 * Another advantage of this system is that we can load songs from cache given only their
 * path; we don't have to actually look in the directory (to find out the directory hash)
 * in order to find the cache file.
 */
const string CACHE_INDEX = SpecialFiles::CACHE_DIR + "index.cache";


SongCacheIndex *SONGINDEX; // global and accessible from anywhere in our program

RString SongCacheIndex::GetCacheFilePath( const RString &sGroup, const RString &sPath )
{
	/* Don't use GetHashForFile, since we don't want to spend time
	 * checking the file size and date. */
	RString s;
	
	if( sPath.size() > 2 && sPath[0] == '/' && sPath[sPath.size()-1] == '/' )
		s.assign( sPath, 1, sPath.size() - 2 );
	else if( sPath.size() > 1 && sPath[0] == '/' )
		s.assign( sPath, 1, sPath.size() - 1 );
	else
		s = sPath;
	/* Change slashes and invalid utf-8 characters to _.
	 * http://en.wikipedia.org/wiki/UTF-8
	 * Mac OS X doesn't support precomposed unicode characters in files names and
	 * so we should probably replace them with combining diacritics.
	 * XXX How do we do this and is it even worth it? */
	const char *invalid = "/\xc0\xc1\xfe\xff\xf8\xf9\xfa\xfb\xfc\xfd\xf5\xf6\xf7";
	for( size_t pos = s.find_first_of(invalid); pos != RString::npos; pos = s.find_first_of(invalid, pos) )
		s[pos] = '_';
	// CACHE_DIR ends with a /.
	return ssprintf( "%s%s/%s", SpecialFiles::CACHE_DIR.c_str(), sGroup.c_str(), s.c_str() );
}

SongCacheIndex::SongCacheIndex()
{
	ReadCacheIndex();
	dbempty = !OpenDB();
}

/*	Load a song from Cache DB
	Returns true if it was loaded**/
bool SongCacheIndex::LoadSongFromCache(Song* song, string dir)
{
	/*
	LOG->Trace("Loading '%s' from cache file '%s'.",
	m_sSongDir.c_str(),
	GetCacheFilePath().c_str());
	*/
	/*db->exec("CREATE TABLE IF NOT EXISTS songs(VERSION, TITLE, "
		"SUBTITLE, ARTIST, BANNER, BACKGROUND, CDTITLE, MUSIC, OFFSET, SAMPLESTART, "
		"SAMPLELENGTH, SELECTABLE, BPMS, TIMESIGNATURES, TICKCOUNTS, "
		"COMBOS, SPEEDS, SCROLLS, LABELS, FIRSTSECOND, LASTSECOND, "
		"SONGFILENAME, HASMUSIC, HASBANNER, MUSICLENGTH, DIR)");*/
	/*db->exec("CREATE TABLE IF NOT EXISTS charts (NOTEDATA, "
		"STEPSTYPE, DIFFICULTY, METER, MSDVALUES, CHARTKEY, "
		"RADARVALUES, STEPFILENAME)");*/
	db->exec("CREATE TABLE IF NOT EXISTS songs (VERSION, "
		"TITLE, SUBTITLE, ARTIST, TITLETRANSLIT, SUBTITLETRANSLIT, "
		"ARTISTTRANSLIT, GENRE, ORIGIN, CREDIT, BANNER, BACKGROUND, "
		"PREVIEWVID, JACKET, CDIMAGE, DISCIMAGE, LYRICSPATH, CDTITLE, "
		"MUSIC, PREVIEW, INSTRUMENTTRACK, MUSICLENGTH, LASTSECONDHINT, "
		"SAMPLESTART, SAMPLELENGTH, "
		"DISPLAYBPM, SELECTABLE, ANIMATIONS, "
		"FGCHANGES, KEYSOUNDS, OFFSET, "
		"STOPS, DELAYS, BPMS, WARPS, LABELS, "
		"TIMESIGNATURES, TICKCOUNTS, COMBOS, SPEEDS, SCROLLS, FAKES, "
		"FIRSTSECOND, LASTSECOND, SONGFILENAME, HASMUSIC, HASBANNER)");//SSC

	db->exec("CREATE TABLE IF NOT EXISTS charts (VERSION, "
		"CHARTNAME, STEPSTYPE, CHARTSTYLE, DESCRIPTION, DIFFICULTY, "
		"METER, RADARVALUES, CREDIT, MUSIC, BPMS, STOPS, DELAYS, "
		"TIMESIGNATURES, TICKCOUNTS, COMBOS, WARPS, SPEEDS, SCROLLS, "
		"FAKES, LABELS, OFFSET, DISPLAYBPM, CHARTKEY, MSDVALUES");

	
	SQLite::Statement query(*db, "SELECT * FROM songs WHERE DIR="+dir);
	if (!query.tryExecuteStep())
		return false;

	//SSC::StepsTagInfo reused_steps_info(&*song, &out, dir, true);
	SSCLoader loader;
	Steps* pNewNotes = nullptr;
	TimingData stepsTiming;
	pNewNotes->m_Timing = stepsTiming;
	int songid = query.getColumn(0);
	int index = 1;
	song->m_fVersion = static_cast<double>(query.getColumn(index++));
	song->m_sMainTitle = static_cast<const char *>(query.getColumn(index++));
	song->m_sSubTitle = static_cast<const char *>(query.getColumn(index++));
	song->m_sArtist = static_cast<const char *>(query.getColumn(index++));
	song->m_sMainTitleTranslit = static_cast<const char *>(query.getColumn(index++));
	song->m_sSubTitleTranslit = static_cast<const char *>(query.getColumn(index++));
	song->m_sArtistTranslit = static_cast<const char *>(query.getColumn(index++));
	song->m_sGenre = static_cast<const char *>(query.getColumn(index++));
	song->m_sOrigin = static_cast<const char *>(query.getColumn(index++));
	song->m_sCredit = static_cast<const char *>(query.getColumn(index++));
	Trim(song->m_sCredit);
	song->m_sBannerFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sBackgroundFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sPreviewVidFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sJacketFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sCDFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sDiscFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sLyricsFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sCDTitleFile = static_cast<const char *>(query.getColumn(index++));
	song->m_sMusicFile = static_cast<const char *>(query.getColumn(index++));
	song->m_PreviewFile = static_cast<const char *>(query.getColumn(index++));
	loader.ProcessInstrumentTracks(*song, static_cast<const char *>(query.getColumn(index++)));
	song->m_fMusicLengthSeconds = static_cast<double>(query.getColumn(index++));
	song->SetSpecifiedLastSecond(static_cast<double>(query.getColumn(index++)));
	song->m_fMusicSampleStartSeconds = static_cast<double>(query.getColumn(index++));
	song->m_fMusicSampleLengthSeconds = static_cast<double>(query.getColumn(index++));
	song->m_SongTiming.m_fBeat0OffsetInSeconds = static_cast<double>(query.getColumn(index++));
	string BPMmin = static_cast<const char *>(query.getColumn(index++));
	string BPMmax = static_cast<const char *>(query.getColumn(index++));
	if (BPMmin == "*")
	{
		song->m_DisplayBPMType = DISPLAY_BPM_RANDOM;
	}
	else
	{
		song->m_DisplayBPMType = DISPLAY_BPM_SPECIFIED;
		song->m_fSpecifiedBPMMin = StringToFloat(BPMmin);
		if (BPMmax.empty())
		{
			song->m_fSpecifiedBPMMax = song->m_fSpecifiedBPMMin;
		}
		else
		{
			song->m_fSpecifiedBPMMax = StringToFloat(BPMmax);
		}
	}
	RString selection = static_cast<const char *>(query.getColumn(index++));
	if (selection.EqualsNoCase("YES"))
	{
		song->m_SelectionDisplay = song->SHOW_ALWAYS;
	}
	else if (selection.EqualsNoCase("NO"))
	{
		song->m_SelectionDisplay = song->SHOW_NEVER;
	}
	// ROULETTE from 3.9 is no longer in use.
	else if (selection.EqualsNoCase("ROULETTE"))
	{
		song->m_SelectionDisplay = song->SHOW_ALWAYS;
	}
	/* The following two cases are just fixes to make sure simfiles that
	* used 3.9+ features are not excluded here */
	else if (selection.EqualsNoCase("ES") || selection.EqualsNoCase("OMES"))
	{
		song->m_SelectionDisplay = song->SHOW_ALWAYS;
	}
	else if (StringToInt(selection) > 0)
	{
		song->m_SelectionDisplay = song->SHOW_ALWAYS;
	}
	else
	{
		LOG->UserLog("Song file", dir, "has an unknown #SELECTABLE value, \"%s\"; ignored.", selection.c_str());
	}
	string animations = static_cast<const char *>(query.getColumn(index++));
	string animationstwo = static_cast<const char *>(query.getColumn(index++));
	loader.ProcessBGChanges(*song, animations,
		dir, animationstwo);
	vector<RString> aFGChangeExpressions;
	split(static_cast<const char *>(query.getColumn(index++)), ",", aFGChangeExpressions);

	for (size_t b = 0; b < aFGChangeExpressions.size(); ++b)
	{
		BackgroundChange change;
		if (loader.LoadFromBGChangesString(change, aFGChangeExpressions[b]))
		{
			song->AddForegroundChange(change);
		}
	}
	RString keysounds = static_cast<const char *>(query.getColumn(index++));
	if (keysounds.length() >= 2 && keysounds.substr(0, 2) == "\\#")
	{
		keysounds = keysounds.substr(1);
	}
	split(keysounds, ",", song->m_vsKeysoundFile);
	loader.ProcessStops(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessDelays(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessBPMs(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessWarps(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)), song->m_fVersion);
	loader.ProcessLabels(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessTimeSignatures(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessTickcounts(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessCombos(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessSpeeds(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessScrolls(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	loader.ProcessFakes(song->m_SongTiming, static_cast<const char *>(query.getColumn(index++)));
	song->SetFirstSecond(static_cast<double>(query.getColumn(index++)));
	song->SetLastSecond(static_cast<double>(query.getColumn(index++)));
	song->m_sSongFileName = static_cast<const char *>(query.getColumn(index++));
	song->m_bHasMusic = static_cast<int>(query.getColumn(index++)) != 0;
	song->m_bHasBanner = static_cast<int>(query.getColumn(index++)) != 0;

	SQLite::Statement qSteps(*db, "SELECT * FROM steps WHERE SONGID=" + to_string(songid));
	while (query.tryExecuteStep()) {

	}

	//Up to here its in order
	//wip

	// Functions for steps tags go below this line. -Kyz
	/****************************************************************/

		song->m_fVersion = StringToFloat(static_cast<const char *>(query.getColumn(index++)));{
		RString name = static_cast<const char *>(query.getColumn(index++));
		Trim(name);
		info.steps->SetChartName(name);
		info.steps->m_StepsType = GAMEMAN->StringToStepsType(static_cast<const char *>(query.getColumn(index++)));
		info.steps->m_StepsTypeStr = static_cast<const char *>(query.getColumn(index++));
		info.steps->SetChartStyle(static_cast<const char *>(query.getColumn(index++)));
		RString name = static_cast<const char *>(query.getColumn(index++));
		Trim(name);
			info.steps->SetChartName(name);
		info.steps->SetDifficulty(StringToDifficulty(static_cast<const char *>(query.getColumn(index++)));
		info.steps->SetMeter(StringToInt(static_cast<const char *>(query.getColumn(index++)));
		vector<RString> values;
		split((*info.params)[1], ",", values, true);
		RadarValues rv;
		rv.Zero();
		for (size_t i = 0; i < NUM_RadarCategory; ++i)
			rv[i] = StringToInt(values[i]);
		//info.steps->SetCachedRadarValues(rv);
	//info.steps->SetCredit((*info.params)[1]);
	//info.steps->SetMusicFile((*info.params)[1]);
		loader.ProcessBPMs(*info.timing, (*info.params)[1]);
		loader.ProcessStops(*info.timing, (*info.params)[1]);
		loader.ProcessDelays(*info.timing, (*info.params)[1]);
		loader.ProcessTimeSignatures(*info.timing, (*info.params)[1]);
		loader.ProcessTickcounts(*info.timing, (*info.params)[1]);
		loader.ProcessCombos(*info.timing, (*info.params)[1]);
		loader.ProcessWarps(*info.timing, (*info.params)[1], song->m_fVersion);
		loader.ProcessSpeeds(*info.timing, (*info.params)[1]);
		loader.ProcessScrolls(*info.timing, (*info.params)[1]);
		loader.ProcessFakes(*info.timing, (*info.params)[1]);
		loader.ProcessLabels(*info.timing, (*info.params)[1]);
		info.timing->m_fBeat0OffsetInSeconds = StringToFloat((*info.params)[1]);
	void SetStepsDisplayBPM(SSC::StepsTagInfo& info)
	{
		// #DISPLAYBPM:[xxx][xxx:xxx]|[*];
		if ((*info.params)[1] == "*")
		{
			info.steps->SetDisplayBPM(DISPLAY_BPM_RANDOM);
		}
		else
		{
			info.steps->SetDisplayBPM(DISPLAY_BPM_SPECIFIED);
			float min = StringToFloat((*info.params)[1]);
			info.steps->SetMinBPM(min);
			if ((*info.params)[2].empty())
			{
				info.steps->SetMaxBPM(min);
			}
			else
			{
				info.steps->SetMaxBPM(StringToFloat((*info.params)[2]));
			}
		}
	}

		//info.steps->SetChartKey((*info.params)[1]);

	vector<float> msdsplit(const RString& s) {
		vector<float> o;
		for (size_t i = 0; i < s.size(); ++i) {
			o.emplace_back(StringToFloat(s.substr(i, 5)));
			i += 5;
		}
		return o;
	}

	void SetMSDValues(SSC::StepsTagInfo& info) {
		MinaSD o;

		// Optimize by calling those only once instead of multiple times inside the loop.
		auto params = (*info.params);
		auto size = params.params.size();
		// Start from index 1
		for (size_t i = 1; i <= size; i++)
			o.emplace_back(msdsplit(params[i]));
		info.steps->SetAllMSD(o);
	}
	//end wip
	song->m_SongTiming.m_sFile = dir; // songs still have their fallback timing.
	song->m_sSongFileName = dir;


	song->m_fVersion = STEPFILE_VERSION_NUMBER;
	SMLoader::TidyUpData(*song, true);


	if (song->m_sMainTitle == "" || (song->m_sMusicFile == "" && song->m_vsKeysoundFile.empty()))
	{
		LOG->Warn("Main title or music file for '%s' came up blank, forced to fall back on TidyUpData to fix title and paths.  Do not use # or ; in a song title.", dir.c_str());
		// Tell TidyUpData that it's not loaded from the cache because it needs
		// to hit the song folder to find the files that weren't found. -Kyz
		song->TidyUpData(false, false);
	}
}	
/*	Save a song to the cache db*/
bool SaveSong(Song* song, string dir)
{
	return true;
}
/*	Reset the DB/
	Must be open already	*/
void SongCacheIndex::ResetDB()
{
	if (db == nullptr)
		return;
	SQLite::Statement   qTables(*db, "SELECT name FROM sqlite_master WHERE type='table'");
	qTables.exec();
	while (qTables.executeStep())
	{
		string table = static_cast<const char*>(qTables.getColumn(0));
		db->exec("DROP TABLE IF EXISTS  " + table);
	}
	LOG->Trace("Cache database is out of date.  Deleting all cache files.");
	db->exec("VACUUM"); //Shrink to fit
}

/*	Returns weather or not the db has valid data*/
bool SongCacheIndex::OpenDB()
{
	//Try to open ane existing db
	try {
		db = new SQLite::Database(FILEMAN->ResolvePath(CACHE_INDEX), SQLite::OPEN_READWRITE);
		SQLite::Statement   qDBInfo(*db, "SELECT * FROM dbinfo");

		//Should only have one row so no executeStep loop
		if (!qDBInfo.executeStep()) {
			ResetDB();
			return false;
		}
		int iCacheVersion = -1;
		iCacheVersion = qDBInfo.getColumn(0);
		if (iCacheVersion == FILE_CACHE_VERSION)
			return true; // OK
		ResetDB();
	}
	catch (std::exception& e)
	{
		try {
			// Open a database file
			db = new SQLite::Database(FILEMAN->ResolvePath(CACHE_INDEX), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		}
		catch (std::exception& e)
		{
			db = nullptr;
		}
	}
	return false;
}

SongCacheIndex::~SongCacheIndex()
= default;

void SongCacheIndex::ReadFromDisk()
{
	ReadCacheIndex();
}

static void EmptyDir( RString dir )
{
	ASSERT(dir[dir.size()-1] == '/');

	vector<RString> asCacheFileNames;
	GetDirListing( dir, asCacheFileNames );
	for( unsigned i=0; i<asCacheFileNames.size(); i++ )
	{
		if( !IsADirectory(dir + asCacheFileNames[i]) )
			FILEMAN->Remove( dir + asCacheFileNames[i] );
	}
}

void SongCacheIndex::ReadCacheIndex()
{
	CacheIndex.ReadFile( CACHE_INDEX );	// don't care if this fails

	int iCacheVersion = -1;
	CacheIndex.GetValue( "Cache", "CacheVersion", iCacheVersion );
	if( iCacheVersion == FILE_CACHE_VERSION )
		return; // OK

	LOG->Trace( "Cache format is out of date.  Deleting all cache files." );
	EmptyDir( SpecialFiles::CACHE_DIR );
	EmptyDir( SpecialFiles::CACHE_DIR+"Banners/" );
	//EmptyDir( SpecialFiles::CACHE_DIR+"Backgrounds/" );
	EmptyDir( SpecialFiles::CACHE_DIR+"Songs/" );
	EmptyDir( SpecialFiles::CACHE_DIR+"Courses/" );

	CacheIndex.Clear();
	/* This is right now in place because our song file paths are apparently being
	 * cached in two distinct areas, and songs were loading from paths in FILEMAN.
	 * This is admittedly a hack for now, but this does bring up a good question on
	 * whether we really need a dedicated cache for future versions of StepMania.
	 */
	FILEMAN->FlushDirCache();
}

void SongCacheIndex::SaveCacheIndex()
{
	CacheIndex.WriteFile(CACHE_INDEX);
}

void SongCacheIndex::AddCacheIndex(const RString &path, unsigned hash)
{
	if( hash == 0 )
		++hash; /* no 0 hash values */
	CacheIndex.SetValue( "Cache", "CacheVersion", FILE_CACHE_VERSION );
	CacheIndex.SetValue( "Cache", MangleName(path), hash );
	if(!delay_save_cache)
	{
		CacheIndex.WriteFile(CACHE_INDEX);
	}
}

unsigned SongCacheIndex::GetCacheHash( const RString &path ) const
{
	unsigned iDirHash = 0;
	if( !CacheIndex.GetValue( "Cache", MangleName(path), iDirHash ) )
		return 0;
	if( iDirHash == 0 )
		++iDirHash; /* no 0 hash values */
	return iDirHash;
}

RString SongCacheIndex::MangleName( const RString &Name )
{
	/* We store paths in an INI.  We can't store '='. */
	RString ret = Name;
	ret.Replace( "=", "");
	return ret;
}

/*
 * (c) 2002-2003 Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
