class UavaDSPPresetFactoryNew : public UFactory
{
	DECLARE_CLASS(UavaDSPPresetFactoryNew,UFactory,CLASS_CollapseCategories,avaGame)
	void StaticConstructor();
	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UObject* FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn );
};

class UavaSoundScapePropertyFactoryNew : public UFactory
{
	DECLARE_CLASS(UavaSoundScapePropertyFactoryNew,UFactory,CLASS_CollapseCategories,avaGame)
	void StaticConstructor();
	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UObject* FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn );
};