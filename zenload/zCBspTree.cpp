#include "zCBspTree.h"

#include "zCMesh.h"
#include "zenParser.h"
#include "utils/logger.h"

using namespace ZenLoad;

zCBspTreeData zCBspTree::readObjectData(ZenLoad::ZenParser& parser, ZenLoad::zCMesh* mesh) {
  zCBspTreeData info;

  // Information about the whole file we are reading here
  BinaryFileInfo fileInfo{};

  // Read information about the current file. Mainly size is important here.
  parser.readStructure(fileInfo);

  // Calculate ending location and thus, the filesize
  const size_t binFileEnd = parser.getSeek() + size_t(fileInfo.size);

  // Read the BSP-Tree first
  size_t meshPosition = parser.getSeek();
  zCMesh::skip(parser);
  //mesh->readObjectData(parser);

  // Information about a single chunk
  BinaryChunkInfo chunkInfo{};

  bool done = false;
  while (!done && parser.getSeek() < binFileEnd)
  {
      // Read chunk header and calculate position of next chunk
      parser.readStructure(chunkInfo);

      size_t chunkEnd = parser.getSeek() + chunkInfo.length;

      switch (chunkInfo.id) {
        case EBspChunk::CHUNK_BSP:
          info.version = parser.readBinaryWord();
          info.mode = static_cast<zCBspTreeData::TreeMode>(parser.readBinaryDWord());
          break;

        case CHUNK_BSP_POLYLIST: {
          uint32_t numPolys = parser.readBinaryDWord();
          info.treePolyIndices.resize(numPolys);
          parser.readBinaryRaw(info.treePolyIndices.data(),numPolys*sizeof(uint32_t));
          }
          break;

        case CHUNK_BSP_TREE: {
          uint32_t numNodes = parser.readBinaryDWord();
          uint32_t numLeafs = parser.readBinaryDWord();

          if(!numNodes) {
            if(parser.getSeek()!=chunkEnd)
              LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes - bsp tree without nodes";
            parser.setSeek(chunkEnd);  // Skip chunk
            break;
            }

          info.nodes.reserve(numNodes);
          info.leafIndices.reserve(numLeafs);
          info.nodes.emplace_back();

          loadRec(parser,fileInfo,info,0,true);
          assert(numNodes==info.nodes.size());
          assert(numLeafs==info.leafIndices.size());
          }
          break;

        case CHUNK_BSP_LEAF_LIGHT:
          for(auto& l:info.leafIndices)
            parser.readStructure(info.nodes[l].light);
          if(parser.getSeek()!=chunkEnd)
            LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes - leaf_light";
          parser.setSeek(chunkEnd);  // Skip chunk
          break;

        case CHUNK_BSP_OUTDOOR_SECTORS: {
          uint32_t numSectors = parser.readBinaryDWord();

          // Sectors
          for(uint32_t i = 0; i < numSectors; i++) {
            info.sectors.emplace_back();
            zCSector& sector = info.sectors.back();

            sector.name = parser.readLine(false);
            if(fileInfo.version==(uint32_t)WorldVersion::VERSION_G1_08k) // FIXME: not only g1 probably ... its about leading zeros, underlines in sector name that are mixed up between saved variant and queries made upon the data
              modifyToTwoDigitNumbers(sector.name);
            uint32_t numSectorNodes = parser.readBinaryDWord();
            uint32_t numSectorPortals = parser.readBinaryDWord();

            // Read Nodes this sector belongs to
            sector.bspNodeIndices.resize(numSectorNodes);
            parser.readBinaryRaw(sector.bspNodeIndices.data(),numSectorNodes*sizeof(uint32_t));

            // Read portals in/out of this sector
            sector.portalPolygonIndices.resize(numSectorPortals);
            parser.readBinaryRaw(sector.portalPolygonIndices.data(),numSectorPortals*sizeof(uint32_t));
            }

          // Portal-list
          uint32_t numPortals = parser.readBinaryDWord();
          info.portalPolyIndices.resize(numPortals);
          parser.readBinaryRaw(info.portalPolyIndices.data(),numPortals*sizeof(uint32_t));
          }
          break;

        case CHUNK_BSP_END:
          done = true;
          /*uint8_t unknown_end =*/ parser.readBinaryByte(); // checksum or whatever 0x42 == 66
          break;
        }
      if(parser.getSeek()!=chunkEnd)
        LogInfo() << "Skipping " << chunkEnd-parser.getSeek() << " bytes";
      parser.setSeek(chunkEnd);  // Skip chunk
  }

  // Now get the list of non-lod polygons to load the worldmesh without them
  std::vector<size_t> lodPolys;
  std::vector<size_t> nonLodPolys = getNonLodPolygons(info,lodPolys);

  std::sort(nonLodPolys.begin(), nonLodPolys.end());
  nonLodPolys.erase(std::unique(nonLodPolys.begin(), nonLodPolys.end()), nonLodPolys.end());

  // Reset to mesh position
  /*size_t seek = */parser.getSeek();
  parser.setSeek(meshPosition);

  bool forceG132bitIndices = false;
  ZenParser::ZenHeader zenHeader = parser.getZenHeader();
  if (!zenHeader.user.empty())
  {
    forceG132bitIndices = zenHeader.user.find("XZEN") != std::string::npos;
  }

  mesh->readObjectData(parser, nonLodPolys, forceG132bitIndices);

  // Make access to portals and sectors easier by packing them in better structures
  connectPortals(info, mesh);
  parser.setSeek(binFileEnd);

  return info;
  }

void zCBspTree::loadRec(ZenParser& parser, const BinaryFileInfo& fileInfo, zCBspTreeData& info, size_t idx, bool isNode) {
  zCBspNode& n = info.nodes[idx];

  parser.readStructure(n.bbox3dMin);
  parser.readStructure(n.bbox3dMax);

  // Read indices to the polys this contains
  n.treePolyIndex = static_cast<size_t>(parser.readBinaryDWord());
  n.numPolys      = static_cast<size_t>(parser.readBinaryDWord());

  // Only need to load data if this isn't a leaf
  if(isNode) {
    /**
       * Flags:
       * 1: front
       * 2: back
       * 4: front is leaf
       * 8: back is leaf
       */
    enum {
      FLAG_FRONT         = 1,
      FLAG_BACK          = 2,
      FLAG_FRONT_IS_LEAF = 4,
      FLAG_BACK_IS_LEAF  = 8,
      };

    // flags tell if this node got children and whether they are leafs
    uint8_t flags = parser.readBinaryByte();
    parser.readStructure(n.plane.w);
    parser.readStructure(n.plane.x);
    parser.readStructure(n.plane.y);
    parser.readStructure(n.plane.z);

    // G1 has an extra byte here
    if(fileInfo.version==Gothic_18k)
      n.lodFlag = parser.readBinaryByte()!=0; // Lod-flag

    uint32_t front = zCBspNode::INVALID_NODE;
    uint32_t back  = zCBspNode::INVALID_NODE;


    if((flags & FLAG_FRONT)!=0) {
      front=uint32_t(info.nodes.size());
      info.nodes.emplace_back();
      }
    // Read front node
    if(front != zCBspNode::INVALID_NODE) {
      // Assign index and add actual node
      info.nodes[idx].front = front;
      // Set new nodes parent
      info.nodes[front].parent = uint32_t(idx);

      // If this is a leaf, add it to the leaf-list
      if((flags & FLAG_FRONT_IS_LEAF)!=0)
        info.leafIndices.push_back(front);

      // Continue to load the tree
      loadRec(parser,fileInfo,info,front,(flags & FLAG_FRONT_IS_LEAF) == 0);
      }

    if((flags & FLAG_BACK)!=0) {
      back=uint32_t(info.nodes.size());
      info.nodes.emplace_back();
      }
    // Read back node
    if(back != zCBspNode::INVALID_NODE) {
      // Assign index and add actual node
      info.nodes[idx].back = back;
      // Set new nodes parent
      info.nodes[back].parent = uint32_t(idx);

      // If this is a leaf, add it to the leaf-list
      if((flags & FLAG_BACK_IS_LEAF) != 0)
        info.leafIndices.push_back(back);

      // Continue to load the tree
      loadRec(parser,fileInfo,info,back,(flags & FLAG_BACK_IS_LEAF) == 0);
      }
    }
  // else
  //   {
  //   LogInfo() << idx << " Leaf!";
  //   }
  }

void zCBspTree::connectPortals(zCBspTreeData& info, zCMesh* worldMesh) {
  for(const zCMaterialData& m : worldMesh->getMaterials()) {
    if(isMaterialForPortal(m))
      {
      info.portals.emplace_back();
      zCPortal& portal = info.portals.back();

      portal.frontSectorName = extractSourceSectorFromMaterialName(m.matName);
      portal.backSectorName  = extractDestSectorFromMaterial      (m.matName);

      portal.frontSectorIndex = findSectorIndexByName(info, portal.frontSectorName);
      portal.backSectorIndex  = findSectorIndexByName(info, portal.backSectorName);

      //LogInfo() << "Portal material: " << m.matName;
      //LogInfo() << " - Source: " << extractSourceSectorFromMaterialName(m.matName);
      //LogInfo() << " - Dest: " << extractDestSectorFromMaterial(m.matName);
      }
    else if(isMaterialForSector(m)) {
      std::string to = extractDestSectorFromMaterial(m.matName);

      info.portals.emplace_back();
      zCPortal& portal = info.portals.back();

      // Inner sector portals get the same sector as front and back.
      // They're named like "S:_dest"
      portal.backSectorIndex =  findSectorIndexByName(info, to);
      portal.frontSectorIndex = portal.backSectorIndex;

      portal.frontSectorName = to;
      portal.backSectorName = to;

      //LogInfo() << "Sector material: " << m.matName;
      //LogInfo() << " - Source: " << extractSourceSectorFromMaterialName(m.matName);
      //LogInfo() << " - Dest: " << extractDestSectorFromMaterial(m.matName);
      }
    }
  }

void zCBspTree::modifyToTwoDigitNumbers(std::string& name) {
  // FIXME: KI, HH, and H�TTE / HÜTTE in G1 are queried for one digit format - skip those - also this reveals some text encoding inconsistencies - Utf8, Ascii, Utf16 ... I don't know .. this will lead to some bigger refactoring to make this clean
  if(name.size()<=3)
    return;
  if(name.find("H\xfcTTE")!=std::string::npos /*|| name.compare("HHMH1")==0 - G2 room not found on load ... but this is probably wrong to hack it here*/)
    return;
  auto it = name.begin();
  while(it!=name.end() && !std::isdigit(*it))
    ++it;
  if(std::distance(it,name.end())==1)
    name=name.substr(0,std::distance(name.begin(),it))+"0"+*it;
  }
