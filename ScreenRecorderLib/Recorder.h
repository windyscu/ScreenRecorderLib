// ScreenRecorder.h

#pragma once
#include <atlbase.h>
#include <vcclr.h>
#include <vector>
#include "fifo_map.h"
#include "internal_recorder.h"
#include "ManagedIStream.h"
using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


delegate void InternalStatusCallbackDelegate(int status);
delegate void InternalCompletionCallbackDelegate(std::wstring path, nlohmann::fifo_map<std::wstring, int>);
delegate void InternalErrorCallbackDelegate(std::wstring path);

namespace ScreenRecorderLib {

	public enum class RecorderStatus {
		Idle,
		Recording,
		Paused,
		Finishing
	};
	public enum class AudioChannels {
		Mono = 1,
		Stereo = 2,
		FivePointOne = 6
	};
	public enum class AudioBitrate {
		bitrate_96kbps = 12000,
		bitrate_128kbps = 16000,
		bitrate_160kbps = 20000,
		bitrate_192kbps = 24000,

	};

	public enum class RecorderMode {
		Video,
		Slideshow,
		Snapshot
	};
	public enum class H264Profile
	{
		Baseline = 66,
		Main = 77,
		High = 100
	};
	public ref class FrameData {
	public:
		property String^ Path;
		property int Delay;
		FrameData() {}
		FrameData(String^ path, int delay) {
			Path = path;
			Delay = delay;
		}
	};
	public ref class DisplayOptions {
	public:
		property int Monitor;
		property int Left;
		property int Top;
		property int Right;
		property int Bottom;
		DisplayOptions() {

		}
		DisplayOptions(int monitor) {
			Monitor = monitor;
		}
		DisplayOptions(int monitor, int left, int top, int right, int bottom) {
			Monitor = monitor;
			Left = left;
			Top = top;
			Right = right;
			Bottom = bottom;
		}
	};

	public ref class VideoOptions {
	public:
		VideoOptions() {
			Framerate = 30;
			Bitrate = 4000 * 1000;
			IsFixedFramerate = false;
			IsMousePointerEnabled = true;
			EncoderProfile = H264Profile::Baseline;
		}
		property H264Profile EncoderProfile;	
		/// <summary>
		///Framerate in frames per second.
		/// </summary>
		property int Framerate;
		/// <summary>
		///Bitrate in bits per second
		/// </summary>
		property int Bitrate;
		/// <summary>
		///Display the mouse cursor on the recording
		/// </summary>
		property bool IsMousePointerEnabled;
		/// <summary>
		///Send data to the video encoder every frame, even if it means duplicating the previous frame(s). Can fix stutter issues in fringe cases, but uses more resources.
		/// </summary>
		property bool IsFixedFramerate;
	};
	public ref class AudioOptions {
	public:
		AudioOptions() {
			IsAudioEnabled = false;
			Bitrate = AudioBitrate::bitrate_96kbps;
			Channels = AudioChannels::Stereo;
		}
		property bool IsAudioEnabled;
		property AudioBitrate Bitrate;
		property AudioChannels Channels;
	};
	public ref class RecorderOptions {
	public:
		RecorderOptions() {
			IsThrottlingDisabled = false;
			IsLowLatencyEnabled = false;
			IsHardwareEncodingEnabled = true;
			IsMp4FastStartEnabled = true;
		}
		property RecorderMode RecorderMode;
		/// <summary>
		///Disable throttling of video renderer. If this is disabled, all frames are sent to renderer as fast as they come. Can cause out of memory crashes.
		/// </summary>
		property bool IsThrottlingDisabled;
		/// <summary>
		///Faster rendering, but can affect quality. Use when speed is more important than quality.
		/// </summary>
		property bool IsLowLatencyEnabled;
		/// <summary>
		///Enable hardware encoding if available. This is enabled by default.
		/// </summary>
		property bool IsHardwareEncodingEnabled;
		/// <summary>
		/// Place the mp4 header at the start of the file. This makes the playback start faster when streaming. This option is only available when recording to a stream.
		/// </summary>
		property bool IsMp4FastStartEnabled;
		property VideoOptions^ VideoOptions;
		property DisplayOptions^ DisplayOptions;
		property AudioOptions^ AudioOptions;
	};

	public ref class RecordingStatusEventArgs :System::EventArgs {
	public:
		property RecorderStatus Status;
		RecordingStatusEventArgs(RecorderStatus status) {
			Status = status;
		}
	};
	public ref class RecordingCompleteEventArgs :System::EventArgs {
	public:
		property String^ FilePath;
		property  List<FrameData^>^ FrameInfos;
		RecordingCompleteEventArgs(String^ path, List<FrameData^>^ frameInfos) {
			FilePath = path;
			FrameInfos = frameInfos;
		}
	};
	public ref class RecordingFailedEventArgs :System::EventArgs {
	public:
		property String^ Error;
		RecordingFailedEventArgs(String^ error) {
			Error = error;
		}
	};
	public ref class Recorder {
	private:
		Recorder(RecorderOptions^ options);
		~Recorder();
		!Recorder();
		RecorderStatus _status;
		void createErrorCallback();
		void createCompletionCallback();
		void createStatusCallback();
		void EventComplete(std::wstring str, nlohmann::fifo_map<std::wstring, int> delays);
		void EventFailed(std::wstring str);
		void EventStatusChanged(int status);
		GCHandle _statusChangedDelegateGcHandler;
		GCHandle _errorDelegateGcHandler;
		GCHandle _completedDelegateGcHandler;
	public:
		property RecorderStatus Status {
			RecorderStatus get() {
				return _status;
			}
	private:
		void set(RecorderStatus value) {
			_status = value;
		}
		}
		internal_recorder *lRec;
		void Record(System::String^ path);
		void Record(System::Runtime::InteropServices::ComTypes::IStream^ stream);
		void Record(System::IO::Stream^ stream);
		void Pause();
		void Resume();
		void Stop();
		static Recorder^ CreateRecorder();
		static Recorder^ CreateRecorder(RecorderOptions^ options);
		event EventHandler<RecordingCompleteEventArgs^>^ OnRecordingComplete;
		event EventHandler<RecordingFailedEventArgs^>^ OnRecordingFailed;
		event EventHandler<RecordingStatusEventArgs^>^ OnStatusChanged;
		ManagedIStream *m_ManagedStream;
	};
}
