class avaMathHelp extends Object;

const URotToRadian=0.000095873799;

static final postoperator float URotations( float URots )
{ return URots * URottoRadian; }

static function Quat RotToQuat( rotator Rot )
{
  local Quat X, Y, Z;
  
  X = QuatFromAxisAndAngle( Vect(1,0,0), Rot.Roll URotations );
  Y = QuatFromAxisAndAngle( Vect(0,1,0), Rot.Pitch URotations );
  Z = QuatFromAxisAndAngle( Vect(0,0,1), Rot.Yaw URotations );

  X = QuatProduct( X, Y );
  X = QuatProduct( X, Z );

  return X;
}


