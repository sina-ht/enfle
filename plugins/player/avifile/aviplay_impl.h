#include <config.h>

#include <ctype.h>
#include <aviplay.h>
#include <avifile.h>
#include <videodecoder.h>
#include <videoencoder.h>
#include <audiodecoder.h>
#include <vector>
#include <except.h>
#include "aviplay_audio.h"
using namespace std;

#define STAT_SIZE 25

class Statistic
{
    struct field
    {
	int start;
	int size;
        int max_size;
	char* name;
	float* values;
	field():name(0), values(0), max_size(0) {}
	field(char* name, int msize=STAT_SIZE):start(0), size(0), max_size(msize)
	{
            if(max_size<=0)max_size=STAT_SIZE;
	    this->name=new char[strlen(name)+1];
	    strcpy(this->name, name);
	    values=new float[max_size];
	}
	field(const field& p):name(0), values(0), max_size(0)
	{

	    if(p.name)
	    {
		name=new char[strlen(p.name)+1];
		strcpy(name, p.name);
	    }
	    if(p.values)
	    {
		values=new float[p.max_size];
		memcpy(values, p.values, sizeof(float)*p.max_size);
	    }
	    start=p.start;
	    size=p.size;
            max_size=p.max_size;
	}
	~field()
	{
	    if(name)delete[] name;
	    if(values)delete[] values;
	}
	float average() const
	{
	    float sum=0;
	    if(size==0)
		return 0;
	    if(values==0)
		return 0;
	    int i;
	    if(size!=max_size)
		for(i=(start%max_size); i!=((start+size)%max_size); i++,i%=max_size)
		    sum+=values[i];
		else
		for(i=0; i<max_size; i++)
		    sum+=values[i];

	    return sum/size;
	}
	void insert(float v)
	{
	    if(values==0)
		return;
	    values[(start+size)%max_size]=v;
	    if(size<max_size)
		size++;
	    else
		start++;
	    start%=max_size;
	}
    };
    vector<field> fields;
public:
    Statistic()
    {
	fields.clear();
    }
    void AddField(char* name, int freq=0)
    {
	field f(name, freq);
	fields.push_back(f);
    }
    void Insert(char* name, float value)
    {
	int i;
	for(i=0; i<fields.size(); i++)
	if(strcmp(fields[i].name, name)==0)
	{
	    fields[i].insert(value);
	    return;
	}
    }
    float average(const char* name) const
    {
	int i;
	for(i=0; i<fields.size(); i++)
	    if(strcmp(fields[i].name, name)==0)
		return fields[i].average();
	return 0;	    
    }
/*    QString string()
    {
	char* buffer=new char[50*fields.size()];
	buffer[0]=0;
	QString answer;
        	for(int i=0; i<fields.size(); i++)
	{
	    if(i!=0)
		strcat(buffer, "/");
//		answer+=";";
	    sprintf(buffer+strlen(buffer), "%.1f", fields[i].average());
//	    answer+=QString(fields[i].name);
//	    answer+=": ";
//	    answer+=QString(fields[i].average());
//	    answer+=" ms ";
	}
	answer=QString(buffer);
//	printf("%s\n", buffer);
	return answer;
    }*/
};

/***************************************************

   Here aviplay interface goes
   
****************************************************/  

struct subtitle_line
{
	int start;
	int end;
	char name[512];
	subtitle_line():start(0), end(0){name[0]=0;}
	int read(FILE* file, double frame_time)
	{
		int dummy, sh, sm, ss, su, eh, em, es, eu;
		int c=0;
		if (fscanf(file, "{%d}{%d}", &start, &end) == 2)
		{
		    char* pos=name;
		    // we copy as much as we can
		    while ((c = fgetc(file)) != EOF && c != '\r' && c != '\n')
			if(pos-name < sizeof(name)-1)
			    *pos++=c;
		    // zero terminate
		    *pos = '\0';
		    // skip newlines
		    while (c == '\r' || c == '\n')
			c = fgetc(file);
		    ungetc(c, file);	
		    return 0;
		}
		else if (fscanf(file, "%d", &dummy) == 1)
		{
		    // we must have frame_time!
		    if (!frame_time)
			return -1;
		    // we skip nl
		    do 
			c = fgetc(file);
		    while (c != EOF && c != '\n');
		    char buffer[512];
		    if (!fgets(buffer, sizeof(buffer), file) ||
			sscanf(buffer, "%d:%d:%d,%d --> %d:%d:%d,%d", 
				&sh, &sm, &ss, &su, &eh, &em, &es, &eu) != 8)
			return -1;
		    start = (int)((((double)sh*60 + sm)*60 + ss + (double)su/1000.0) / frame_time);
		    end   = (int)((((double)eh*60 + em)*60 + es + (double)eu/1000.0) / frame_time);
		    // now there can be 0, 1 or 2 lines of subtitles... ended by a newline
		    memset(name, '\0', 512);
		    char* pos=name;
		    while (fgets(buffer, sizeof(buffer), file))
		    {
			// we skip whitespace and check whether the line's empty
			char *p = buffer;
			while (isspace(*p))
			    p++;
			if (*p == '\0')
			    break;	// this was a blank line -> end of titles here
			// line's not empty, we add as much as we can to name
			p = buffer;
			while (*p && pos-name < sizeof(name)-1)
			{
			    if (*p == '\n')
				*pos++='|';
			    else if (*p != '\r')
				*pos++=*p;
			    
			    p++;
			}
		    }
		    return 0;
		}
		return -1;
	}
};


/*
typedef void (*DRAWFUNC)(char*);
typedef void (*KILLHANDLER)(int);*/
void* main_thread(void*);
void* main_thread_novideo(void*);
class AviPlayer: public IAviPlayer
{
    friend void* main_thread(void*);
    friend void* main_thread_novideo(void*);
protected:
    double getAsync(int update=0);
    DRAWFUNC drawfunc;
    DRAWFUNC2 drawfunc2;
    KILLHANDLER killhandler;
    AUDIOFUNC audiofunc;
    IAviReadFile* clip;
 public:
    IAviReadStream* stream;
    IAviReadStream* audiostream;
 protected:
    Statistic drop;
    string _fn;
    vector<subtitle_line> subtitles;
    int initSubtitles(const char* filename);
    int HasSubtitles();
    float m_fVolume;    
public:
    AviPlayer();
    ~AviPlayer();
    int initPlayer(const char* filename, int bitdepth, const char*) throw(FatalError);
    int setColorSpace(int csp, bool test_only);
    void endPlayer();
    
    int width(){return m_width;}
    int height(){return m_height;}
    void setDrawCallback(DRAWFUNC func)
    {
        drawfunc=func;
    }
    void setDrawCallback2(DRAWFUNC2 func)
    {
        drawfunc2=func;
    }
    void setKillHandler(KILLHANDLER handler)
    {
	killhandler=handler;
    }
    void setAudioFunc(AUDIOFUNC func)
    {
	audiofunc=func;
    }
    double GetPos()
    {
	if(stream)
	    return stream->GetTime();
	if(ar)
	    return ar->getTime();
	return 0;
    }	    
    double reseek(double pos);
    int reseek_exact(double pos);
    int page_up();
    int page_down();
    void play(){pause(0);time_start=0;}
    void start();
    void stop();
    void pause(bool state)
    {
	if(!initialized)return;
        if(!state)time_start=0;
        paused=state;
	if(ar)ar->pause(state);
    }
    const char* GetCurrentSubtitles();
    double GetAudioLength() const
    {
        if(ar)return ar->getLength();
        return 0;
    }
    double GetVideoLength() const
    {
        if(stream)return stream->GetEndTime();
        return 0;
    }    
    double GetDrop() const
    {
        return drop.average("Drop");
    }
    int isValid(){return (clip!=NULL);};
    int isStopped()
    {
	if(clip==NULL)return 0;
	if(initialized==0)return 1;
	return 0;
    }	
    int isPlaying()
    {
	return initialized;
    }	
    int isPaused(){return paused;};
    
    const char* GetAudioFormat() const
    {
	if(ar) return ar->GetAudioFormat();
	return 0;
    }
    const char* GetVideoFormat() const
    {
	if(!stream)return "";
	BITMAPINFOHEADER bh;
	stream->GetVideoFormatInfo(&bh);
	const CodecInfo* ci=CodecInfo::match(bh.biCompression);
	if(!ci)return "unknown";
	return ci->text.c_str();
    }
    int fps()
    {
	if(!stream)return 0;
	double t=stream->GetFrameTime();
	return (int)(1./t);
    }
    virtual const char* fileName()
    {
	return _fn.c_str();
    }
    virtual void setAsync(float async)
    {
	m_async=async;
	if(ar)ar->setAsync(async);
    }
    virtual float readAsync(){return m_async;}
    virtual void setVolume(float volume);
    virtual float readVolume(){return m_fVolume;}
    virtual void setSubAsync(float async){m_sub_async=async;}
    virtual float readSubAsync(){return m_sub_async;}    
    virtual const CodecInfo& GetCodecInfo() const
    {
	return stream->GetDecoder()->GetCodecInfo();
    }
    virtual void Restart();
    virtual IRtConfig* GetRuntimeConfig(){return dynamic_cast<IRtConfig*>(stream->GetDecoder());}

    virtual void setVideo(int v){video_mute=v;}
    virtual void setAudio(int v){audio_mute=v;}
    virtual int getVideo(){return video_mute;}
    virtual int getAudio(){return audio_mute;}

protected:
    int video_mute;
    int audio_mute;

    short fs_seg;
    float m_async;
    float m_sub_async;
    IAudioRenderer* ar;
    AudioRenderer* _qwerty;
    int frames_video;		//Just counters
    int frame_drop;
    
    int paused;			// 1 if we are paused
    int initialized;		// 1 if we are playing OR paused
    
    int hangup;			// Signals main_thread to enter 'waiting' state
				// and set initialized=0

    int m_width;
    int m_height;

    char bitmapinfo[100];	// BITMAPINFOHEADER

    //	These vars are used in syncing video with time
    long long time_start;	// Timestamp of moment when we started playing
				// updated after each reseek or pause
    double frame_start;		// Precise position of video at time_start ( needed
				// for timing of video-only streams ) 
    int debug_status; 		// used for debugging console output
	 
    int quit;			// nonzero value signals to all processing threads
				// to terminate
				
    pthread_t main_thread;	// Performs decompression, output and sync
    
    int video_sync;		// 0 here means that player will display all frames, not caring about a/v sync
};

void CtrlCHandler(int);
