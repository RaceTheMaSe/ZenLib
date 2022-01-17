#include "zCMaterial.h"

#include "zenParserPropRead.h"
#include "zenParser.h"

using namespace ZenLoad;

static const char* const MaterialGroupNames[MaterialGroup::NUM_MAT_GROUPS] = {
  "UNDEF",
  "METAL",
  "STONE",
  "WOOD",
  "EARTH",
  "WATER",
  "SNOW",
  };

zCMaterialData zCMaterial::readObjectData(ZenParser& parser, uint16_t version) {
  zCMaterialData materialInfo;
  if(version != 0x9C03) { // Gothic I
    ReadObjectProperties(parser,
      Prop("MaterialName"                , materialInfo.matName),
      Prop("MaterialGroup"               , materialInfo.matGroup),
      Prop("Color"                       , materialInfo.color),
      Prop("SmoothAngle"                 , materialInfo.smoothAngle),
      Prop("Texture"                     , materialInfo.texture),
      Prop("TextureScale"                , materialInfo.texScale),
      Prop("TextureAniFPS"               , materialInfo.texAniFPS),
      Prop("TextureAniMapMode"           , materialInfo.texAniMapMode),
      Prop("TextureAniMapDir"            , materialInfo.texAniMapDir),
      Prop("NoCollisionDetection"        , materialInfo.noCollDet),
      Prop("NoLightmap"                  , materialInfo.noLighmap),
      Prop("LoadDontCollapse"            , materialInfo.loadDontCollapse),
      Prop("DetailObject"                , materialInfo.detailObject),
      Prop("DefaultMapping"              , materialInfo.defaultMapping));
    } else { // Gothic II
    ReadObjectProperties(parser,
      Prop("MaterialName"                , materialInfo.matName),
      Prop("MaterialGroup"               , materialInfo.matGroup),
      Prop("Color"                       , materialInfo.color),
      Prop("SmoothAngle"                 , materialInfo.smoothAngle),
      Prop("Texture"                     , materialInfo.texture),
      Prop("TextureScale"                , materialInfo.texScale),
      Prop("TextureAniFPS"               , materialInfo.texAniFPS),
      Prop("TextureAniMapMode"           , materialInfo.texAniMapMode),
      Prop("TextureAniMapDir"            , materialInfo.texAniMapDir),
      Prop("NoCollisionDetection"        , materialInfo.noCollDet),
      Prop("NoLightmap"                  , materialInfo.noLighmap),
      Prop("LoadDontCollapse"            , materialInfo.loadDontCollapse),
      Prop("DetailObject"                , materialInfo.detailObject),
      Prop("DetailTextureScale"          , materialInfo.detailTextureScale),
      Prop("ForceOccluder"               , materialInfo.forceOccluder),
      Prop("EnvironmentMapping"          , materialInfo.environmentMapping),
      Prop("EnvironmentalMappingStrength", materialInfo.environmentalMappingStrength),
      Prop("WaveMode"                    , materialInfo.waveMode),
      Prop("WaveSpeed"                   , materialInfo.waveSpeed),
      Prop("WaveMaxAmplitude"            , materialInfo.waveMaxAmplitude),
      Prop("WaveGridSize"                , materialInfo.waveGridSize),
      Prop("IgnoreSun"                   , materialInfo.ignoreSun),
      Prop("AlphaFunc"                   , materialInfo.alphaFunc),
      Prop("DefaultMapping"              , materialInfo.defaultMapping));
    }
  return materialInfo;
  }

const char* zCMaterial::getMatGroupString(MaterialGroup group) {
  if(group>=MaterialGroup::NUM_MAT_GROUPS)
    return MaterialGroupNames[MaterialGroup::UNDEF];
  return MaterialGroupNames[group];
  }
