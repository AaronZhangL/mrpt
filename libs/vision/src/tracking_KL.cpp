/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2012, MAPIR group, University of Malaga                |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */

#include <mrpt/vision.h>  // Precompiled headers

#include <mrpt/system/memory.h>
#include <mrpt/vision/tracking.h>
#include <mrpt/vision/CFeatureExtraction.h>

// Universal include for all versions of OpenCV
#include <mrpt/otherlibs/do_opencv_includes.h> 


using namespace mrpt;
using namespace mrpt::vision;
using namespace mrpt::utils;
using namespace std;


/** Track a set of features from old_img -> new_img using sparse optimal flow (classic KL method)
  *  Optional parameters that can be passed in "extra_params":
  *		- "window_width"  (Default=15)
  *		- "window_height" (Default=15)
  *
  *  \sa OpenCV's method cvCalcOpticalFlowPyrLK
  */
template <typename FEATLIST>
void CFeatureTracker_KL::trackFeatures_impl_templ(
	const CImage &old_img,
	const CImage &new_img,
	FEATLIST &featureList )
{
MRPT_START

	const unsigned int 	window_width = extra_params.getWithDefaultVal("window_width",15);
	const unsigned int 	window_height = extra_params.getWithDefaultVal("window_height",15);

	const int 	LK_levels    = extra_params.getWithDefaultVal("LK_levels",3);
	const int 	LK_max_iters = extra_params.getWithDefaultVal("LK_max_iters",10);
	const int 	LK_epsilon   = extra_params.getWithDefaultVal("LK_epsilon",0.1);
	const float LK_max_tracking_error = extra_params.getWithDefaultVal("LK_max_tracking_error",150.0f);

#if MRPT_HAS_OPENCV
	// Both images must be of the same size
	ASSERT_( old_img.getWidth() == new_img.getWidth() && old_img.getHeight() == new_img.getHeight() );

	const size_t  img_width  = old_img.getWidth();
	const size_t  img_height = old_img.getHeight();

	const size_t nFeatures	= featureList.size();					// Number of features

	// Grayscale images
	const CImage prev_gray(old_img, FAST_REF_OR_CONVERT_TO_GRAY);
	const CImage cur_gray(new_img, FAST_REF_OR_CONVERT_TO_GRAY);

	// Array conversion MRPT->OpenCV
	if (nFeatures>0)
	{
		CvPoint2D32f *points[2];

		points[0] = reinterpret_cast<CvPoint2D32f *>( mrpt_alloca(sizeof(CvPoint2D32f)*nFeatures) );
		points[1] = reinterpret_cast<CvPoint2D32f *>( mrpt_alloca(sizeof(CvPoint2D32f)*nFeatures) );

		std::vector<char>	status(nFeatures);

		for(size_t i=0;i<nFeatures;++i)
		{
			points[0][i].x = featureList.getFeatureX(i);
			points[0][i].y = featureList.getFeatureY(i);
		} // end for

		// local scope for auxiliary variables around cvCalcOpticalFlowPyrLK()
		const IplImage *prev_gray_ipl = prev_gray.getAs<IplImage>();
		const IplImage *cur_gray_ipl  = cur_gray.getAs<IplImage>();

		// Pyramids
		// JL: It seems that cache'ing the pyramids of previous images doesn't really improve the efficiency (!?!?)
		IplImage* pPyr = NULL;
		IplImage* cPyr = NULL;

		int	flags = 0;

		float* track_error = reinterpret_cast<float*>( mrpt_alloca(sizeof(float)*nFeatures) );

		cvCalcOpticalFlowPyrLK(prev_gray_ipl, cur_gray_ipl, pPyr, cPyr,
			&points[0][0], &points[1][0], nFeatures, cvSize( window_width, window_height ), LK_levels, &status[0], track_error,
			cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,LK_max_iters,LK_epsilon), flags );

		cvReleaseImage( &pPyr );
		cvReleaseImage( &cPyr );

		for(size_t i=0;i<nFeatures;++i)
		{
			const bool trck_err_too_large = track_error[i]>LK_max_tracking_error;

			if( status[i] == 1 &&
				!trck_err_too_large &&
				points[1][i].x > 0 && points[1][i].y > 0 &&
				points[1][i].x < img_width && points[1][i].y < img_height )
			{
				// Feature could be tracked
				featureList.setFeatureXf(i, points[1][i].x );
				featureList.setFeatureYf(i, points[1][i].y );
				featureList.setTrackStatus(i, statusKLT_TRACKED );
			} // end if
			else	// Feature could not be tracked
			{
				featureList.setFeatureX(i,-1);
				featureList.setFeatureY(i,-1);
				featureList.setTrackStatus(i, trck_err_too_large ? status_LOST : statusKLT_OOB );
			} // end else
		} // end for

		mrpt_alloca_free( points[0] );
		mrpt_alloca_free( points[1] );
		mrpt_alloca_free( track_error );


		// In case it needs to rebuild a kd-tree or whatever
		featureList.mark_as_outdated();
	}

#else
	THROW_EXCEPTION("The MRPT has been compiled with MRPT_HAS_OPENCV=0 !");
#endif

	MRPT_END
} // end trackFeatures


void CFeatureTracker_KL::trackFeatures_impl(
	const CImage &old_img,
	const CImage &new_img,
	CFeatureList &featureList )
{
	trackFeatures_impl_templ<CFeatureList>(old_img,new_img,featureList);
}

void CFeatureTracker_KL::trackFeatures_impl(
	const CImage &old_img,
	const CImage &new_img,
	TSimpleFeatureList &featureList )
{
	trackFeatures_impl_templ<TSimpleFeatureList>(old_img,new_img,featureList);
}

void CFeatureTracker_KL::trackFeatures_impl(
	const CImage &old_img,
	const CImage &new_img,
	TSimpleFeaturefList &featureList )
{
	trackFeatures_impl_templ<TSimpleFeaturefList>(old_img,new_img,featureList);
}



