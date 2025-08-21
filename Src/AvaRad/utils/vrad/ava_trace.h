// Redduck Inc, 2007
#pragma once

struct lightinfo_t;

// trace
extern int skip_id;
extern int self_id;
int TestLineWithOpcode( Vector const &start, Vector const &stop );
int TestLineWithSkipByOpcode( Vector const &start, Vector const &stop );

bool SampleAmbientWithOpcode( Vector const &start, Vector const &stop, Vector *sample_color );
bool SampleDiffuseReflectionWithOpcode( Vector const &start, Vector const &stop, Vector *sample_color );

// collisiion model
void InitOpcode();
void CreateOpcodeModels();
void FinalizeOpcode();

class AVA_StaticMesh
{
public:
	AVA_StaticMesh() :
		id_(-1),
		startIndex_(0), numtriangles_(0),
		startvertex_(0), numvertices_(0),
		flags_(0), sampleToAreaRatio_(1.0f)
	{}

	AVA_StaticMesh( int id, int startIndex, int numtriangles, int startvertex, int numvertices, int flags, float sampleToAreaRatio )
		:	id_(id),
			startIndex_(startIndex), numtriangles_(numtriangles),
			startvertex_(startvertex), numvertices_(numvertices),
			flags_(flags), sampleToAreaRatio_(sampleToAreaRatio)
	{}

	int GetId() const { return id_; }
	int GetStartTriangleIndex() const { return (startIndex_ / 3); }
	int GetNumTrisnagles() const;
	void GetTriangleIndices( int triangleId, int *i0, int *i1, int *i2 ) const;
	int GetStartVertex() const;
	int GetNumVertices() const;
	const Vector& GetVertex( int index ) const;
	const Vector& GetTangentS( int index ) const;
	const Vector& GetTangentT( int index ) const;
	const Vector& GetVertexNormal( int index ) const;
	float GetSampleToAreaRatio() const
	{
		return sampleToAreaRatio_;
	}
	boolean NeedsMultisample() const
	{
		return (flags_ & 0x01);
	}
	boolean HasEmissive() const
	{
		return (flags_ & 0x02);
	}
	boolean NeedsSelfShadow() const
	{
		return (flags_ & 0x04);
	}
	boolean DoCastShadow() const
	{
		return (flags_ & 0x08);
	}


private:
	int id_;
	int startIndex_;
	int numtriangles_;
	int startvertex_;
	int	numvertices_;
	int flags_;
	float sampleToAreaRatio_;
};

class AVA_StaticMeshManager
{
public:
	static int GetNumMeshes();
	static const AVA_StaticMesh& GetStaticMesh( int meshid );
};

extern CUtlVector<int> sun_visibility_array;