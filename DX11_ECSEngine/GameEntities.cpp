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

void GameEntities::VertexBoneData::AddBoneData(UINT BoneID, float Weight,UINT x)
{
	for (UINT i = 0; i < sizeof(IDs); i++) {
		if (Weights[i] == 0.0) {
			IDs[i] = BoneID;
			Weights[i] = Weight;
			return;
		}
	}

	// should never get here - more bones than we have space for
	assert(0);
}

// -----------------------------------------------------------------------
// - Iterate 'n' times through all entities having the two mesh components.
// - Store vb, ib, and indices after reading from the obj file.
// - Assimp for model loading. Need to add animation.
// -----------------------------------------------------------------------
void GameEntities::LoadMesh(const char* objFile, entt::registry& registry)
{

	auto new_MeshEntity = registry.create();
	auto mycompMesh = registry.view<MeshEntityData, MeshRenderVars>(); // mesh

	for (auto [entity, obj_MeshEntityDefault, mesh_comp] : mycompMesh.each()) 
	{

		new_MeshEntity = entity;

		// Variables used while reading the file
		std::vector<DirectX::XMFLOAT3> positions;     // Positions from the file
		std::vector<DirectX::XMFLOAT3> normals;       // Normals from the file
		std::vector<DirectX::XMFLOAT2> uvs;           // UVs from the file
		std::vector<Vertex> verts;           // Verts we're assembling
		std::vector<UINT> indices;           // Indices of these verts

		// Open Assimp to load the file. Create object.
		Assimp::Importer importer;

		// Triangulate- convert corner points to vertices, 
		// ConvertToLeftHanded- convert to dx format (default OpenGL format), 
		// FlipUVs-for dx format 
		const aiScene* pScene = importer.ReadFile(objFile,
			aiProcess_Triangulate |
			aiProcess_FlipWindingOrder |
			aiProcess_FlipUVs |
			aiProcess_MakeLeftHanded);

		// 1 mesh for now
		aiMesh* mesh = pScene->mMeshes[0];

		//store the positions, normals, uvs and push it in struct buffer 'verts'. Track 'vertCounter'.
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			//vertex.Normal.x = mesh->mNormals[i].x;
			//vertex.Normal.y = mesh->mNormals[i].y;
			//vertex.Normal.z = mesh->mNormals[i].z;

			vertex.UV.x = mesh->mTextureCoords[0][i].x;
			vertex.UV.y = mesh->mTextureCoords[0][i].y;

			verts.push_back(vertex);
		}

		// Calculate the indices
		for (UINT c = 0; c < mesh->mNumFaces; c++)
			for (UINT e = 0; e < mesh->mFaces[c].mNumIndices; e++)
			{
				indices.push_back(mesh->mFaces[c].mIndices[e]);
			}

		// Use tag?
		//auto vbib_comp = registry.view<MeshRenderVars, entt::tag<"VbIb"_hs>>();

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
		//auto mesh_comp = registry.get<MeshRenderVars>(new_MeshEntity);

		// Store vertex buffer, index buffer & indices.
		for (auto entity : m_comp)
		{
			// store the GPU vars comp
			auto& mycomp = m_comp.get<RendererMainVars>(entity);

			D3D11_BUFFER_DESC vbd;
			vbd.Usage = D3D11_USAGE_IMMUTABLE;
			vbd.ByteWidth = sizeof(Vertex) * verts.size(); // Number of vertices
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
			ibd.ByteWidth = sizeof(unsigned int) * verts.size(); // Number of indices
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.CPUAccessFlags = 0;
			ibd.MiscFlags = 0;
			ibd.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA initialIndexData;
			initialIndexData.pSysMem = &indices[0];
			mycomp.device->CreateBuffer(&ibd, &initialIndexData, &mesh_comp.ib);

			// Save the indices
			mesh_comp.numIndices = verts.size();

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

		LoadBones(mesh);
	}

	
}

void GameEntities::LoadBones(aiMesh* mesh)
{
	std::vector<VertexBoneData> Bones;

	Bones.resize(mesh->mNumVertices);

	for (int i = 0; i < mesh->mNumBones; i++)
	{
		int BoneIndex = 0;
		std::string BoneName(mesh->mBones[i]->mName.data);

		if (mBoneMapping.find(BoneName) == mBoneMapping.end())
		{
			BoneIndex = mNumBones;
			mNumBones++;
			BoneInfo bi;
			mBoneInfo.push_back(bi);

		}
		else
		{
			BoneIndex = mBoneMapping[BoneName];
		}
		aiMatrix4x4 offset = mesh->mBones[i]->mOffsetMatrix;
		//DirectX::XMMatrixTranspose(temp);
		DirectX::XMMATRIX meshToBoneTransform = DirectX::XMMatrixTranspose(
			DirectX::XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
				offset.b1, offset.b2, offset.b3, offset.b4,
				offset.c1, offset.c2, offset.c3, offset.c4,
				offset.d1, offset.d2, offset.d3, offset.d4));
		mBoneInfo[BoneIndex].BoneOffset = meshToBoneTransform;
		mBoneMapping[BoneName] = BoneIndex;

		for (int x = 0; x < mesh->mBones[i]->mNumWeights; x++)
		{
			int VertexID = mesh->mBones[i]->mWeights[x].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[x].mWeight;

			Bones[VertexID].AddBoneData(BoneIndex, Weight, x);
		}
	}

}


// -----------------------------------------------------------------------
// - Iterate 'n' times through all entities having the two mesh components.
// - Store vb, ib, and indices after reading from the obj file.
// - Assimp for model loading. Need to add animation.
// -----------------------------------------------------------------------
void GameEntities::LoadMeshSky(const char* objFile, entt::registry& registry)
{
	auto new_MeshEntity = registry.create();

	auto mycompMesh = registry.view<MeshEntityDataSky, MeshRenderVarsSky>(); // sky mesh

	for (auto entity : mycompMesh)
	{

		new_MeshEntity = entity;

		// Variables used while reading the file
		std::vector<DirectX::XMFLOAT3> positions;     // Positions from the file
		std::vector<DirectX::XMFLOAT3> normals;       // Normals from the file
		std::vector<DirectX::XMFLOAT2> uvs;           // UVs from the file
		std::vector<Vertex> verts;					  // Verts we're assembling
		std::vector<UINT> indices;                    // Indices of these verts

		// Open Assimp to load the file. Create object.
		Assimp::Importer importer;

		// Triangulate- convert corner points to vertices, 
		// ConvertToLeftHanded- convert to dx format (default OpenGL format), 
		// FlipUVs-for dx format 
		const aiScene* pScene = importer.ReadFile(objFile,
			aiProcess_Triangulate |
			aiProcess_FlipWindingOrder |
			aiProcess_FlipUVs |
			aiProcess_MakeLeftHanded);

		// 1 mesh for now each time using Assimp. Could be optimized further probably as part of ECS.
		aiMesh* mesh = pScene->mMeshes[0];

		//store the positions, normals, uvs and push it in struct buffer 'verts'. Track 'vertCounter'.
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;

			vertex.UV.x = mesh->mTextureCoords[0][i].x;
			vertex.UV.y = mesh->mTextureCoords[0][i].y;

			verts.push_back(vertex);
		}

		// Calculate the indices
		for (UINT c = 0; c < mesh->mNumFaces; c++)
			for (UINT e = 0; e < mesh->mFaces[c].mNumIndices; e++)
			{
				indices.push_back(mesh->mFaces[c].mIndices[e]);
			}

		// Use tag?
		//auto vbib_comp = registry.view<MeshRenderVars, entt::tag<"VbIb"_hs>>();

		// Local mesh properties
		MeshEntityDataSky obj_MeshEntityDefault;

		// Set up mesh transforms
		DirectX::XMStoreFloat4x4(&obj_MeshEntityDefault.worldMatrix, DirectX::XMMatrixIdentity());


		obj_MeshEntityDefault.position = DirectX::XMFLOAT3(0.0f,0.0f,0.0f);
		obj_MeshEntityDefault.rotation = DirectX::XMFLOAT3(0, 0, 0);
		obj_MeshEntityDefault.scale = DirectX::XMFLOAT3(2.0f, 2.0f, 2.0f);

		// GPU comp vars for storing the vb and ib.
		auto m_comp = registry.view<RendererMainVars>();

		// Do we have this vb,ib & indices component in the registry?
		auto mesh_comp = registry.get<MeshRenderVarsSky>(new_MeshEntity);

		// Store vertex buffer, index buffer & indices.
		for (auto entity : m_comp)
		{
			// store the GPU vars comp
			auto& mycomp = m_comp.get<RendererMainVars>(entity);

			D3D11_BUFFER_DESC vbd;
			vbd.Usage = D3D11_USAGE_IMMUTABLE;
			vbd.ByteWidth = sizeof(Vertex) * verts.size(); // Number of vertices
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
			ibd.ByteWidth = sizeof(unsigned int) * verts.size(); // Number of indices
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.CPUAccessFlags = 0;
			ibd.MiscFlags = 0;
			ibd.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA initialIndexData;
			initialIndexData.pSysMem = &indices[0];
			mycomp.device->CreateBuffer(&ibd, &initialIndexData, &mesh_comp.ib);

			// Save the indices
			mesh_comp.numIndices = verts.size();

			// Tell the input assembler stage of the pipeline what kind of
			// geometric primitives (points, lines or triangles) we want to draw.  
			// Essentially: "What kind of shape should the GPU draw with our data?"
			mycomp.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// replace the components back to the registry with this entity
			registry.replace<RendererMainVars>(entity, mycomp.swapChain, mycomp.device, mycomp.context, mycomp.backBufferRTV, mycomp.depthStencilView);
			registry.replace<MeshRenderVarsSky>(new_MeshEntity, mesh_comp.vb, mesh_comp.ib, mesh_comp.numIndices);
		}

		registry.replace<MeshEntityDataSky>(new_MeshEntity, obj_MeshEntityDefault.worldMatrix, obj_MeshEntityDefault.position, obj_MeshEntityDefault.rotation
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