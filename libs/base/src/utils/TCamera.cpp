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

#include <mrpt/base.h>  // Precompiled headers

#include <mrpt/utils/TCamera.h>
#include <mrpt/utils/CConfigFileMemory.h>
#include <mrpt/math/ops_matrices.h>  // For "<<" ">>" operators.

using namespace mrpt::utils;
using namespace mrpt::math;
using namespace std;


/* Implements serialization for the TCamera struct as it will be included within CObservations objects */
IMPLEMENTS_SERIALIZABLE( TCamera, CSerializable, mrpt::utils )

/** Dumps all the parameters as a multi-line string, with the same format than \a saveToConfigFile.  \sa saveToConfigFile */
std::string TCamera::dumpAsText() const
{
	mrpt::utils::CConfigFileMemory cfg;
	saveToConfigFile("",cfg);
	return cfg.getContent();
}


// WriteToStream
void TCamera::writeToStream( CStream &out, int *version ) const
{
	if( version )
		*version = 2;
	else
	{
		out << focalLengthMeters;
		for(unsigned int k = 0; k < 5; k++) out << dist[k];
		out << intrinsicParams;
		// version 0 did serialize here a "CMatrixDouble15"
		out << nrows << ncols; // New in v2
	} // end else
}

// ReadFromStream
void TCamera::readFromStream( CStream &in, int version )
{
	switch( version )
	{
	case 0:
	case 1:
	case 2:
		{
			in >> focalLengthMeters;

			for(unsigned int k = 0; k < 5; k++)
				in >> dist[k];

			in >> intrinsicParams;

			if (version==0)
			{
				CMatrixDouble15 __distortionParams;
				in >> __distortionParams;
			}

			if (version>=2)
				in >> nrows >> ncols;
			else {
				nrows = 480;
				ncols = 640;
			}


		} break;
	default:
		MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION( version )
	}
}


/**  Save as a config block:
  *  \code
  *  [SECTION]
  *  resolution = NCOLS NROWS
  *  cx         = CX
  *  cy         = CY
  *  fx         = FX
  *  fy         = FY
  *  dist       = K1 K2 T1 T2 T3
  *  focal_length = FOCAL_LENGTH
  *  \endcode
  */
void TCamera::saveToConfigFile(const std::string &section,  mrpt::utils::CConfigFileBase &cfg ) const
{
	cfg.write(section,"resolution",format("[%u %u]",(unsigned int)ncols,(unsigned int)nrows));
	cfg.write(section,"cx", format("%.05f",cx()) );
	cfg.write(section,"cy", format("%.05f",cy()) );
	cfg.write(section,"fx", format("%.05f",fx()) );
	cfg.write(section,"fy", format("%.05f",fy()) );
	cfg.write(section,"dist", format("[%e %e %e %e %e]", dist[0],dist[1],dist[2],dist[3],dist[4] ) );
	if (focalLengthMeters!=0) cfg.write(section,"focal_length", focalLengthMeters );
}

/**  Load all the params from a config source, in the format described in saveToConfigFile()
  */
void TCamera::loadFromConfigFile(const std::string &section,  const mrpt::utils::CConfigFileBase &cfg )
{
	vector<uint64_t>  out_res;
	cfg.read_vector(section,"resolution",vector<uint64_t>(),out_res,true);
	if (out_res.size()!=2) THROW_EXCEPTION("Expected 2-length vector in field 'resolution'");
	ncols = out_res[0];
	nrows = out_res[1];

	double fx, fy, cx, cy;
    fx = cfg.read_double(section,"fx",0, true);
    fy = cfg.read_double(section,"fy",0, true);
    cx = cfg.read_double(section,"cx",0, true);
    cy = cfg.read_double(section,"cy",0, true);

    if( fx < 2.0 ) fx *= ncols;
    if( fy < 2.0 ) fy *= nrows;
    if( cx < 2.0 ) cx *= ncols;
    if( cy < 2.0 ) cy *= nrows;

	setIntrinsicParamsFromValues( fx, fy, cx, cy );

	vector_double dists;
	cfg.read_vector(section,"dist",vector_double(), dists, true);
	if (dists.size()!=4 && dists.size()!=5) THROW_EXCEPTION("Expected 4 or 5-length vector in field 'dist'");

	dist.Constant(0);
	for (vector_double::Index i=0;i<dists.size();i++)
		dist[i] = dists[i];

	focalLengthMeters = cfg.read_double(section,"focal_length",0, false /* optional value */ );

}

/** Rescale all the parameters for a new camera resolution (it raises an exception if the aspect ratio is modified, which is not permitted).
  */
void TCamera::scaleToResolution(unsigned int new_ncols, unsigned int new_nrows)
{
	if (ncols == new_ncols && nrows == new_nrows)
		return; // already done

	ASSERT_(new_nrows>0 && new_ncols>0)

	const double prev_aspect_ratio = ncols/double(nrows);
	const double new_aspect_ratio  = new_ncols/double(new_nrows);

	ASSERTMSG_(std::abs(prev_aspect_ratio-new_aspect_ratio)<1e-3, "TCamera: Trying to scale camera parameters for a resolution of different aspect ratio." )

	const double K = new_ncols / double(ncols);

	ncols = new_ncols;
	nrows = new_nrows;

	// fx fy cx cy
	intrinsicParams(0,0)*=K;
	intrinsicParams(1,1)*=K;
	intrinsicParams(0,2)*=K;
	intrinsicParams(1,2)*=K;

	// distortion params: unmodified.
}

bool mrpt::utils::operator ==(const mrpt::utils::TCamera& a, const mrpt::utils::TCamera& b)
{
	return 
		a.ncols==b.ncols &&
		a.nrows==b.nrows && 
		a.intrinsicParams==b.intrinsicParams && 
		a.dist==b.dist && 
		a.focalLengthMeters==b.focalLengthMeters;
}
bool mrpt::utils::operator !=(const mrpt::utils::TCamera& a, const mrpt::utils::TCamera& b)
{
	return !(a==b);
}
