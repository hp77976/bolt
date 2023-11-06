#include "../../core/scene.h"
//#include "../../core/images/include.h"
#include "../filters/gaussian.h"

//opens an image file and puts results into scene file. does not make textures!
void scene::import_image(FILE* f)
{
	uint64_t id;
	uint32_t len;
	
	fread(&id,sizeof(id),1,f);
	fread(&len,sizeof(len),1,f);

	char* c_path = new char[len];
	fread(c_path,sizeof(char),len,f);
	std::string path = std::string(c_path,len);
	delete[] c_path;

	log(LOG_DEBUG,"Image path: " + path + "\n");

	//TODO: fix this
	//gaussian_filter* fi = new gaussian_filter(1.0f,1.0f,1);
	//filter* fi = import_filter(f);

	/*bitmap* bmp = new bitmap(path.c_str());
	mipmap* mmp = new mipmap(bmp,fi,mipmap::EBilinear,filter::ERepeat,filter::ERepeat,1.0f,false);

	bmp->m_id = id;
	mmp->m_id = id;

	bitmaps.push_back(bmp);
	mipmaps.push_back(mmp);
	mipmap_name_ptr.emplace(path,mmp);

	logger::log(LOG_DEBUG,"Image ID: " + std::to_string(id) + "\n");*/

	//delete g; //could this be breaking something???
};