class AavaShatterGlassActor : public AActor
{
public:
	//## BEGIN PROPS avaShatterGlassActor
	class UavaShatterGlassComponent* ShatterGlassComponent;
	
	//## END PROPS avaShatterGlassActor

	DECLARE_CLASS(AavaShatterGlassActor,AActor,0,avaGame)
	virtual void PostBeginPlay();
	/**
	* Function that gets called from within Map_Check to allow this actor to check itself
	* for any potential errors and register them with map check dialog.
	*/
	virtual void CheckForErrors();
};