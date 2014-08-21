$modelname "D:\HL\zbytky\_khed\fyzbox\ed_tri.mdl"
$cd "D:\HL\zbytky\_khed\fyzbox"
$cdtexture "D:\HL\zbytky\_khed\fyzbox"
$cliptotextures
$colortex 6
$bodygroup body
{
studio "ed_tri"
}
$sequence idle "ed_tri" loop fps 1 ACT_IDLE 1

$controller 0 "X" X -1024 0
$controller 1 "Y" Y -1024 0
//sharp
$controller 2 "Y" X 0 1024
//blunt
#$controller 2 "Y" X -1024 0
