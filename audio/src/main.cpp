#include <al.h>
#include <alc.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <redirect.hpp>
#include <windows.h>
#include <sstream>
#include <cmath>

using namespace std;

const int SRATE = 88200;
const int SSIZE = 10 * 88200;
std::vector<short> buffer(SSIZE, 0);
ALint sample;

std::ofstream output("C:/Projects/audio/output.txt", ios::out | ios::binary);

template <typename OutIt>
OutIt split(const std::string &text, char sep, OutIt out)
{
    size_t start = 0, end = 0;

    while((end = text.find(sep, start)) != std::string::npos)
    { 
        *out++ = text.substr(start, end - start);
        start = end + 1;
    }

    *out++ = text.substr(start);

    return out;
}

void generateWaveform(const std::string& filename) {
	std::ofstream out(filename.c_str(), ios::out | ios::binary);
	int PITCH = 440;
	const int SECONDS = 10;
	double twopi = 2.0 * 3.14159;
	double amp = 32768;
	double volume = 0.20;
	for (int i = 0; i < SRATE * SECONDS; ++i) {
		if (i % 50000 == 0)
			PITCH += 50;
			
		double val = std::sin(static_cast<double>(i) * twopi * (double) PITCH / (double) SRATE);
		val = val * amp * volume;
		short act = (short) val;
		
		out.write((const char*) &act, 2);
	}
	out.flush();
}

void enumerateCapture(std::vector<std::string>& devices) {
	const char* deviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	split(deviceList, '\0', std::back_inserter(devices));
}

void enumeratePlayback(std::vector<std::string>& devices) {
	const char* deviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	split(deviceList, '\0', std::back_inserter(devices));
}

void captureAudio() {
	std::vector<std::string> devices;
	enumerateCapture(devices);
	for (auto device : devices)
		std::cout << device << std::endl;

    ALCdevice *device = alcCaptureOpenDevice(devices[0].c_str(), SRATE, AL_FORMAT_MONO16, SSIZE);
    if (alGetError() != AL_NO_ERROR) {
		cout << "Error" << endl;
        return;
    }
    alcCaptureStart(device);

	for (int i = 0; i < 10000000; ++i) {
        alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &sample);
        alcCaptureSamples(device, (ALCvoid *)&buffer[0], sample);
		
		output.write((const char*) buffer.data(), 2 * sample);
		output.flush();
        // ... do something with the buffer 
    }

    alcCaptureStop(device);
    alcCaptureCloseDevice(device);
}

void playAudio(const std::string& filename) {
	std::ifstream input(filename.c_str(), ios::in | ios::binary);
	
	std::vector<std::string> devices;
	enumeratePlayback(devices);
	for (auto device : devices)
		std::cout << device << std::endl;
	
	ALCdevice *device = alcOpenDevice(devices[0].c_str());
	if (alGetError() != AL_NO_ERROR) {
		cout << "Error" << endl;
        return;
    }
	ALCcontext *context = alcCreateContext(device, NULL);
	if (alGetError() != AL_NO_ERROR) {
		cout << "Error" << endl;
		return;
	}
	alcMakeContextCurrent(context);
	if (alGetError() != AL_NO_ERROR) {
		cout << "Error" << endl;
		return;
	}
	
	ALuint source, bufferId;
	alGenBuffers(1, &bufferId);
	alGenSources(1, &source);
	if (alGetError() != AL_NO_ERROR) {
		cout << "Error" << endl;
		return;
	}
	
	ALint val;
	while(!input.eof()) {
		//alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
		//if (val <= 0)
		//	continue;
			
		input.read((char *) buffer.data(), 2 * buffer.size());
		alBufferData(bufferId, AL_FORMAT_MONO16, (ALCvoid *)&buffer[0], buffer.size(), SRATE);
		
		//Sound setting variables
		ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };                                    //Position of the source sound
		ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };                                    //Velocity of the source sound
		ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };                                  //Position of the listener
		ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };                                  //Velocity of the listener
		ALfloat ListenerOri[] = { 0.0, 0.0, 1.0,  0.0, 1.0, 0.0 };                  //Orientation of the listener
																					//First direction vector, then vector pointing up) 
		//Listener                                                                               
		alListenerfv(AL_POSITION,    ListenerPos);                                  //Set position of the listener
		alListenerfv(AL_VELOCITY,    ListenerVel);                                  //Set velocity of the listener
		alListenerfv(AL_ORIENTATION, ListenerOri);                                  //Set orientation of the listener
		
		//Source
		alSourcei(source, AL_BUFFER, bufferId);                                     //Link the buffer to the source
		
		alSourcef (source, AL_PITCH,    1.0f);                                      //Set the pitch of the source
		alSourcef (source, AL_GAIN,     2.0f);                                 	    //Set the gain of the source
		alSourcefv(source, AL_POSITION, SourcePos);                                 //Set the position of the source
		alSourcefv(source, AL_VELOCITY, SourceVel);                                 //Set the velocity of the source
		alSourcei (source, AL_LOOPING,  false);                                     //Set if source is looping sound

		//PLAY 
		alSourcePlay(source);                                                       //Play the sound buffer linked to the source
	}
	
	do {
		alGetSourcei(source, AL_SOURCE_STATE, &val);
	} while(val == AL_PLAYING);
	
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &bufferId);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

int main(int argc, char *argv[]) {
	RedirectIOToConsole();
	std::string output = "C:/Projects/audio/pitch.txt";
	//generateWaveform(output);
	playAudio(output);
	
    return 0;
}