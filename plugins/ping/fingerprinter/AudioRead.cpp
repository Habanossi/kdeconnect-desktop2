#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <iostream>	      
#include <fstream>
#include <vector>
#include <QFile>
#include <QDebug>
#include <QTextStream>
using namespace std;

void AudioRead(QString directory, QString file)
{
  	int i;
  	int err;  
  
  	unsigned int rate = 16000;		//44100;
	int buffer_frames = 2;
  	snd_pcm_t *capture_handle;
  	snd_pcm_hw_params_t *hw_params;
 	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	

  	if ((err = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    	cerr << "cannot open audio device " << " (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "audio interface opened\n";
		   
  	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    	cerr << "cannot allocate hardware parameter structure (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "hw_params allocated\n";
				 
  	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    	cerr << "cannot initialize hardware parameter structure (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "hw_params initialized\n";
	
  	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    	cerr << "cannot set access type (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "hw_params access setted\n";
	
  	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
    	cerr << "cannot set sample format (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "hw_params format setted\n";
	
  	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    	cerr << "cannot set sample rate (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}
	
  	cout << "hw_params rate setted\n";

  	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
    	cerr << "cannot set channel count (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "hw_params channels setted\n";
	
  	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    	cerr << "cannot set parameters (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "hw_params setted\n";
	
  	snd_pcm_hw_params_free (hw_params);

  	cout << "hw_params freed\n";
	
  	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    	cerr << "cannot prepare audio interface for use (" << snd_strerror (err) << ")\n";
    	exit (1);
  	}

  	cout << "audio interface prepared\n";

  	int buffer[buffer_frames * snd_pcm_format_width(format) / 8 * 2];    //bytes from two channels * frames per s ??

  	cout << "buffer allocated\n";



	QFile outFile(directory + file);
	if(!outFile.exists()) qDebug() << "OUTFILE DOES NOT EXIST";
	if(!outFile.open(QIODevice::WriteOnly | QFile::Truncate)){
		qDebug() << "something wrong with outFile (AudioRead())";
	}
	QTextStream out(&outFile);
  	for (i = 0; i < rate * buffer_frames; ++i) {
    	if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
      		cerr << "read from audio interface failed \n";
      		exit (1);
    	}
    	//cout << "read " << i << " done\n";
		out << *buffer/65536 << "\n";
	}
	outFile.close();
	snd_pcm_close (capture_handle);
	cout << "audio interface closed\n";

  return;
}
