/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreMIDI/CoreMIDI.h>
#include <pthread.h>
#include <unistd.h>

static OSStatus LoadSMF(const char *filename, MusicSequence& sequence, MusicSequenceLoadFlags loadFlags, bool useV1);
static OSStatus GetSynthFromGraph (AUGraph & inGraph, AudioUnit &outSynth);
static OSStatus SetFrameSize (AUGraph &inGraph, UInt32 frameSize);
static OSStatus PathToFS(const char *filename, FSSpec *outSpec, FSRef *outRef);

#define PLAY_CMD 		"[-p] Play the Sequence\n\t"
#define MIDI_CMD 		"[-e] Use a MIDI Endpoint\n\t"
#define START_TIME_CMD 	"[-t startTime-Beats]\n\t"
#define NUM_FRAMES_CMD 	"[-f frameSize] default is 512\n\t"
#define SRATE_CMD 		"[-s sampleRate] default is 44.1KHz\n\t"
#define SMF_CHAN_CMD 	"[-c] Will Parse MIDI file into channels\n\t"
#define NO_PRINT_CMD 	"[-n] Don't print the sequence\n\t"
#define BANK_CMD 		"[-b /Path/To/Sound/Bank.dls]\n\t"
#define DISK_STREAM		"[-d] Turns disk streaming on\n\t"
#define V1_CMD			"[-v1] Uses V1 version of default graph]\n\t"
#define WAIT_CMD		"[-w] Play for 10 seconds, then dispose all objects and wait at end]\n\t"

#define SRC_FILE_CMD 	"/Path/To/File.mid"

static char* usageStr = "Usage: PlaySequence\n\t" 
				PLAY_CMD 
				MIDI_CMD
				START_TIME_CMD
				NUM_FRAMES_CMD
				SRATE_CMD
				SMF_CHAN_CMD
				NO_PRINT_CMD
				BANK_CMD
				DISK_STREAM
				V1_CMD
				WAIT_CMD
				SRC_FILE_CMD;
							
UInt64	startRunningTime;

UInt32 didOverload = 0;
UInt64	overloadTime = 0;

int main (int argc, const char * argv[]) 
{
	if (argc == 1) {
		fprintf (stderr, "%s\n", usageStr);
		exit(0);
	}
	
	char* filePath = 0;
	bool shouldPlay = false;
	bool shouldSetBank = false;
    bool shouldUseMIDIEndpoint = false;
	bool shouldPrint = true;
	bool useV1 = false;
	bool wait = false;
	bool diskStream = false;
	
	MusicSequenceLoadFlags	loadFlags = 0;
	
	char* bankPath = 0;
	Float32 startTime = 0;
	UInt32 frameSize = 512;
	Float64 sampleRateToUse = 44100;
	
	for (int i = 1; i < argc; ++i)
	{
		if (!strcmp ("-p", argv[i]))
		{
			shouldPlay = true;
		}
		else if (!strcmp ("-w", argv[i]))
		{
			wait = true;
		}
		else if (!strcmp ("-d", argv[i]))
		{
			diskStream = true;
		}
		else if (!strcmp ("-b", argv[i])) 
		{
			shouldSetBank = true;
			if (++i == argc) goto malformedInput;
			bankPath = const_cast<char*>(argv[i]);
		}
		else if (!strcmp ("-n", argv[i]))
		{
			shouldPrint = false;
		}
		else if ((argv[i][0] == '/') && (filePath == 0))
		{
			filePath = const_cast<char*>(argv[i]);
		}
		else if (!strcmp ("-s", argv[i])) 
		{
			if (++i == argc) goto malformedInput;
			sscanf (argv[i], "%lf", &sampleRateToUse);
		}
		else if (!strcmp ("-t", argv[i])) 
		{
			if (++i == argc) goto malformedInput;
			sscanf (argv[i], "%f", &startTime);
		}
		else if (!strcmp("-e", argv[i]))
        {
            shouldUseMIDIEndpoint = true;
        }
		else if (!strcmp("-c", argv[i]))
        {
            loadFlags = kMusicSequenceLoadSMF_ChannelsToTracks;
        }
        else if (!strcmp ("-f", argv[i])) 
		{
			if (++i == argc) goto malformedInput;
			sscanf (argv[i], "%ld", &frameSize);
		}
		else if (!strcmp("-v1", argv[i]))
        {
            useV1 = true;
        }
		else
		{
malformedInput:
			fprintf (stderr, "%s\n", usageStr);
			exit (1);
		}
	}
	
	if (filePath == 0) {
		fprintf (stderr, "You have to specify a MIDI file to print or play\n");
		fprintf (stderr, "%s\n", usageStr);
		exit (1);
	}
	
	MusicSequence sequence;
	OSStatus result;
	
	require_noerr (result = LoadSMF (filePath, sequence, loadFlags, useV1), fail);
			
	if (shouldPrint) 
		CAShow (sequence);
	
	if (shouldPlay)
	{
        AUGraph graph = 0;
        AudioUnit theSynth = 0;
        
        if (shouldUseMIDIEndpoint) 
		{
			MIDIClientRef	theMidiClient;
			MIDIClientCreate(CFSTR("Play Sequence"), NULL, NULL, &theMidiClient);		
            
			ItemCount destCount = MIDIGetNumberOfDestinations();
            if (destCount == 0) {
                fprintf (stderr, "No MIDI Endpoints to play to.\n");
                exit(1);
            }
            
            require_noerr (result = MusicSequenceSetMIDIEndpoint (sequence, MIDIGetDestination(0)), fail);
        } 
		else 
		{    			
			require_noerr (result = MusicSequenceGetAUGraph (sequence, &graph), fail);
			
			require_noerr (result = AUGraphOpen (graph), fail);
			
			require_noerr (result = GetSynthFromGraph (graph, theSynth), fail);

            if (shouldSetBank)
            {                
                FSSpec soundBankSpec;
				FSRef soundBankRef;
				
                require_noerr (result = PathToFS (bankPath, &soundBankSpec, &soundBankRef), fail);
                
                printf ("Setting Sound Bank:%s\n", bankPath);
					
					// try the FSRef property first (as FSSpec's are deprecated with Tiger)
                if (result = AudioUnitSetProperty (theSynth,
										kMusicDeviceProperty_SoundBankFSRef,
										kAudioUnitScope_Global, 0,
										&soundBankRef, sizeof(soundBankRef)))
				{
					require_noerr (result = AudioUnitSetProperty (theSynth,
											kMusicDeviceProperty_SoundBankFSSpec,
											kAudioUnitScope_Global, 0,
											&soundBankSpec, sizeof(soundBankSpec)), fail);
				}
			}
			
			require_noerr (result = AudioUnitSetProperty (theSynth,
									kAudioUnitProperty_SampleRate,
									kAudioUnitScope_Output, 0,
									&sampleRateToUse, sizeof(sampleRateToUse)), fail);
			
			if (diskStream) {
				UInt32 value = diskStream;
				require_noerr (result = AudioUnitSetProperty (theSynth,
									kMusicDeviceProperty_StreamFromDisk,
									kAudioUnitScope_Global, 0,
									&value, sizeof(value)), fail);
			}
									
			require_noerr (result = AUGraphInitialize (graph), fail);

			require_noerr (result = SetFrameSize (graph, frameSize), fail);

            if (shouldPrint)
				CAShow (graph);
        }
        
		MusicPlayer player;
		require_noerr (result = NewMusicPlayer (&player), fail);

		require_noerr (result = MusicPlayerSetSequence (player, sequence), fail);

	// figure out sequence length
		UInt32 ntracks;
		require_noerr(MusicSequenceGetTrackCount (sequence, &ntracks), fail);
		MusicTimeStamp sequenceLength = 0;
		for (UInt32 i = 0; i < ntracks; ++i) {
			MusicTrack track;
			MusicTimeStamp trackLength;
			UInt32 propsize = sizeof(MusicTimeStamp);
			require_noerr (result = MusicSequenceGetIndTrack(sequence, i, &track), fail);
			require_noerr (result = MusicTrackGetProperty(track, kSequenceTrackProperty_TrackLength,
							&trackLength, &propsize), fail);
			if (trackLength > sequenceLength)
				sequenceLength = trackLength;
		}
	
	// now I'm going to add 10 beats on the end for the reverb/long releases to tail off...
		sequenceLength += 10;
		
		require_noerr (result = MusicPlayerSetTime (player, startTime), fail);
		
		require_noerr (result = MusicPlayerPreroll (player), fail);
		
		if (shouldPrint) {
			if (!shouldUseMIDIEndpoint) {
				UInt32 value;
				UInt32 size = sizeof(value);
				require_noerr (result = AudioUnitGetProperty (theSynth,
									kMusicDeviceProperty_StreamFromDisk,
									kAudioUnitScope_Global, 0,
									&value, &size), fail);
				printf ("Disk Streaming is enabled: %c\n", (value ? 'T' : 'F'));
			}
			printf ("Ready to play <%f beats>:%s\n\t<Enter> to continue: ", startTime, filePath); 

			getc(stdin);
		}
		
		startRunningTime = AudioGetCurrentHostTime ();
		
		require_noerr (result = MusicPlayerStart (player), fail);

		if (shouldPrint) {
			MusicTimeStamp time;
			require_noerr (result = MusicPlayerGetTime (player, &time), fail);
			printf ("Started Playing:%s, %f beats\n", filePath, time);
		}
		
		int waitCounter = 0;
		while (1) {
			usleep (1 * 1000 * 1000);
			
			if (didOverload) {
				printf ("* * * * * %ld Overloads detected on device playing audio\n", didOverload);
				overloadTime = AudioConvertHostTimeToNanos (overloadTime - startRunningTime);
				printf ("\tSeconds after start = %lf\n", double(overloadTime / 1000000000.));
				didOverload = 0;
			}

			if (wait && ++waitCounter > 10) break;
			
			MusicTimeStamp time;
			require_noerr (result = MusicPlayerGetTime (player, &time), fail);
						
			if (shouldPrint) {
				printf ("current time:%f beats,", time);
				if (!shouldUseMIDIEndpoint) {
					Float32 load;
					require_noerr (result = AUGraphGetCPULoad(graph, &load), fail);
					printf ("CPU load = %f\n", load);
				} else
					printf ("\n"); //no cpu load on AUGraph - its not running - if just playing out to MIDI
			}
			
			if (time >= sequenceLength)
				break;
		}
		
		require_noerr (result = MusicPlayerStop (player), fail);
		if (shouldPrint) printf ("finished playing\n");
			
// this shows you how you should dispose of everything
		require_noerr (result = DisposeMusicPlayer (player), fail);
		require_noerr (result = DisposeMusicSequence(sequence), fail);
	// don't own the graph so don't dispose it (the seq owns it....)
	
	}
	else {
		require_noerr (result = DisposeMusicSequence(sequence), fail);
	}
	
	while (wait)
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.25, false);
		
    return 0;
	
fail:
	if (shouldPrint) printf ("Error = %ld\n", result);
	return result;
}

OSStatus GetSynthFromGraph (AUGraph& inGraph, AudioUnit& outSynth)
{	
	UInt32 nodeCount;
	OSStatus result = noErr;
	bool found = false;
	require_noerr (result = AUGraphGetNodeCount (inGraph, &nodeCount), home);
	
	for (UInt32 i = 0; i < nodeCount; ++i) 
	{
		AUNode node;
		require_noerr (result = AUGraphGetIndNode(inGraph, i, &node), home);

		ComponentDescription desc;
		require_noerr (result = AUGraphGetNodeInfo(inGraph, node, &desc, 0, 0, 0), home);
		
			// search for either a V1 or V2 version of the synth
		if (desc.componentType == kAudioUnitType_MusicDevice
				|| (desc.componentType == 'aunt' && desc.componentSubType == kAudioUnitSubType_MusicDevice)) 
		{
			require_noerr (result = AUGraphGetNodeInfo(inGraph, node, 0, 0, 0, &outSynth), home);
			found = true;
			break;
		}
	}
	
	if (!found) result = -1;
home:	
	return result;
}

OSStatus OverlaodListenerProc(	AudioDeviceID			inDevice,
								UInt32					inChannel,
								Boolean					isInput,
								AudioDevicePropertyID	inPropertyID,
								void*					inClientData)
{
	didOverload++;
	overloadTime = AudioGetCurrentHostTime();
	return noErr;
}


OSStatus SetFrameSize (AUGraph &inGraph, UInt32 frameSize)
{
	UInt32 nodeCount;
	OSStatus result = noErr;
	AudioUnit theOutputUnit = 0;
	UInt32 theSize;
	require_noerr (result = AUGraphGetNodeCount (inGraph, &nodeCount), home);
	
	for (UInt32 i = 0; i < nodeCount; ++i) 
	{
		AUNode node;
		require_noerr (result = AUGraphGetIndNode(inGraph, i, &node), home);

		ComponentDescription desc;
		require_noerr (result = AUGraphGetNodeInfo(inGraph, node, &desc, 0, 0, 0), home);
		
		if (desc.componentType == kAudioUnitType_Output || 
			(desc.componentType == 'aunt' && desc.componentSubType == 'out ')) 
		{
			require_noerr (result = AUGraphGetNodeInfo(inGraph, node, 0, 0, 0, &theOutputUnit), home);
			break;
		}		
	}

	AudioDeviceID theDevice;
	theSize = sizeof (theDevice);
	require_noerr (result = AudioUnitGetProperty (theOutputUnit, 
								kAudioOutputUnitProperty_CurrentDevice, 
								0, 0, &theDevice, &theSize), home);
	
	theSize = sizeof (frameSize);
	require_noerr (result = AudioDeviceSetProperty (theDevice, 
										0, 0, false, 
										kAudioDevicePropertyBufferFrameSize, 
										theSize, &frameSize), home);
	
	require_noerr (result = AudioDeviceAddPropertyListener(theDevice, 0, false,
								kAudioDeviceProcessorOverload, OverlaodListenerProc, 0), home);
home:
	return result;
}

OSStatus LoadSMF(const char *filename, MusicSequence& sequence, MusicSequenceLoadFlags loadFlags, bool useV1)
{
	OSStatus result = noErr;
	
	require_noerr (result = NewMusicSequence(&sequence), home);

	FSSpec fsSpec;
	FSRef fsRef;
	require_noerr (result = PathToFS (filename, &fsSpec, &fsRef), home);
	
	if (useV1) {
			// this is deprecated (FSSpec as of Tiger)
		require_noerr (result = MusicSequenceLoadSMF (sequence, &fsSpec), home);
	} else {
		require_noerr (result = MusicSequenceLoadSMFWithFlags (sequence, &fsRef, loadFlags), home);
	}
	
home:
	return result;
}

OSStatus PathToFS(const char *filename, FSSpec *outSpec, FSRef *outRef)
{	
	OSStatus result = noErr;

	require_noerr (result = FSPathMakeRef ((const UInt8*)filename, outRef, 0), home);
				
	require_noerr (result = FSGetCatalogInfo(outRef, kFSCatInfoNone, NULL, NULL, outSpec, NULL), home);

home:
	return result;
}
