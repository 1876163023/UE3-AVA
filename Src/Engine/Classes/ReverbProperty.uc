class ReverbProperty extends Object
	collapsecategories
	hidecategories(Object)
	native(DSP);

var() int			Environment;
var() float			EnvSize;           /* [in/out] 1.0   , 100.0 , 7.5    , environment size in meters (win32 only) */
var() float			EnvDiffusion;      /* [in/out] 0.0   , 1.0   , 1.0    , environment diffusion (win32/Xbox/GameCube) */
var() int			Room;              /* [in/out] -10000, 0     , -1000  , room effect level (at mid frequencies) (win32/Xbox/Xbox 360/GameCube/software) */
var() int			RoomHF;            /* [in/out] -10000, 0     , -100   , relative room effect level at high frequencies (win32/Xbox/Xbox 360) */
var() int			RoomLF;            /* [in/out] -10000, 0     , 0      , relative room effect level at low frequencies (win32 only) */
var() float			DecayTime;         /* [in/out] 0.1   , 20.0  , 1.49   , reverberation decay time at mid frequencies (win32/Xbox/Xbox 360/GameCube) */
var() float			DecayHFRatio;      /* [in/out] 0.1   , 2.0   , 0.83   , high-frequency to mid-frequency decay time ratio (win32/Xbox/Xbox 360) */
var() float			DecayLFRatio;      /* [in/out] 0.1   , 2.0   , 1.0    , low-frequency to mid-frequency decay time ratio (win32 only) */
var() int			Reflections;       /* [in/out] -10000, 1000  , -2602  , early reflections level relative to room effect (win32/Xbox/Xbox 360/GameCube) */
var() float			ReflectionsDelay;  /* [in/out] 0.0   , 0.3   , 0.007  , initial reflection delay time (win32/Xbox/Xbox 360) */
var() vector		ReflectionsPan;		/* [in/out]       ,       , [0,0,0], early reflections panning vector (win32 only) */
var() int			Reverb;            /* [in/out] -10000, 2000  , 200    , late reverberation level relative to room effect (win32/Xbox/Xbox 360) */
var() float			ReverbDelay;       /* [in/out] 0.0   , 0.1   , 0.011  , late reverberation delay time relative to initial reflection (win32/Xbox/Xbox 360/GameCube) */
var() vector		ReverbPan;			/* [in/out]       ,       , [0,0,0], late reverberation panning vector (win32 only) */
var() float			EchoTime;          /* [in/out] .075  , 0.25  , 0.25   , echo time (win32 or ps2 FMOD_PRESET_PS2_Echo/FMOD_PRESET_PS2_Delay only) */
var() float			EchoDepth;         /* [in/out] 0.0   , 1.0   , 0.0    , echo depth (win32 or ps2 FMOD_PRESET_PS2_Echo only) */
var() float			ModulationTime;    /* [in/out] 0.04  , 4.0   , 0.25   , modulation time (win32 only) */
var() float			ModulationDepth;   /* [in/out] 0.0   , 1.0   , 0.0    , modulation depth (win32/GameCube) */
var() float			AirAbsorptionHF;   /* [in/out] -100  , 0.0   , -5.0   , change in level per meter at high frequencies (win32 only) */
var() float			HFReference;       /* [in/out] 1000.0, 20000 , 5000.0 , reference high frequency (hz) (win32/Xbox/Xbox 360) */
var() float			LFReference;       /* [in/out] 20.0  , 1000.0, 250.0  , reference low frequency (hz) (win32 only) */
var() float			RoomRolloffFactor; /* [in/out] 0.0   , 10.0  , 0.0    , like rolloffscale in System::set3DSettings but for reverb room size effect (win32/Xbox/Xbox 360) */
var() float			Diffusion;         /* [in/out] 0.0   , 100.0 , 100.0  , Value that controls the echo density in the late reverberation decay. (Xbox/Xbox 360) */
var() float			Density;           /* [in/out] 0.0   , 100.0 , 100.0  , Value that controls the modal density in the late reverberation decay (Xbox/Xbox 360) */
var() bool			bDecayTimeScale;		/* 'EnvSize' affects reverberation decay time */
var() bool			bReflectionsScale;	/* 'EnvSize' affects reflection level */
var() bool			bReflectionsDelayScale;/* 'EnvSize' affects initial reflection delay time */
var() bool			bReverbScale;		/* 'EnvSize' affects reflections level */
var() bool			bReverbDelayScale;	/* 'EnvSize' affects late reverberation delay time */
var() bool			bDecayHFLimit;		/* AirAbsorptionHF affects DecayHFRatio */
var() bool			bEchoTimeScale;		/* 'EnvSize' affects echo time */
var() bool			bModulationTimeScale;/* 'EnvSize' affects modulation time */

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

defaultproperties
{	
	Environment = 0
	EnvSize = 7.5
	EnvDiffusion = 1.00      
	Room = -1000
	RoomHF = -100       
	DecayTime = 1.49
	DecayHFRatio = 0.83   
	DecayLFRatio = 1.0
	Reflections = -2602 
	ReflectionsDelay = 0.007
	Reverb = 200
	ReverbDelay = 0.011       
	EchoTime = 0.25
	ModulationTime = 0.25
	AirAbsorptionHF = -5.0
	HFReference = 5000
	LFReference = 250 
	Diffusion = 100
	Density = 100    
	bDecayTimeScale = true
	bReflectionsScale = true
	bReflectionsDelayScale = true
	bReverbScale = true
	bReverbDelayScale= true	
	bDecayHFLimit = true	
}