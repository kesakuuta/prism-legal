#ifndef GAME_CLIENT_COMPONENTS_CHROMA_MUSIC_PLAYER_H
#define GAME_CLIENT_COMPONENTS_CHROMA_MUSIC_PLAYER_H

#include <engine/sound.h>
#include <game/client/component.h>
#include <vector>
#include <string>

class CChromaMusicPlayer : public CComponent
{
public:
	struct STrack
	{
		char m_aTitle[64];
		char m_aArtist[64];
		char m_aPath[128];
		int m_SampleId;
	};

	CChromaMusicPlayer();
	virtual ~CChromaMusicPlayer();

	virtual int Sizeof() const override { return sizeof(*this); }
	virtual void OnInit() override;
	virtual void OnRender() override;
	virtual void OnReset() override;

	void Play();
	void Pause();
	void TogglePlay();
	void NextTrack();
	void PrevTrack();
	void SetTrack(int Index);
	void Seek(float Progress); // 0.0f - 1.0f

	bool IsPlaying() const { return m_IsPlaying; }
	bool IsShuffle() const { return m_Shuffle; }
	bool IsRepeat() const { return m_Repeat; }

	void SetShuffle(bool Shuffle) { m_Shuffle = Shuffle; }
	void SetRepeat(bool Repeat) { m_Repeat = Repeat; }

	const STrack *CurrentTrack() const;
	const std::vector<STrack> &TrackList() const { return m_vTracks; }

private:
	std::vector<STrack> m_vTracks;
	int m_CurrentTrackIndex;
	bool m_IsPlaying;
	bool m_Shuffle;
	bool m_Repeat;
	bool m_TrackListOpen;
	bool m_DraggingScrubber;

	ISound::CVoiceHandle m_Voice;

	void ScanTracks();
	void StartCurrentTrack();
	void StopCurrentTrack();
};

#endif // GAME_CLIENT_COMPONENTS_CHROMA_MUSIC_PLAYER_H
