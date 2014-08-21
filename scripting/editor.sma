#include <amxmodx>
#include <fakemeta>
#include <hamsandwich>
#include <editor>
#include <engine>
#include <cstrike>
#include <amxmisc>


#define DEFAULT_GRID_DISTANCE 64
#define DEFAULT_GRID_SNAP 16
#define MIN_GRID_SNAP 4
#define MAX_GRID_SNAP 64
#define MIN_GRID_DISTANCE 16
#define MAX_GRID_DISTANCE 256
#define DEFAULT_GRID_COLOR {255,0,0}
new cl_grid_distance[33]
new cl_grid_snap[33]
new cl_grid_color[33][3]
new cl_grid_point[33][3]
new cl_grid_hide[33]

new cl_prev_point[33][3]
new cl_cb_active[33]

new cl_menu_cur[33]
new cl_menu_curmenu[33][32]

new cl_brush_sel[33]
new cl_brush_selbb[33][2][3]

new Float:cl_angles[33][3]

new cl_tex_lock[33]
new cl_tex_cur[33]
new cl_tex_recent[33][9]

new cl_entclass_cur[33]

new cl_editor_active[33]

public plugin_init() {
	register_plugin("cs-editor","0","Backup")
	register_forward(FM_ClientConnect,"ClientConnect",1)
	register_forward(FM_PlayerPreThink,"PlayerPreThink")
	register_forward(FM_UpdateClientData,"UpdateClientData",1)
	RegisterHam(Ham_Spawn, "player", "fwHamPlayerSpawnPost", 1)
	set_task(0.1,"draw_grid",33,_,_,"b")
	
	register_clcmd("say /editor","ed_menu",_,"Open editor menu.")
	register_clcmd("say /ed","ed_menu",_,"Open editor menu.")
	register_clcmd("ed_menu_grid_col","ed_menu_grid_col")
	register_clcmd("ed_menu_tex_brws","ed_menu_tex_brws")
	register_clcmd("ed_local", "ed_local")
}

new beamsprite

public plugin_precache(){
	beamsprite = precache_model("sprites/laserbeam.spr")
}

public fwHamPlayerSpawnPost(id){
	if (is_user_alive(id) && cl_editor_active[id]){
		set_pev(id,pev_movetype,8)
	}
}

public ClientConnect(id){
	cl_grid_distance[id] = DEFAULT_GRID_DISTANCE
	cl_grid_snap[id] = DEFAULT_GRID_SNAP
	cl_grid_color[id] = DEFAULT_GRID_COLOR
	cl_tex_cur[id] = -1
	cl_entclass_cur[id] = -1
	for(new i = 0; i < 9; i++){
		cl_tex_recent[id][i] = -1
	}
}

ed_exec(id,func[]){
	if(func[0] && funcidx(func)>=0)
		set_task(0.0,func,id)
}

public PlayerPreThink(id){
	if(!cl_editor_active[id])
		return PLUGIN_CONTINUE
	new button = pev(id,pev_button)
	new oldbuttons = pev(id,pev_oldbuttons)
	if(button & IN_ATTACK && !(oldbuttons & IN_ATTACK) && cl_menu_curmenu[id][0]){
		new pos = strlen(cl_menu_curmenu[id])
		copy(cl_menu_curmenu[id][pos],5,"_lmb")
		num_to_str(cl_menu_cur[id],cl_menu_curmenu[id][pos+4],2)
		ed_exec(id,cl_menu_curmenu[id])
		cl_menu_curmenu[id][pos] = 0
		pev(id,pev_v_angle,cl_angles[id])
	}
	/*if(button & IN_ATTACK2 && !(oldbuttons & IN_ATTACK2)){
	}*/
	button &= ~(IN_ATTACK|IN_ATTACK2)
	oldbuttons &= ~(IN_ATTACK|IN_ATTACK2)
	set_pev(id,pev_button,button)
	set_pev(id,pev_oldbuttons,oldbuttons)
	return PLUGIN_HANDLED
}


public UpdateClientData(id,sendweapons,cd_handle )
{
	if (/*!is_user_alive(id) || */!cl_editor_active[id])
		return FMRES_IGNORED;
	set_cd(cd_handle, CD_ID, 0);
	return FMRES_HANDLED;
}

ed_menu_create(title[]){
	new menu = menu_create(title,"ed_menu_handler")
	menu_setprop(menu,MPROP_PERPAGE,0)
	//menu_setprop(menu,MPROP_EXIT,MEXIT_NEVER)
	return menu
}

ed_menu_addblank(menu, func[]){
	menu_additem(menu,"",func)
}

ed_menu_addblanks(menu, func[], count){
	for(new i = 0; i < count; i++)
		ed_menu_addblank(menu,func)
}

public ed_menu_handler(id, menu, item){
	new access,callback,info[32]
	
	menu_item_getinfo(menu,item,access,info,32,_,_,callback)
	ed_exec(id,info)
	menu_destroy(menu)
	cl_menu_cur[id] = item
	cl_menu_curmenu[id][0] = 0
}

public ed_menu(id){
	
	if(!cl_editor_active[id]){
		new Float:origin[3], Float:angles[3]
		new CsTeams:team = cs_get_user_team(id)
		if(team == CS_TEAM_SPECTATOR || team == CS_TEAM_UNASSIGNED){
			origin[0] = 0.0
			origin[1] = 0.0
			origin[2] = -2048.0
		}else{
			pev(id, pev_origin, origin)
		}
		if(team != CS_TEAM_SPECTATOR){
			cs_set_user_team(id, CS_TEAM_SPECTATOR)
		}
		
		pev(id, pev_v_angle, angles)
		dllfunc(DLLFunc_Spawn, id)
		set_pev(id, pev_angles, angles)
		set_pev(id, pev_fixangle, 1)
		
		if(origin[2] > 0)
			origin[2] -= 4096
		set_pev(id, pev_origin, origin)
		
		cl_editor_active[id] = 1
		set_pev(id, pev_movetype, 8)
	}
	
	new menu = ed_menu_create("Editor menu")
	menu_additem(menu, "Grid menu", "ed_menu_grid")
	menu_additem(menu, "Brush menu", "ed_menu_brush")
	menu_additem(menu, "Texture menu", "ed_menu_tex")
	menu_additem(menu, "Entity menu", "ed_menu_ents")
	if(access(id, ADMIN_MAP)){
		ed_menu_addblank(menu, "ed_menu")
		menu_additem(menu, "Compile map", "ed_menu_compile")
	}
	ed_menu_addblank(menu, "ed_menu")
	menu_additem(menu, "Return", "ed_menu_return")
	menu_display(id, menu)
	return PLUGIN_HANDLED
}

public ed_menu_compile(id){
	editor_world_compile()
	ed_menu(id)
}

public ed_menu_return(id){
	new menu = menu_create("Return as","ed_menu_return_h")
	menu_additem(menu,"Terrorist Force")
	menu_additem(menu,"Counter-Terrorist Force")
	menu_display(id,menu)
}

public ed_menu_return_h(id, menu, item){
	menu_destroy(menu)
	if(item<0 || item >1){
		ed_menu(id)
		return
	}
	if(item == 0)
		cs_set_user_team(id, CS_TEAM_T)
	if(item == 1)
		cs_set_user_team(id, CS_TEAM_CT)
	new Float:origin[3]
	pev(id, pev_origin, origin)
	origin[2] -= -4096
	set_pev(id, pev_origin, origin)
	
	cl_editor_active[id] = 0
	set_pev(id, pev_movetype, 3)
}

public ed_menu_grid(id){
	new menu = ed_menu_create("Grid menu")
	menu_additem(menu,"Smaller","ed_menu_grid_s")
	menu_additem(menu,"Larger","ed_menu_grid_l")
	menu_additem(menu,"Closer","ed_menu_grid_c")
	menu_additem(menu,"Farther","ed_menu_grid_f")
	menu_additem(menu,"Set color (rgb, 0-255, space sep.)","ed_menu_grid_col")
	menu_additem(menu,cl_grid_hide[id]?"Show":"Hide","ed_menu_grid_h")
	ed_menu_addblanks(menu,"ed_menu_grid",3)
	menu_additem(menu,"Editor menu","ed_menu")
	menu_display(id,menu)
}

public ed_menu_grid_h(id){
	cl_grid_hide[id]=!cl_grid_hide[id]
	ed_menu_grid(id)
}

public ed_menu_grid_s(id){
	if(cl_grid_snap[id]!=MIN_GRID_SNAP){cl_grid_snap[id]/=2;}
	ed_menu_grid(id)
}
public ed_menu_grid_l(id){
	if(cl_grid_snap[id]!=MAX_GRID_SNAP){cl_grid_snap[id]*=2;}
	ed_menu_grid(id)
}
public ed_menu_grid_c(id){
	if(cl_grid_distance[id]!=MIN_GRID_DISTANCE){cl_grid_distance[id]/=2;}
	ed_menu_grid(id)
}
public ed_menu_grid_f(id){
	if(cl_grid_distance[id]!=MAX_GRID_DISTANCE){cl_grid_distance[id]*=2;}
	ed_menu_grid(id)
}
public ed_menu_grid_col(id){
	new buff[13]
	read_args(buff,1)
	if(!read_argc() || buff[0]!='"'){
		client_cmd(id,"messagemode ed_menu_grid_col")
		return
	}
	new color[3][4]
	read_argv(1,buff,12)
	parse(buff,color[0],3,color[1],3,color[2],3)
	for(new i = 0; i < 3; i++){
		cl_grid_color[id][i]=min(max(str_to_num(color[i]),0),255)
	}
	ed_menu_grid(id)
}

public ed_menu_brush(id){
	new menu = ed_menu_create("Brush menu")
	menu_additem(menu,"Create","ed_menu_brush")
	menu_additem(menu,"Move","ed_menu_brush")
	menu_additem(menu,"Clone","ed_menu_brush")
	menu_additem(menu,"Scale","ed_menu_brush")
	menu_additem(menu,"\dMorph","ed_menu_brush")
	menu_additem(menu,"Delete","ed_menu_brush")
	ed_menu_addblanks(menu,"ed_menu_bruhs",3)
	menu_additem(menu,"Editor menu","ed_menu")
	
	cl_menu_cur[id] = max(min(cl_menu_cur[id],5),0)
	new buffer,name[32]
	menu_item_getinfo(menu,cl_menu_cur[id],buffer,"",0,name,32,buffer)
	strcat(name," (Active)",32)
	menu_item_setname(menu,cl_menu_cur[id],name)
	
	copy(cl_menu_curmenu[id],32,"ed_menu_brush")
	
	cl_cb_active[id] = 0
	cl_brush_sel[id] = -1
	
	menu_display(id,menu)
}

ed_world_raycast(id){
	new Float:origin[3], iorg[3], Float:direction[3]
	get_user_origin(id, iorg, 1)
	for(new i = 0; i < 3; i++)
		origin[i] = float(iorg[i])
	//pev(id,pev_v_angle,direction)
	angle_vector(cl_angles[id], 1, direction)
	new face
	return editor_world_raycast(origin, direction, face)
}

//Brush->Create
public ed_menu_brush_lmb0(id){
	cl_cb_active[id] = !cl_cb_active[id]
	if(cl_cb_active[id]){
		cl_prev_point[id] = cl_grid_point[id]
	}else{
		new mins[3], maxs[3], minsize = 8192, maxsize = 0
		for(new i=0; i<3; i++){
			mins[i] = min(cl_prev_point[id][i], cl_grid_point[id][i])
			maxs[i] = max(cl_prev_point[id][i], cl_grid_point[id][i])
			minsize = min(minsize, maxs[i] - mins[i])
			maxsize = max(maxsize, maxs[i] - mins[i])
		}
		if(minsize >= 4 && maxsize <= 1024){
			editor_brush_add(id, mins, maxs, cl_tex_cur[id])
		}else{
			//TODO: some kind of error message?
		}
	}
}

//check owner
ed_brush_chckown(id, brush){
	if(access(id, ADMIN_MAP))
		return 1
	return editor_brush_chckown(id, brush)
}

//move/clone
ed_menu_brush_lmb1_(id, clone){
	new brush = cl_brush_sel[id]
	if(brush == -1){
		brush = ed_world_raycast(id)
		if(brush == -1 || !ed_brush_chckown(id,brush))
			return
		
		editor_brush_getbbox(brush, cl_brush_selbb[id][0], cl_brush_selbb[id][1])
		cl_brush_sel[id] = brush
		cl_prev_point[id] = cl_grid_point[id]
		
		return
	}
	new offset[3]
	for(new i = 0; i < 3; i++)
		offset[i] = cl_grid_point[id][i]-cl_prev_point[id][i]
	if(clone){
		brush = editor_brush_clone(brush)
	}
	editor_brush_move(brush,offset)
	cl_brush_sel[id] = -1
}
//Brush->Move
public ed_menu_brush_lmb1(id){
	ed_menu_brush_lmb1_(id, 0)
}
//Brush->Clone
public ed_menu_brush_lmb2(id){
	ed_menu_brush_lmb1_(id, 1)
}

//Brush->Scale
public ed_menu_brush_lmb3(id){
	
}

//Brush->Delete
public ed_menu_brush_lmb5(id){
	new brush = ed_world_raycast(id)
	if(brush == -1 || !ed_brush_chckown(id,brush))
		return
	editor_brush_remove(brush)
}

public ed_menu_tex(id){
	new menu = ed_menu_create("Texture menu")
	menu_additem(menu,"Browse textures","ed_menu_tex_brws")
	menu_additem(menu,"Recent textures","ed_menu_tex_rcnt")
	ed_menu_addblank(menu,"ed_menu_tex")
	menu_additem(menu,"\dApply on faces","ed_menu_tex")
	menu_additem(menu,"\dApply on brushes","ed_menu_tex")
	ed_menu_addblank(menu,"ed_menu_tex")
	menu_additem(menu,"\dAdjust texture","ed_menu_tex")
	menu_additem(menu,cl_tex_lock[id]?"\dDisable texture lock":"\dEnable texture lock","ed_menu_tex_tl")
	ed_menu_addblank(menu,"ed_menu_tex")
	menu_additem(menu,"Editor menu","ed_menu")
	
	cl_menu_cur[id] = max(min(cl_menu_cur[id],4),3)
	new buffer,name[32]
	menu_item_getinfo(menu,cl_menu_cur[id],buffer,"",0,name,32,buffer)
	strcat(name," (Active)",32)
	menu_item_setname(menu,cl_menu_cur[id],name)
	
	copy(cl_menu_curmenu[id],32,"ed_menu_tex")
	
	menu_display(id,menu)
}

new cl_islocal[33]

public ed_local(id){
	cl_islocal[id] = 1
}

ed_motd(id, url[], title[]){
	new content[128], addr[64]
	//get_user_ip(0, addr, 23, 0)
	get_cvar_string("net_address", addr, 64)
	/*copy(addr,23,"xxx.xxx.xxx.xxx:27015")
	if(cl_islocal[id])
		copy(addr,23,"10.0.0.xxx:27015")*/
	format(content,128,"<meta http-equiv=^"refresh^" content=^"0; url=http://%s/%s^">",addr,url)
	show_motd(id,content,title)
}

public ed_menu_tex_brws(id){
	new buffer[16]
	read_args(buffer,1)
	if(!read_argc() || buffer[0]!='"'){
		client_cmd(id,"messagemode ed_menu_tex_brws")
		return
	}
	read_argv(1,buffer,16)
	
	new url[48]
	format(url,48,"textures/filter=%s,%d",buffer,id)
	ed_motd(id,url,"Texture browser")
	ed_menu_tex(id)
}

public editor_texturebrowser_set(texture, uid){
	cl_tex_cur[uid] = texture
	ed_menu_tex_recent_add(uid, texture)
	return PLUGIN_HANDLED
}

ed_menu_tex_recent_add(id, tex){
	if(cl_tex_recent[id][0] == tex)
		return
	new i
	for(i = 1; i < 9; i++){
		new curtex = cl_tex_recent[id][i]
		if(curtex == tex || curtex == -1)
			break
	}
	for(; i > 0; i--){
		cl_tex_recent[id][i] = cl_tex_recent[id][i-1]
	}
	cl_tex_recent[id][0] = tex
}

public ed_menu_tex_rcnt(id){
	new menu = menu_create("Recent textures","ed_menu_tex_rcnt_h")
	//menu_setprop(menu,MPROP_PERPAGE,0)
	menu_setprop(menu,MPROP_EXIT,MEXIT_ALL)
	
	new buff[16]
	for(new i = 0; i < 9; i++){
		new curtex = cl_tex_recent[id][i]
		if(curtex == -1)
			break
		editor_texture_getname(curtex,buff)
		menu_additem(menu,buff)
	}

	menu_display(id,menu)
}

public ed_menu_tex_rcnt_h(id, menu, item){
	menu_destroy(menu)
	if(item >= 0){
		cl_tex_cur[id] = cl_tex_recent[id][item]
		ed_menu_tex_recent_add(id, cl_tex_cur[id])
	}
	ed_menu_tex(id)
}

public ed_menu_tex_tl(id){
	cl_tex_lock[id] = !cl_tex_lock[id]
	//todo: accual texture lock, maybe?
	ed_menu_tex(id)
}

public ed_menu_ents(id){
	new menu = ed_menu_create("Entity menu")
	menu_additem(menu,"Choose entity class","ed_menu_ents_ch")
	ed_menu_addblank(menu,"ed_menu_ents")
	menu_additem(menu,"Create entity","ed_menu_ents")
	menu_additem(menu,"Entity properties","ed_menu_ents")
	ed_menu_addblanks(menu,"ed_menu_ents",5)
	menu_additem(menu,"Editor menu","ed_menu")
	
	cl_menu_cur[id] = max(min(cl_menu_cur[id],3),2)
	new buffer,name[32]
	menu_item_getinfo(menu,cl_menu_cur[id],buffer,"",0,name,32,buffer)
	strcat(name," (Active)",32)
	menu_item_setname(menu,cl_menu_cur[id],name)
	
	copy(cl_menu_curmenu[id],32,"ed_menu_ents")
	
	menu_display(id,menu)
}

public ed_menu_ents_ch(id){
	new menu = menu_create("Entity classes","ed_menu_ents_ch_h")
	new index = 0, name[32], desc[64], title[96], info[4]
	while((index = editor_classes_getnext(index, PointClass, name, desc)) != -1){
		format(title, 96, "%s (%s)", name, desc)
		num_to_str(index, info, 4)
		menu_additem(menu, title, info)
		index++
	}
	menu_display(id,menu)
}

public ed_menu_ents_ch_h(id, menu, item){
	new access, info[4], callback
	menu_item_getinfo(menu, item, access, info, 4, "", 0, callback)
	menu_destroy(menu)
	cl_entclass_cur[id] = str_to_num(info)
	ed_menu_ents(id)
}

//Entity->Create
public ed_menu_ents_lmb2(id){
	if(cl_entclass_cur[id] != -1){
		new entity = editor_entity_add(id, cl_grid_point[id], cl_entclass_cur[id])
		ed_menu_ents_props(id,entity)
	}
}

ed_menu_ents_props(id,entity){
	new url[32]
	format(url,32,"entities/entity=%d,%d",entity,id)
	ed_motd(id,url,"Entity properties")
}

//Entity->Properties
public ed_menu_ents_lmb3(id){
	new entity = ed_world_raycast(id)
	if(entity != -1 && ed_brush_chckown(id,entity)){	
		ed_menu_ents_props(id,entity)
	}
}

public draw_grid(){
	for(new i=1;i<=32;i++){
		if(is_user_alive(i) && cl_editor_active[i]){
			//nehybat s bodem, pokud mame neco rozdelaneho
			if(!task_exists(i)){
				point(i)
			}
			if(cl_cb_active[i]){
				cb_preview(i)
			}
			if(cl_brush_sel[i]!=-1){
				bmove_preview(i)
			}
			if(!cl_grid_hide[i]){
				point_box(i)
				grid(i)
			}
			display_info(i)
		}
	}
}

display_info(id){
	set_hudmessage(cl_grid_color[id][0], cl_grid_color[id][1], cl_grid_color[id][2], 1.0, 0.0, 0, 0.0, 0.1, 0.01, 0.0, -1)
	
	//dist/size
	static offset[3]
	for(new i = 0; i < 3; i++){
		offset[i] = cl_grid_point[id][i] - cl_prev_point[id][i]
	}
	
	//texture
	static texbuf[16]
	if(cl_tex_cur[id] == -1)
		copy(texbuf, 16, "-- NONE --")
	else
		editor_texture_getname(cl_tex_cur[id], texbuf)

	//entity
	static entbuf[32]
	if(cl_entclass_cur[id] == -1)
		copy(entbuf, 32, "-- NONE --")
	else
		editor_classes_getname(cl_entclass_cur[id], entbuf)
	
	//owner
	new Float:origin[3], iorg[3], Float:dir[3]
	get_user_origin(id,iorg,1)
	for(new i = 0; i < 3; i++)
		origin[i] = float(iorg[i])
	pev(id,pev_v_angle,dir)
	angle_vector(dir,1,dir)
	new face
	new owner[32]
	get_user_name(editor_brush_getown(editor_world_raycast(origin,dir,face)), owner, 31)
	
	//compiler
	new status = editor_compiler_status()
	new compiler[32]
	if(status == -1)
		copy(compiler, 32, "Not compiling.")
	else{
		new exe[4]
		if(status == 0)
			copy(exe, 4, "CSG")
		else if(status == 1)
			copy(exe, 4, "BSP")
		else if(status == 2)
			copy(exe, 4, "VIS")
		else if(status == 3)
			copy(exe, 4, "RAD")
		format(compiler, 32, "Compiling %s...", exe)
	}
	
	show_hudmessage(id, "Point position: [%d;%d;%d]^n\
	Distance/Size: [%d;%d;%d]^n\
	Current texture: %s^n\
	Current entclass: %s^n\
	Brush owner: %s^n\
	Face texture: %s^n\
	%s^n\
	",
	cl_grid_point[id][0], cl_grid_point[id][1], cl_grid_point[id][2]+2048,
	offset[0],offset[1],offset[2],
	texbuf,
	entbuf,
	owner,
	"-- UNKNOWN --",
	compiler
	)
}

bmove_preview(id){
	static offset[3]
	for(new i = 0; i < 3; i++){
		offset[i] = cl_grid_point[id][i]-cl_prev_point[id][i]
	}
	message_begin(MSG_ONE_UNRELIABLE ,SVC_TEMPENTITY,{0,0,0},id)
	write_byte(TE_BOX)
	write_coord(cl_brush_selbb[id][0][0]+offset[0])
	write_coord(cl_brush_selbb[id][0][1]+offset[1])
	write_coord(cl_brush_selbb[id][0][2]+offset[2])
	write_coord(cl_brush_selbb[id][1][0]+offset[0])
	write_coord(cl_brush_selbb[id][1][1]+offset[1])
	write_coord(cl_brush_selbb[id][1][2]+offset[2])
	write_short(1)
	write_byte(255)
	write_byte(0)
	write_byte(0)
	message_end()	
}

cb_preview(id){
	message_begin(MSG_ONE_UNRELIABLE ,SVC_TEMPENTITY,{0,0,0},id)
	write_byte(TE_BOX)
	write_coord(cl_prev_point[id][0])
	write_coord(cl_prev_point[id][1])
	write_coord(cl_prev_point[id][2])
	write_coord(cl_grid_point[id][0])
	write_coord(cl_grid_point[id][1])
	write_coord(cl_grid_point[id][2])
	write_short(1)
	write_byte(255)
	write_byte(255)
	write_byte(255)
	message_end()
}

point(id){
	new Float:angles[3],Float:matrix3[3][3],Float:matrix4[3][4]
	pev(id,pev_v_angle,angles)
	
	angle_vector(angles,2,matrix3[0])
	angle_vector(angles,3,matrix3[1])
	for(new i=0;i<3;i++){
		angles[i] = floatround(angles[i]/90.0)*90.0
	}
	angle_vector(angles,1,matrix3[2])
	matrix3_to_matrix4(id,matrix3,matrix4)
	new Float:d,Float:result[3]
	d=matrix4_d(matrix4,3)
	for(new i=0;i<3;i++){
		result[i]=matrix4_d(matrix4,i)/d
	}
	result[1]=-result[1]//for some sick fucking reason it's reversed!!!
	snap(id,result)
	for(new i=0;i<3;i++){
		cl_grid_point[id][i]=floatround(result[i])
	}
}

point_box(id){
	message_begin(MSG_ONE_UNRELIABLE ,SVC_TEMPENTITY,{0,0,0},id)
	write_byte(TE_BOX)
	write_coord(cl_grid_point[id][0]-1)
	write_coord(cl_grid_point[id][1]-1)
	write_coord(cl_grid_point[id][2]-1)
	write_coord(cl_grid_point[id][0]+1)
	write_coord(cl_grid_point[id][1]+1)
	write_coord(cl_grid_point[id][2]+1)
	write_short(1)
	write_byte(cl_grid_color[id][0])
	write_byte(cl_grid_color[id][1])
	write_byte(cl_grid_color[id][2])
	message_end()
}

Float:matrix4_d(Float:matrix4[3][4],dis){
	new Float:matrix3[3][3]
	for(new i=0;i<3;i++){
		new i3=0
		for(new i2=0;i2<3;i2++){
			if(i3==dis){
				i3++
			}
			matrix3[i][i2]=matrix4[i][i3]
			i3++
		}
	}
	return 	(matrix3[0][0]*matrix3[1][1]*matrix3[2][2]+
		matrix3[1][0]*matrix3[2][1]*matrix3[0][2]+
		matrix3[0][1]*matrix3[1][2]*matrix3[2][0])-
		(matrix3[0][2]*matrix3[1][1]*matrix3[2][0]+
		matrix3[0][0]*matrix3[1][2]*matrix3[2][1]+
		matrix3[1][0]*matrix3[0][1]*matrix3[2][2])
}

matrix3_to_matrix4(id,Float:matrix3[3][3],Float:matrix4[3][4]){
	for(new i=0;i<3;i++){
		for(new i2=0;i2<3;i2++){
			matrix4[i][i2]=matrix3[i][i2]
		}
	}
	
	new origin[3]
	get_user_origin(id,origin,1)
	for(new i=0;i<3;i++){
		matrix4[i][3] = 0.0
		for(new i2=0;i2<3;i2++){
			if(i!=2){
				matrix4[i][3] += matrix4[i][i2]*origin[i2]
			}else{
				matrix4[i][3] += matrix4[i][i2]*(origin[i2]+matrix4[i][i2]*cl_grid_distance[id])
			}
		}
	}
}

grid(id){
	new Float:angles[3]
	pev(id,pev_v_angle,angles)
	
	for(new i=0;i<3;i++){
		angles[i] = floatround(angles[i]/90.0)*90.0
	}
	new Float:normals[3]
	angle_vector(angles,1,normals)
	new bools[3]
	for(new i=0;i<3;i++){
		bools[i] = !floatround(normals[i])
	}
	new origin[3]
	get_user_origin(id,origin,1)
	
	for(new i=0;i<3;i++){
		if(bools[i]){
			for(new i2=-cl_grid_distance[id];i2<=cl_grid_distance[id];i2+=cl_grid_snap[id]){
				new Float:from[3],Float:to[3]
				for(new i3=0;i3<3;i3++){
					from[i3]=origin[i3]+normals[i3]*cl_grid_distance[id]
					to[i3]=from[i3]
					if(i==i3){
						from[i3]+=i2
						to[i3]=from[i3]
					}else{
						from[i3]-=bools[i3]*cl_grid_distance[id]
						to[i3]+=bools[i3]*cl_grid_distance[id]
					}
				}
				snap(id,from)
				snap(id,to)
				beam(id,from,to)
			}
		}
	}
}

snap(id,Float:val[3]){
	for(new i=0;i<3;i++){
		val[i]=float(floatround(val[i]/cl_grid_snap[id])*cl_grid_snap[id])
	}
}

beam(id,Float:from[3],Float:to[3]){
	message_begin(MSG_ONE_UNRELIABLE ,SVC_TEMPENTITY,{0,0,0},id)
	write_byte(TE_BEAMPOINTS)
	write_coord(floatround(from[0]))	// start position
	write_coord(floatround(from[1]))
	write_coord(floatround(from[2]))
	write_coord(floatround(to[0]))	// end position
	write_coord(floatround(to[1]))
	write_coord(floatround(to[2]))
	write_short(beamsprite)	// sprite index
	write_byte(0)	// starting frame
	write_byte(0)	// frame rate in 0.1's
	write_byte(1)	// life in 0.1's
	write_byte(cl_grid_distance[id]/MIN_GRID_DISTANCE)	// line width in 0.1's
	write_byte(0)	// noise amplitude in 0.01's
	write_byte(cl_grid_color[id][0])	// Red
	write_byte(cl_grid_color[id][1])	// Green
	write_byte(cl_grid_color[id][2])	// Blue
	write_byte(255)	// brightness
	write_byte(0)	// scroll speed in 0.1's
	message_end()
}
