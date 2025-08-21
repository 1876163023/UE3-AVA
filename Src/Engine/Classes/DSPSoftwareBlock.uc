class DSPSoftwareBlock extends Object
	collapsecategories
	editinlinenew
	abstract	
	native(DSP)
	hidecategories(Object);

enum EDelayType
{
	DLY_Plain,
	DLY_LowPass,
	DLY_AllPass,
};

enum EQualityType
{
	QUA_Low,
	QUA_Medium,
	QUA_High,
	QUA_VeryHigh
};

enum EFilterType
{
	FILTER_LP,
	FILTER_HP
};

enum ELFOType
{
	LFO_Sinusoidal,
	LFO_Triangle,
	LFO_Square,
	LFO_Saw,
	LFO_Random,
	LFO_LogIn,
	LFO_LogOut,
	LFO_LinearIn,
	LFO_LinearOut
};

enum EEnvelopeType
{
	ENV_Linear,
	ENV_Exponential
};

struct native Descriptor
{
	var int				type;
	var float			params[16];
};

simulated event FillValue( out Descriptor desc );