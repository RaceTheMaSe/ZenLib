#pragma once
// #include "graphics/mesh/submesh/packedmesh.h" // originally for zenlob packed mesh modified to opengothic packedmesh class
#include <zenload/zTypes.h>
#include <algorithm>

namespace Utils
{
    class Exporter
    {
      public:
      Exporter()
      {
        if(inst==nullptr)
          inst = new Exporter("LevelObjects");
      }

      // bool exportPackedMeshToObj(const PackedMesh& mesh, const std::string& file, const std::string& name = "Object")
      // {
      //   // FILE* fp = fopen((file + ".obj").c_str(), "rb");
      //   // if (fp) // FIXME: file already exists - assume its correcty exported and take it - otherwise would do this export often
      //   //   return false;
      //   FILE* f = fopen((file + ".obj").c_str(), "w");

      //   if (!f)
      //       return false;

      //   fputs("# File exported by ZenLib\n", f);
      //   fputs(("o " + name + "\n").c_str(), f);

      //   // Write positions
      //   for (auto& v : mesh.vertices)
      //   {
      //       fputs(("v " + std::to_string(v.Position.x) + " " + std::to_string(v.Position.y) + " " + std::to_string(-v.Position.z) + "\n").c_str(), f);
      //   }

      //   // Write texture coords
      //   for (auto& v : mesh.vertices)
      //   {
      //       fputs(("v " + std::to_string(v.TexCoord.x) + " " + std::to_string(v.TexCoord.y) + "\n").c_str(), f);
      //   }

      //   // Write normals
      //   for (auto& v : mesh.vertices)
      //   {
      //       fputs(("v " + std::to_string(v.Normal.x) + " " + std::to_string(v.Normal.y) + " " + std::to_string(v.Normal.z) + "\n").c_str(), f);
      //   }

      //   fputs("s off\n", f);

      //   // Write faces
      //   for (auto& s : mesh.subMeshes)
      //   {
      //       fputs(("g " + s.material.matName + "\n").c_str(), f);
      //       fputs(("usemtl " + s.material.matName + "\n").c_str(), f);

      //       for (size_t i = 0; i < s.indices.size(); i += 3)
      //       {
      //           uint32_t idx[] = {s.indices[i], s.indices[i + 1], s.indices[i + 2]};
      //           std::string f1 = std::to_string(idx[0] + 1) + "/" + std::to_string(idx[0] + 1) + "/" + std::to_string(idx[0] + 1);
      //           std::string f2 = std::to_string(idx[1] + 1) + "/" + std::to_string(idx[1] + 1) + "/" + std::to_string(idx[1] + 1);
      //           std::string f3 = std::to_string(idx[2] + 1) + "/" + std::to_string(idx[2] + 1) + "/" + std::to_string(idx[2] + 1);

      //           fputs(("f " + f1 + " " + f2 + " " + f3 + "\n").c_str(), f);
      //       }
      //   }

      //   fclose(f);

      //   // Write mtl
      //   FILE* mf = fopen((file + ".mtl").c_str(), "w");

      //   if (!mf)
      //       return false;

      //   for (auto& s : mesh.subMeshes)
      //   {
      //       fputs(("newmtl " + s.material.matName + "\n").c_str(), mf);

      //       fputs(("map_Kd " + s.material.texture + "\n").c_str(), mf);

      //       ZMath::float4 color;
      //       color.fromABGR8(s.material.color);

      //       fputs(("Kd " + std::to_string(color.x) + " " + std::to_string(color.y) + " " + std::to_string(color.z) + "\n").c_str(), mf);
      //       fputs(("Ka " + std::to_string(color.x) + " " + std::to_string(color.y) + " " + std::to_string(color.z) + "\n").c_str(), mf);

      //       fputs("\n", mf);
      //   }

      //   fclose(mf);

      //   return true;
      // }

      bool exportPackedMeshToLevelObject(const ZenLoad::PackedMesh& mesh, const std::string& file, const std::string& name = "Object")
      {
          if(std::find(fileNames.begin(),fileNames.end(),file)!=fileNames.end())
            return true; // not sure of true or false ... depends on program flow
          // FILE* fp = fopen((file + ".obj").c_str(), "rb");
          // if (fp) // FIXME: file already exists - assume its correcty exported and take it - otherwise would do this export often
          //   return false;
          FILE* f = fopen(("single_export/" + file + ".obj").c_str(), "a");

          if (!f)
              return false;

          // fputs("# File exported by ZenLib\n", f);
          fputs("",f);
          fputs(("o " + name + "\n").c_str(), f);

          // Write positions
          for (auto& v : mesh.vertices)
          {
              fputs(("v " + std::to_string(v.Position.x) + " " + std::to_string(v.Position.y) + " " + std::to_string(-v.Position.z) + "\n").c_str(), f);
          }

          // Write texture coords
          for (auto& v : mesh.vertices)
          {
              fputs(("v " + std::to_string(v.TexCoord.x) + " " + std::to_string(v.TexCoord.y) + "\n").c_str(), f);
          }

          // Write normals
          for (auto& v : mesh.vertices)
          {
              fputs(("v " + std::to_string(v.Normal.x) + " " + std::to_string(v.Normal.y) + " " + std::to_string(v.Normal.z) + "\n").c_str(), f);
          }

          fputs("s off\n", f);

          // Write faces
          for (auto& s : mesh.subMeshes)
          {
              fputs(("g " + s.material.matName + "\n").c_str(), f);
              fputs(("usemtl " + s.material.matName + "\n").c_str(), f);

              for (size_t i = 0; i < s.indexSize; i += 3)
              {
                  uint32_t idx[] = {mesh.indices[s.indexOffset+i], mesh.indices[s.indexOffset+i + 1], mesh.indices[s.indexOffset+i + 2]};
                  std::string f1 = std::to_string(idx[0] + 1) + "/" + std::to_string(idx[0] + 1) + "/" + std::to_string(idx[0] + 1);
                  std::string f2 = std::to_string(idx[1] + 1) + "/" + std::to_string(idx[1] + 1) + "/" + std::to_string(idx[1] + 1);
                  std::string f3 = std::to_string(idx[2] + 1) + "/" + std::to_string(idx[2] + 1) + "/" + std::to_string(idx[2] + 1);
                  std::string fres = f1; fres+=f2; fres += f3;

                  fputs(("f " + fres + "\n").c_str(), f);
              }
          }

          fclose(f);

          // Write mtl
          FILE* mf = fopen(("single_export/" + file + ".mtl").c_str(), "a");

          if (!mf)
              return false;

          for (auto& s : mesh.subMeshes)
          {
              if(std::find(materials.begin(),materials.end(),s.material.matName)!=materials.end())
                continue;
              fputs(("newmtl " + s.material.matName + "\n").c_str(), mf);

              fputs(("map_Kd " + s.material.texture + "\n").c_str(), mf);

              ZMath::float4 color;
              color.fromABGR8(s.material.color);

              fputs(("Kd " + std::to_string(color.x) + " " + std::to_string(color.y) + " " + std::to_string(color.z) + "\n").c_str(), mf);
              fputs(("Ka " + std::to_string(color.x) + " " + std::to_string(color.y) + " " + std::to_string(color.z) + "\n").c_str(), mf);

              fputs("\n", mf);
          }

          fclose(mf);

          return true;
      }
     
      private:
      Exporter(std::string name) { fileName=name; }
    
      std::vector<ZenLoad::PackedMesh> meshes;
      std::vector<std::string> fileNames;
      std::vector<std::string> materials;
      static Exporter* inst;
      static std::string fileName;
    };
    /**
     * Exports the given packed-mesh structure into a wavefront-OBJ-file
     * @param mesh Mesh to export
     * @param file Target OBJ-File
     * @param name Name of the object to export
     * @return Whether the export succeeded
     */
    
}  // namespace Utils
