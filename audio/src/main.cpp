#include <AL/al.h>
#include <AL/alc.h>
//#include <AL/alext.h>
#include <iostream>
#include <fstream>
#include <vector>
//#include <windows.h>
#include <sstream>
#include <cmath>

#include <list>
#include <thread>
#include <chrono>

using namespace std;

const int SRATE = 44100;
const int SEC = 2;

void split(std::vector<std::string>& devices, const char* deviceList) {
  char last = 'x';
  char next = 'x';

  unsigned int counter = 0;
  std::stringstream  nextDevice;
  while (!(last == '\0' && next == '\0')) {
    last = next;
	next = deviceList[counter++];
	if (next != '\0')
		nextDevice << next;
	else {
		devices.push_back(nextDevice.str());
		nextDevice.str("");
	}
  }
}

void generateWaveform(const std::string& filename) {
	std::ofstream out(filename.c_str(), ios::binary);
	int PITCH = 440;
	double twopi = 2.0 * 3.14159;
	double amp = 32768;
	double volume = 0.40;
	for (int i = 0; i < SRATE * SEC; ++i) {
		double val = std::sin(static_cast<double>(i) * twopi * (double) PITCH / (double) SRATE);
		val = val * amp * volume;
		short act = (short) val;
		
		out.write((const char*) &act, 2);
	}
	out.flush();
	out.close();
}

void enumerateCapture(std::vector<std::string>& devices) {
	const char* deviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	split(devices, deviceList);
}

void enumeratePlayback(std::vector<std::string>& devices) {
	const char* deviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	split(devices, deviceList);
}

struct AudioBuffer {
    unsigned int id;
	std::vector<char> data;
	
	AudioBuffer() : data(16384, 0) { }
};

class AudioCapture {
public:
	AudioCapture(const std::string& deviceName) 
		: device(0), bufferSize(4096), sleepDuration(50), format(AL_FORMAT_MONO16) 
	{
		std::cout << deviceName << std::endl;
		device = alcCaptureOpenDevice(deviceName.c_str(), SRATE, format, bufferSize);
	}
	
	~AudioCapture() {
		alcCaptureCloseDevice(device);
	}
	
	static std::vector<std::string> deviceList() {
		std::vector<std::string> devices;
		const char* deviceList = alcGetString(0, ALC_CAPTURE_DEVICE_SPECIFIER);
		split(devices, deviceList);
		return devices;
	}
	
	void capture(const std::string& filename) {
		std::ofstream output(filename.c_str(), ios::out | ios::binary);
		ALint sample = 0;

		std::vector<short> buffer(bufferSize, 0);
		alcCaptureStart(device);
		unsigned int length = 0;
		while(length < SEC * SRATE) {
			alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &sample);
			alcCaptureSamples(device, (ALCvoid *)&buffer[0], sample);
			
			output.write((char*) buffer.data(), 2 * sample);
			length += sample;
			output.flush();
      std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
		}
		
		output.close();
		alcCaptureStop(device);
		std::cout << "Closed" << std::endl;
	}
	
private:
	ALCdevice *device;
	
	const unsigned int bufferSize;
  const unsigned int sleepDuration;
	unsigned int format;
};

void captureAudio(const std::string& filename) {
	static std::vector<std::string> devices = AudioCapture::deviceList();
	static AudioCapture capture(devices[1]);
	
	int x;
	std::cout << "Enter any key to start recording" << std::endl;
	std::cin >> x;

    capture.capture(filename);
}

class AudioPlayer {
public:
	AudioPlayer(const std::string& deviceName) {
		device = alcOpenDevice(deviceName.c_str());
		context = alcCreateContext(device, 0);
		alcMakeContextCurrent(context);
		alGenSources(1, &source);
		
		setDefaultSource();
		setDefaultListener();
	}
	
	~AudioPlayer() {
		cleanup();
	}
	
	void cleanup() {
		alDeleteSources(1, &source);
		alcMakeContextCurrent(0);
		alcDestroyContext(context);
		alcCloseDevice(device);
	}
	
	static std::vector<std::string> deviceList() {
		std::vector<std::string> devices;
		const char* deviceList = alcGetString(0, ALC_ALL_DEVICES_SPECIFIER);
		split(devices, deviceList);
		return devices;
	}
	
	void play(const std::string& filename) {
		std::ifstream input(filename.c_str(), ios::binary);
		
		const unsigned int numBuffers = 2;
		ALuint bufferIds[numBuffers];
		alGenBuffers(numBuffers, bufferIds);
		
		std::vector<AudioBuffer> buffers(numBuffers);
		for (unsigned int i = 0; i < numBuffers; ++i)
			buffers[i].id = bufferIds[i];
		
		int val = 0;
		std::list<AudioBuffer*> queued;
		std::list<AudioBuffer*> unqueued;
		for (auto& buf : buffers)
			unqueued.push_back(&buf);
		
		while(input.good()) {
			alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
			for (int i = 0; i < val; ++i) {
				unsigned int id = queued.front()->id;
				alSourceUnqueueBuffers(source, 1, &id);
				unqueued.push_back(queued.front());
				queued.pop_front();
			}
			if (unqueued.empty())
				continue;
				
			AudioBuffer* nextBuffer = unqueued.front();
			queued.push_back(nextBuffer);
			unqueued.pop_front();

			unsigned int read = input.readsome(&nextBuffer->data[0], nextBuffer->data.size());
			if (read == 0)
				break;
			alBufferData(nextBuffer->id, AL_FORMAT_MONO16, &nextBuffer->data[0], read, SRATE);
		
			//Source
			alSourceQueueBuffers(source, 1, &nextBuffer->id);
			
			alGetSourcei(source, AL_SOURCE_STATE, &val);
			if(val != AL_PLAYING)
				alSourcePlay(source);
		}
		
		do {
			alGetSourcei(source, AL_SOURCE_STATE, &val);
		} while(val == AL_PLAYING);
		
		alSourceStop(source);
		alSourcei(source, AL_BUFFER, 0);

		alDeleteBuffers(1, bufferIds);
	}
	
	void setDefaultSource() {
		ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };
		ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };
		
		alSourcef(source, AL_PITCH, 1.0f); 
		alSourcef(source, AL_GAIN, 1.0f);
		alSourcefv(source, AL_POSITION, SourcePos);
		alSourcefv(source, AL_VELOCITY, SourceVel);
		alSourcei(source, AL_LOOPING, false);
	}
	
	void setDefaultListener() {
		ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
		ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
		ALfloat ListenerOri[] = { 0.0, 0.0, 1.0, 0.0, 1.0, 0.0 };
                                                                       
		alListenerfv(AL_POSITION, ListenerPos);
		alListenerfv(AL_VELOCITY, ListenerVel);
		alListenerfv(AL_ORIENTATION, ListenerOri);
	}
	
private:
	ALCdevice *device;
	ALCcontext *context;
	ALuint source;
};

void playAudio(const std::string& filename) {
	static std::vector<std::string> devices = AudioPlayer::deviceList();
	static AudioPlayer player(devices[0]);
	player.play(filename);
}

int main(int argc, char *argv[]) {
	//std::string output = "C:/Projects/audio/pitch.txt";
	std::string output = "C:/Projects/audio/output.raw";
	//generateWaveform(output);
	captureAudio(output);
	std::cout << "Play 1" << std::endl;
	playAudio(output);
	std::cout << "Done" << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::cout << "Play 2" << std::endl;
	playAudio(output);
	std::cout << "Done" << std::endl;
	
	int x;
	std::cin >> x;

    return 0;
}
