#include "global.h"
#include "ScreenInstallOverlay.h"
#include "RageFileManager.h"
#include "ScreenManager.h"
#include "Preference.h"
#include "RageLog.h"
#include "json/value.h"
#include "JsonUtil.h"
#include "SpecialFiles.h"
class Song;
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"
#include "ScreenSelectMusic.h"
#include "CommonMetrics.h"
#include "SongManager.h"
#include "CommandLineActions.h"
#include "ScreenDimensions.h"
#include "StepMania.h"
#include "ActorUtil.h"
#include "Song.h"
#include "NoteData.h"
#include "Steps.h"
#include "MinaCalc.h"
#include "JsonUtil.h"
#include "json/writer.h"
#if !defined(WITHOUT_NETWORKING)
#include "DownloadManager.h"
#endif


const RString TEMP_OS_MOUNT_POINT = "/@temp-os/";

struct FileCopyResult
{
	FileCopyResult(RString _sFile, RString _sComment) : sFile(_sFile), sComment(_sComment) {}
	RString sFile, sComment;
};

void InstallSmzipOsArg(const string &sOsZipFile)
{
	SCREENMAN->SystemMessage("Installing " + sOsZipFile);

	RString sOsDir, sFilename, sExt;
	splitpath(sOsZipFile, sOsDir, sFilename, sExt);

	if (!FILEMAN->Mount("dir", sOsDir, TEMP_OS_MOUNT_POINT))
		FAIL_M("Failed to mount " + sOsDir);
	DLMAN->InstallSmzip(TEMP_OS_MOUNT_POINT + sFilename + sExt);

	FILEMAN->Unmount("dir", sOsDir, TEMP_OS_MOUNT_POINT);

}
static bool IsHTTPProtocol(const RString &arg)
{
	return BeginsWith(arg, "http://") || BeginsWith(arg, "https://");
}

static bool IsPackageFile(const RString &arg)
{
	RString ext = GetExtension(arg);
	return ext.EqualsNoCase("smzip") || ext.EqualsNoCase("zip");
}

void DoInstalls(CommandLineActions::CommandLineArgs args)
{
	bool reload = false;
	for (int i = 0; i<(int)args.argv.size(); i++)
	{
		RString s = args.argv[i];
		if (s == "recalc") {
			Json::Value root;
			Json::Value charts;
			FOREACH_CONST(Song*, SONGMAN->GetAllSongs(), iSong) {
				Song *pSong = (*iSong);
				FOREACH_CONST(Steps*, pSong->GetAllSteps(), iSteps) {
					Steps* steps = (*iSteps);
					TimingData* td = steps->GetTimingData();
					NoteData nd;
					steps->GetNoteData(nd);

					nd.LogNonEmptyRows();
					auto& nerv = nd.GetNonEmptyRowVector();
					auto& etaner = td->BuildAndGetEtaner(nerv);
					auto& serializednd = nd.SerializeNoteData(etaner);
					Json::Value chart;
					for (float percent = 70.0; percent <= 97.0; percent += 1.0) {
						auto dakine = MinaSDCalc(serializednd, steps->GetNoteData().GetNumTracks(), percent, 1.f, td->HasWarps());
						for(int i=0; i< dakine.size(); i++)
							FOREACH_ENUM(Skillset, ss)
								chart[to_string(ss)][to_string(0.7+(0.1*i)).substr(0, 3)][to_string(percent).substr(0, 5)] = static_cast<int>(dakine[i][ss]*100);
					}

					charts[steps->GetChartKey()] = chart;
					td->UnsetEtaner();
					nd.UnsetNerv();
					nd.UnsetSerializedNoteData();
					steps->Compress();
				}
			}
			root["charts"] = charts;
			RageFile f;
			std::string output;
			Json::FastWriter writer;
			string out = writer.write(root);
			f.Open("ssrcache.json", 0x2);//WRITE
			f.Write(out);
			f.Close();
			sm_crash();
		}
		if (IsHTTPProtocol(s))
		{

#if !defined(WITHOUT_NETWORKING)
			DLMAN->DownloadAndInstallPack(s);
#else
			// TODO: Figure out a meaningful log message.
#endif
		}
		else if (IsPackageFile(s)) {
			InstallSmzipOsArg(s);
			reload = true;
		}
	}
	if (reload) {
		auto screen = SCREENMAN->GetScreen(0);
		if (screen && screen->GetName() == "ScreenSelectMusic") {
			static_cast<ScreenSelectMusic*>(screen)->DifferentialReload();
		}
		else
		{
			SONGMAN->DifferentialReload();
		}
	}
	return;
}

REGISTER_SCREEN_CLASS(ScreenInstallOverlay);

ScreenInstallOverlay::~ScreenInstallOverlay()
= default;
void ScreenInstallOverlay::Init()
{
	Screen::Init();

	m_textStatus.LoadFromFont(THEME->GetPathF("ScreenInstallOverlay", "status"));
	m_textStatus.SetName("Status");
	LOAD_ALL_COMMANDS_AND_SET_XY_AND_ON_COMMAND(m_textStatus);


	ActorUtil::LoadAllCommandsAndSetXY(m_textStatus, "ScreenInstallOverlay");
	this->AddChild(&m_textStatus);
}

bool ScreenInstallOverlay::Input(const InputEventPlus &input)
{
	/*
	if( input.DeviceI.button == g_buttonLogin && input.type == IET_FIRST_PRESS )
	{
	HOOKS->GoToURL("http://www.stepmania.com/launch.php");
	return true;
	}
	*/

	return Screen::Input(input);
}

void ScreenInstallOverlay::Update(float fDeltaTime)
{
	Screen::Update(fDeltaTime);
	while (CommandLineActions::ToProcess.size() > 0)
	{
		CommandLineActions::CommandLineArgs args = CommandLineActions::ToProcess.back();
		CommandLineActions::ToProcess.pop_back();
		DoInstalls(args);
	}
#if !defined(WITHOUT_NETWORKING)
	DLMAN->UpdateAndIsFinished(fDeltaTime);
	vector<Download*>& dls = DLMAN->downloads;
	vector<RString> vsMessages;
	for(auto &dl : dls)
	{
		vsMessages.push_back(dl->Status());
	}
	m_textStatus.SetText(join("\n", vsMessages));

#endif
}

/*
* (c) 2001-2005 Chris Danford, Glenn Maynard
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
