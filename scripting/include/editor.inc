native editor_brush_add(owner, mins[3], maxs[3], texture)
native editor_entity_add(owner, origin[3], class)
native editor_brush_remove(brush)
native editor_brush_chckown(owner, brush)
native editor_brush_getown(brush)
native editor_brush_getbbox(brush, mins[3], maxs[3])
native editor_brush_move(brush, offset[3])
native editor_brush_clone(brush)

native editor_world_raycast(Float:origin[3], Float:direction[3], &face)
native editor_world_compile()
native editor_compiler_status()

native editor_texture_getname(texture, texname[16])

enum EDITOR_CLASS_TYPE {
	BaseClass = 0,
	PointClass,
	SolidClass
};

native editor_classes_getnext(index, EDITOR_CLASS_TYPE:type, name[32], desc[64])
native editor_classes_getname(index, name[32])

forward editor_texturebrowser_set(texture, uid)