$modelname "D:\HL\zbytky\_khed\fyzbox\ed_brush.mdl"
$cd "D:\HL\zbytky\_khed\fyzbox"
$cdtexture "D:\HL\zbytky\_khed\fyzbox"
$cliptotextures
//$externaltextures
$colortex 6
$hldscolors
$bodygroup body
{
studio "ed_brush"
}
$sequence idle "ed_brush" loop fps 1 ACT_IDLE 1

//X a Y je prohozene
$controller 0 "Y" X -1020 0
$controller 0 "XY" X -1020 0
$controller 0 "XYZ" X -1020 0
$controller 1 "X" Y -1020 0
$controller 2 "Z" Z -1020 0
$controller 2 "XZ" Z -1020 0
$controller 2 "YZ" Z -1020 0
$controller 2 "XYZ" Z -1020 0