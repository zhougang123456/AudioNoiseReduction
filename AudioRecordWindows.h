#ifndef _AUDIO_RECORD_WINDOWS_H_
#define _AUDIO_RECORD_WINDOWS_H_
#include <Windows.h>
#include <iostream>
#include <array>
#include <vector>
#pragma comment(lib,"winmm.lib")
#define CHANNEL_NUM  1                                           // 声道数
#define SAMPLE_RATE 48000                                      // 每秒采样率
#define SAMPLE_BITS 16                                           // 采样位深
#define AUDIO_DATA_BLOCK_SIZE (480 * SAMPLE_BITS / 8 * 1)  // 缓存数据块大小 = 采样率*位深/2*秒（字节）                                
#define BUFFER_NUM 10         
// 缓冲区层数
//!
//! @brief 录音状态枚举 
//! 
enum RecordStatus
{
	OpenDevice,
	RecordStart,
	RecordWriteData,
	RecordStop,
	CloseDevice
};
//!
//! @brief 回调函数
//! 
typedef void(*AudioRecordCallback)(std::array <char, AUDIO_DATA_BLOCK_SIZE> audioDataBlock, RecordStatus recordStatus);
namespace AudioRecordSpace
{
	//!
	//! @brief wav文件头
	//! 
	typedef struct WavHeader
	{
		char            riff[4];                    // = "RIFF"
		UINT32          size_8;                     // = FileSize - 8
		char            wave[4];                    // = "WAVE"
		char            fmt[4];                     // = "fmt "
		UINT32          fmt_size;                   // = PCMWAVEFORMAT的大小 : 
		//PCMWAVEFORMAT
		UINT16          format_tag;                 // = PCM : 1
		UINT16          channels;                   // = 通道数 : 1
		UINT32          samples_per_sec;            // = 采样率 : 8000 | 6000 | 11025 | 16000
		UINT32          avg_bytes_per_sec;          // = 每秒平均字节数 : samples_per_sec * bits_per_sample / 8
		UINT16          block_align;                // = 每采样点字节数 : wBitsPerSample / 8
		UINT16          bits_per_sample;            // = 量化精度: 8 | 16
		char            data[4];                    // = "data";
		//DATA
		UINT32          data_size;                  // = 裸数据长度 
	};
	class AudioRecordWindows
	{
	public:
		AudioRecordWindows();
		virtual~AudioRecordWindows();
		//!
		//! @brief 打开录音设备
		//!
		bool OpenAudioDevice();
		//!
		//! @brief 关闭录音设备
		//!
		void CloseAudioDevice();
		//!
		//! @brief 初始化录音参数
		//!
		void InitWaveFormat(LPWAVEFORMATEX WaveFormat, WORD ChannelNum, DWORD SampleRate, WORD BitsPerSample);
		//!
		//! @brief 开始录音
		//!
		void StartRecord();
		//!
		//! @brief 停止录音
		//!
		void StopRecord();
		//!
		//! @brief 重置录音
		//!
		void ResetRecord();
		//!
		//! @brief 设置需要录音wav文件名称
		//!
		void SetWavFileName(const char* filePath);
		//!
		//! @brief 设置需要录音wav文件名称
		//!
		void SetPcmFileName(const char* filePath);
		//!
		//! @brief 写录音wav文件
		//!
		void WriteWavFile();
		//!
		//! @brief 写录音pcm文件
		//!
		void WritePcmFile();
		//!
		//! @brief 注册回调函数
		//!
		void RegisterCallback(AudioRecordCallback audioCallback);
		//!
		//! @brief 系统录音回调函数
		//!
		static DWORD(CALLBACK WaveAPI_Callback)(        // WaveAPI回调函数
			HWAVEIN hWaveIn,                            //  输入设备
			UINT uMsg,                                  //  消息
			DWORD_PTR dwInstance,                       //  保留
			DWORD_PTR dwParam1,                         //  刚填充好的缓冲区指针
			DWORD_PTR dwParam2                          //  保留
			);
	private:
		HWAVEIN m_AudioDevice;                                                   // 音频输入设备
		WAVEHDR m_WaveHdrBuffer[BUFFER_NUM];                                     // 声明缓冲区
		static std::array<char, AUDIO_DATA_BLOCK_SIZE> m_AudioDataBlock;       // 当前录制一帧音频数据
		static std::vector<std::array<char, AUDIO_DATA_BLOCK_SIZE>> m_AudioData; // 存储所有录制的音频数据
		static bool m_bStopRecord;                                               // 是否停止录音
		static bool m_bPushData;                                                 // 是否向m_AudioDataBlock中push数据
		std::string m_WavFilePath;                                               // Wav录音文件名称
		std::string m_PcmFilePath;                                               // Pcm录音文件名称
		bool m_bSaveWavFile;                                                     // 是否保存wav文件
		FILE* m_WavFileOpen;                                                     // 录音wav文件指针
		bool m_bSavePcmFile;                                                     // 是否保存Pcm文件
		FILE* m_PcmFileOpen;                                                     // 录音Pcm文件指针
		WavHeader m_WavHeader;                                                   // wav文件头
		static AudioRecordCallback m_Callback;                                   // 外部回调函数
		static bool m_bCallback;                                                 // 是否回调外部回调函数
	};
}
#endif // !_AUDIO_RECORD_WINDOWS_H_