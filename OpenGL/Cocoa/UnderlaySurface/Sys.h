enum SysInputs
{
	SysScrollInput,
	SysScrollUpInput,
	SysScrollDownInput,
	NumSysInputs
};

class Sys
{
public:
	
	double now;
	double d_t;
	
	float inputs[NumSysInputs];

	virtual void Init(float update_hz, id callback_obj, SEL callback_sel) = 0;
	virtual void Reset() = 0;

	virtual bool HandleEvent(NSEvent* event) = 0;
	virtual void Process() = 0;
};

extern Sys& sys;
