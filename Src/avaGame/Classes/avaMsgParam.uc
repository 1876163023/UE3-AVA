class avaMsgParam extends Object;

var int			IntParam[2];
var float		FloatParam[2];
var string		StringParam[2];
var bool		BoolParam1;
var bool		BoolParam2;

static function avaMsgParam Create()
{
	return new() class'avaMsgParam';
}

function SetInt(int n1, optional int n2)
{
	IntParam[0] = n1;
	IntParam[1] = n2;
}

function SetFloat(Float f1, optional Float f2)
{
	FloatParam[0] = f1;
	FloatParam[1] = f2;
}

function SetString(String f1, optional String f2)
{
	StringParam[0] = f1;
	StringParam[1] = f2;
}

function SetBool(bool b1, optional bool b2)
{
	BoolParam1 = b1;
	BoolParam2 = b2;
}