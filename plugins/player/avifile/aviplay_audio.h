#ifndef _AUDIO_H
#define _AUDIO_H
#include <config.h>

#include <avifile.h>
#ifdef USE_SDL
#include <SDL.h>
#include <SDL_audio.h>
#endif
#include <except.h>
using namespace std;
class audio_queue
{
public:
    audio_queue();
    ~audio_queue();
    int push(const char* data, int size);
    int write(int fd);
    int read(char* data, int size, int volume);
    int unread(int size);
    void clear();
    int size() const {return frame_size;}
    int wait() //wait until there are any data in buffer
    {
	pthread_mutex_lock(&mutex);
	if(frame_size!=0)
	{
	    pthread_mutex_unlock(&mutex);
	    return 0;
	}    
	pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	return -1;
    }
    void broadcast() {pthread_cond_broadcast(&cond);}
protected:
    pthread_mutex_t mutex;	// Controls access to audio buffer
    pthread_cond_t cond;	// Broadcast to notify audio_thread about
				// new available data OR about quit=1
     
    int frame_size;		// Current amount of data
    static const int frame_max=400000; 
				// Max allowed frame_size
    volatile int frame_in;	
    volatile int frame_out;	// Input and output positions in queue
    unsigned char audio_frame[2*frame_max];
				// Audio queue

};
class IAudioRenderer
{
public:
    virtual double getTime();
    virtual double getLength();
    virtual const char* GetAudioFormat() const;
    virtual void doAudioExtract(double);
    virtual void doAudioSkip(double);
    virtual ~IAudioRenderer();
    virtual void reseek(double pos);
    virtual void start();
    virtual void stop();
    virtual void pause(bool state);
    virtual double buffer_time() =0;	//for how long can we play without doAudioExtract()'s?
    virtual void clear();		//remove all data from buffers
    virtual void wake();		//restart thread if it waits
    virtual int Eof();
    virtual void setAsync(float async);
    virtual void setVolume(float volume);
protected:
    virtual void Pause(int);
    virtual void Reset();
    IAviReadStream* audiostream;
    pthread_mutex_t mutex;
    char audio_desc[100];
    int paused;			// 1 if we are paused
    int hangup;
    int initialized;		// 1 if we are playing OR paused
    long long time_start;	// Timestamp of moment when we started playing
				// updated after each reseek or pause
    long long audio_time;	// Timestamp of moment when last write to audio_fd
				// has finished
    double audio_realpos;	// Precise position of audio at audio_time
    char audio_ext[100];

    WAVEFORMATEX audio;		// format of audio stream
    WAVEFORMATEX owf;		// Format of data we write to audio_fd

    int quit;			// nonzero value signals to thread
				// to terminate
    audio_queue queue;    
    int m_pVolume;
    float m_async;		//by default 0, hardware-dependent
    int reseekflg;		// is seeked ?
    char *localframe;		// audio LocalFrame
};
class AudioRenderer: public IAudioRenderer
{
public:
//    void doAudioExtract(double);
    AudioRenderer(IAviReadStream* stream);
    ~AudioRenderer();
    double buffer_time();	//for how long can we play without doAudioExtract()'s?
				//restart thread if it waits
protected:
    virtual void Reset();
    static void* doAudioOut(void* arg);

    int audio_fd;		// /dev/dsp or ESD socket ( syntactically the same behaviour )

    unsigned Snd_Limit;		// DMA buffer size
    int debug_status; 		// used for debugging console output				
    pthread_t audio_thread;	// Writes data to audio_fd
    
};

#ifdef USE_SDL
void fill_audio(void* userdata, unsigned char* stream, int len);
class SDL_AudioRenderer: public IAudioRenderer
{
public:
    SDL_AudioRenderer(IAviReadStream* stream);
    ~SDL_AudioRenderer();
    double buffer_time();	//for how long can we play without doAudioExtract()'s?
    void wake();
protected:
    static void fill_audio(void* userdata, unsigned char* stream, int len);
    virtual void Pause(int v);
    
    SDL_AudioSpec m_spec;
    int first_read;
};
#endif /*USE_SDL*/
class Copy_AudioRenderer: public IAudioRenderer
{
public:
    Copy_AudioRenderer(IAviReadStream* stream, AUDIOFUNC func);
    ~Copy_AudioRenderer(){}
    //double buffer_time();	//for how long can we play without doAudioExtract()'s?
    virtual void doAudioExtract(double);
protected:

    AUDIOFUNC _af;    
};
#endif
