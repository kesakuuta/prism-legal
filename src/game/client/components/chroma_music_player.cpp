#include "chroma_music_player.h"

#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/sound.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <engine/font_icons.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/localization.h>

#include <algorithm>
#include <random>

CChromaMusicPlayer::CChromaMusicPlayer() :
	m_CurrentTrackIndex(0),
	m_IsPlaying(false),
	m_Shuffle(false),
	m_Repeat(true),
	m_TrackListOpen(false),
	m_DraggingScrubber(false)
{
}

CChromaMusicPlayer::~CChromaMusicPlayer()
{
	StopCurrentTrack();
}

void CChromaMusicPlayer::OnInit()
{
	ScanTracks();
	if(!m_vTracks.empty())
	{
		m_CurrentTrackIndex = 0;
	}
}

void CChromaMusicPlayer::OnReset()
{
	m_DraggingScrubber = false;
	m_TrackListOpen = false;
}

void CChromaMusicPlayer::ScanTracks()
{
	m_vTracks.clear();

	// 1. Primary track: Leanin' (slowed) by cormill
	STrack MainTrack;
	str_copy(MainTrack.m_aTitle, "Leanin' (Slowed)");
	str_copy(MainTrack.m_aArtist, "cormill");
	str_copy(MainTrack.m_aPath, "audio/music_menu.wv");
	MainTrack.m_SampleId = Sound()->LoadWV(MainTrack.m_aPath, IStorage::TYPE_ALL);
	m_vTracks.push_back(MainTrack);

	// 2. Scan audio/music/ directory for user tracks
	struct SScanContext
	{
		CChromaMusicPlayer *pPlayer;
		IStorage *pStorage;
	} Context = {this, Storage()};

	Storage()->ListDirectory(IStorage::TYPE_ALL, "audio/music", [](const char *pName, int IsDir, int DirType, void *pUser) -> int {
		if(IsDir)
			return 0;

		auto *pCtx = (SScanContext *)pUser;
		const char *pExt = str_endswith(pName, ".wv");
		bool IsOpus = false;
		if(!pExt)
		{
			pExt = str_endswith(pName, ".opus");
			IsOpus = true;
		}

		if(pExt)
		{
			STrack Track;
			char aStem[64];
			size_t NameLen = pExt - pName;
			if(NameLen >= sizeof(aStem))
				NameLen = sizeof(aStem) - 1;
			mem_copy(aStem, pName, NameLen);
			aStem[NameLen] = '\0';

			const char *pDash = str_find(aStem, " - ");
			if(pDash)
			{
				size_t ArtistLen = pDash - aStem;
				if(ArtistLen >= sizeof(Track.m_aArtist))
					ArtistLen = sizeof(Track.m_aArtist) - 1;
				mem_copy(Track.m_aArtist, aStem, ArtistLen);
				Track.m_aArtist[ArtistLen] = '\0';
				str_copy(Track.m_aTitle, pDash + 3);
			}
			else
			{
				str_copy(Track.m_aTitle, aStem);
				str_copy(Track.m_aArtist, "Unknown Artist");
			}

			str_format(Track.m_aPath, sizeof(Track.m_aPath), "audio/music/%s", pName);
			Track.m_SampleId = IsOpus ? pCtx->pPlayer->Sound()->LoadOpus(Track.m_aPath, DirType) :
			                            pCtx->pPlayer->Sound()->LoadWV(Track.m_aPath, DirType);
			pCtx->pPlayer->m_vTracks.push_back(Track);
		}
		return 0;
	}, &Context);
}

const CChromaMusicPlayer::STrack *CChromaMusicPlayer::CurrentTrack() const
{
	if(m_CurrentTrackIndex >= 0 && m_CurrentTrackIndex < (int)m_vTracks.size())
		return &m_vTracks[m_CurrentTrackIndex];
	return nullptr;
}

void CChromaMusicPlayer::StartCurrentTrack()
{
	const STrack *pTrack = CurrentTrack();
	if(!pTrack || pTrack->m_SampleId < 0)
		return;

	int Flags = ISound::FLAG_NO_PANNING;
	if(m_Repeat)
		Flags |= ISound::FLAG_LOOP;

	float Vol = (g_Config.m_SndVolume / 100.0f);
	m_Voice = Sound()->Play(CSounds::CHN_MUSIC, pTrack->m_SampleId, Flags, Vol);
	m_IsPlaying = true;
}

void CChromaMusicPlayer::StopCurrentTrack()
{
	const STrack *pTrack = CurrentTrack();
	if(pTrack && pTrack->m_SampleId >= 0)
	{
		Sound()->Stop(pTrack->m_SampleId);
	}
	m_IsPlaying = false;
}

void CChromaMusicPlayer::Play()
{
	if(m_IsPlaying)
		return;

	const STrack *pTrack = CurrentTrack();
	if(pTrack && pTrack->m_SampleId >= 0)
	{
		StartCurrentTrack();
	}
}

void CChromaMusicPlayer::Pause()
{
	if(!m_IsPlaying)
		return;

	const STrack *pTrack = CurrentTrack();
	if(pTrack && pTrack->m_SampleId >= 0)
	{
		Sound()->Pause(pTrack->m_SampleId);
		m_IsPlaying = false;
	}
}

void CChromaMusicPlayer::TogglePlay()
{
	if(m_IsPlaying)
		Pause();
	else
		Play();
}

void CChromaMusicPlayer::NextTrack()
{
	if(m_vTracks.empty())
		return;

	StopCurrentTrack();
	if(m_Shuffle && m_vTracks.size() > 1)
	{
		int NextIdx = rand() % m_vTracks.size();
		if(NextIdx == m_CurrentTrackIndex)
			NextIdx = (NextIdx + 1) % m_vTracks.size();
		m_CurrentTrackIndex = NextIdx;
	}
	else
	{
		m_CurrentTrackIndex = (m_CurrentTrackIndex + 1) % m_vTracks.size();
	}
	StartCurrentTrack();
}

void CChromaMusicPlayer::PrevTrack()
{
	if(m_vTracks.empty())
		return;

	StopCurrentTrack();
	m_CurrentTrackIndex = (m_CurrentTrackIndex - 1 + (int)m_vTracks.size()) % m_vTracks.size();
	StartCurrentTrack();
}

void CChromaMusicPlayer::SetTrack(int Index)
{
	if(Index < 0 || Index >= (int)m_vTracks.size() || Index == m_CurrentTrackIndex)
		return;

	StopCurrentTrack();
	m_CurrentTrackIndex = Index;
	StartCurrentTrack();
}

void CChromaMusicPlayer::Seek(float Progress)
{
	const STrack *pTrack = CurrentTrack();
	if(!pTrack || pTrack->m_SampleId < 0)
		return;

	float TotalTime = Sound()->GetSampleTotalTime(pTrack->m_SampleId);
	if(TotalTime > 0.0f)
	{
		float TargetTime = std::clamp(Progress, 0.0f, 1.0f) * TotalTime;
		Sound()->SetSampleCurrentTime(pTrack->m_SampleId, TargetTime);
	}
}

void CChromaMusicPlayer::OnRender()
{
	// Only render on start/main menu when offline
	if(!m_pClient->m_Menus.IsActive() || m_pClient->Client()->State() != IClient::STATE_OFFLINE)
		return;

	CUIRect Screen = *Ui()->Screen();
	const STrack *pCurTrack = CurrentTrack();

	float CurTime = 0.0f;
	float TotalTime = 1.0f;
	if(pCurTrack && pCurTrack->m_SampleId >= 0)
	{
		CurTime = Sound()->GetSampleCurrentTime(pCurTrack->m_SampleId);
		TotalTime = Sound()->GetSampleTotalTime(pCurTrack->m_SampleId);
		if(TotalTime <= 0.0f)
			TotalTime = 1.0f;
	}

	float Progress = std::clamp(CurTime / TotalTime, 0.0f, 1.0f);

	// Compact Player Bar Dimensions (Bottom-right corner pill widget)
	const float PlayerWidth = 260.0f;
	const float PlayerHeight = 62.0f;
	const float MarginRight = 24.0f;
	const float MarginBottom = 22.0f;

	CUIRect WidgetRect;
	WidgetRect.x = Screen.w - PlayerWidth - MarginRight;
	WidgetRect.y = Screen.h - PlayerHeight - MarginBottom;
	WidgetRect.w = PlayerWidth;
	WidgetRect.h = PlayerHeight;

	// Dark pill container with crisp white/subtle outline
	WidgetRect.Draw(ColorRGBA(0.06f, 0.06f, 0.08f, 0.92f), IGraphics::CORNER_ALL, 14.0f);
	RenderTools()->DrawRoundRectOutline(&WidgetRect, ColorRGBA(0.85f, 0.85f, 0.90f, 0.40f), 14.0f, 1.25f);

	// Scrubber / Seeker Area (Top portion of widget)
	CUIRect TopArea, BottomArea;
	WidgetRect.HSplitTop(26.0f, &TopArea, &BottomArea);
	TopArea.Margin(8.0f, &TopArea);

	// Title / Artist text
	char aTitleBuf[128];
	if(pCurTrack)
		str_format(aTitleBuf, sizeof(aTitleBuf), "%s • %s", pCurTrack->m_aTitle, pCurTrack->m_aArtist);
	else
		str_copy(aTitleBuf, "chroma. Player • No Track");

	CUIRect TitleRect, SeekerRect;
	TopArea.HSplitTop(12.0f, &TitleRect, &SeekerRect);
	TextRender()->TextColor(ColorRGBA(0.82f, 0.84f, 0.88f, 0.90f));
	TextRender()->Text(TitleRect.x + 4.0f, TitleRect.y, 9.5f, aTitleBuf);

	// Seeker bar line
	SeekerRect.HSplitTop(3.0f, nullptr, &SeekerRect);
	SeekerRect.HSplitTop(3.5f, &SeekerRect, nullptr);
	SeekerRect.Draw(ColorRGBA(0.25f, 0.25f, 0.30f, 0.80f), IGraphics::CORNER_ALL, 2.0f);

	CUIRect FilledSeeker = SeekerRect;
	FilledSeeker.w *= Progress;
	FilledSeeker.Draw(ColorRGBA(0.95f, 0.95f, 1.0f, 1.0f), IGraphics::CORNER_ALL, 2.0f);

	// Knob
	CUIRect Knob;
	Knob.x = FilledSeeker.x + FilledSeeker.w - 3.5f;
	Knob.y = SeekerRect.y - 1.5f;
	Knob.w = 7.0f;
	Knob.h = 6.5f;
	Knob.Draw(ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f), IGraphics::CORNER_ALL, 3.5f);

	// Seeker interaction
	CUIRect ClickableSeeker = SeekerRect;
	ClickableSeeker.y -= 4.0f;
	ClickableSeeker.h += 8.0f;
	if(Ui()->MouseInside(&ClickableSeeker))
	{
		if(Ui()->MouseButton(0))
		{
			m_DraggingScrubber = true;
		}
	}

	if(m_DraggingScrubber)
	{
		if(Ui()->MouseButton(0))
		{
			float NewProgress = (Ui()->MouseX() - SeekerRect.x) / SeekerRect.w;
			Seek(NewProgress);
		}
		else
		{
			m_DraggingScrubber = false;
		}
	}

	// Bottom Area: Controls Row
	BottomArea.Margin(4.0f, &BottomArea);
	float BtnW = 28.0f;
	float Space = (BottomArea.w - (6 * BtnW)) / 7.0f;

	float CurX = BottomArea.x + Space;
	float BtnY = BottomArea.y + 2.0f;
	float BtnH = 22.0f;

	// 1. Tracklist button
	CUIRect ListBtn = {CurX, BtnY, BtnW, BtnH};
	static CButtonContainer s_ListBtnId;
	if(m_pClient->m_Menus.DoButton_Menu(&s_ListBtnId, FontIcon::LIST_UL, 0, &ListBtn, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.0f, m_TrackListOpen ? ColorRGBA(0.4f, 0.4f, 0.5f, 0.5f) : ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		m_TrackListOpen = !m_TrackListOpen;
	}

	CurX += BtnW + Space;

	// 2. Previous button
	CUIRect PrevBtn = {CurX, BtnY, BtnW, BtnH};
	static CButtonContainer s_PrevBtnId;
	if(m_pClient->m_Menus.DoButton_Menu(&s_PrevBtnId, FontIcon::BACKWARD_STEP, 0, &PrevBtn, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.0f, ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		PrevTrack();
	}

	CurX += BtnW + Space;

	// 3. Play/Pause button (prominent center pill)
	CUIRect PlayBtn = {CurX, BtnY - 2.0f, BtnW + 4.0f, BtnH + 4.0f};
	static CButtonContainer s_PlayBtnId;
	const char *pPlayIcon = m_IsPlaying ? FontIcon::PAUSE : FontIcon::PLAY;
	if(m_pClient->m_Menus.DoButton_Menu(&s_PlayBtnId, pPlayIcon, 0, &PlayBtn, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 12.0f, 0.0f, ColorRGBA(1.0f, 1.0f, 1.0f, 0.25f)))
	{
		TogglePlay();
	}

	CurX += (BtnW + 4.0f) + Space;

	// 4. Next button
	CUIRect NextBtn = {CurX, BtnY, BtnW, BtnH};
	static CButtonContainer s_NextBtnId;
	if(m_pClient->m_Menus.DoButton_Menu(&s_NextBtnId, FontIcon::FORWARD_STEP, 0, &NextBtn, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.0f, ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		NextTrack();
	}

	CurX += BtnW + Space;

	// 5. Shuffle button
	CUIRect ShuffleBtn = {CurX, BtnY, BtnW, BtnH};
	static CButtonContainer s_ShuffleBtnId;
	if(m_pClient->m_Menus.DoButton_Menu(&s_ShuffleBtnId, FontIcon::ARROWS_LEFT_RIGHT, 0, &ShuffleBtn, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.0f, m_Shuffle ? ColorRGBA(0.35f, 0.65f, 1.0f, 0.40f) : ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		m_Shuffle = !m_Shuffle;
	}

	CurX += BtnW + Space;

	// 6. Repeat button
	CUIRect RepeatBtn = {CurX, BtnY, BtnW, BtnH};
	static CButtonContainer s_RepeatBtnId;
	if(m_pClient->m_Menus.DoButton_Menu(&s_RepeatBtnId, FontIcon::ARROWS_ROTATE, 0, &RepeatBtn, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 6.0f, 0.0f, m_Repeat ? ColorRGBA(0.35f, 0.65f, 1.0f, 0.40f) : ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		m_Repeat = !m_Repeat;
	}

	// TrackList popup if open
	if(m_TrackListOpen && !m_vTracks.empty())
	{
		CUIRect PopupRect;
		PopupRect.w = 260.0f;
		PopupRect.h = std::min(180.0f, (float)m_vTracks.size() * 24.0f + 16.0f);
		PopupRect.x = WidgetRect.x;
		PopupRect.y = WidgetRect.y - PopupRect.h - 8.0f;

		PopupRect.Draw(ColorRGBA(0.08f, 0.08f, 0.10f, 0.95f), IGraphics::CORNER_ALL, 10.0f);
		RenderTools()->DrawRoundRectOutline(&PopupRect, ColorRGBA(0.8f, 0.8f, 0.85f, 0.35f), 10.0f, 1.0f);

		CUIRect ItemArea = PopupRect;
		ItemArea.Margin(6.0f, &ItemArea);

		for(size_t i = 0; i < m_vTracks.size(); ++i)
		{
			CUIRect TrackItem;
			ItemArea.HSplitTop(22.0f, &TrackItem, &ItemArea);
			ItemArea.HSplitTop(2.0f, nullptr, &ItemArea);

			bool IsSelected = ((int)i == m_CurrentTrackIndex);
			if(IsSelected)
				TrackItem.Draw(ColorRGBA(0.25f, 0.45f, 0.85f, 0.35f), IGraphics::CORNER_ALL, 4.0f);

			static CButtonContainer s_TrackItemIds[64];
			char aItemLabel[128];
			str_format(aItemLabel, sizeof(aItemLabel), "%d. %s - %s", (int)i + 1, m_vTracks[i].m_aTitle, m_vTracks[i].m_aArtist);

			if(m_pClient->m_Menus.DoButton_Menu(&s_TrackItemIds[i % 64], aItemLabel, 0, &TrackItem, BUTTONFLAG_LEFT, nullptr, IGraphics::CORNER_ALL, 4.0f, 0.0f, ColorRGBA(0.0f, 0.0f, 0.0f, 0.0f)))
			{
				SetTrack(i);
				m_TrackListOpen = false;
			}
		}
	}

	TextRender()->TextColor(TextRender()->DefaultTextColor());
}
