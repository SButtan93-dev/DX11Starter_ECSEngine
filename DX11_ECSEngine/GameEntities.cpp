#include "GameEntities.h"
// DirectX Tool Kit headers
#include "WICTextureLoader.h" // WIC = Windows Imaging Component

float pos_xz = 5.0f;

float pos_y = -2.0f;

GameEntities::GameEntities()
{
}


GameEntities::~GameEntities()
{
}


// -----------------------------------------------------------------------
// - Iterate 'n' times through all entities having the two mesh components.
// - Store vb, ib, and indices after reading from the obj file.
// - Could be improved with ASSIMP.
// -----------------------------------------------------------------------
void GameEntities::LoadMesh(const char * objFile, entt::registry & registry)
{
	auto new_MeshEntity = registry.create();
	auto mycompMesh = registry.view<MeshEntityData, MeshRenderVars>(); // mesh
	
	// 
	for (auto entity : mycompMesh)
	{

		new_MeshEntity = entity;

		std::ifstream obj(objFile);

		// Check for successful open
		if (!obj.is_open())
			return;

		// Variables used while reading the file
		std::vector<DirectX::XMFLOAT3> positions;     // Positions from the file
		std::vector<DirectX::XMFLOAT3> normals;       // Normals from the file
		std::vector<DirectX::XMFLOAT2> uvs;           // UVs from the file
		std::vector<Vertex> verts;					  // Verts we're assembling
		std::vector<UINT> indices;                    // Indices of these verts
		unsigned int vertCounter = 0;                 // Count of vertices/indices
		char chars[100];                              // String for line reading

		// Still have data left?
		while (obj.good())
		{
			// Get the line (100 characters should be more than enough)
			obj.getline(chars, 100);

			// Check the type of line
			if (chars[0] == 'v' && chars[1] == 'n')
			{
				// Read the 3 numbers directly into an XMFLOAT3
				DirectX::XMFLOAT3 norm;
				sscanf_s(
					chars,
					"vn %f %f %f",
					&norm.x, &norm.y, &norm.z);

				// Add to the list of normals
				normals.push_back(norm);
			}
			else if (chars[0] == 'v' && chars[1] == 't')
			{
				// Read the 2 numbers directly into an XMFLOAT2
				DirectX::XMFLOAT2 uv;
				sscanf_s(
					chars,
					"vt %f %f",
					&uv.x, &uv.y);

				// Add to the list of uv's
				uvs.push_back(uv);
			}
			else if (chars[0] == 'v')
			{
				// Read the 3 numbers directly into an XMFLOAT3
				DirectX::XMFLOAT3 pos;
				sscanf_s(
					chars,
					"v %f %f %f",
					&pos.x, &pos.y, &pos.z);

				// Add to the positions
				positions.push_back(pos);
			}
			else if (chars[0] == 'f')
			{
				// Read the face indices into an array
				unsigned int i[12];
				int facesRead = sscanf_s(
					chars,
					"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
					&i[0], &i[1], &i[2],
					&i[3], &i[4], &i[5],
					&i[6], &i[7], &i[8],
					&i[9], &i[10], &i[11]);

				// - Create the verts by looking up
				//    corresponding data from vectors
				// - OBJ File indices are 1-based, so
				//    they need to be adusted
				Vertex v1;
				v1.Position = positions[i[0] - 1];
				v1.UV = uvs[i[1] - 1];
				v1.Normal = normals[i[2] - 1];

				Vertex v2;
				v2.Position = positions[i[3] - 1];
				v2.UV = uvs[i[4] - 1];
				v2.Normal = normals[i[5] - 1];

				Vertex v3;
				v3.Position = positions[i[6] - 1];
				v3.UV = uvs[i[7] - 1];
				v3.Normal = normals[i[8] - 1];

				// The model is most likely in a right-handed space,
				// especially if it came from Maya.  We want to convert
				// to a left-handed space for DirectX.  This means we 
				// need to:
				//  - Invert the Z position
				//  - Invert the normal's Z
				//  - Flip the winding order
				// We also need to flip the UV coordinate since DirectX
				// defines (0,0) as the top left of the texture, and many
				// 3D modeling packages use the bottom left as (0,0)

				// Flip the UV's since they're probably "upside down"
				v1.UV.y = 1.0f - v1.UV.y;
				v2.UV.y = 1.0f - v2.UV.y;
				v3.UV.y = 1.0f - v3.UV.y;

				// Flip Z (LH vs. RH)
				v1.Position.z *= -1.0f;
				v2.Position.z *= -1.0f;
				v3.Position.z *= -1.0f;

				// Flip normal Z
				v1.Normal.z *= -1.0f;
				v2.Normal.z *= -1.0f;
				v3.Normal.z *= -1.0f;

				// Add the verts to the vector (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v3);
				verts.push_back(v2);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;

				// Was there a 4th face?
				if (facesRead == 12)
				{
					// Make the last vertex
					Vertex v4;
					v4.Position = positions[i[9] - 1];
					v4.UV = uvs[i[10] - 1];
					v4.Normal = normals[i[11] - 1];

					// Flip the UV, Z pos and normal
					v4.UV.y = 1.0f - v4.UV.y;
					v4.Position.z *= -1.0f;
					v4.Normal.z *= -1.0f;

					// Add a whole triangle (flipping the winding order)
					verts.push_back(v1);
					verts.push_back(v4);
					verts.push_back(v3);

					// Add three more indices
					indices.push_back(vertCounter); vertCounter += 1;
					indices.push_back(vertCounter); vertCounter += 1;
					indices.push_back(vertCounter); vertCounter += 1;
				}
			}
		}

		// Close the file and create the actual buffers
		obj.close();

		// Use tag?
		//auto vbib_comp = registry.view<MeshRenderVars, entt::tag<"VbIb"_hs>>();

		// Local mesh properties
		MeshEntityData obj_MeshEntityDefault;

		// Set up mesh transforms
		DirectX::XMStoreFloat4x4(&obj_MeshEntityDefault.worldMatrix, DirectX::XMMatrixIdentity());

		if (pos_xz > 80.0f)
		{
			obj_MeshEntityDefault.position = DirectX::XMFLOAT3(pos_xz, pos_y, pos_xz + 2.0f);
			pos_y += 1.0f;
			pos_xz = 5.0f;
		}
		else
		{
			obj_MeshEntityDefault.position = DirectX::XMFLOAT3(pos_xz, pos_y, pos_xz + 2.0f);
			pos_xz += 1.0f;
		}

		obj_MeshEntityDefault.rotation = DirectX::XMFLOAT3(0, 0, 0);
		obj_MeshEntityDefault.scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		// GPU comp vars for storing the vb and ib.
		auto m_comp = registry.view<RendererMainVars>();

		// Do we have this vb,ib & indices component in the registry?
		auto mesh_comp = registry.get<MeshRenderVars>(new_MeshEntity);

		// Store vertex buffer, index buffer & indices.
		for (auto entity : m_comp)
		{
			// store the GPU vars comp
			auto& mycomp = m_comp.get<RendererMainVars>(entity);

			D3D11_BUFFER_DESC vbd;
			vbd.Usage = D3D11_USAGE_IMMUTABLE;
			vbd.ByteWidth = sizeof(Vertex) * vertCounter; // Number of vertices
			vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbd.CPUAccessFlags = 0;
			vbd.MiscFlags = 0;
			vbd.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA initialVertexData;
			initialVertexData.pSysMem = &verts[0];
			mycomp.device->CreateBuffer(&vbd, &initialVertexData, &mesh_comp.vb);

			// Create the index buffer
			D3D11_BUFFER_DESC ibd;
			ibd.Usage = D3D11_USAGE_IMMUTABLE;
			ibd.ByteWidth = sizeof(unsigned int) * vertCounter; // Number of indices
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.CPUAccessFlags = 0;
			ibd.MiscFlags = 0;
			ibd.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA initialIndexData;
			initialIndexData.pSysMem = &indices[0];
			mycomp.device->CreateBuffer(&ibd, &initialIndexData, &mesh_comp.ib);

			// Save the indices
			mesh_comp.numIndices = vertCounter;

			// Tell the input assembler stage of the pipeline what kind of
			// geometric primitives (points, lines or triangles) we want to draw.  
			// Essentially: "What kind of shape should the GPU draw with our data?"
			mycomp.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// replace the components back to the registry with this entity
			registry.replace<RendererMainVars>(entity, mycomp.swapChain, mycomp.device, mycomp.context, mycomp.backBufferRTV, mycomp.depthStencilView);
			registry.replace<MeshRenderVars>(new_MeshEntity, mesh_comp.vb, mesh_comp.ib, mesh_comp.numIndices);
		}

		registry.replace<MeshEntityData>(new_MeshEntity, obj_MeshEntityDefault.worldMatrix, obj_MeshEntityDefault.position, obj_MeshEntityDefault.rotation
			, obj_MeshEntityDefault.scale);

	}
}

// ---------------------------------------------------
// - Loads and saves textures using DX texture library
// - To load multiple textures, create more entities
// ---------------------------------------------------
void GameEntities::InitTexture(entt::registry& registry)
{
	// Find the texture through registry.
	entt::entity entityTexture;
	auto t_CompId = registry.view<TextureData>();
	for (auto entity : t_CompId)
	{
		entityTexture = entity;
	}
	auto t_comp = t_CompId.get<TextureData>(entityTexture);

	// GPU vars comp
	auto ref_CompRender = registry.view<RendererMainVars>();
	
	// Search for all the entities holding the GPU vars
	for (auto entity : ref_CompRender)
	{
		// get GPU components for this entity
		auto r_comp = ref_CompRender.get<RendererMainVars>(entity);

		// Load some textures
		DirectX::CreateWICTextureFromFile(
			r_comp.device,				// The Direct3D device for resource creation
			r_comp.context,				// Rendering context (this will auto-generate mip maps!!!)
			L"Textures/crate.png",		// Path to the file ("L" means wide characters)
			0,							// Texture ref?  No thanks!  (0 means we don't want an extra ref)
			&t_comp.crateSRV);			// Actual SRV for use with shaders

		DirectX::CreateWICTextureFromFile(r_comp.device, r_comp.context, L"Textures/rusty.jpg", 0, &t_comp.rustSRV);

		DirectX::CreateWICTextureFromFile(r_comp.device, r_comp.context, L"Textures/rustySpec.png", 0, &t_comp.specSRV);

		// Create a sampler state for sampling options
		D3D11_SAMPLER_DESC sampDesc = {}; // " = {}" fills the whole struct with zeros!
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;			// This will ensure mip maps are used!
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;		//D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// Tri-linear filtering
		sampDesc.MaxAnisotropy = 16; // Must be set for anisotropic filtering

		r_comp.device->CreateSamplerState(&sampDesc, &t_comp.sampler);

		registry.replace<RendererMainVars>(entity, r_comp.swapChain, r_comp.device, r_comp.context, r_comp.backBufferRTV, r_comp.depthStencilView);
	}

	registry.replace<TextureData>(entityTexture, t_comp.crateSRV, t_comp.rustSRV, t_comp.specSRV, t_comp.sampler);
}

// --------------------------------------------------
// - Used for test to show that texture components
//   can be toggled in game loop.
// - BasicSample, Diffuse & Spec are set to zero.
// - Call InitTexture() to load the components again.
// --------------------------------------------------
void GameEntities::CleanUpTexture(entt::registry& registry)
{
	// Texture
	entt::entity entityTexture;
	auto t_CompId = registry.view<TextureData>();
	for (auto entity : t_CompId)
	{
		entityTexture = entity;
	}
	auto t_comp = t_CompId.get<TextureData>(entityTexture);

	// GPU vars comp
	auto ref_CompRender = registry.view<RendererMainVars>();

	// Search for all the entities holding the GPU vars
	for (auto entity : ref_CompRender)
	{
		auto renderer = ref_CompRender.get<RendererMainVars>(entity);

		// Turn everything to zero
		// Create a sampler state for sampling options
		D3D11_SAMPLER_DESC sampDesc = {}; // " = {}" fills the whole struct with zeros!
		renderer.device->CreateSamplerState(&sampDesc, &t_comp.sampler);
		t_comp.rustSRV = { 0 };
		t_comp.specSRV = { 0 };

		registry.replace<RendererMainVars>(entity, renderer.swapChain, renderer.device, renderer.context, renderer.backBufferRTV, renderer.depthStencilView);
		registry.replace<TextureData>(entityTexture, t_comp.crateSRV, t_comp.rustSRV, t_comp.specSRV, t_comp.sampler);
	}
}

// not called... replaced in Draw(), could be called here again
void GameEntities::UpdateWorldMatrix(MeshEntityData * obj_meshData)
{
	DirectX::XMMATRIX trans = DirectX::XMMatrixTranslation(obj_meshData->position.x, obj_meshData->position.y, obj_meshData->position.z);
	DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(obj_meshData->rotation.x);
	DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(obj_meshData->rotation.y);
	DirectX::XMMATRIX rotZ = DirectX::XMMatrixRotationZ(obj_meshData->rotation.z);
	DirectX::XMMATRIX sc = DirectX::XMMatrixScaling(obj_meshData->scale.x, obj_meshData->scale.y, obj_meshData->scale.z);

	DirectX::XMMATRIX total = sc * trans;
	XMStoreFloat4x4(&obj_meshData->worldMatrix, XMMatrixTranspose(total));
}