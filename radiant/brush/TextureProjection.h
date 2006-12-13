#ifndef TEXTUREPROJECTION_H_
#define TEXTUREPROJECTION_H_

#include "texturelib.h"
#include "winding.h"
#include "math/aabb.h"

enum TexdefTypeId
{
  TEXDEFTYPEID_QUAKE,
  TEXDEFTYPEID_BRUSHPRIMITIVES,
  TEXDEFTYPEID_HALFLIFE,
};

struct bp_globals_t
{
  // tells if we are internally using brush primitive (texture coordinates and map format)
  // this is a shortcut for IntForKey( g_qeglobals.d_project_entity, "brush_primit" )
  // NOTE: must keep the two ones in sync
  TexdefTypeId m_texdefTypeId;
};

extern bp_globals_t g_bp_globals;

/* greebo: A texture projection contains the texture definition
 * as well as the brush primitive texture definition. 
 */
class TextureProjection {
public:
	TexDef m_texdef;
	BrushPrimitTexDef m_brushprimit_texdef;
	Vector3 m_basis_s;
	Vector3 m_basis_t;

	// Constructor
	TextureProjection() {}
	
	// Copy Constructor
	TextureProjection(
		const TexDef& texdef,
		const BrushPrimitTexDef& brushprimit_texdef,
		const Vector3& basis_s,
		const Vector3& basis_t
	) :
		m_texdef(texdef),
		m_brushprimit_texdef(brushprimit_texdef),
		m_basis_s(basis_s),
		m_basis_t(basis_t)
	{}
	
	// Assigns an <other> projection to this one
	void assign(const TextureProjection& other) {
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
			m_brushprimit_texdef = other.m_brushprimit_texdef;
		}
		else {
			m_texdef = other.m_texdef;
			if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_HALFLIFE) {
				m_basis_s = other.m_basis_s;
				m_basis_t = other.m_basis_t;
			}
		}
	}
	
	/* greebo: Uses the transformation matrix <transform> to set the internal texture
	 * definitions. Checks the matrix for validity and passes it on to
	 * the according internal texture definitions (TexDef or BPTexDef)
	 */
	void setTransform(float width, float height, const Matrix4& transform) {
		// Check the matrix for validity
		if ((transform[0] != 0 || transform[4] != 0) && (transform[1] != 0 || transform[5] != 0)) {
			// Decide which TexDef to use
			if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
				m_brushprimit_texdef = BrushPrimitTexDef(transform);
			}
			else {
				m_texdef = TexDef(width, height, transform);
			}
		} else {
			std::cout << "invalid texture matrix\n";
		}
	}
	
	/* greebo: Returns the transformation matrix from the
	 * texture definitions members. 
	 */
	Matrix4 getTransform(float width, float height) const {
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
    		return m_brushprimit_texdef.getTransform();
		}
		else {
			return m_texdef.getTransform(width, height);
		}
	}
	
	void shift(float s, float t) {
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
			m_brushprimit_texdef.shift(s, t);
		}
		else {
			m_texdef.shift(s, t);
		}
	}

	void scale(float s, float t) {
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
			m_brushprimit_texdef.scale(s, t);
		}
		else {
			m_texdef.scale(s, t);
		}
	}
	
	void rotate(float angle) {
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
			m_brushprimit_texdef.rotate(angle);
		}
		else {
			m_texdef.rotate(angle);
		}
	}
	
	// Normalise projection for a given texture width and height.
	void normalise(float width, float height) {
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
			m_brushprimit_texdef.normalise(width, height);
		}
		else {
			m_texdef.normalise(width, height);
		}
	}
	
	Matrix4 getBasisForNormal(const Vector3& normal) const {
		
		Matrix4 basis;
		
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_BRUSHPRIMITIVES) {
			basis = g_matrix4_identity;
			ComputeAxisBase(normal, basis.x().getVector3(), basis.y().getVector3());
			basis.z().getVector3() = normal;
			basis.transpose();
			//DebugAxisBase(normal);
		}
		else if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_HALFLIFE) {
			basis = g_matrix4_identity;
			basis.x().getVector3() = m_basis_s;
			basis.y().getVector3() = -m_basis_t;
			basis.z().getVector3() = basis.x().getVector3().crossProduct(basis.y().getVector3()).getNormalised();
			matrix4_multiply_by_matrix4(basis, matrix4_rotation_for_z_degrees(-m_texdef._rotate));
			//globalOutputStream() << "debug: " << projection.m_basis_s << projection.m_basis_t << normal << "\n";
			basis.transpose();
		}
		else {
			Normal_GetTransform(normal, basis);
		}
		
		return basis;
	}
	
	void transformLocked(std::size_t width, std::size_t height, const Plane3& plane, const Matrix4& identity2transformed) {
		//globalOutputStream() << "identity2transformed: " << identity2transformed << "\n";

		//globalOutputStream() << "plane.normal(): " << plane.normal() << "\n";

		Vector3 normalTransformed(matrix4_transformed_direction(identity2transformed, plane.normal()));

	  //globalOutputStream() << "normalTransformed: " << normalTransformed << "\n";
	
	  // identity: identity space
	  // transformed: transformation
	  // stIdentity: base st projection space before transformation
	  // stTransformed: base st projection space after transformation
	  // stOriginal: original texdef space
	
	  // stTransformed2stOriginal = stTransformed -> transformed -> identity -> stIdentity -> stOriginal

		Matrix4 identity2stIdentity = getBasisForNormal(plane.normal());
		//globalOutputStream() << "identity2stIdentity: " << identity2stIdentity << "\n";
		
		if (g_bp_globals.m_texdefTypeId == TEXDEFTYPEID_HALFLIFE) {
			matrix4_transform_direction(identity2transformed, m_basis_s);
			matrix4_transform_direction(identity2transformed, m_basis_t);
		}

		Matrix4 transformed2stTransformed = getBasisForNormal(normalTransformed);
		
		Matrix4 stTransformed2identity(matrix4_affine_inverse(matrix4_multiplied_by_matrix4(transformed2stTransformed, identity2transformed)));
		
		Vector3 originalProjectionAxis(matrix4_affine_inverse(identity2stIdentity).z().getVector3());
		
		Vector3 transformedProjectionAxis(stTransformed2identity.z().getVector3());
		
		Matrix4 stIdentity2stOriginal = getTransform((float)width, (float)height);
		Matrix4 identity2stOriginal(matrix4_multiplied_by_matrix4(stIdentity2stOriginal, identity2stIdentity));

		//globalOutputStream() << "originalProj: " << originalProjectionAxis << "\n";
		//globalOutputStream() << "transformedProj: " << transformedProjectionAxis << "\n";
		double dot = originalProjectionAxis.dot(transformedProjectionAxis);
		//globalOutputStream() << "dot: " << dot << "\n";
		if (dot == 0) {
			// The projection axis chosen for the transformed normal is at 90 degrees
			// to the transformed projection axis chosen for the original normal.
			// This happens when the projection axis is ambiguous - e.g. for the plane
			// 'X == Y' the projection axis could be either X or Y.
			//globalOutputStream() << "flipped\n";
#if 0
			globalOutputStream() << "projection off by 90\n";
			globalOutputStream() << "normal: ";
			print_vector3(plane.normal());
			globalOutputStream() << "original projection: ";
			print_vector3(originalProjectionAxis);
			globalOutputStream() << "transformed projection: ";
			print_vector3(transformedProjectionAxis);
#endif

			Matrix4 identityCorrected = matrix4_reflection_for_plane45(plane, originalProjectionAxis, transformedProjectionAxis);

			identity2stOriginal = matrix4_multiplied_by_matrix4(identity2stOriginal, identityCorrected);
		}

		Matrix4 stTransformed2stOriginal = matrix4_multiplied_by_matrix4(identity2stOriginal, stTransformed2identity);

		setTransform((float)width, (float)height, stTransformed2stOriginal);
		normalise((float)width, (float)height);
	}
	
	/* Fits a texture to a brush face
	 */
	void fitTexture(std::size_t width, std::size_t height, const Vector3& normal, const Winding& w, float s_repeat, float t_repeat) {
		if (w.numpoints < 3) {
			return;
		}

		Matrix4 st2tex = getTransform((float)width, (float)height);

		// the current texture transform
		Matrix4 local2tex = st2tex;
		{
			Matrix4 xyz2st; 
			xyz2st = getBasisForNormal(normal);
			matrix4_multiply_by_matrix4(local2tex, xyz2st);
		}

		// the bounds of the current texture transform
		AABB bounds;
		for (Winding::const_iterator i = w.begin(); i != w.end(); ++i) {
			Vector3 texcoord = matrix4_transformed_point(local2tex, i->vertex);
			bounds.includePoint(texcoord);
		}
		bounds.origin.z() = 0;
		bounds.extents.z() = 1;
		
		// the bounds of a perfectly fitted texture transform
		AABB perfect(Vector3(s_repeat * 0.5, t_repeat * 0.5, 0), Vector3(s_repeat * 0.5, t_repeat * 0.5, 1));
		
		// the difference between the current texture transform and the perfectly fitted transform
		Matrix4 matrix = Matrix4::getTranslation(bounds.origin - perfect.origin);
		matrix4_pivoted_scale_by_vec3(matrix, bounds.extents / perfect.extents, perfect.origin);
		matrix4_affine_invert(matrix);
		
		// apply the difference to the current texture transform
		matrix4_premultiply_by_matrix4(st2tex, matrix);
		
		setTransform((float)width, (float)height, st2tex);
		normalise((float)width, (float)height);
	}
	
}; // class TextureProjection

#endif /*TEXTUREPROJECTION_H_*/
