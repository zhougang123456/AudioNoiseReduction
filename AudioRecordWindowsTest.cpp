#include "AudioRecordWindows.h"
#include "conio.h"
using namespace std;
using namespace AudioRecordSpace;
void Callback(std::array <char, AUDIO_DATA_BLOCK_SIZE> audioDataBlock, RecordStatus recordStatus)
{
	if (recordStatus == RecordStatus::OpenDevice)
	{
		cout << "回调 打开设备" << endl;
	}
	else if (recordStatus == RecordStatus::RecordStart)
	{
		cout << "回调 开始录音" << endl;
	}
	else if (recordStatus == RecordStatus::RecordWriteData)
	{
	//	cout << "回调 正在写入数据" << endl;
	}
	else if (recordStatus == RecordStatus::RecordStop)
	{
		cout << "回调 停止录音" << endl;
	}
	else if (recordStatus == RecordStatus::CloseDevice)
	{
		cout << "回调 关闭设备" << endl;
	}
}
int main()
{
	char ch = 0;
	AudioRecordWindows sound_gbr;
	while (ch != 'q')
	{
		ch = _getch();
		switch (ch)
		{
		case 's':
			cout << "开始录音" << endl;
			sound_gbr.RegisterCallback(Callback);
			sound_gbr.OpenAudioDevice();
			//sound_gbr.SetPcmFileName("test.pcm");
			sound_gbr.SetWavFileName("test.wav");
			sound_gbr.StartRecord();
			break;
		case 'q':
			cout << "结束录音" << endl;
			sound_gbr.StopRecord();
			sound_gbr.CloseAudioDevice();
			sound_gbr.ResetRecord();
			break;
		default:
			break;
		}
	}
	getchar();
	return 0;
}
