#include "zCMesh.h"

#include <cstdint>
#include <map>
#include <string>

#include "zCMaterial.h"
#include "zTypes.h"
#include "zenParser.h"
#include "utils/logger.h"
#include "vdfs/fileIndex.h"
#include "parserImpl.h"
#include "utils/alignment.h"
#include "zenload/ztex2dds.h"

using namespace ZenLoad;

// Types of chunks we will find in a zCMesh-Section
static const unsigned short MSID_MESH = 0xB000;
static const unsigned short MSID_BBOX3D = 0xB010;
static const unsigned short MSID_MATLIST = 0xB020;
static const unsigned short MSID_LIGHTMAPLIST = 0xB025;
static const unsigned short MSID_LIGHTMAPLIST_SHARED = 0xB026;
static const unsigned short MSID_VERTLIST = 0xB030;
static const unsigned short MSID_FEATLIST = 0xB040;
static const unsigned short MSID_POLYLIST = 0xB050;
static const unsigned short MSID_MESH_END = 0xB060;

enum EVersion
{
  G1_1_08k = 9,
  G2_2_6fix = 265
};

/**
* @brief Loads the mesh from the given VDF-Archive
*/
zCMesh::zCMesh(const std::string& fileName, VDFS::FileIndex& fileIndex) {
  std::vector<uint8_t> data;
  fileIndex.getFileData(fileName, data);

  if (data.empty()) {
    LogInfo() << "Failed to find mesh " << fileName;
    return;  // TODO: Throw an exception or something
    }

  try {
    // Create parser from memory
    ZenLoad::ZenParser parser(data.data(), data.size());

    // .MSH-Files are just saved zCMeshes
    readObjectData(parser);
    }
  catch (std::exception& e) {
    LogError() << e.what();
    return;
    }
  }

/**
* @brief Reads the oriented bounding boxes
*/
  void zCMesh::readObb(ZenParser& parser, zCOBBox3D& box) {
    parser.readStructure(box.center);
    parser.readStructure(box.axis  );
    parser.readStructure(box.extend);
    uint16_t numChildren = parser.readBinaryWord();
    for(uint16_t c=0;c<numChildren;++c) {
      box.childs.push_back(zCOBBox3D());
      readObb(parser,box.childs.back());
    }
  };

/**
* @brief Reads the mesh-object from the given binary stream
*/
void zCMesh::readObjectData(ZenParser& parser, const std::vector<size_t>& skipPolys, bool forceG132bitIndices) {
  uint16_t version = 0;

  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  // Read chunks until we left the virtual binary file or got to the end-chunk
  // Each chunk starts with a header (BinaryChunkInfo) which gives information
  // about what to do and how long the chunk is
  bool doneReadingChunks = false;
  while (!doneReadingChunks) {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;

    switch (chunkInfo.id) {
      case MSID_MESH: {
        // uint32 - version
        // zDate - Structure
        // \n terminated string for the name
        version = parser.readBinaryWord();
        zDate date{};
        parser.readStructure(date);
        std::string name = parser.readLine(false);
        (void)date;

        LogInfo() << "Reading mesh" << (name.empty() ? "" : " '"+name+"'") << " (Version: " << version << ")";
        
        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";
        
        parser.setSeek(chunkEnd); // Skip chunk
        }
      break;

      case MSID_BBOX3D: {
        parser.readStructure(m_BBMin);
        parser.readStructure(m_BBMax);
        readObb(parser,m_obb);
        
        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";
        
        parser.setSeek(chunkEnd); // Skip chunk
        }
      break;

      case MSID_MATLIST: {
        // ZenArchive - Header
        // uint32_t - Num materials
        // For each material:
        //  - String - Name
        //  - zCMaterial-Chunk

        //ZenParser p2(&parser.getData()[parser.getSeek()], parser.getData().size() - parser.getSeek());
        //p2.readHeader();

        ZenLoad::ParserImpl* oldImpl = parser.getImpl();
        ZenLoad::ZenParser::ZenHeader oldHeader = parser.getZenHeader();

        // These are binary, init the 'new' header
        parser.readHeader();

        // Read number of materials
        uint32_t numMaterials = parser.readBinaryDWord();
        m_Materials.reserve(numMaterials);

        // Read every stored material
        for (uint32_t i = 0; i < numMaterials; i++) {
          std::string aName = parser.readLine(false);  // Read unused material name (Stored a second time later)

          // Skip chunk headers - we know these are zCMaterial
          uint32_t chunksize = parser.readBinaryDWord(); (void)chunksize;
          uint16_t mversion = parser.readBinaryWord();
          uint32_t objectIndex = parser.readBinaryDWord(); (void)objectIndex;

          parser.skipSpaces();

          // Skip chunk-header
          std::string name = parser.readLine();
          std::string classname = parser.readLine();

          // Save into vector
          m_Materials.emplace_back(zCMaterial::readObjectData(parser, mversion));
          if(aName!=m_Materials.back().matName)
            LogInfo() << "Material name mismatch";
          }

        if(version!=9 || parser.getZenHeader().version!=1)
          m_IsUsingAlphaTest = parser.readBinaryByte()!=0;
        else
          m_IsUsingAlphaTest = true; // G1 doesn't have this information, so unfortunately do it for all materials
        if(m_IsUsingAlphaTest) {
          for(auto& m : m_Materials)
            m.alphaFunc = 1 /*AlphaTest*/;
          }
        // Restore old header and impl
        delete parser.getImpl();
        parser.setImpl(oldImpl);
        parser.setZenHeader(oldHeader);
        
        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";

        parser.setSeek(chunkEnd);  // Skip chunk
        }
      break;

      case MSID_LIGHTMAPLIST:
        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes - lightmap list not implemented";
        parser.setSeek(chunkEnd);  // Skip chunk
        break;

      case MSID_LIGHTMAPLIST_SHARED: {
        uint32_t numTextures=parser.readBinaryDWord();
        for(uint32_t i=0;i<numTextures;++i) {
          std::vector<uint8_t> ddsData;
          if(convertZTEX2DDS(parser,ddsData)!=0) {
            LogInfo() << "Error reading lightmaplist shared. Skipping " << chunkEnd-parser.getSeek() << " bytes";
            break;
          }
          m_lightMapTextures.push_back(std::move(ddsData));
        }
        uint32_t numEntries = parser.readBinaryDWord();
        m_lightMaps.resize(numEntries);
        for(uint32_t i=0;i<numEntries;++i)
          parser.readStructure(m_lightMaps[i]);
        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes - lightmap list read but unused";
        parser.setSeek(chunkEnd);  // Skip chunk
        }
        break;

      case MSID_VERTLIST: {
        // Read how many vertices we have in this chunk
        uint32_t numVertices = parser.readBinaryDWord();

        // Read vertex data and emplace into m_Vertices
        m_Vertices.resize(numVertices);
        parser.readBinaryRaw(m_Vertices.data(), numVertices * sizeof(float) * 3);

        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";
        }
      break;

      case MSID_FEATLIST: {
        // Read how many feats we have
        uint32_t numFeats = parser.readBinaryDWord();

        // Read features
        m_Features.resize(numFeats);
        parser.readBinaryRaw(m_Features.data(), numFeats * sizeof(zTMSH_FeatureChunk));
        
        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";
        
        parser.setSeek(chunkEnd); // just in case there is a mismatch
        }
      break;

      case MSID_POLYLIST: {
        // Current entry of the skip list. This is increased as we go.
        size_t skipListEntry = 0;

        // Read number of polys
        auto const numPolys = parser.readBinaryDWord();

        // Fake a read here, to get around an additional copy of the data
        const uint8_t* blockPtr = parser.getDataPtr();
        // set pointer to end of chunk, as blockPtr is used for reading this chunk
        parser.setSeek(parser.getSeek() + chunkInfo.length-sizeof(uint32_t));

        size_t blockSize = version == EVersion::G2_2_6fix
                                ? sizeof(polyData1Packed<PolyFlags2_6fix>)
                                : sizeof(polyData1Packed<PolyFlags1_08k>);

        size_t indicesSize = version == EVersion::G2_2_6fix
                                  ? sizeof(polyData2<uint32_t, PolyFlags2_6fix>::IndexPacked)
                                  : sizeof(polyData2<uint16_t, PolyFlags1_08k>::IndexPacked);

        if (forceG132bitIndices)
          indicesSize = sizeof(polyData2<uint32_t, PolyFlags1_08k>::IndexPacked);

        // Iterate throuh every poly
        m_Triangles.reserve(numPolys);
        m_TriangleMaterialIndices.reserve(numPolys);
        m_TriangleLightmapIndices.reserve(numPolys);

        m_Indices.reserve(numPolys*3);
        m_FeatureIndices.reserve(numPolys*3);

        for(size_t i=0; i < numPolys; i++) {
          // Convert to a generic version
          polyData2<uint32_t, PolyFlags> p;
          if (version == EVersion::G2_2_6fix) {
            polyData2<uint32_t, PolyFlags2_6fix> d;
            d.read(blockPtr);
            p.from(d);
            }
          else {
            if (forceG132bitIndices) {
              polyData2<uint32_t, PolyFlags1_08k> d;
              d.read(blockPtr);
              p.from(d);
              }
            else {
              polyData2<uint16_t, PolyFlags1_08k> d;
              d.read(blockPtr);
              p.from(d);
              }
            }

          if (skipPolys.empty() || (skipPolys[skipListEntry] == i)) {
            // TODO: Store these somewhere else
            // TODO: lodFlag isn't set to something useful in Gothic 1. Also the portal-flags aren't set? Investigate!
            if (!p.flags.ghostOccluder && !p.flags.portalPoly &&
                !p.flags.portalIndoorOutdoor) {
              if (p.polyNumVertices != 0) {
                if (p.polyNumVertices == 3) {
                  // Write indices directly to a vector
                  WorldVertex vx[3];
                  for (int v = 0; v < 3; v++) {
                    m_Indices.emplace_back(p.indices[v].VertexIndex);
                    m_FeatureIndices.emplace_back(p.indices[v].FeatIndex);

                    // Gather vertex information
                    vx[v].Position = m_Vertices[p.indices[v].VertexIndex];
                    vx[v].Color = m_Features[p.indices[v].FeatIndex].lightStat;

                    vx[v].TexCoord = ZMath::float2(m_Features[p.indices[v].FeatIndex].uv[0],
                                                    m_Features[p.indices[v].FeatIndex].uv[1]);
                    vx[v].Normal = m_Features[p.indices[v].FeatIndex].vertNormal;
                    }

                  // Save material index for the written triangle
                  m_TriangleMaterialIndices.push_back(p.materialIndex);

                  // Save lightmap-index
                  m_TriangleLightmapIndices.push_back(p.lightmapIndex);

                  WorldTriangle triangle;
                  triangle.flags = p.flags;
                  memcpy((ZenLoad::WorldVertex*)triangle.vertices, (ZenLoad::WorldVertex*)vx, sizeof(vx));

                  // Save triangle
                  m_Triangles.push_back(triangle);
                  }
                else {
                  // Triangulate a triangle-fan
                  //for(unsigned int i = p.polyNumVertices - 2; i >= 1; i--)
                  for (int idx = 1; idx < p.polyNumVertices - 1; idx++) {
                    m_Indices.emplace_back(p.indices[0].VertexIndex);
                    m_Indices.emplace_back(p.indices[idx].VertexIndex);
                    m_Indices.emplace_back(p.indices[idx + 1].VertexIndex);

                    m_FeatureIndices.emplace_back(p.indices[0].FeatIndex);
                    m_FeatureIndices.emplace_back(p.indices[idx].FeatIndex);
                    m_FeatureIndices.emplace_back(p.indices[idx + 1].FeatIndex);

                    // Save material index for the written triangle
                    m_TriangleMaterialIndices.push_back(p.materialIndex);

                    // Save lightmap-index
                    m_TriangleLightmapIndices.push_back(p.lightmapIndex);

                    WorldTriangle triangle;
                    triangle.flags = p.flags;
                    uint32_t indices[] = {p.indices[0].VertexIndex, p.indices[idx].VertexIndex,
                                      p.indices[idx + 1].VertexIndex};

                    // Gather vertex information
                    for (int v = 0; v < 3; v++) {
                      triangle.vertices[v].Position = m_Vertices[indices[v]];
                      triangle.vertices[v].Color = m_Features[indices[v]].lightStat;
                      triangle.vertices[v].TexCoord = ZMath::float2(m_Features[indices[v]].uv[0],
                                                                    m_Features[indices[v]].uv[1]);
                      triangle.vertices[v].Normal = m_Features[indices[v]].vertNormal;
                      }

                    // Start filling in the flags
                    m_Triangles.push_back(triangle);
                    }
                  }
                }
              }
            skipListEntry++;
            }

          // Goto next polygon using this weird shit
          blockPtr += blockSize + indicesSize * p.polyNumVertices;
          }

        if(parser.getSeek()!=chunkEnd)
          LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";
        
        parser.setSeek(chunkEnd); // Skip chunk, there could be more data here which is never read
        }
      break;

      case MSID_MESH_END:
        doneReadingChunks = true;
        break;

      default:
          if(parser.getSeek()!=chunkEnd)
            LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes - unrecognized chunk " << chunkInfo.id << " - length " << chunkInfo.length;

          parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  }

void zCMesh::skip(ZenParser& parser) {
  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  // Read chunks until we left the virtual binary file or got to the end-chunk
  // Each chunk starts with a header (BinaryChunkInfo) which gives information
  // about what to do and how long the chunk is
  while (true) {
    // Read chunk header and calculate position of next chunk
    parser.readStructure(chunkInfo);

    size_t chunkEnd = parser.getSeek() + chunkInfo.length;

    // Just skip all of it
    switch (chunkInfo.id) {
      case MSID_MESH_END:
        return;

      default:
        parser.setSeek(chunkEnd);  // Skip chunk
      }
    }
  }

void zCMesh::packMesh(PackedMesh& mesh, float scale, bool removeDoubles) {
	std::vector<WorldVertex>& newVertices = mesh.vertices;
	std::vector<uint32_t> newIndices;
	newIndices.reserve(m_Indices.size());

	// Map of vertex-indices and their used feature-indices to a vertex in "newVertices"
	std::map<std::tuple<uint32_t, uint32_t, int16_t>, size_t> vfToNewVx;

	// Map of the new indices and the old indices to the index-vector
	//std::unordered_map<uint32_t, uint32_t> newToOldIdxIdx;

	if(removeDoubles) {
		// Get vertices
		for(size_t i = 0, end = m_Indices.size(); i < end; i++) {
			uint32_t featidx = m_FeatureIndices[i];
			uint32_t vertidx = m_Indices[i];
			int16_t lightmap = m_TriangleLightmapIndices[i / 3];

			// Check if we already got this pair of vertex/feature
			auto it = vfToNewVx.find(std::make_tuple(vertidx, featidx, lightmap));
			if(it == vfToNewVx.end()) {
				// Add new entry
				vfToNewVx[std::make_tuple(vertidx, featidx, lightmap)] = newVertices.size();
				WorldVertex vx;

				// Extract vertex information
				vx.Position = m_Vertices[vertidx] * scale;
				vx.Color = m_Features[featidx].lightStat;
				vx.TexCoord = ZMath::float2(m_Features[featidx].uv[0], m_Features[featidx].uv[1]);
				vx.Normal = m_Features[featidx].vertNormal;

				// Add index to this very vertex
				newIndices.push_back((uint32_t)newVertices.size());

				newVertices.push_back(vx);
			  }
			else {
				// Simply put an index to the existing new vertex
				newIndices.push_back((uint32_t)(*it).second);
			  }

			// Store what this new index was before
			//newToOldIdxIdx[newIndices.end()-1] = i;
      }
    }
	else {
		// Just add them as triangles
		newVertices.reserve(m_Indices.size());
		for(size_t i = 0, end = m_Indices.size(); i < end; i++) {
			uint32_t featidx = m_FeatureIndices[i];
			uint32_t vertidx = m_Indices[i];
			int16_t lightmap = m_TriangleLightmapIndices[i / 3];
      (void)lightmap;

			WorldVertex vx;

			// Extract vertex information
			vx.Position = m_Vertices[vertidx] * scale;
			vx.Color = m_Features[featidx].lightStat;
			vx.TexCoord = ZMath::float2(m_Features[featidx].uv[0], m_Features[featidx].uv[1]);
			vx.Normal = m_Features[featidx].vertNormal;

			newVertices.push_back(vx);
      newIndices.push_back(uint32_t(newVertices.size()-1));
      }
    }

	// Assign materials to packed mesh
	for(auto& m : m_Materials) {
		mesh.subMeshes.emplace_back();
		mesh.subMeshes.back().material = m;
	  }
  
  std::vector<std::vector<uint32_t>> indicesPerSubMesh;
  indicesPerSubMesh.resize(m_Materials.size());
	// Sort indices by material
	for(size_t i = 0, end = newIndices.size(); i < end; i += 3)	{
		// Get material info of this triangle
		uint32_t matIdx = m_TriangleMaterialIndices[i / 3];
    // Add this triangle to its submesh
    for(size_t j = 0; j < 3; j++)
      indicesPerSubMesh[matIdx].push_back(newIndices[i + j]);
	  }

  newIndices.clear();
  newIndices.reserve(m_Indices.size());
  size_t idxOffset=0;
  size_t subMeshIdx=0;
  for(const auto& smi:indicesPerSubMesh) {
    size_t idxSize=0;
    for(const auto& i:smi) {
      newIndices.emplace_back(i);
      idxSize++;
      }
    mesh.subMeshes[subMeshIdx].indexOffset=idxOffset;
    mesh.subMeshes[subMeshIdx].indexSize=idxSize;
    idxOffset+=idxSize;
    }
  mesh.indices=newIndices;

	// Store triangles with more information attached as well
	mesh.triangles.reserve(m_Triangles.size());
	for(size_t i = 0; i < m_Triangles.size(); i++) {
    // Add submesh index to this triangle
		m_Triangles[i].submeshIndex = m_TriangleMaterialIndices[i];
		mesh.triangles.push_back(m_Triangles[i]);

		for(int v = 0; v < 3; v++)
			mesh.triangles.back().vertices[v].Position = mesh.triangles.back().vertices[v].Position * scale;
	  }

	mesh.bbox[0] = m_BBMin * scale;
	mesh.bbox[1] = m_BBMax * scale;
  mesh.isUsingAlphaTest = m_IsUsingAlphaTest;
  }
