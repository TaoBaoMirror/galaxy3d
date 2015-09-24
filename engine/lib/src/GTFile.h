#ifndef __GTFile_h__
#define __GTFile_h__

#include <vector>
#include <string>

namespace Galaxy3D
{
	class GTFile
	{
	public:
		static bool Exist(const std::string &path);
        // ����֮���ֶ�free�ͷ�
		static void *ReadAllBytes(const std::string &path, int *size);
		static void ReadAllText(const std::string &path, std::string &str);
		static void WriteAllBytes(const std::string &path, void *data, int size);
	};
}

#endif