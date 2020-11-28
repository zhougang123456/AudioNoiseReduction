#include "AudioRecordWindows.h"
#include "rnnoise.h"
#include <string.h>
#include <speex/speex_preprocess.h>
namespace AudioRecordSpace
{
	// 静态变量初始化
	int nRet = 0;
	FILE *fpIn = NULL;
	FILE *fpOut = NULL;

	char *pInBuffer = NULL;
	char *pOutBuffer = NULL;

	
	AudioDataDenoise m_audioDataDenoise;
	//SpeexPreprocessState *state = speex_preprocess_state_init(480, SAMPLE_RATE);

	std::array <char, AUDIO_DATA_BLOCK_SIZE> AudioRecordWindows::m_AudioDataBlock = {};
	std::vector<std::array<char, AUDIO_DATA_BLOCK_SIZE>> AudioRecordWindows::m_AudioData = { {} };
	bool AudioRecordWindows::m_bStopRecord = false;
	bool AudioRecordWindows::m_bPushData = true;
	bool AudioRecordWindows::m_bCallback = false;;
	AudioRecordCallback AudioRecordWindows::m_Callback = NULL;
	AudioRecordWindows::AudioRecordWindows()
	{
		/*int denoise = 1;

		int noiseSuppress = -25;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DENOISE, &denoise);

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress);



		int i;

		i = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_AGC, &i);

		i = 80000;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);

		i = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DEREVERB, &i);

		float f = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);

		f = 0;

		speex_preprocess_ctl(state, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
*/
		m_audioDataDenoise.Init();
		m_WavFileOpen = NULL;
		m_PcmFileOpen = NULL;
		m_bSaveWavFile = false;
		m_bSavePcmFile = false;
		m_WavFilePath = "";
		m_PcmFilePath = "";
		m_WavHeader =
		{
		{ 'R', 'I', 'F', 'F' },

		0,
		{ 'W', 'A', 'V', 'E' },
		{ 'f', 'm', 't', ' ' },
		sizeof(PCMWAVEFORMAT) ,
		WAVE_FORMAT_PCM,
		1,
		SAMPLE_RATE,
		SAMPLE_RATE*(SAMPLE_BITS / 8),
		SAMPLE_BITS / 8,
		SAMPLE_BITS,
		{ 'd', 'a', 't', 'a' },
		0
		};
	}
	AudioRecordWindows::~AudioRecordWindows()
	{
	}
	bool AudioRecordWindows::OpenAudioDevice()
	{
		int audioDeviceNum = waveInGetNumDevs();
		if (audioDeviceNum <= 0)
		{
			std::cout << "Windows没有找到录音设备，请确认Windows找到了录音设备" << std::endl;
			return false;
		}
		else
		{
			for (unsigned int i = 0; i < audioDeviceNum; ++i)
			{
				WAVEINCAPS waveInCaps;
				MMRESULT mmResult = waveInGetDevCaps(i, &waveInCaps, sizeof(WAVEINCAPS));
				if (i == 0)
				{
					std::cout << "当前默认的录音设备信息描述：" << waveInCaps.szPname << std::endl;
				}
				else
				{
					std::cout << "其他录音设备信息描述" << waveInCaps.szPname << std::endl;
				}
			}
		}
		WAVEFORMATEX waveFormate;
		InitWaveFormat(&waveFormate, CHANNEL_NUM, SAMPLE_RATE, SAMPLE_BITS);
		waveInOpen(&m_AudioDevice, WAVE_MAPPER, &waveFormate, (DWORD_PTR)WaveAPI_Callback, DWORD_PTR(this), CALLBACK_FUNCTION);
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::OpenDevice);
		}
		return true;
	}
	void AudioRecordWindows::CloseAudioDevice()
	{
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::CloseDevice);
		}
		waveInClose(m_AudioDevice);
	}
	void AudioRecordWindows::InitWaveFormat(LPWAVEFORMATEX WaveFormat, WORD ChannelNum, DWORD SampleRate, WORD BitsPerSample)
	{
		// 配置音频波形参数
		WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
		WaveFormat->nChannels = ChannelNum;
		WaveFormat->nSamplesPerSec = SampleRate;
		WaveFormat->nAvgBytesPerSec = SampleRate * ChannelNum * BitsPerSample / 8;
		WaveFormat->nBlockAlign = ChannelNum * BitsPerSample / 8;
		WaveFormat->wBitsPerSample = BitsPerSample;
		WaveFormat->cbSize = 0;
		std::cout << "采样参数：" << std::endl;
		std::cout << "声道数" << ChannelNum << std::endl;
		std::cout << "采样率" << SampleRate << "Hz" << std::endl;
		std::cout << "位深" << BitsPerSample << std::endl;
	}
	void AudioRecordWindows::StartRecord()
	{
		for (unsigned int i = 0; i < BUFFER_NUM; ++i)
		{
			m_WaveHdrBuffer[i].lpData = new char[AUDIO_DATA_BLOCK_SIZE];
			m_WaveHdrBuffer[i].dwBufferLength = AUDIO_DATA_BLOCK_SIZE;
			m_WaveHdrBuffer[i].dwBytesRecorded = 0;
			m_WaveHdrBuffer[i].dwUser = i;
			m_WaveHdrBuffer[i].dwFlags = 0;
			m_WaveHdrBuffer[i].dwLoops = 0;
			m_WaveHdrBuffer[i].lpNext = NULL;
			m_WaveHdrBuffer[i].reserved = 0;
			// 排进缓冲区
			waveInPrepareHeader(m_AudioDevice, &m_WaveHdrBuffer[i], sizeof(WAVEHDR));
			waveInAddBuffer(m_AudioDevice, &m_WaveHdrBuffer[i], sizeof(WAVEHDR));
		}
		// 清除视频缓冲
		m_AudioData.clear();
		m_AudioData.shrink_to_fit();
		m_AudioData.resize(0);
		// 开始录音
		waveInStart(m_AudioDevice);
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::RecordStart);
		}
	}
	void AudioRecordWindows::StopRecord()
	{
		m_bStopRecord = true;
		// 停止录音设备
		waveInStop(m_AudioDevice);
		waveInReset(m_AudioDevice);
		if (m_bCallback)
		{
			m_Callback(m_AudioDataBlock, RecordStatus::RecordStop);
		}
		// 释放缓冲区
		for (unsigned int i = 0; i < BUFFER_NUM; ++i)
		{
			waveInUnprepareHeader(m_AudioDevice, &m_WaveHdrBuffer[i], sizeof(WAVEHDR));
			delete m_WaveHdrBuffer[i].lpData;
			m_WaveHdrBuffer[i].lpData = NULL;
		}
		// 保存wav文件
		if (m_bSaveWavFile)
		{
			WriteWavFile();
		}
		// 保存pcm文件
		if (m_bSavePcmFile)
		{
			WritePcmFile();
		}
	}
	void AudioRecordWindows::ResetRecord()
	{
		m_AudioData.clear();
		m_AudioData.shrink_to_fit();
		m_bSaveWavFile = false;
		m_bSavePcmFile = false;
		m_bPushData = true;
		m_bStopRecord = false;
		m_bCallback = false;
		m_Callback = NULL;
		m_WavFileOpen = NULL;
		m_PcmFileOpen = NULL;
		m_WavFilePath = "";
		m_PcmFilePath = "";
	}
	void AudioRecordWindows::SetWavFileName(const char * filePath)
	{
		m_bSaveWavFile = true;
		m_WavFilePath = filePath;
		// 尝试打开文件，创建文件
		errno_t err = fopen_s(&m_WavFileOpen, m_WavFilePath.c_str(), "wb");
		if (err > 0)
		{
			std::cout << "文件创建失败：" << err << " 检查文件名和占用" << std::endl;
			m_bSaveWavFile = false;
		}
	}
	void AudioRecordWindows::SetPcmFileName(const char * filePath)
	{
		m_bSavePcmFile = true;
		m_PcmFilePath = filePath;
		// 尝试打开文件，创建文件
		errno_t err = fopen_s(&m_PcmFileOpen, m_PcmFilePath.c_str(), "wb");
		if (err > 0)
		{
			std::cout << "文件创建失败：" << err << " 检查文件名和占用" << std::endl;
			m_bSavePcmFile = false;
		}
	}
	void AudioRecordWindows::WriteWavFile()
	{
		// 编辑并写入Wave头信息
		m_WavHeader.data_size = AUDIO_DATA_BLOCK_SIZE * m_AudioData.size();
		m_WavHeader.size_8 = m_WavHeader.data_size + 32;
		fwrite(&m_WavHeader, sizeof(m_WavHeader), 1, m_WavFileOpen);
		// 追加RawData
		fwrite(m_AudioData.data(), AUDIO_DATA_BLOCK_SIZE * m_AudioData.size(), 1, m_WavFileOpen);
		// 写入结束
		fclose(m_WavFileOpen);
	}
	void AudioRecordWindows::WritePcmFile()
	{
		// 追加RawData
		fwrite(m_AudioData.data(), AUDIO_DATA_BLOCK_SIZE * m_AudioData.size(), 1, m_PcmFileOpen);
		// 写入结束
		fclose(m_PcmFileOpen);
	}
	void AudioRecordWindows::RegisterCallback(AudioRecordCallback audioCallback)
	{
		m_bCallback = true;
		m_Callback = audioCallback;
	}
	DWORD(AudioRecordWindows::WaveAPI_Callback)(HWAVEIN hWaveIn, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		// 消息switch
		switch (uMsg)
		{
		case WIM_OPEN:      // 设备成功已打开
			std::cout << "设备成功打开" << std::endl;
			break;
		case WIM_DATA:      // 缓冲区数据填充完毕
		// 停止后会频繁发出WIM_DATA,已经将数据转移所以不必理会后继数据【后继数据在这里来看是是重复的】
			if (m_bPushData)
			{
				//std::cout << "缓冲区数据填充完毕" << std::endl;
				// 把缓冲区数据拷贝出来
				memcpy(m_AudioDataBlock.data(), ((LPWAVEHDR)dwParam1)->lpData, AUDIO_DATA_BLOCK_SIZE);
				// 没有录进去的被填充为0xcd,改成0来避免末尾出现爆音【只在结束录音时进行，不影响添加缓存效率】
				if (((LPWAVEHDR)dwParam1)->dwBytesRecorded < AUDIO_DATA_BLOCK_SIZE)
				{
					for (size_t i = ((LPWAVEHDR)dwParam1)->dwBytesRecorded; i < AUDIO_DATA_BLOCK_SIZE; i++)
					{
						m_AudioDataBlock.at(i) = 0;
					}
				}


				//m_audioDataDenoise.DealWithWebRtc((short*)m_AudioDataBlock.data());
				m_audioDataDenoise.DealWithRnnoise((short*)m_AudioDataBlock.data());

				//speex_preprocess_run(state, (spx_int16_t*)(m_AudioDataBlock.data()));


				// 添加这一帧
				m_AudioData.push_back(m_AudioDataBlock);
				// 如果你设置了回调函数
				if (m_bCallback)
				{
					m_Callback(m_AudioDataBlock, RecordStatus::RecordWriteData);
				}
			}
			// 如果需要停止录音则不继续添加缓存
			if (!m_bStopRecord)
			{
				waveInAddBuffer(hWaveIn, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR));//添加到缓冲区
			}
			else
			{
				// 如果已经停止了录制，就不要再写入数据
				m_bPushData = false;
			}
			break;
		case WIM_CLOSE:
			// 操作成功完成
			std::cout << "录音设备已经关闭..." << std::endl;
			break;
		default:
			break;
		}
		return 0;
	}
}