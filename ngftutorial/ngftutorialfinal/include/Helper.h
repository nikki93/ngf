//---------------------------------------------------------------------------------------
// HELPER.H
//---------------------------------------------------------------------------------------

void loaderHelperFunction(String type, String name, Vector3 pos, Quaternion rot, NGF::PropertyList props)
{
	if(type == "Player")
	{
		Globals::gom->createObject<Player>(pos, rot, props, name);
	}
	
	if(type == "LevelGeometry")
	{
		Globals::gom->createObject<LevelGeometry>(pos, rot, props, name);
	}
}
