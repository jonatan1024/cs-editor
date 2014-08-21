#include "face.h"

editorFace::editorFace(){
	vertices.resize(0);
}

editorFace::editorFace(Vector v1,Vector v2,Vector v3,Vector v4){
	/*vertices_c = 4;
	vertices = (Vector*)malloc(sizeof(Vector)*4);*/
	vertices.resize(4);
	vertices[0]=v1;
	vertices[1]=v2;
	vertices[2]=v3;
	vertices[3]=v4;

	offset_u = 0;
	offset_v = 0;
	rotation = 0;
	scale_u = 1;
	scale_v = 1;
	calcUV(false);	//align to world as default
}

void editorFace::calcUV(bool align_face){
	const Vector FaceNormals[6] = {
		Vector(0, 0, 1),		// floor
		Vector(0, 0, -1),		// ceiling
		Vector(0, -1, 0),		// north wall
		Vector(0, 1, 0),		// south wall
		Vector(-1, 0, 0),		// east wall
		Vector(1, 0, 0),		// west wall
	};
	const Vector DownVectors[6] = {
		Vector(0, -1, 0),		// floor
		Vector(0, -1, 0),		// ceiling
		Vector(0, 0, -1),		// north wall
		Vector(0, 0, -1),		// south wall
		Vector(0, 0, -1),		// east wall
		Vector(0, 0, -1),		// west wall
	};
	const Vector RightVectors[6] = {
		Vector(1, 0, 0),		// floor
		Vector(1, 0, 0),		// ceiling
		Vector(1, 0, 0),		// north wall
		Vector(1, 0, 0),		// south wall
		Vector(0, 1, 0),		// east wall
		Vector(0, 1, 0),		// west wall
	};

	normal = CrossProduct(vertices[1]-vertices[0], vertices[2]-vertices[0]);
	normal.Normalize();

	float maxdot = 0;
	int dir = -1;
	for (int i = 0; i < 6; i++){
		float dot = DotProduct(normal, FaceNormals[i]);
		if (dot >= maxdot){
			maxdot = dot;
			dir = i;
		}
	}
	if(dir == -1)
		return;
	
	axis_v = DownVectors[dir];
	if(!align_face){
		axis_u = RightVectors[dir];
	}else{
		axis_u = CrossProduct(normal, axis_v);
		axis_u.Normalize();
		axis_v = CrossProduct(axis_u, normal);
		axis_v.Normalize();
	}
	//todo: implement rotation (rip off CMapFace::RotateTextureAxes)
}

bool editorFace::intersect(Vector origin, Vector direction){
	return false;//todo implement, don't forget to discard when facing wrong direction
}

void editorFace::move(Vector offset){
	for(unsigned int i = 0; i < vertices.size(); i++){
		vertices[i] = vertices[i] + offset;
	}
}

void editorFace::toFile(FILE * f){
	int vertices_c = vertices.size();
	fwrite(&vertices_c, sizeof(int), 1, f);
	for(unsigned int i = 0; i < vertices.size(); i++){
		fwrite(&vertices[i], sizeof(Vector), 1, f);
	}
	//fwrite(vertices, sizeof(Vector), vertices_c, f);
	fwrite(&offset_u, sizeof(float), 1, f);
	fwrite(&offset_v, sizeof(float), 1, f);
	fwrite(&rotation, sizeof(float), 1, f);
	fwrite(&scale_u, sizeof(float), 1, f);
	fwrite(&scale_v, sizeof(float), 1, f);
	fwrite(&texture, sizeof(int), 1, f);
}

//load from file
editorFace::editorFace(FILE * f){
	int vertices_c;
	fread(&vertices_c, sizeof(int), 1, f);
	/*vertices = (Vector*)malloc(sizeof(Vector)*vertices_c);
	fread(vertices, sizeof(Vector), vertices_c, f);*/
	vertices.resize(vertices_c);
	for(int i = 0; i < vertices_c; i++){
		fread(&vertices[i], sizeof(Vector), 1, f);
	}
	fread(&offset_u, sizeof(float), 1, f);
	fread(&offset_v, sizeof(float), 1, f);
	fread(&rotation, sizeof(float), 1, f);
	fread(&scale_u, sizeof(float), 1, f);
	fread(&scale_v, sizeof(float), 1, f);
	fread(&texture, sizeof(int), 1, f);

	calcUV(false);	//align to world as default
}

void editorFace::toMAP(FILE * f){
	texlist_t * tex;
	g_textures->IDWadTex(texture, NULL, &tex);
	fprintf(f, "( %.0f %.0f %.0f ) ( %.0f %.0f %.0f ) ( %.0f %.0f %.0f ) %s [ %.0f %.0f %.0f %.0f ] [ %.0f %.0f %.0f %.0f ] %.0f %.0f %.0f\n",
		vertices[0][0], vertices[0][1], vertices[0][2]+VERTICAL_OFFSET,
		vertices[1][0], vertices[1][1], vertices[1][2]+VERTICAL_OFFSET,
		vertices[2][0], vertices[2][1], vertices[2][2]+VERTICAL_OFFSET,
		tex->texname,
		axis_u[0], axis_u[1], axis_u[2], offset_u,
		axis_v[0], axis_v[1], axis_v[2], offset_v,
		rotation, scale_u, scale_v);
}

editorFace::~editorFace(){
	//free(vertices);
	vertices.clear();
}