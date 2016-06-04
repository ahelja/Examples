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
/*=============================================================================
	AUMixer3DView.cpp
	
=============================================================================*/

#include "AUMixer3DView.h"

#include <math.h>
#include <AudioUnit/AudioUnitProperties.h>

COMPONENT_ENTRY(AUMixer3DView)

// prototypes
extern void SetInput(int n);
extern void SetFileInput(int n);

UInt32 gCurrentHRTF = 0;


class TrackingLine
{
public:
	TrackingLine(const RGBColor &inColor, int inOffsetX, int inOffsetY, int inWidth, int inHeight )
		: mColor(inColor), width(inWidth), height(inHeight)
	{
		viewOffsetX = inOffsetX;
		viewOffsetY = inOffsetY;
		
		
		centerx = 0.5*width;
		centery = 0.5*height;
		
		lastAngle = 0.0;
		anchorAngle = 0.0;
		anchorDistance = 150.0;
		CartesianFromAngle(lastAngle, anchorDistance, lastx, lasty );
		anchorx = lastx;
		anchory = lasty;
	}
	
	void			Track(float x, float y )
	{
		float angle;
		CartesianToAngle(x, y, angle );

		//RGBColor color = {65535,65535,65535};
		Draw(sBackgroundColor);
				
		lastx = x;
		lasty = y;
		lastAngle = angle;

		Anchor();
		
		Draw(mColor);
	};
	

	void		Anchor()
	{
		anchorx = lastx;
		anchory = lasty;
		anchorAngle = lastAngle;
		anchorDistance = Distance(centerx, centery);
	}	

	void			Draw(const RGBColor &inColor  )
	{
		RGBForeColor(&inColor);
		DoDraw();
	}
	
	void			Draw()
	{
		RGBForeColor(&mColor);
		DoDraw();
	}
        
	void			DoDraw()
	{
		const int k = 5;
		
		if( !(lastx - k < 0 || lastx + k > 400
			|| lasty - k < 0 || lasty + k > 400) )	// don't draw outside bounds...
		{
			Rect r;
			r.left = short(viewOffsetX + lastx - k);
			r.right = short(viewOffsetX + lastx + k);
			r.top = short(viewOffsetY + lasty - k);
			r.bottom = short(viewOffsetY + lasty + k);
			PaintOval(&r);
		}
	}
		
		
	float		Rotate(float inRotationAngle )
	{
		Draw(sBackgroundColor);

		float distance = Distance(centerx, centery);
		
		float newAngle = anchorAngle + inRotationAngle;
		if(newAngle > 180.0) newAngle -= 360.0;
		if(newAngle < -180.0) newAngle += 360.0;
		
		CartesianFromAngle(newAngle, distance, lastx, lasty );
		
		lastAngle = newAngle;
		
		Draw(mColor);
		
		return lastAngle;
	}
	
	float		ScaleDistance(float inScale )
	{
		Draw(sBackgroundColor);

		float newDistance = anchorDistance * inScale;
		
		CartesianFromAngle(lastAngle, newDistance, lastx, lasty );
		
		
		Draw(mColor);
		
		return newDistance;
	}
	
	void		Offset(float inOffsetX, float inOffsetY, float &outAngle, float &outDistance )
	{
		Draw(sBackgroundColor);

		lastx = anchorx +  inOffsetX;
		lasty = anchory +  inOffsetY;
		
		CartesianToAngle(lastx, lasty, lastAngle );
		
		Draw(mColor);


		outDistance = Distance(centerx,centery);
		
		outAngle = lastAngle;
	}
	
#if 0
	void		MoveRandomly(float &outAngle, float &outDistance )
	{
		Draw(sBackgroundColor);

		const long kWiggle = 20L;
		float deltax = GetRandomLong(kWiggle) - 0.5*kWiggle;
		float deltay = GetRandomLong(kWiggle) - 0.5*kWiggle;
		
		deltax += lastx-centerx;
		deltay += lasty-centery;
		
		deltax *= 0.99;
		deltay *= 0.99;
		
		lastx = deltax+centerx;
		lasty = deltay+centery;
		
		
		CartesianToAngle(lastx, lasty, lastAngle );
		
		Draw(mColor);
		
		outDistance = Distance(centerx,centery);
		
		outAngle = lastAngle;
	}
#endif
	
	// returns distance between this point and our last point
	float			Distance(float x, float y )
	{
		//return fabs( inAngle - lastAngle );
                return sqrt((x-lastx)*(x-lastx) + (y-lasty)*(y-lasty) );
	};
	
	float			GetAngle()
	{
		return lastAngle;
	};


	static void		SetBackgroundColor(const RGBColor &inColor) {sBackgroundColor = inColor;};
	
private:
	void			CartesianFromAngle(float inAngle, float inDistance, float &outX, float &outY )
	{
		float rangle = 3.14159 * inAngle/180.0;
		outX = centerx + sin(rangle) * inDistance;
		outY = centery - cos(rangle) * inDistance;
	};
	
	void			CartesianToAngle(float inX, float inY, float &outAngle )
	{
		float x = inX - centerx;
		float y = -(inY - centery);
		
		float rangle = atan2(x,y);
		outAngle = 180.0 * rangle / 3.14159;
	};
	
	RGBColor		mColor;

	int				viewOffsetX;
	int				viewOffsetY;
	
	int				width;
	int				height;
	float				lastx;
	float				lasty;
	
	float				centerx;
	float				centery;
	
	float			lastAngle;
	float			anchorAngle;
	float			anchorx;
	float			anchory;
	float			anchorDistance;

	static RGBColor	sBackgroundColor;

};

RGBColor	TrackingLine::sBackgroundColor = {65535, 65535, 65535};


const int kNTrackers = 3;
TrackingLine *gTrackers[3] = {NULL,NULL,NULL};

TrackingLine *gListener = NULL;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	DrawPowerMeter
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void DrawPowerMeter(float avePower, float peakValue, int yoffset )
{
	if(avePower < -50.0 ) avePower = -50.0;		// only show from -50 -> 0dB
	float scaledPower = 200.0 * (avePower + 50.0) / 50.0;
	
	if(peakValue < -50.0 ) peakValue = -50.0;		// only show from -50 -> 0dB
	float scaledPeak = 200.0 * (peakValue + 50.0) / 50.0;
	
	RGBColor color;
	color.red = 0;
	color.blue = 0;
	color.green = 65535;
	RGBForeColor(&color);
	
	const int xoffset = 20;
	
	
	Rect powerRect = {yoffset, xoffset, yoffset+8, xoffset + int(scaledPower) };
	PaintRect(&powerRect);
	
	powerRect.left = xoffset + int(scaledPower);
	powerRect.right = xoffset + 200;
	color.red = 32768;
	color.blue = 32768;
	RGBForeColor(&color );
	PaintRect(&powerRect);
	
	color.red = 40000;
	color.blue = 40000;
	color.green = 40000;
	RGBForeColor(&color );
	MoveTo(xoffset + int(scaledPeak), yoffset );
	LineTo(xoffset + int(scaledPeak), yoffset+7 );
}

AUMixer3DView *gAUMixer3DView = NULL;


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	MixerTimerProc
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MixerTimerProc(EventLoopTimerRef inTimer, void *inUserData)
{
//printf("MixerTimerProc() : %x, %x\n", inUserData, gAUMixer3DView );

	AUMixer3DView *This = (AUMixer3DView*)gAUMixer3DView;


	
	CGrafPtr gp = GetWindowPort(This->GetCarbonWindow() );

//printf("gp = %x\n", gp );
	
#if 1
	CGrafPtr save;
	GetPort(&save);
	SetPort(gp );

// output metering
	for(int i = 0; i < 5; i++ )
	{
		Float32 avePower;
		Float32 peak;
		AudioUnitGetParameter(This->GetEditAudioUnit(),
			1000+i /* k3DMixerParam_PreAveragePower */,
			kAudioUnitScope_Output,
			0,
			&avePower );
		
		AudioUnitGetParameter(This->GetEditAudioUnit(),
			2000+i /* peak */,
			kAudioUnitScope_Output,
			0,
			&peak );
		
		DrawPowerMeter(avePower, peak, 470 + i*10 );
	}
	

// input metering
	for(int i = 0; i < 3; i++ )
	{
		Float32 avePower;
		Float32 peak;
		AudioUnitGetParameter(This->GetEditAudioUnit(),
			3000,
			kAudioUnitScope_Input,
		i,
			&avePower );
		
		AudioUnitGetParameter(This->GetEditAudioUnit(),
			4000,
			kAudioUnitScope_Input,
			i,
			&peak );
		
		DrawPowerMeter(avePower, peak, 420 + i*10 );
	}
	
	SetPort(save );
#endif
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	MyEventHandlerDispatch
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static pascal OSStatus MyEventHandlerDispatch(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	AUMixer3DView *This = (AUMixer3DView*)inUserData;
	
	return This->MyEventHandler(inHandlerCallRef, inEvent );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	MyEventHandler
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AUMixer3DView::MyEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent)
{
	OSStatus result = noErr;
	
	UInt32 eventClass = GetEventClass(inEvent);
	UInt32 eventKind = GetEventKind(inEvent);

	// draw event
	if(eventClass == kEventClassWindow)
	{
		if(eventKind == kEventWindowDrawContent ) 
		{
			Rect r = {mOffsetY, mOffsetX, mOffsetY + 400, mOffsetX + 400};
			RGBForeColor(&mBackgroundColor );
			PaintRect(&r);
	
			for(int i = 0; i < kNTrackers; i++)
			{
				gTrackers[i]->Draw();
			}
			
			gListener->Draw();
	
			
			return noErr;
		}
	}



	CGrafPtr gp = GetWindowPort(GetCarbonWindow());
	
	CGrafPtr save;
	GetPort(&save);
	SetPort(gp );



	const float kDistanceScale = 0.1 /*0.3*/;


/*
	k3DMixerRenderingFlags_InterAuralDelay			= (1L << 0),
	k3DMixerRenderingFlags_DopplerShift				= (1L << 1),
	k3DMixerRenderingFlags_DistanceAttenuation		= (1L << 2),
	k3DMixerRenderingFlags_DistanceFilter			= (1L << 3),
	k3DMixerRenderingFlags_DistanceDiffusion		= (1L << 4)
*/	
	if(eventClass == kEventClassCommand )
	{
		HICommand	command;
		GetEventParameter (inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);

		SInt16 value = ::GetControlValue((ControlRef)command.menu.menuRef );
	
		//char *p = ((char*)&command.commandID );
		//printf("%x command.commandID = %d : %d : %c%c%c%c\n", command.menu.menuRef, command.commandID, int(value), p[0],p[1],p[2],p[3] );

		
		if(command.commandID == 'algo' )
		{
			// set rendering algorithm
			
			UInt32 algo = value - 1;
			
			for(int i = 0; i < kNTrackers; i++)
			{
				AudioUnitSetProperty(
					GetEditAudioUnit(),
					kAudioUnitProperty_SpatializationAlgorithm,
					kAudioUnitScope_Input,
					i,
					&algo,
					sizeof(algo) );
			}
			
			// rendering flags have changed with setting of rendering algorithm
			SetRenderingFlagsCheckboxes();
		}
		else if(command.commandID == 'volm' )
		{
			Float32 s = float(value) / 100.0;
			s = s*s;
			
			if(s == 0.0)
			{
				s = -96.0;
			}
			else
			{
				s = 20.0 * log10(s);
			}
			//printf("volume = %f dB\n", s);

			// set master gain in dB
			AudioUnitSetParameter(GetEditAudioUnit(),
				k3DMixerParam_Gain,
				kAudioUnitScope_Output,
				0,
				s,
				0 );
			
		}
		else if(command.commandID == 'attn' )
		{
			// volume attenuation curve
			
			Float64 s = float(value) / 100.0;
			
			//printf("volume atten curve= %f\n", s);

			// volume attenuation curve follows the law:   1.0 / (1.0 + s *(distance - 1.0) )
			//
			// where distance is in meters, and s is generally between 0.1 and 1.0 (0.3 is good default)
			//
			// there is no attenuation if distance <= 1.0 meter

			for(int i = 0; i < kNTrackers; i++)
			{
				result = AudioUnitSetProperty(	GetEditAudioUnit(),
												kAudioUnitProperty_3DMixerDistanceAtten,
												kAudioUnitScope_Input,
												i,
												&s,
												sizeof(s) );
			}
		}
		else
		{
			UInt32 mask = 0;
			switch(command.commandID )
			{
				case 'atr0':
					mask = k3DMixerRenderingFlags_InterAuralDelay;
					break;
				case 'atr1':
					mask = k3DMixerRenderingFlags_DopplerShift;
					break;
				case 'atr2':
					mask = k3DMixerRenderingFlags_DistanceAttenuation;
					break;
				case 'atr3':
					mask = k3DMixerRenderingFlags_DistanceFilter;
					break;
				case 'atr4':
					mask = k3DMixerRenderingFlags_DistanceDiffusion;
					break;
				case 'rvrb':
				{
					UInt32 usesReverb = value;
					AudioUnitSetProperty(
						GetEditAudioUnit(),
						kAudioUnitProperty_UsesInternalReverb,
						kAudioUnitScope_Input,
						0,
						&usesReverb,
						sizeof(usesReverb) );
						
					break;
				}
			}
			
			if(mask != 0)
			{		
				for(int i = 0; i < kNTrackers; i++)
				{
					UInt32 flags;
					UInt32 size = sizeof(flags);
					
					AudioUnitGetProperty(
						GetEditAudioUnit(),
						kAudioUnitProperty_3DMixerRenderingFlags,
						kAudioUnitScope_Input,
						i,
						&flags,
						&size );
					
					if(value > 0)
					{
						flags |= mask;
					}
					else
					{
						flags &= ~mask;
					}
					
					AudioUnitSetProperty(
						GetEditAudioUnit(),
						kAudioUnitProperty_3DMixerRenderingFlags,
						kAudioUnitScope_Input,
						i,
						&flags,
						size );
				}
				
				return noErr;
			}
		}
		
		return eventNotHandledErr;
	}
	else		
	if(eventClass == kEventClassMouse )
	{

		HIPoint			mousePos;
		result =  GetEventParameter(  	inEvent,
										kEventParamMouseLocation,
										typeHIPoint,
										NULL,       /* can be NULL */
										sizeof(HIPoint),
										NULL,       /* can be NULL */
										&mousePos);

		UInt32	modifiers;
		result =  GetEventParameter(  	inEvent,
										kEventParamKeyModifiers,
										typeUInt32,
										NULL,       /* can be NULL */
										sizeof(UInt32),
										NULL,       /* can be NULL */
										&modifiers);
	
	
		Point pt;
		pt.h = short(mousePos.x);
		pt.v = short(mousePos.y);
		
		
		Rect r2 = GetPortBitMapForCopyBits(gp)->bounds;
		
		GlobalToLocal(&pt);
		pt.h -= mOffsetX;
		pt.v -= mOffsetY;	
		if(!mIsTrackingMouse && (pt.h < 0 || pt.v < 0) ) return eventNotHandledErr;

		// check for title bar
		if(pt.v < 0 && pt.v > -30 )
		{
			return eventNotHandledErr;
		}


		Rect r1;
		GetPortBounds(gp, &r1 );
		//int width = r1.right-r1.left;
		//int height = r1.bottom-r1.top;
		
		//!!@ hardcoded
		int width = 400;
		int height = 400;

		int centerx = int(0.5*width);
		int centery = int(0.5*height);
		
		static int currentLine = 0;
		
		float x = pt.h - centerx;
		float y = -(pt.v - centery);
		
		float rangle = atan2(x,y);
		float angle = 180.0 * rangle / 3.14159;
		
		static int hitx = 0;
		static int hity = 0;
		static float hitAngle = 0.0;


		switch(eventKind)
		{
			case kEventMouseDown:
			{
				mIsTrackingMouse = true;
				
				// determine the closest source

				float bestDistance = 100000.0;
				int bestIndex = 0;
				
				for(int i = 0; i < kNTrackers; i++)
				{
					if(gTrackers[i]->Distance(pt.h, pt.v) < bestDistance )
					{
						bestIndex = i;
						bestDistance = gTrackers[i]->Distance(pt.h, pt.v);
					}
				}
				
				if(modifiers & (1L << shiftKeyBit)) {
					
					Float32 gain;
					
					AudioUnitGetParameter(GetEditAudioUnit(),
							k3DMixerParam_Gain,
							kAudioUnitScope_Input,
							bestIndex,
							&gain);
					
					if (gain > -120)
						gain = -120;
					else
						gain = 0;
						
					AudioUnitSetParameter(GetEditAudioUnit(),
							k3DMixerParam_Gain,
							kAudioUnitScope_Input,
							bestIndex,
							gain,
							0);
				}

				currentLine = bestIndex;
				SetInput(currentLine);
				
				for(int i = 0; i < kNTrackers; i++)
				{
					if(i != currentLine) gTrackers[i]->Draw();
					gTrackers[i]->Anchor();
				}
				
				hitx = (int)x;
				hity = (int)y;
				hitAngle = angle;
			}
				
			case kEventMouseDragged:
			{
				if(modifiers & (1L << optionKeyBit) )
				{
					for(int i = 0; i < kNTrackers; i++)
					{
						float newAngle = gTrackers[i]->Rotate(angle - hitAngle );
						
						AudioUnitSetParameter(GetEditAudioUnit(),
							0 /* azimuth */,
							kAudioUnitScope_Input,
							i,
							newAngle,
							0 );
					}
					
					gListener->Draw();
				}
				else if(modifiers & (1L << cmdKeyBit) )
				{

					for(int i = 0; i < kNTrackers; i++)
					{
						float angle;
						float pixelDistance;
						
						gTrackers[i]->Offset(x-hitx, -(y-hity), angle, pixelDistance );

						
						float distance = kDistanceScale *  pixelDistance;
		
						AudioUnitSetParameter(GetEditAudioUnit(),
							0 /* azimuth */,
							kAudioUnitScope_Input,
							i,
							angle,
							0 );

						AudioUnitSetParameter(GetEditAudioUnit(),
							2 /* distance */,
							kAudioUnitScope_Input,
							i,
							distance,
							0 );
					}
					
					gListener->Draw();
				}
				else
				{
					TrackingLine *trackingLine = gTrackers[currentLine];
					
					if(trackingLine)
					{
						trackingLine->Track(pt.h, pt.v);
											
						for(int i = 0; i < kNTrackers; i++)
						{
							if(i != currentLine) gTrackers[i]->Draw();
							gTrackers[i]->Anchor();
						}
						
						gListener->Draw();
					}
					
					AudioUnitSetParameter(GetEditAudioUnit(),
						0 /* azimuth */,
						kAudioUnitScope_Input,
						currentLine,
						angle,
						0 );
					
					float distance = kDistanceScale *   sqrt( x*x + y*y);
	
					AudioUnitSetParameter(GetEditAudioUnit(),
						2 /* distance */,
						kAudioUnitScope_Input,
						currentLine,
						distance,
						0 );
				}
				
				break;
			}
		
			case kEventMouseUp:
				mIsTrackingMouse = false;
				break;
		
		}




		SetPort(save);

		return noErr;
	}
	else
	{
		UInt8			keyCode;
		result =  GetEventParameter(  	inEvent,
												kEventParamKeyMacCharCodes,
												typeChar,
												NULL,       /* can be NULL */
												sizeof(UInt8),
												NULL,       /* can be NULL */
												&keyCode);


#if 1
		switch(keyCode)
		{
			case 'q':
				SetFileInput(0);
				break;

			case 'w':
				SetFileInput(1);
				break;

			case 'e':
				SetFileInput(2);
				break;

			case 'r':
				SetFileInput(3);
				break;

			case 't':
				SetFileInput(4);
				break;

			case 'y':
				SetFileInput(5);
				break;

			case 'u':
				SetFileInput(6);
				break;
				
			case 'i':
				SetFileInput(7);
				break;
				
			case 'o':
				SetFileInput(8);
				break;
				
			case 'p':
				SetFileInput(9);
				break;
				
			case 'a':
				SetFileInput(10);
				break;
				
			case 's':
				SetFileInput(11);
				break;
				
			case 'd':
				SetFileInput(12);
				break;
								
			default:
				break;
		
		}
#endif

		SetPort(save);
		return noErr;
	}
	
	return eventNotHandledErr;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DView::SetRenderingAlgorithmControl
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUMixer3DView::SetRenderingAlgorithmControl(int inAlgorithm )
{
	WindowRef window = GetCarbonWindow();
	
	ControlID controlID;
	controlID.signature = 'algo';
	controlID.id = 0;
	
	ControlRef control;
	OSStatus result = ::GetControlByID(window, &controlID, &control );

	if(result == noErr)
	{
		SetControlValue(control, inAlgorithm+1 );
		ShowControl(control);
	}
	
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DView::SetRenderingFlagsCheckboxes
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUMixer3DView::SetRenderingFlagsCheckboxes()
{
	// set the check boxes according to the current rendering flags
	UInt32 flags;
	UInt32 size = sizeof(flags);
	
	AudioUnitGetProperty(
		GetEditAudioUnit(),
		kAudioUnitProperty_3DMixerRenderingFlags,
		kAudioUnitScope_Input,
		0,
		&flags,
		&size );

	UInt32 usesReverb = 0;
	AudioUnitGetProperty(
		GetEditAudioUnit(),
		kAudioUnitProperty_UsesInternalReverb,
		kAudioUnitScope_Input,
		0,
		&usesReverb,
		&size );

	SetCheckbox('atr0', 1, flags & k3DMixerRenderingFlags_InterAuralDelay );
	SetCheckbox('atr1', 2, flags & k3DMixerRenderingFlags_DopplerShift );
	SetCheckbox('atr2', 3, flags & k3DMixerRenderingFlags_DistanceAttenuation );
	SetCheckbox('atr3', 4, flags & k3DMixerRenderingFlags_DistanceFilter );
	SetCheckbox('atr4', 5, flags & k3DMixerRenderingFlags_DistanceDiffusion );
	SetCheckbox('rvrb', 6, usesReverb );
	::UpdateControls(GetCarbonWindow(), NULL );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DView::SetCheckbox
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUMixer3DView::SetCheckbox(OSType inSig, SInt32 inID, SInt16 inValue )
{
	WindowRef window = GetCarbonWindow();
	
	ControlID controlID;
	controlID.signature = inSig;
	controlID.id = inID;
	
	ControlRef control;
	OSStatus result = ::GetControlByID(window, &controlID, &control );

	if(result == noErr)
	{
		SetControlValue(control, int(inValue != 0 ) );
		ShowControl(control);
	}
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUMixer3DView::CreateUI
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus	AUMixer3DView::CreateUI(Float32	inXOffset, Float32 	inYOffset)
{
	EventHandlerUPP  handlerUPP = NewEventHandlerUPP(MyEventHandlerDispatch);

	EventTypeSpec    eventTypes[] =
	{
		{kEventClassKeyboard, kEventRawKeyDown},
		{kEventClassKeyboard, kEventRawKeyRepeat},
		{kEventClassMouse, kEventMouseDown},
		{kEventClassMouse, kEventMouseUp},
		{kEventClassMouse, kEventMouseDragged},
		{kEventClassCommand, kEventCommandProcess},
		
		{kEventClassWindow, kEventWindowDrawContent},
		
		{kEventClassWindow, kEventWindowUpdate},
		
	};


	SizeControl(mCarbonPane, 400, 400);

	InstallWindowEventHandler( GetCarbonWindow(), handlerUPP, sizeof(eventTypes) / sizeof(EventTypeSpec), eventTypes, this, NULL);

	RGBColor color1;
	color1.red = 65535;
	color1.green = 0;
	color1.blue = 0;
	gTrackers[0] = new TrackingLine(color1, (int)inXOffset, (int)inYOffset, 400, 400);

	RGBColor color2;
	color2.red = 0;
	color2.green = 65535;
	color2.blue = 0;
	gTrackers[1]  = new TrackingLine(color2, (int)inXOffset, (int)inYOffset, 400, 400);
        
	RGBColor color3;
	color3.red = 0;
	color3.green = 0;
	color3.blue = 65535;
	gTrackers[2]  = new TrackingLine(color3, (int)inXOffset, (int)inYOffset, 400, 400);
        
	RGBColor color4;
	color4.red = 20000;
	color4.green = 20000;
	color4.blue = 20000;
	gListener  = new TrackingLine(color4, (int)inXOffset, (int)inYOffset, 400, 400);
	gListener->ScaleDistance(0.0);
	
	
	mOffsetX = (int)inXOffset;
	mOffsetY = (int)inYOffset;

	SetRenderingFlagsCheckboxes();


	UInt32 algo;
	UInt32 size = sizeof(algo);
	AudioUnitGetProperty(
		GetEditAudioUnit(),
		kAudioUnitProperty_SpatializationAlgorithm,
		kAudioUnitScope_Input,
		0,
		&algo,
		&size );

	SetRenderingAlgorithmControl(algo);

	mBackgroundColor.red = 50000;
	mBackgroundColor.green = 50000;
	mBackgroundColor.blue = 50000;
	
	TrackingLine::SetBackgroundColor(mBackgroundColor );
	
	mIsTrackingMouse = false;

	
	EventLoopTimerUPP timerUPP = ::NewEventLoopTimerUPP(MixerTimerProc);

//printf("timerUPP = %x\n", timerUPP );
	
	EventLoopTimerRef timerRef;
	EventLoopRef mainEventLoop = GetMainEventLoop();

//printf("mainEventLoop = %x\n", mainEventLoop);
//printf("this = %x\n", (int)this );

gAUMixer3DView = this;
	
	OSStatus timerResult =  ::InstallEventLoopTimer(
									mainEventLoop,
									0.005 /* delay */,
									0.020 /* interval */,
									timerUPP,
									this,
									&timerRef );                       

//printf("timerResult = %d\n", timerResult );
//printf("timerRef = %x\n", timerRef );
  
	return timerResult;
}
