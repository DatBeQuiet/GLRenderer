// Copyright (c) 2019 Guillem Costa Miquel, kayter72@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. The above copyright notice and this permission notice shall be included in
//	  all copies or substantial portions of the Software.

#include "texture.h"

namespace_begin

Texture::Texture()
	: m_ID(0)
	, m_currentBufferSize(0, 0)
{
}

Texture::~Texture()
{
	assert(m_ID == 0);
}

void Texture::Create()
{
	assert(m_ID == 0);
	glGenTextures(1, &m_ID);
}

void Texture::Free()
{
	assert(m_ID != 0);
	glDeleteTextures(1, &m_ID);
	m_ID = 0;
	m_currentBufferSize = { 0,0 };
}

void Texture::Bind(int textureIndex) const
{
	assert(m_ID != 0);
	glActiveTexture(GL_TEXTURE0 + textureIndex);
	glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::DefineParameter(uint32 parameter, uint32 value) const
{
	assert(m_ID != 0);
	glTexParameteri(GL_TEXTURE_2D, parameter, value);
}

void Texture::DefineBuffer(const glm::vec2& size, uint32 level, uint32 internalFormat, uint32 format, uint32 dataType, const void* data)
{
	assert(m_ID != 0);
	m_currentBufferSize = size;
	glTexImage2D(GL_TEXTURE_2D, level, internalFormat, size.x, size.y, 0, format, dataType, data);
}

void Texture::DefineBuffer3D(const glm::vec2& size, uint32 level, uint32 numberOfTextures, uint32 internalFormat, uint32 format, uint32 dataType, const void* data)
{
	glTexImage3D(GL_TEXTURE_2D, level, internalFormat, size.x, size.y, numberOfTextures, 0, format, dataType, data);
}

void Texture::DefineSubBuffer3D(const glm::vec2& size, const glm::vec3& offsets, uint32 level, uint32 numberOfTextures, uint32 format, uint32 dataType, const void* data)
{
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, offsets.x, offsets.y, offsets.z, size.x, size.y, 1, format, dataType, data);
}

void Texture::GenerateMipMaps() const
{
	assert(m_ID != 0);
	glGenerateMipmap(GL_TEXTURE_2D);
}

uint32 Texture::GetID() const
{
	return m_ID;
}

const glm::vec2& Texture::GetCurrentBufferSize() const
{
	return m_currentBufferSize;
}

namespace_end