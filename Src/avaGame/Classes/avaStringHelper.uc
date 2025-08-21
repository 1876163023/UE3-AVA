class avaStringHelper extends Object
	native;


static native function string Replace(coerce string str, string match, string Message);

static native function string GetString(coerce string SourceName);

static native function string Trim(coerce string SourceStr);

static native function string TrimQuotes(coerce string SourceStr, optional out int bQuotesRemoved );

static native function string PackString( array<string> StringsToPack, string Delim = "|" );