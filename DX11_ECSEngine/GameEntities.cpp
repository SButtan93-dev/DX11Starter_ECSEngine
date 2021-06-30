#include "GameEntities.h"
// DirectX Tool Kit headers
#include "WICTextureLoader.h" // WIC = Windows Imaging Component

float pos_xz = 5.0f;

float pos_y = -2.0f;
Vertex vertex;
VertexBoneData2 temp_bone_data;

GameEntities::GameEntities()
{

}


GameEntities::~GameEntities()
{
}

void GameEntities::VertexBoneData::AddBoneData(UINT BoneID, float Weight)
{

	for (UINT i = 0; i < sizeof(IDs); i++) 
	{
		if (Weights[i] == 0.0) 
		{
			IDs[i] = BoneID;
			Weights[i] = Weight;

			if (i == 0)
			{
				//verts2.push_back()
				//temp_bone_data.IDs.x = BoneID;
				//temp_bone_data.Weights.x = Weight;
			}
			else if (i == 1)
			{
				//temp_bone_data.IDs.y = BoneID;
				//temp_bone_data.Weights.y = Weight;
			}
			else if (i == 2)
			{
				//temp_bone_data.IDs.z = BoneID;
				//temp_bone_data.Weights.z = Weight;
			}
			else if (i == 3)
			{
				//temp_bone_data.IDs.w = BoneID;
				//temp_bone_data.Weights.w = Weight;
			}
			
			//verts2.push_back();
			//verts->push_back(vertex);

			return;
		}

	}

	// should never get here - more bones than we have space for
	assert(0);
	//return;
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
          // Verts we're assembling
		std::vector<UINT> indices;           // Indices of these verts

		// Open Assimp to load the file. Create object.
		

		// Triangulate- convert corner points to vertices, 
		// ConvertToLeftHanded- convert to dx format (default OpenGL format), 
		// FlipUVs-for dx format 
		 pScene = importer.ReadFile(objFile,
			aiProcess_Triangulate |
			aiProcess_FlipWindingOrder |
			aiProcess_FlipUVs |
			aiProcess_MakeLeftHanded);

		// 1 mesh for now
		aiMesh* mesh = pScene->mMeshes[0];

		// Vertex vertex;

		// Calculate the indices
		for (UINT c = 0; c < mesh->mNumFaces; c++)
			for (UINT e = 0; e < mesh->mFaces[c].mNumIndices; e++)
			{
				indices.push_back(mesh->mFaces[c].mIndices[e]);
			}

		aiMatrix4x4 offset = pScene->mRootNode->mTransformation;
		GlobalInverseTransform = DirectX::XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
				offset.b1, offset.b2, offset.b3, offset.b4,
				offset.c1, offset.c2, offset.c3, offset.c4,
				offset.d1, offset.d2, offset.d3, offset.d4);

		GlobalInverseTransform = DirectX::XMMatrixInverse(nullptr, GlobalInverseTransform);

		//store the positions, normals, uvs and push it in struct buffer 'verts'. Track 'vertCounter'.
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			vertex.Position.x = mesh->mVertices[i].x;
			vertex.Position.y = mesh->mVertices[i].y;
			vertex.Position.z = mesh->mVertices[i].z;

			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;

			vertex.UV.x = mesh->mTextureCoords[0][i].x;
			vertex.UV.y = mesh->mTextureCoords[0][i].y;

			verts.push_back(vertex);
			//verts.at(i).Position.x = vertex.Position.x;
			//verts.at(i).Position.y = vertex.Position.y;
		}

		LoadBones(mesh);

		//unsigned int IDs2[NUM_BONES_PER_VEREX];
		//float Weights2[NUM_BONES_PER_VEREX];
		
		for (unsigned int k = 0; k < Bones.size(); k++)
		{

				verts.at(k).BoneIDs.x = Bones[k].IDs[0];
				verts.at(k).BoneIDs.y = Bones[k].IDs[1];
				verts.at(k).BoneIDs.z = Bones[k].IDs[2];
				verts.at(k).BoneIDs.w = Bones[k].IDs[3];

				verts.at(k).Weights.x = Bones[k].Weights[0];
				verts.at(k).Weights.y = Bones[k].Weights[1];
				verts.at(k).Weights.z = Bones[k].Weights[2];
				verts.at(k).Weights.w = Bones[k].Weights[3];

			
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

		obj_MeshEntityDefault.rotation = DirectX::XMFLOAT3(0.0f,0.0f, 0.0f);
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
			//  mesh_comp.ib = 0;
			//	mesh_comp.ib->Release();
			mycomp.device->CreateBuffer(&ibd, &initialIndexData, &mesh_comp.ib);

			// Save the indices
			mesh_comp.numIndices = verts.size();

			// Tell the input assembler stage of the pipeline what kind of
			// geometric primitives (points, lines or triangles) we want to draw.  
			// Essentially: "What kind of shape should the GPU draw with our data?"
			mycomp.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


			//D3D11_BUFFER_DESC vbd2;
			//vbd2.Usage = D3D11_USAGE_IMMUTABLE;
			//vbd2.ByteWidth = sizeof(VertexBoneData2) * verts.size(); // Number of vertices
			//vbd2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			//vbd2.CPUAccessFlags = 0;
			//vbd2.MiscFlags = 0;
			//vbd2.StructureByteStride = 0;
			//D3D11_SUBRESOURCE_DATA initialVertexData;
			//initialVertexData.pSysMem = &verts[0];
			//mycomp.device->CreateBuffer(&vbd, &initialVertexData, &mesh_comp.vb);

			//// Create the index buffer
			//D3D11_BUFFER_DESC ibd2;
			//ibd2.Usage = D3D11_USAGE_IMMUTABLE;
			//ibd2.ByteWidth = sizeof(unsigned int) * verts.size(); // Number of indices
			//ibd2.BindFlags = D3D11_BIND_INDEX_BUFFER;
			//ibd2.CPUAccessFlags = 0;
			//ibd2.MiscFlags = 0;
			//ibd2.StructureByteStride = 0;
			//D3D11_SUBRESOURCE_DATA initialIndexData;
			//initialIndexData.pSysMem = &indices[0];
			////  mesh_comp.ib = 0;
			////	mesh_comp.ib->Release();
			//mycomp.device->CreateBuffer(&ibd, &initialIndexData, &mesh_comp.ib);

			// replace the components back to the registry with this entity
			registry.replace<RendererMainVars>(entity, mycomp.swapChain, mycomp.device, mycomp.context, mycomp.backBufferRTV, mycomp.depthStencilView);
			registry.replace<MeshRenderVars>(new_MeshEntity, mesh_comp.vb, mesh_comp.ib, mesh_comp.numIndices);
		}

		registry.replace<MeshEntityData>(new_MeshEntity, obj_MeshEntityDefault.worldMatrix, obj_MeshEntityDefault.position, obj_MeshEntityDefault.rotation
			, obj_MeshEntityDefault.scale);

	}

}


void GameEntities::LoadBones(aiMesh* mesh)
{
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
		DirectX::XMMATRIX meshToBoneTransform = DirectX::XMMATRIX(offset.a1, offset.a2, offset.a3, offset.a4,
				offset.b1, offset.b2, offset.b3, offset.b4,
				offset.c1, offset.c2, offset.c3, offset.c4,
				offset.d1, offset.d2, offset.d3, offset.d4);
		mBoneInfo[BoneIndex].BoneOffset = meshToBoneTransform;
		mBoneMapping[BoneName] = BoneIndex;

		for (int x = 0; x < mesh->mBones[i]->mNumWeights; x++)
		{
			int VertexID = mesh->mBones[i]->mWeights[x].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[x].mWeight;

			//AddBoneData2(BoneIndex, Weight, VertexID);

					Bones[VertexID].AddBoneData(BoneIndex, Weight);
					
			// AddBoneData(BoneIndex, Weight, x);
		}
	}

}


//void GameEntities::AddBoneData2(UINT BoneID, float Weight, int vID)
//{
//	for (UINT j = 0; j < 4; j++) {
//		if (verts.at(vID).Weights.x == 0.0) {
//			verts.at(vID).IDs.x = BoneID;
//			verts.at(vID).Weights.x = Weight;
//
//			return;
//		}
//
//		// should never get here - more bones than we have space for
//		//assert(0);
//	}
//}


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


DirectX::XMMATRIX GameEntities::BoneTransform(float TimeInSeconds, std::vector<DirectX::XMFLOAT4X4>& Transforms)
{
	DirectX::XMMATRIX Identity = DirectX::XMMatrixIdentity();
	//Identity.InitIdentity();
	//DirectX::XMFLOAT4X4 abc;
	//DirectX::XMStoreFloat4x4(&abc, Identity);
	float TicksPerSecond = pScene->mAnimations[0]->mTicksPerSecond != 0 ?
		pScene->mAnimations[0]->mTicksPerSecond : 30.0f;
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, pScene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, pScene->mRootNode, Identity);

	Transforms.resize(mNumBones);

	for (UINT i = 0; i < mNumBones; i++) {
		//DirectX::XMFLOAT4X4 temp2;
		//DirectX::XMStoreFloat4x4(&temp2, mBoneInfo[i].FinalTransformation);

		//DirectX::XMMatrixTranspose(mBoneInfo[i].FinalTransformation);

		DirectX::XMStoreFloat4x4(&Transforms[i], mBoneInfo[i].FinalTransformation);
		//Transforms[i] = 
		//Transforms = mBoneInfo[i].FinalTransformation;
	}

	return Identity;
}


void GameEntities::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const DirectX::XMMATRIX& ParentTransform)
{
	std::string NodeName(pNode->mName.data);
	const aiAnimation* pAnim = pScene->mAnimations[0];

	NodeTransformation = DirectX::XMMATRIX(&pNode->mTransformation.a1);
	//I just read aiMatrix4x4 (aiMatrix to XMMATRIX format)
	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnim, NodeName);

	DirectX::XMMATRIX anim = DirectX::XMMatrixIdentity();
	if (pNodeAnim) {
		aiVector3D s;
		CalcInterpolatedScaling(s, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX ScalingM = DirectX::XMMatrixScaling(s.x, s.y, s.z);


		aiQuaternion q;
		CalcInterpolatedRotation(q, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX RotationM = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(q.x, q.y, q.z, q.w));


		aiVector3D t;
		CalcInterpolatedPosition(t, AnimationTime, pNodeAnim);
		DirectX::XMMATRIX TranslationM = DirectX::XMMatrixTranslation(t.x, t.y, t.z);



		NodeTransformation = ScalingM * RotationM * TranslationM;
		NodeTransformation = XMMatrixTranspose(NodeTransformation);
		//I applied transpos
	}


	DirectX::XMMATRIX GlobalTransformation = ParentTransform * NodeTransformation;

	if (mBoneMapping.find(NodeName) != mBoneMapping.end()) {
		UINT BoneIndex = mBoneMapping[NodeName];
		mBoneInfo[BoneIndex].FinalTransformation = GlobalInverseTransform * GlobalTransformation * mBoneInfo[BoneIndex].BoneOffset;
		//mBoneInfo[BoneIndex].FinalTransformation = XMMatrixTranspose(mBoneInfo[BoneIndex].FinalTransformation);
	}


	for (UINT i = 0; i < pNode->mNumChildren; ++i) {

		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
	}
}

const aiNodeAnim* GameEntities::FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName)
{
	for (UINT i = 0; i < pAnimation->mNumChannels; i++) {
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}

	return NULL;
}

void GameEntities::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	UINT PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	UINT NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void GameEntities::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	UINT ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	UINT NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

void GameEntities::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	UINT RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	UINT NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

UINT GameEntities::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	for (UINT i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


UINT GameEntities::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

UINT GameEntities::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (UINT i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}