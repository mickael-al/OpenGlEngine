#ifndef __IMAGE_DATA__
#define __IMAGE_DATA__

#include <iostream>
#include "Color.hpp"

struct stbimg
{
	unsigned char* data;
	int height;
	int width;
	int ch;
};

class ImageData
{
public:
	ImageData(const char* src, const char* dst);
	ImageData(const char * path);
	ImageData(ImageData * id, int width,int height);	
	~ImageData();
	Color** getColor() const;
	int getWidth() const;
	int getHeight() const;
	int getChannelCount() const;
	bool isLoad() const;//isload
	void write();
	std::string getPath() const;
	static stbimg loadData(const std::string path);
	static void freeData(stbimg stbi);
private:
	Color** m_color;
	int m_width;
	int m_height;
	int m_channel_count;
	std::string m_path;
	bool m_isload = false;
};

#endif // !__IMAGE_DATA__