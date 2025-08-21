class avaUIVoteMessage extends avaUISimpleText native;

enum avaUIVoteMessageType
{
	VOTEMSG_TITLE,
	VOTEMSG_WARN,
	VOTEMSG_VOTING,
	VOTEMSG_PROGRESS,
	VOTEMSG_RESULT,
	VOTEMSG_TIME
};

var()	avaUIVoteMessageType	MessageType;

cpptext
{
	virtual UBOOL UpdateString();		
}
