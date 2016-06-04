#include <sys/time.h>

class _Sys : public Sys
{
	NSTimer* update_timer;
	double prev_now;
	
	double get_time()
	{
		struct timeval tod;
		gettimeofday(&tod, NULL);
		
		return tod.tv_sec + tod.tv_usec*1.0E-6;
	}

	void update_timing()
	{
		now = get_time();
	
		if (prev_now)
			d_t = now - prev_now;
	
		prev_now = now;
	}

public:

	_Sys()
	{
		update_timer = nil;
		prev_now = 0.0;
		now = get_time();
		d_t = 0.0;

		for (int i = 0; i < NumSysInputs; i++)
			inputs[i] = 0.0f;
	}
	
	void Init(float update_hz, id callback_obj, SEL callback_sel)
	{
		update_timer = [[NSTimer scheduledTimerWithTimeInterval: 1.0/update_hz target: callback_obj selector: callback_sel userInfo:nil repeats:true] retain];

		[[NSRunLoop currentRunLoop] addTimer: update_timer forMode: NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer: update_timer forMode: NSModalPanelRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer: update_timer forMode: NSEventTrackingRunLoopMode];
	}
	
	void Reset()
	{
		[update_timer invalidate];
		[update_timer release];
		update_timer= nil;
	}

	bool HandleEvent(NSEvent* event)
	{
		if ([event type] == NSScrollWheel)
		{
			inputs[SysScrollInput] += [event deltaY];
			
			return YES;
		}
		
		return NO;
	}

	void Process()
	{
		// calculate current scroll inputs:
	
		static float prev_scroll_input = 0.0f;
		
		float scroll_input = inputs[SysScrollInput];
		
		inputs[SysScrollUpInput] = scroll_input > prev_scroll_input;
		inputs[SysScrollDownInput] = scroll_input < prev_scroll_input;
		
		prev_scroll_input = scroll_input;


		// update timing:

		update_timing();
	}
	
} _sys;

Sys& sys = _sys;
