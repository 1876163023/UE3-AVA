// Redduck inc, 2007

#pragma once

extern bool	g_bUseMPI;
extern bool g_bMPIMaster;

int GetNumVertexLightingJobs();
int GetNumAmbientCubeJobs();

void AVA_MPI_BuildFacelights();
void AVA_MPI_BuildVertexlights();
void AVA_MPI_BuildVertexAmbientlights();
void AVA_MPI_BuildAmbientcubes();
void AVA_MPI_DistributeLightingData();

// 2007. 5. 1 ~
// Implement AVA_Render
// Improve Diffuse Interreflection
void AVA_MPI_BuildFacelights2();
void AVA_MPI_DistributeBouncedLight();
void AVA_MPI_GatherFaceAmbientLights();
void AVA_MPI_GatherVertexAmbientLights();
void AVA_MPI_BuildAmbientcubes_GI();

// 2007. 8. 16 changmin
// Secondary Light Source
void AVA_MPI_BuildSecondaryFacelights();
void AVA_MPI_DistributeSecondaryFacelights();
