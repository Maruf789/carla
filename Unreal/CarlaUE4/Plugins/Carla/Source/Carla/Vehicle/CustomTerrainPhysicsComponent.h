// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Carla/Math/DVector.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Carla/MapGen/LargeMapManager.h"
THIRD_PARTY_INCLUDES_START
#include <carla/pytorch/pytorch.h>
THIRD_PARTY_INCLUDES_END

#include <unordered_map>
#include <vector>
#include "Misc/ScopeLock.h"
#include <string>

#include "CustomTerrainPhysicsComponent.generated.h"

struct FParticle
{
  // It formats particle data to "XValue YValue ZValue RadiusValue \n"
  std::string ToString() const;

  // String received must have format "XValue YValue ZValue RadiusValue \n"
  void ModifyDataFromString(const std::string& BaseString);
  FDVector Position; // position in m
  FVector Velocity;
  float Radius = 0.02f;
};
struct FHeightMapData
{
  void InitializeHeightmap(
    UTexture2D* Texture, FDVector Size, FDVector Origin,
      float MinHeight, float MaxHeight, FDVector Tile0);
  float GetHeight(FDVector Position) const; // get height at a given global 2d position
  void Clear();
private:
  FDVector WorldSize;
  FDVector Offset;
  uint32_t Size_X;
  uint32_t Size_Y;
  float MinHeight = 0.0f;
  float MaxHeight = 10.0f;
  FDVector Tile0Position;
  std::vector<float> Pixels;
};
struct FDenseTile
{
  void InitializeTile(float ParticleSize, float Depth, 
      FDVector TileOrigin, FDVector TileEnd, const FString& SavePath, const FHeightMapData &HeightMap);
  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  void GetParticlesInRadius(FDVector Position, float Radius, std::vector<FParticle*> &ParticlesInRadius);
  void GetParticlesInBox(const FOrientedBox& OBox, std::vector<FParticle*> &ParticlesInRadius);

  // Format DenseTile to "PosX PosY PosZ\n Particles"
  // WARNING LookAt ParticlesFormat
  std::string ToString() const;

  // String received must have format "PosX PosY PosZ\n ParticlesList"
  void ModifyDataFromString(const std::string& BaseString);

  std::vector<FParticle> Particles;
  FDVector TilePosition;
};
class FSparseHighDetailMap
{
public:

  FSparseHighDetailMap(float ParticleDiameter = 0.02f, float Depth = 0.4f)
    : ParticleSize(ParticleDiameter), TerrainDepth(Depth) {};

  void Init(float ParticleDiameter = 0.02f, float Depth = 0.4f)
  {
    ParticleSize = ParticleDiameter;
    TerrainDepth = Depth;
  }

  inline float GetTileSize() const {
    return TileSize;
  }
  std::vector<FParticle*> GetParticlesInRadius(FDVector Position, float Radius);
  std::vector<FParticle*> GetParticlesInBox(const FOrientedBox& OBox);

  FDenseTile& GetTile(uint32_t Tile_X, uint32_t Tile_Y);
  FDenseTile& GetTile(FDVector Position);
  FDenseTile& GetTile(uint64_t TileId);

  FDenseTile& InitializeRegion(uint32_t Tile_X, uint32_t Tile_Y);
  FDenseTile& InitializeRegion(uint64_t TileId);

  uint64_t GetTileId(uint32_t Tile_X, uint32_t Tile_Y);
  uint64_t GetTileId(uint64_t TileId);
  uint64_t GetTileId(FDVector Position);
  FIntVector GetVectorTileId(FDVector Position);
  FDVector GetTilePosition(uint64_t TileId);
  FDVector GetTilePosition(uint32_t Tile_X, uint32_t Tile_Y);

  float GetHeight(FDVector Position) {
    return Heightmap.GetHeight(Position);
  }

  void InitializeMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size, float MinHeight, float MaxHeight);
  
  void UpdateHeightMap(UTexture2D* HeightMapTexture,
      FDVector Origin, FDVector MapSize, float Size, float MinHeight, float MaxHeight);


  void LoadTilesAtPosition(FDVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);

  void Update(FVector Position, float RadiusX, float RadiusY);

  void SaveMap();

  void Clear();

  void LockMutex()
  {
    Lock_Map.Lock();
  }

  void UnLockMutex()
  {
    Lock_Map.Unlock();
  }

  FString SavePath;
private:
  std::unordered_map<uint64_t, FDenseTile> Map;
  std::unordered_map<uint64_t, FDenseTile> TilesToWrite;
  FDVector Tile0Position;
  FDVector Extension;
  float TileSize = 1.f; // 1m per tile
  FHeightMapData Heightmap;
  float ParticleSize = 0.02f;
  float TerrainDepth = 0.4f;
  FVector PositionToUpdate;
  FCriticalSection Lock_Map; // UE4 Mutex
  FCriticalSection Lock_Position; // UE4 Mutex

};

USTRUCT(BlueprintType)
struct FForceAtLocation
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Force;
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FVector Location;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCustomTerrainPhysicsComponent : public UActorComponent
{
  GENERATED_BODY()

public:

  UCustomTerrainPhysicsComponent(const FObjectInitializer& ObjectInitializer);

  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void TickComponent(float DeltaTime,
      ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

  // UFUNCTION(BlueprintCallable)
  // TArray<FHitResult> SampleTerrainRayCast(const TArray<FVector> &Locations);

  UFUNCTION(BlueprintCallable)
  void AddForces(const TArray<FForceAtLocation> &Forces);

  UFUNCTION(BlueprintCallable)
  TArray<FVector> GetParticlesInRadius(FVector Position, float Radius);

  UFUNCTION(BlueprintCallable)
  FVector GetTileCenter(FVector Position);

  UFUNCTION(BlueprintCallable, Category="Tiles")
  void LoadTilesAtPosition(FVector Position, float RadiusX = 100.0f, float RadiusY = 100.0f);
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D *HeightMap;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UTexture2D *TextureToUpdate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString NeuralModelFile = "";

  FVector LastUpdatedPosition;

  FString SavePath; 

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  float ForceMulFactor = 1.0;
  UPROPERTY(EditAnywhere)
  bool NNVerbose = false;
private:

  void RunNNPhysicsSimulation(
      ACarlaWheeledVehicle *Vehicle, float DeltaTime);
  // TArray<FParticle*> GetParticlesInRange(...);
  void SetUpParticleArrays(std::vector<FParticle*>& ParticlesIn, 
      TArray<float>& ParticlePosOut, 
      TArray<float>& ParticleVelOut);
  void SetUpWheelArrays(ACarlaWheeledVehicle *Vehicle, int WheelIdx,
      TArray<float>& WheelPos, 
      TArray<float>& WheelOrientation, 
      TArray<float>& WheelLinearVelocity, 
      TArray<float>& WheelAngularVelocity);
  void UpdateParticles(
      std::vector<FParticle*> Particles, std::vector<float> Forces,
      float DeltaTime);
  void ApplyForcesToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3,
      FVector TorqueWheel0, FVector TorqueWheel1, FVector TorqueWheel2, FVector TorqueWheel3);
  void ApplyMeanAccelerationToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3);
  void ApplyAccelerationToVehicle(
      ACarlaWheeledVehicle *Vehicle,
      FVector ForceWheel0, FVector ForceWheel1, FVector ForceWheel2, FVector ForceWheel3);
  void ApplyForces();

  void DrawParticles(UWorld* World, std::vector<FParticle*>& Particles);
  void DrawOrientedBox(UWorld* World, const TArray<FOrientedBox>& Boxes);

  UPROPERTY(EditAnywhere)
  TArray<FForceAtLocation> ForcesToApply;
  UPROPERTY(EditAnywhere)
  UPrimitiveComponent* RootComponent;
  UPROPERTY(EditAnywhere)
  float RayCastRange = 10.0f;
  UPROPERTY(EditAnywhere)
  FVector WorldSize = FVector(200000,200000,0);
  UPROPERTY(EditAnywhere)
  float SearchRadius = 100;
  UPROPERTY(EditAnywhere)
  float ParticleDiameter = 2;
  UPROPERTY(EditAnywhere)
  float TerrainDepth = 40;
  UPROPERTY(EditAnywhere)
  AActor *FloorActor = nullptr;
  UPROPERTY(EditAnywhere)
  bool bUpdateParticles = false;
  UPROPERTY(EditAnywhere)
  bool bUseDynamicModel = false;

  UPROPERTY(EditAnywhere)
  float TireRadius = 33.0229f;
  UPROPERTY(EditAnywhere)
  float TireWidth = 21.21f;
  UPROPERTY(EditAnywhere)
  float BoxSearchForwardDistance = 114.39f;
  UPROPERTY(EditAnywhere)
  float BoxSearchLateralDistance = 31.815f;
  UPROPERTY(EditAnywhere)
  float BoxSearchDepthDistance = 20.f;
  UPROPERTY(EditAnywhere)
  bool bDisableVehicleGravity = false;
  UPROPERTY(EditAnywhere)
  float MaxForceMagnitude = 1000000.f;
  UPROPERTY(EditAnywhere)
  float FloorHeight = 0.f;
  UPROPERTY(EditAnywhere)
  bool bUseImpulse = false;
  UPROPERTY(EditAnywhere)
  bool DrawDebugInfo = true;
  UPROPERTY(EditAnywhere)
  bool bUseMeanAcceleration = false;
  UPROPERTY(EditAnywhere)
  bool bShowForces = true;
  UPROPERTY(EditAnywhere)
  float MinHeight = 0;
  UPROPERTY(EditAnywhere)
  float MaxHeight = 10;
  UPROPERTY(EditAnywhere)
  FVector Tile0Origin;
  UPROPERTY(EditAnywhere)
  bool bDrawHeightMap = false;
  UPROPERTY(EditAnywhere)
  FVector DrawStart = FVector(0);
  UPROPERTY(EditAnywhere)
  FVector DrawEnd = FVector(1000, 1000, 0);
  UPROPERTY(EditAnywhere)
  FVector DrawInterval = FVector(100,100,0);
  UPROPERTY(EditAnywhere)
  int CUDADevice = 0;
  UPROPERTY(EditAnywhere)
  FVector HeightMapScaleFactor = FVector(1, 1, 1);
  UPROPERTY(EditAnywhere)
  FVector HeightMapOffset = FVector(0, 0, 0);

  UPROPERTY(EditAnywhere)
  FVector Radius = FVector(10,10,10);
  
  UPROPERTY(VisibleAnywhere)
  FIntVector CurrentLargeMapTileId = FIntVector(-1,-1,0);

  UPROPERTY(VisibleAnywhere)
  ALargeMapManager* LargeMapManager = nullptr;

  FSparseHighDetailMap SparseMap;

  TArray<ACarlaWheeledVehicle*> Vehicles;
  carla::learning::NeuralModel TerramechanicsModel;

  class FRunnableThread* Thread;
  struct FTilesWorker* TilesWorker;

};

struct FTilesWorker : public FRunnable
{
	FTilesWorker(class UCustomTerrainPhysicsComponent* TerrainComp, FVector NewPosition, float NewRadiusX, float NewRadiusY );

	virtual ~FTilesWorker() override;

	virtual uint32 Run() override; 

  class UCustomTerrainPhysicsComponent* CustomTerrainComp;
  FVector Position;
  float RadiusX; 
  float RadiusY;
  bool bShouldContinue = true;
};