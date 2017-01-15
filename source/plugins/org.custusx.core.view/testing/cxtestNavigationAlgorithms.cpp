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

#include "catch.hpp"
#include "cxNavigationAlgorithms.h"
#include "cxBoundingBox3D.h"
#include "cxLogger.h"

void test_findCameraPosOnLineFixedDistanceFromFocus(cx::Vector3D p_line, cx::Vector3D e_line, double distance, cx::Vector3D focus)
{
	cx::Vector3D p = cx::NavigationAlgorithms::findCameraPosOnLineFixedDistanceFromFocus(p_line,
																						 e_line,
																						 distance,
																						 focus);
	CHECK(cx::similar((p-focus).length(), distance));
}

TEST_CASE("NavigationAlgorithms: findCameraPosOnLineFixedDistanceFromFocus ", "[unit]")
{
	test_findCameraPosOnLineFixedDistanceFromFocus(cx::Vector3D(0,0,0), cx::Vector3D(0,1,0),  2, cx::Vector3D(1,0,0));
	test_findCameraPosOnLineFixedDistanceFromFocus(cx::Vector3D(0,0,0), cx::Vector3D(0,1,0), 10, cx::Vector3D(1,1,0));
	test_findCameraPosOnLineFixedDistanceFromFocus(cx::Vector3D(2,2,0), cx::Vector3D(0,1,0), 10, cx::Vector3D(1,1,0));
	test_findCameraPosOnLineFixedDistanceFromFocus(cx::Vector3D(2,2,0), cx::Vector3D(1,1,0), 20, cx::Vector3D(1,2,0));
	test_findCameraPosOnLineFixedDistanceFromFocus(cx::Vector3D(2,2,0), cx::Vector3D(1,1,0), 20, cx::Vector3D(1,2,4));
}

TEST_CASE("NavigationAlgorithms: elevateCamera", "[unit]")
{
	// move 45deg up, elevating 100/sqr(2) up from plane and from focus
	double angle = 45.0/180*M_PI;
	cx::Vector3D camera(0,0,0);
	cx::Vector3D focus(100,0,0);
	cx::Vector3D vup(0,0,1);

	cx::Vector3D camera_el = cx::NavigationAlgorithms::elevateCamera(angle, camera, focus, vup);
	double d = 100.0/sqrt(2);
	double angle_el = acos(cx::dot(camera_el-focus,camera-focus)/(camera_el-focus).length()/(camera-focus).length());
	INFO("camera_el " + string_cast(camera_el));
	INFO("angle_el " + string_cast(angle_el/M_PI*180));

	CHECK(cx::similar(angle_el, angle));
	CHECK(cx::similar((camera_el-focus).length(), (camera-focus).length()));
	CHECK(cx::similar(camera_el, cx::Vector3D(100.0-d, 0, d)));
}

TEST_CASE("NavigationAlgorithms: elevateCamera complex", "[unit]")
{
	// move 45deg up, elevating 100/sqr(2) up from plane and from focus
	double angle = 45.0/180*M_PI;
	cx::Vector3D camera(20,10,50);
	cx::Vector3D focus(120,10,50);
	cx::Vector3D vup = cx::Vector3D(0,1,1).normal();

	cx::Vector3D camera_el = cx::NavigationAlgorithms::elevateCamera(angle, camera, focus, vup);
	double d = 100.0/sqrt(2);
	double angle_el = acos(cx::dot(camera_el-focus,camera-focus)/(camera_el-focus).length()/(camera-focus).length());
	INFO("camera_el " + string_cast(camera_el));
	INFO("angle_el " + string_cast(angle_el/M_PI*180));

//	CHECK(cx::similar(angle_el, angle));
	CHECK(cx::similar((camera_el-focus).length(), (camera-focus).length()));
//	CHECK(cx::similar(camera_el, cx::Vector3D(100.0-d, 0, d)));
}

TEST_CASE("NavigationAlgorithms: orthogonalize_vup", "[unit]")
{
	double angle = 45.0/180*M_PI;
	cx::Vector3D camera(0,0,0);
	cx::Vector3D focus(100,0,0);
	cx::Vector3D vpn(0,0,1);
	cx::Vector3D vup_fallback(1,0,0);

	cx::Vector3D vup_o;
	vup_o =  cx::NavigationAlgorithms::orthogonalize_vup(cx::Vector3D(1,1,1), vpn, vup_fallback);
	CHECK(cx::similar(vup_o, cx::Vector3D(1.0/sqrt(2), 1.0/sqrt(2), 0)));

	vup_o =  cx::NavigationAlgorithms::orthogonalize_vup(vpn, vpn, vup_fallback);
	CHECK(cx::similar(vup_o, vup_fallback));
}

TEST_CASE("NavigationAlgorithms: findCameraPosByZoomingToROI", "[unit]")
{
	// look along x-axis, horizontal view (10* vertical, 45* horizontal).
	// A bounding box of 100^3 centered at focus shall be visible by camera.
	double viewAngle_large = 45.0/180*M_PI;;
	double viewAngle_small = 10.0/180*M_PI;
	cx::Vector3D focus(150,0,0);
	cx::Vector3D vpn(-1,0,0);
	cx::Vector3D vup(0,0,1);
	cx::DoubleBoundingBox3D bb(100,200,-50,50,-50,50);

	double d = 50 + 50/tan(viewAngle_small/2);
	cx::Vector3D camera_nominal = focus + vpn*d;

	cx::Vector3D camera_new = cx::NavigationAlgorithms::findCameraPosByZoomingToROI(viewAngle_large,
																					viewAngle_small,
																					focus,
																					vup,
																					vpn,
																					bb);

	INFO("camera_nom " + string_cast(camera_nominal));
	INFO("camera_new " + string_cast(camera_new));
	CHECK(cx::similar(camera_nominal, camera_new));
}

///**
// * Given a scene with focus and vpn, and vertical+horizontal view angles,
// * find a camera pos that keeps the entire input bounding box in view.
// */
//static Vector3D findCameraPosByZoomingToROI(double viewAngle_vertical, double viewAngle_horizontal, Vector3D focus, Vector3D vpn, const DoubleBoundingBox3D& bb);

