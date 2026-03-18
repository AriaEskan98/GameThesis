#pragma once

#include "GameEngine/Core/Buffer.h"

namespace GameEngine {

	class FileSystem
	{
	public:
		// TODO: move to FileSystem class
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};

}
