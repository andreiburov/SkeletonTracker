#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#define SMPL_POSEDIRS_ELEMENTS_COUNT 4278690

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		std::cout << "./ConvertPosedirsToBinary input output\n" << std::endl;
		return 0;
	}

	std::cout << "Converting " << argv[1] << " to " << argv[2] << std::endl;

    float* posedirs = new float[SMPL_POSEDIRS_ELEMENTS_COUNT];

    {
		std::ifstream in(argv[1], std::ifstream::in);

		for (unsigned int i = 0; i < SMPL_POSEDIRS_ELEMENTS_COUNT; i++)
		{
    	    float f;
			in >> f;
    	    posedirs[i] = f;
		}

    	in.close();
    }

	FILE* out = fopen(argv[2], "wb");
	fwrite(posedirs, sizeof(float), SMPL_POSEDIRS_ELEMENTS_COUNT, out);
	fclose(out);

    delete[] posedirs;

    {
        float* p = new float[SMPL_POSEDIRS_ELEMENTS_COUNT];
        FILE* in = fopen(argv[2], "rb");
        fread(p, sizeof(float), SMPL_POSEDIRS_ELEMENTS_COUNT, in);
        fclose(in);
        delete[] p;
    }

	return 0;
}
