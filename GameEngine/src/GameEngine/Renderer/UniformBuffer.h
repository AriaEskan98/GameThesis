#pragma once

#include "GameEngine/Core/Base.h"

namespace GameEngine {

	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
		
		static Handle<UniformBuffer> Create(uint32_t size, uint32_t binding);
	};

}
