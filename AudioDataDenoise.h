#pragma once
//WebRtc需要的头文件
#include "signal_processing_library.h"
#include "noise_suppression_x.h"
#include "noise_suppression.h"
#include "gain_control.h"
//rnnoise需要的头文件
#include "rnnoise.h"
#include "common.h"
//speex需要的头文件
#include <speex/speex_preprocess.h>
class AudioDataDenoise
{
public:
	AudioDataDenoise();
	~AudioDataDenoise();
	void Init();
	void InitWebRtc();
	void InitSpeex();
	void InitRnnoise();
	void DealWithWebRtc(short * audioData);
	void DealWithSpeex(short * audioData);
	void DealWithRnnoise(short * audioData);
private:
	DenoiseState* m_pRnnoise;
	NsHandle * m_pNS_inst;
	SpeexPreprocessState *st;
};

