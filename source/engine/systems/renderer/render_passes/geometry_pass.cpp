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

#include "geometry_pass.h"
#include "../render_sources/geometry_source.h"
#include "lighting.h"

#include "../renderer.h"
#include "../shaders_storage.h"
#include "../../resource_system/resource_system.h"
#include "../../resource_system/resources/framebuffer_object.h"
#include "../../resource_system/resources/program.h"
#include "../../resource_system/resources/mesh.h"
#include "../../resource_system/resources/texture.h"

#include "../../../entity.h"
#include "../../../components/drawable_component.h"
#include "../../../components/transform_component.h"

namespace_begin

GeometryPass::GeometryPass(ResourceSystem& resSystem, const Renderer& renderer)
{
	glm::vec2 viewport = renderer.GetViewport();

	m_fbo = resSystem.Create<FrameBufferObject>();
	m_geometryProgram = resSystem.Create<Program>();
	m_geometryProgram->AttachVertexObject(renderer.shaderStorage->GetGeometryVert().c_str());
	m_geometryProgram->AttachFragmentObject(renderer.shaderStorage->GetGeometryFrag().c_str());
	m_geometryProgram->AttachFragmentObject(renderer.shaderStorage->GetUtils().c_str());
	m_geometryProgram->CompileProgram();
	m_geometryProgram->Bind();

	m_albedoTexture = resSystem.Create<Texture>();
	m_albedoTexture->Create();
	m_albedoTexture->Bind(0);
	m_albedoTexture->DefineParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_albedoTexture->DefineParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_albedoTexture->DefineParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_albedoTexture->DefineParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_albedoTexture->DefineBuffer(viewport, 0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	m_normalsTexture = resSystem.Create<Texture>();
	m_normalsTexture->Create();
	m_normalsTexture->Bind(0);
	m_normalsTexture->DefineParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_normalsTexture->DefineParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_normalsTexture->DefineParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_normalsTexture->DefineParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_normalsTexture->DefineBuffer(viewport, 0, GL_RGB16F, GL_RGB, GL_FLOAT, NULL);

	m_materialTexture = resSystem.Create<Texture>();
	m_materialTexture->Create();
	m_materialTexture->Bind(0);
	m_materialTexture->DefineParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_materialTexture->DefineParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_materialTexture->DefineParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_materialTexture->DefineParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_materialTexture->DefineBuffer(viewport, 0, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	m_velocityTexture = resSystem.Create<Texture>();
	m_velocityTexture->Create();
	m_velocityTexture->Bind(0);
	m_velocityTexture->DefineParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
	m_velocityTexture->DefineParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
	m_velocityTexture->DefineParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_velocityTexture->DefineParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_velocityTexture->DefineBuffer(viewport, 0, GL_RG16F, GL_RG, GL_UNSIGNED_BYTE, NULL);

	m_depthTexture = resSystem.Create<Texture>();
	m_depthTexture->Create();
	m_depthTexture->Bind(0);
	m_depthTexture->DefineParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_depthTexture->DefineParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	m_depthTexture->DefineParameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	m_depthTexture->DefineParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	m_depthTexture->DefineBuffer(viewport, 0, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	m_fbo->Init();
	m_fbo->Bind();
	m_fbo->AttachTarget(m_albedoTexture, GL_COLOR_ATTACHMENT0, 0);
	m_fbo->AttachTarget(m_normalsTexture, GL_COLOR_ATTACHMENT1, 0);
	m_fbo->AttachTarget(m_materialTexture, GL_COLOR_ATTACHMENT2, 0);
	m_fbo->AttachTarget(m_velocityTexture, GL_COLOR_ATTACHMENT3, 0);
	m_fbo->AttachTarget(m_depthTexture, GL_DEPTH_ATTACHMENT, 0);
	uint32 attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	m_fbo->DefineDrawAttachments(attachments, SizeofArray(attachments));
	m_fbo->DefineReadAttachment(GL_NONE);
}

GeometryPass::~GeometryPass() {}

void GeometryPass::Destroy()
{
	m_geometryProgram->Free();

	m_albedoTexture->Free();
	m_normalsTexture->Free();
	m_materialTexture->Free();
	m_velocityTexture->Free();
	m_depthTexture->Free();

	m_fbo->Free();
}

void GeometryPass::Render(const Renderer& renderer, const GeometrySource& source)
{
	glm::vec2 viewport = renderer.GetViewport();

	m_fbo->Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	const glm::mat4x4& projViewMatrix = renderer.GetProjViewMatrix();

	//----GBufferPass----//
	for (std::shared_ptr<DrawableComponent> drawable : source.Drawables)
	{
		std::shared_ptr<TransformComponent> transform = drawable->GetOwner()->GetComponent<TransformComponent>();

		m_geometryProgram->Bind();
		m_geometryProgram->SetUniformMat4("modelMatrix", false, (const float*)glm::value_ptr(transform->GetMatrix()));
		m_geometryProgram->SetUniformMat4("projViewModelMatrix", false, (const float*)glm::value_ptr(projViewMatrix * transform->GetMatrix()));
		m_geometryProgram->SetUniformMat4("projViewPrevModelMatrix", false, (const float*)glm::value_ptr(projViewMatrix * transform->GetPrevFrameMatrix()));

		m_geometryProgram->SetUniformMat3("normalMatrix", false, (const float*)glm::value_ptr(glm::inverseTranspose(glm::mat3x3(transform->GetMatrix()))));
		m_geometryProgram->SetUniformTexture("material.albedoMap", 0);
		m_geometryProgram->SetUniformTexture("material.normalMap", 1);
		m_geometryProgram->SetUniformTexture("material.metallicMap", 2);
		m_geometryProgram->SetUniformTexture("material.roughnessMap", 3);
		m_geometryProgram->SetUniformInt("material.normalsBinded", 1);

		drawable->GetAlbedoMap()->Bind(0);
		drawable->GetNormalMap()->Bind(1);
		drawable->GetMetallicMap()->Bind(2);
		drawable->GetRoughnessMap()->Bind(3);

		m_geometryProgram->SetUniformInt("numTiles", drawable->GetNumTiles());
		m_geometryProgram->SetUniformInt("blurMask", drawable->IsMaskedForMotionBlur());
		m_geometryProgram->SetUniformVec2("viewport", viewport.x, viewport.y);
		drawable->GetMesh()->BindAndDraw();
	}
}

namespace_end