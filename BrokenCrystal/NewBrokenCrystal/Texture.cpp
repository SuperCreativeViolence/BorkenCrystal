#include "Texture.h"
#include "lodepng.h"
#include <iostream>

Texture::Texture(const char* fileName)
{
	image = std::vector<unsigned char>();
	unsigned error = lodepng::decode(image, width, height, fileName);
	printf("Loading texture: %s\n", fileName);
	if (error)
	{
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl << std::endl;
		return;
	}
	loaded = true;
}

btVector3 Texture::GetPixel(unsigned x, unsigned y) const
{
	if (!loaded)
		return btVector3(1, 0, 1);

	double r, g, b;
	r = (double)image.at(y*width * 4 + x) / 255.;
	g = (double)image.at(y*width * 4 + x + 1) / 255.;
	b = (double)image.at(y*width * 4 + x + 2) / 255.;
	return btVector3(r, g, b);
}

btVector3 Texture::GetPixel(double u, double v) const
{
	if (!loaded)
		return (btVector3(1, 0, 1));

	int x = (fmod(fabs(u), 1.0)) * (width - 1);
	int y = (1. - fmod(fabs(v), 1.0)) * (height - 1);

	double r, g, b;
	try
	{
		r = (double)image.at(y*width * 4 + x * 4) / 255.;
		g = (double)image.at(y*width * 4 + x * 4 + 1) / 255.;
		b = (double)image.at(y*width * 4 + x * 4 + 2) / 255.;
		return btVector3(r, g, b);
	}
	catch (const std::out_of_range& e)
	{
		printf("error with uv, yx: %lf, %lf - %i, %i (width, height: %i, %i) \n", u, v, x, y, width, height);
		return btVector3(0, 1, 0);
	}
}

bool Texture::IsLoaded() const
{
	return loaded;
}

