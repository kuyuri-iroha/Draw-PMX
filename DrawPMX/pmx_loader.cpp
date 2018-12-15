#include "pmx_loader.h"
#include <fstream>
#include <array>


bool getPMXStringUTF16(std::ifstream& _file, std::wstring& output)
{
	std::array<wchar_t, 512> wBuffer{};
	int textSize;

	_file.read(reinterpret_cast<char*>(&textSize), 4);

	_file.read(reinterpret_cast<char*>(&wBuffer), textSize);
	output = std::wstring(&wBuffer[0], &wBuffer[0] + textSize / 2);

	return true;
}


bool loadPMX(PMXModelData& data, const std::wstring& _filePath)
{
	if(_filePath.empty()) {return false;}

	// モデルファイルのパスからモデルフォルダのパスを抽出
	std::wstring folderPath{_filePath.begin(), _filePath.begin() + _filePath.rfind(L'\\') + 1};

	// ファイルオープン
	std::ifstream pmxFile{_filePath, (std::ios::binary | std::ios::in)};
	if (pmxFile.fail())
	{
		pmxFile.close();
		return false;
	}

	// ヘッダー -------------------------------
	std::array<byte, 4> pmxHeader{};
	constexpr std::array<byte, 4> PMX_MAGIC_NUMBER{0x50, 0x4d, 0x58, 0x20};
	enum HeaderDataIndex
	{
		ENCODING_FORMAT,
		NUMBER_OF_ADD_UV,
		VERTEX_INDEX_SIZE,
		TEXTURE_INDEX_SIZE,
		MATERIAL_INDEX_SIZE,
		BONE_INDEX_SIZE,
		RIGID_BODY_INDEX_SIZE
	};

	for (int i = 0; i < 4; ++i)
	{
		pmxHeader[i] = pmxFile.get();
	}
	if (pmxHeader != PMX_MAGIC_NUMBER)
	{
		pmxFile.close();
		return false;
	}

	// ver2.0以外は非対応
	float version{};
	pmxFile.read(reinterpret_cast<char*>(&version), 4);
	if (!XMScalarNearEqual(version, 2.0f, g_XMEpsilon.f[0]))
	{
		pmxFile.close();
		return false;
	}

	byte hederDataLength = pmxFile.get();
	if (hederDataLength != 8)
	{
		pmxFile.close();
		return false;
	}
	std::array<byte, 8> hederData{};
	for (int i = 0; i < hederDataLength; ++i)
	{
		hederData[i] = pmxFile.get();
	}
	//UTF-8は非対応
	if (hederData[0] != 0)
	{
		pmxFile.close();
		return false;
	}

	unsigned arrayLength{};
	for (int i = 0; i < 4; ++i)
	{
		pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
		for (unsigned j = 0; j<arrayLength; j++)
		{
			pmxFile.get();
		}
	}

	// 頂点 -----------------------------------
	using Vertex = PMXModelData::Vertex;
	int numberOfVertex{};
	pmxFile.read(reinterpret_cast<char*>(&numberOfVertex), 4);
	data.vertices.resize(numberOfVertex);

	for (int i = 0; i < numberOfVertex; ++i)
	{
		pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].position), 12);
		pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].normal), 12);
		pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].uv), 8);
		if (hederData[NUMBER_OF_ADD_UV] != 0)
		{
			for (int j = 0; j<hederData[NUMBER_OF_ADD_UV]; ++j)
			{
				pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].additionalUV[j]), 16);
			}
		}

		const byte weightMethod = pmxFile.get();
		switch (weightMethod)
		{
		case Vertex::Weight::BDEF1:
			data.vertices[i].weight.type = Vertex::Weight::BDEF1;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			data.vertices[i].weight.born2 = PMXModelData::NO_DATA_FLAG;
			data.vertices[i].weight.born3 = PMXModelData::NO_DATA_FLAG;
			data.vertices[i].weight.born4 = PMXModelData::NO_DATA_FLAG;
			data.vertices[i].weight.weight1 = 1.0f;
			break;

		case Vertex::Weight::BDEF2:
			data.vertices[i].weight.type = Vertex::Weight::BDEF2;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			data.vertices[i].weight.born3 = PMXModelData::NO_DATA_FLAG;
			data.vertices[i].weight.born4 = PMXModelData::NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.weight1), 4);
			data.vertices[i].weight.weight2 = 1.0f - data.vertices[i].weight.weight1;
			break;

		case Vertex::Weight::BDEF4:
			data.vertices[i].weight.type = Vertex::Weight::BDEF4;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born3), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born4), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.weight1), 4);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.weight2), 4);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.weight3), 4);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.weight4), 4);
			break;

		case Vertex::Weight::SDEF:
			data.vertices[i].weight.type = Vertex::Weight::SDEF;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born1), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.born2), hederData[BONE_INDEX_SIZE]);
			data.vertices[i].weight.born3 = PMXModelData::NO_DATA_FLAG;
			data.vertices[i].weight.born4 = PMXModelData::NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.weight1), 4);
			data.vertices[i].weight.weight2 = 1.0f - data.vertices[i].weight.weight1;
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.c), 12);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.r0), 12);
			pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].weight.r1), 12);
			break;

		default:
			pmxFile.close();
			return false;
		}

		pmxFile.read(reinterpret_cast<char*>(&data.vertices[i].edgeMagnif), 4);

		if (data.vertices[i].weight.born1 == PMXModelData::NO_DATA_FLAG)
		{
			pmxFile.close();
			return false;
		}
	}

	// 面  ------------------------------------
	int numOfSurface{};
	pmxFile.read(reinterpret_cast<char*>(&numOfSurface), 4);
	data.surfaces.resize(numOfSurface);

	for (int i = 0; i < numOfSurface; ++i)
	{
		pmxFile.read(reinterpret_cast<char*>(&data.surfaces[i].vertexIndex), hederData[VERTEX_INDEX_SIZE]);

		if (data.surfaces[i].vertexIndex == PMXModelData::NO_DATA_FLAG || data.surfaces[i].vertexIndex == PMXModelData::NO_DATA_FLAG || data.surfaces[i].vertexIndex == PMXModelData::NO_DATA_FLAG)
		{
			pmxFile.close();
			return false;
		}
	}

	// テクスチャ -----------------------------
	int numOfTexture{};
	pmxFile.read(reinterpret_cast<char*>(&numOfTexture), 4);
	data.texturePaths.resize(numOfTexture);

	std::wstring texturePath{};
	for (int i = 0; i < numOfTexture; ++i)
	{
		data.texturePaths[i] = folderPath;
		getPMXStringUTF16(pmxFile, texturePath);
		data.texturePaths[i] += texturePath;
	}

	// マテリアル -----------------------------
	int numOfMaterial{};
	pmxFile.read(reinterpret_cast<char*>(&numOfMaterial), 4);

	data.materials.resize(numOfMaterial);
	for (int i = 0; i < numOfMaterial; ++i)
	{
		for (int j = 0; j<2; ++j)
		{
			pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
			for (unsigned i = 0; i < arrayLength; ++i)
			{
				pmxFile.get();
			}
		}

		pmxFile.read(reinterpret_cast<char*>(&data.materials[i].diffuse), 16);
		pmxFile.read(reinterpret_cast<char*>(&data.materials[i].specular), 12);
		pmxFile.read(reinterpret_cast<char*>(&data.materials[i].specularity), 4);
		pmxFile.read(reinterpret_cast<char*>(&data.materials[i].ambient), 12);

		pmxFile.get();
		for (int i = 0; i < 16; ++i)
		{
			pmxFile.get();
		}
		for (int i = 0; i < 4; ++i)
		{
			pmxFile.get();
		}

		pmxFile.read(reinterpret_cast<char*>(&data.materials[i].colorMapTextureIndex), hederData[TEXTURE_INDEX_SIZE]);
		for (unsigned char i = 0; i < hederData[TEXTURE_INDEX_SIZE]; ++i)
		{
			pmxFile.get();
		}
		pmxFile.get();

		const byte shareToonFlag = pmxFile.get();
		if (shareToonFlag)
		{
			pmxFile.get();
		}
		else
		{
			pmxFile.read(reinterpret_cast<char*>(&data.materials[i].toonTextureIndex), hederData[TEXTURE_INDEX_SIZE]);
		}

		pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
		for (unsigned i = 0; i < arrayLength; ++i)
		{
			pmxFile.get();
		}

		pmxFile.read(reinterpret_cast<char*>(&data.materials[i].vertexNum), 4);
	}

	// ボーン ---------------------------------
	int numOfBone{};
	pmxFile.read(reinterpret_cast<char*>(&numOfBone), 4);

	data.bones.resize(numOfBone);
	int ikLinkSize = 0;
	unsigned char angleLim = 0;

	for (int i = 0; i < numOfBone; ++i)
	{
		getPMXStringUTF16(pmxFile, data.bones[i].name);
		pmxFile.read(reinterpret_cast<char*>(&arrayLength), 4);
		data.bones[i].nameEnglish.resize(arrayLength);
		for (unsigned j = 0; j<arrayLength; ++j)
		{
			data.bones[i].nameEnglish[j] = pmxFile.get();
		}

		pmxFile.read(reinterpret_cast<char*>(&data.bones[i].position), 12);

		pmxFile.read(reinterpret_cast<char*>(&data.bones[i].parentIndex), hederData[BONE_INDEX_SIZE]);
		if (numOfBone <= data.bones[i].parentIndex)
		{
			data.bones[i].parentIndex = PMXModelData::NO_DATA_FLAG;
		}

		pmxFile.read(reinterpret_cast<char*>(&data.bones[i].transformationLevel), 4);

		pmxFile.read(reinterpret_cast<char*>(&data.bones[i].flag), 2);

		enum BoneFlagMask
		{
			ACCESS_POINT = 0x0001,
			IK = 0x0020,
			IMPART_TRANSLATION = 0x0100,
			IMPART_ROTATION = 0x0200,
			AXIS_FIXING = 0x0400,
			LOCAL_AXIS = 0x0800,
			EXTERNAL_PARENT_TRANS = 0x2000,
		};

		if (data.bones[i].flag & ACCESS_POINT)
		{
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].childrenIndex), hederData[BONE_INDEX_SIZE]);
			if (numOfBone <= data.bones[i].childrenIndex)
			{
				data.bones[i].childrenIndex = PMXModelData::NO_DATA_FLAG;
			}
		}
		else
		{
			data.bones[i].childrenIndex = PMXModelData::NO_DATA_FLAG;
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].coordOffset), 12);
		}
		if ((data.bones[i].flag & IMPART_TRANSLATION) || (data.bones[i].flag & IMPART_ROTATION))
		{
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].impartParentIndex), hederData[BONE_INDEX_SIZE]);
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].impartRate), 4);
		}
		if (data.bones[i].flag & AXIS_FIXING)
		{
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].fixedAxis), 12);
		}
		if (data.bones[i].flag & LOCAL_AXIS)
		{
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].localAxisX), 12);
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].localAxisZ), 12);
		}
		if (data.bones[i].flag & EXTERNAL_PARENT_TRANS)
		{
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].externalParentKey), 4);
		}
		if (data.bones[i].flag & IK)
		{
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].ikTargetIndex), hederData[5]);
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].ikLoopCount), 4);
			pmxFile.read(reinterpret_cast<char*>(&data.bones[i].ikUnitAngle), 4);
			pmxFile.read(reinterpret_cast<char*>(&ikLinkSize), 4);
			data.bones[i].ikLinks.resize(ikLinkSize);
			for (int j = 0; j<ikLinkSize; ++j)
			{
				pmxFile.read(reinterpret_cast<char*>(&data.bones[i].ikLinks[j].index), hederData[5]);
				angleLim = pmxFile.get();
				data.bones[i].ikLinks[j].existAngleLimited = false;
				if (angleLim == 1)
				{
					pmxFile.read(reinterpret_cast<char*>(&data.bones[i].ikLinks[j].limitAngleMin), 12);
					pmxFile.read(reinterpret_cast<char*>(&data.bones[i].ikLinks[j].limitAngleMax), 12);
					data.bones[i].ikLinks[j].existAngleLimited = true;
				}
			}
		}
		else
		{
			data.bones[i].ikTargetIndex = PMXModelData::NO_DATA_FLAG;
		}
	}

	pmxFile.close();

	return true;
}