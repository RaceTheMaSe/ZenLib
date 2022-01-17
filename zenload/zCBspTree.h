#pragma once

#include <functional>
#include <algorithm>
#include <cassert>

#include "zTypes.h"

namespace ZenLoad
{
  class ZenParser;
  class zCMesh;

  class zCBspTree
  {
  private:
    static void connectPortals(zCBspTreeData& info);

  public:
    enum EVersion
    {
      Gothic_26f = 0,  // TODO
      Gothic_18k = 34144256
    };

    enum EBspChunk
    {
      CHUNK_BSP                 = 0xC000,
      CHUNK_BSP_POLYLIST        = 0xC010,
      CHUNK_BSP_TREE            = 0xC040,
      CHUNK_BSP_LEAF_LIGHT      = 0xC045,
      CHUNK_BSP_OUTDOOR_SECTORS = 0xC050,
      CHUNK_BSP_END             = 0xC0FF,
    };

    /**
    * Reads this object from an internal zen
    */
    static zCBspTreeData readObjectData(ZenParser& parser, zCMesh* mesh);

    /**
      * Returns a list of polygon-indices which are not LOD-polyons
      * @param d Loaded BSP-Tree data
      * @return List of indices to polygons, which are not LOD-Polygons. These are the
      *         actual indices of the polygons inside the mesh.
      */
    static std::vector<size_t> getNonLodPolygons(const zCBspTreeData& d,std::vector<size_t>& lodReturn);

  private:
    static void loadRec(ZenParser& parser,const BinaryFileInfo& fileInfo,zCBspTreeData& info,size_t idx,bool isNode);
    static SectorIndex findSectorIndexByName(zCBspTreeData& info, const std::string& sectorname);

    /**
      * Given a material name of "X:abcd_efgh", returns "abcd".
      */
    static std::string extractSourceSectorFromMaterialName(const std::string& name);

    /**
      * Given a material name of "X:abcd_efgh", returns "efgh".
      */
    static std::string extractDestSectorFromMaterial(const std::string& name);

    static bool isMaterialForPortal(const zCMaterialData& m);
    static bool isMaterialForSector(const zCMaterialData& m);

    /**
      * Extracts the information given by the various indices inside the BspTree-Structure and packs them
      * into accessible objects.
      * @param info Loaded BSP-Tree data
      * @param worldMesh Loaded world mesh. Needed to access material names, which encode portal information
      */
    static void connectPortals(zCBspTreeData& info, zCMesh* worldMesh);

    /**
      * Sector names are stored without leading zeros, but queried with two digit format
      */
    static void modifyToTwoDigitNumbers(std::string& name);
  };
}  // namespace ZenLoad
