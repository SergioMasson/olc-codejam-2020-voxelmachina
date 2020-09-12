#pragma once

namespace audio
{
	void Initialize();

	void PlayAudioFile(std::wstring filePath, bool loop = false);
}