/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#include "cxBoundingBox3D.h"
#include "cxUtilHelpers.h"
#include "cxTypeConversions.h"

// --------------------------------------------------------
namespace cx
{
//namespace utils
//{
//// --------------------------------------------------------


// --------------------------------------------------------
// IntBoundingBox3D
IntBoundingBox3D::IntBoundingBox3D()
{
}
IntBoundingBox3D::IntBoundingBox3D(int x0, int x1, int y0, int y1, int z0, int z1)
{
	elems[0] = x0;
	elems[1] = x1;
	elems[2] = y0;
	elems[3] = y1;
	elems[4] = z0;
	elems[5] = z1;
}
/**Create a bounding box describing the volume spanned by the two points a and b.
 */
IntBoundingBox3D::IntBoundingBox3D(const Vector3D& a, const Vector3D& b)
{
	Vector3D bl, tr;
	for (unsigned i = 0; i < a.size(); ++i)
	{
		bl[i] = std::min(a[i], b[i]);
		tr[i] = std::max(a[i], b[i]);
	}

	elems[0] = static_cast<int> (bl[0]);
	elems[1] = static_cast<int> (tr[0]);
	elems[2] = static_cast<int> (bl[1]);
	elems[3] = static_cast<int> (tr[1]);
	elems[4] = static_cast<int> (bl[2]);
	elems[5] = static_cast<int> (tr[2]);
}

IntBoundingBox3D::IntBoundingBox3D(const Eigen::Vector3i& a, const Eigen::Vector3i& b)
{
	Eigen::Vector3i bl, tr;
	for (unsigned i = 0; i < a.size(); ++i)
	{
		bl[i] = std::min(a[i], b[i]);
		tr[i] = std::max(a[i], b[i]);
	}

	elems[0] = static_cast<int> (bl[0]);
	elems[1] = static_cast<int> (tr[0]);
	elems[2] = static_cast<int> (bl[1]);
	elems[3] = static_cast<int> (tr[1]);
	elems[4] = static_cast<int> (bl[2]);
	elems[5] = static_cast<int> (tr[2]);
}

IntBoundingBox3D::IntBoundingBox3D(const double* data)
{
	for (unsigned i = 0; i < size(); ++i)
		elems[i] = static_cast<int> (data[i]);
	//	std::copy(data, data+size(), elems); 
}
IntBoundingBox3D::IntBoundingBox3D(const int* data)
{
	std::copy(data, data + size(), elems);
}
// --------------------------------------------------------
Eigen::Vector3i IntBoundingBox3D::bottomLeft() const
{
	return Eigen::Vector3i(elems[0], elems[2], elems[4]);
}
Eigen::Vector3i IntBoundingBox3D::topRight() const
{
	return Eigen::Vector3i(elems[1], elems[3], elems[5]);
}
Eigen::Vector3i IntBoundingBox3D::center() const
{
	return (bottomLeft() + topRight()) / 2.0;
}
Eigen::Vector3i IntBoundingBox3D::range() const
{
	return topRight() - bottomLeft();
}
/**Return the coordinate value of a given corner.
 * Insert 0 or 1 for each of the 3 axis in order to get start or 
 * stop corner along that axis.  
 */
Eigen::Vector3i IntBoundingBox3D::corner(int x, int y, int z) const
{
	Eigen::Vector3i c;
	c[0] = x ? elems[1] : elems[0];
	c[1] = y ? elems[3] : elems[2];
	c[2] = z ? elems[5] : elems[4];
	return c;
}
// --------------------------------------------------------

/** return true if p is inside or on the edge of the bounding box
 */
bool IntBoundingBox3D::contains(const Eigen::Vector3i& p) const
{
	bool inside = true;
	for (unsigned i = 0; i < 3; ++i)
		inside &= ((elems[2 * i] <= p[i]) && (p[i] <= elems[2 * i + 1]));
	return inside;
}

std::ostream& operator<<(std::ostream& s, const IntBoundingBox3D& data)
{
	return stream_range(s, data.begin(), data.end());
}
// --------------------------------------------------------

// --------------------------------------------------------
// DoubleBoundingBox3D 
DoubleBoundingBox3D::DoubleBoundingBox3D()
{
}
DoubleBoundingBox3D::DoubleBoundingBox3D(double x0, double x1, double y0, double y1, double z0, double z1)
{
	elems[0] = x0;
	elems[1] = x1;
	elems[2] = y0;
	elems[3] = y1;
	elems[4] = z0;
	elems[5] = z1;
}
/**Create a bounding box describing the volume spanned by the two points a and b.
 */
DoubleBoundingBox3D::DoubleBoundingBox3D(const Vector3D& a, const Vector3D& b)
{
	Vector3D bl, tr;
	for (unsigned i = 0; i < a.size(); ++i)
	{
		bl[i] = std::min(a[i], b[i]);
		tr[i] = std::max(a[i], b[i]);
	}

	elems[0] = bl[0];
	elems[1] = tr[0];
	elems[2] = bl[1];
	elems[3] = tr[1];
	elems[4] = bl[2];
	elems[5] = tr[2];
}
DoubleBoundingBox3D::DoubleBoundingBox3D(const double* data)
{
	std::copy(data, data + size(), elems);
}
DoubleBoundingBox3D::DoubleBoundingBox3D(const int* data)
{
	std::copy(data, data + size(), elems);
}
DoubleBoundingBox3D::DoubleBoundingBox3D(const IntBoundingBox3D& bb)
{
	for (unsigned i = 0; i < size(); ++i)
	{
		elems[i] = bb.elems[i];
	}
}
// --------------------------------------------------------

/**Create a bounding box based the following input: (xmin,ymin,xmax,ymax)
 */
DoubleBoundingBox3D DoubleBoundingBox3D::fromViewport(const double* data)
{
	return DoubleBoundingBox3D(data[0], data[2], data[1], data[3], 0, 0);
}
// --------------------------------------------------------

Vector3D DoubleBoundingBox3D::bottomLeft() const
{
	return Vector3D(elems[0], elems[2], elems[4]);
}
Vector3D DoubleBoundingBox3D::topRight() const
{
	return Vector3D(elems[1], elems[3], elems[5]);
}
Vector3D DoubleBoundingBox3D::center() const
{
	return (bottomLeft() + topRight()) / 2.0;
}
Vector3D DoubleBoundingBox3D::range() const
{
	return topRight() - bottomLeft();
}
/**Return the coordinate value of a given corner.
 * Insert 0 or 1 for each of the 3 axis in order to get start or 
 * stop corner along that axis.  
 */
Vector3D DoubleBoundingBox3D::corner(int x, int y, int z) const
{
	Vector3D c;
	c[0] = x ? elems[1] : elems[0];
	c[1] = y ? elems[3] : elems[2];
	c[2] = z ? elems[5] : elems[4];
	return c;
}
// --------------------------------------------------------

/** return true if p is inside or on the edge of the bounding box
 */
bool DoubleBoundingBox3D::contains(const Vector3D& p) const
{
	bool inside = true;
	for (unsigned i = 0; i < 3; ++i)
		inside &= ((elems[2 * i] <= p[i]) && (p[i] <= elems[2 * i + 1]));
	return inside;
}

bool similar(const DoubleBoundingBox3D& a, const DoubleBoundingBox3D& b, double tol)
{
	return similar(a.bottomLeft(), b.bottomLeft(), tol) && similar(a.topRight(), b.topRight(), tol);
}
// --------------------------------------------------------
std::ostream& operator<<(std::ostream& s, const DoubleBoundingBox3D& data)
{
	return stream_range(s, data.begin(), data.end());
}
// --------------------------------------------------------

DoubleBoundingBox3D DoubleBoundingBox3D::fromCloud(std::vector<Vector3D> cloud)
{
	if (cloud.empty())
		return DoubleBoundingBox3D::zero();

	Vector3D a = cloud[0]; // min
	Vector3D b = cloud[0]; // max

	for (unsigned int i = 0; i < cloud.size(); ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			a[j] = std::min(a[j], cloud[i][j]);
			b[j] = std::max(b[j], cloud[i][j]);
		}
	}
	return DoubleBoundingBox3D(a, b);
}

DoubleBoundingBox3D DoubleBoundingBox3D::fromString(const QString& text)
{
	std::vector<double> raw = convertQString2DoubleVector(text);
	if (raw.size() != 6)
		return DoubleBoundingBox3D(0, 1, 0, 1, 0, 1);
	return DoubleBoundingBox3D((double*) &(*raw.begin()));
}

DoubleBoundingBox3D DoubleBoundingBox3D::unionWith(const DoubleBoundingBox3D& other) const
{
	std::vector<Vector3D> cloud;
	cloud.push_back(corner(0,0,0));
	cloud.push_back(other.corner(0,0,0));
	cloud.push_back(corner(0,0,1));
	cloud.push_back(other.corner(0,0,1));
	cloud.push_back(corner(0,1,0));
	cloud.push_back(other.corner(0,1,0));
	cloud.push_back(corner(0,1,1));
	cloud.push_back(other.corner(0,1,1));
	cloud.push_back(corner(1,0,0));
	cloud.push_back(other.corner(1,0,0));
	cloud.push_back(corner(1,0,1));
	cloud.push_back(other.corner(1,0,1));
	cloud.push_back(corner(1,1,0));
	cloud.push_back(other.corner(1,1,0));
	cloud.push_back(corner(1,1,1));
	cloud.push_back(other.corner(1,1,1));
	return fromCloud(cloud);
}

DoubleBoundingBox3D intersection(DoubleBoundingBox3D a, DoubleBoundingBox3D b)
{
	DoubleBoundingBox3D bb = a;

	for (int i=0; i<3; ++i)
	{
		bb[2*i]   = std::max(a[2*i],   b[2*i]);
		bb[2*i+1] = std::min(a[2*i+1], b[2*i+1]);
	}

	// check for no intersection
	Vector3D range = bb.range();
	for (int i=0; i<3; ++i)
		if (range[i]<0.0)
			return DoubleBoundingBox3D::zero();

	return bb;
}

// --------------------------------------------------------
//} // namespace utils
} // namespace cx
// --------------------------------------------------------
