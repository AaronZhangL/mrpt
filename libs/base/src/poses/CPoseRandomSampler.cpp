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


#include <mrpt/poses/CPoseRandomSampler.h>
#include <mrpt/poses/CPosePDFGaussian.h>
#include <mrpt/poses/CPosePDFParticles.h>
#include <mrpt/poses/CPosePDFSOG.h>
#include <mrpt/poses/CPose3DPDFGaussian.h>
#include <mrpt/poses/CPose3DPDFParticles.h>
#include <mrpt/poses/CPose3DPDFSOG.h>

#include <mrpt/random.h>
#include <mrpt/math/utils.h>

using namespace mrpt;
using namespace mrpt::poses;
using namespace mrpt::utils;
using namespace mrpt::random;


/*---------------------------------------------------------------
        Constructor
  ---------------------------------------------------------------*/
CPoseRandomSampler::CPoseRandomSampler() :
    m_pdf2D(NULL),
    m_pdf3D(NULL),
    m_fastdraw_gauss_Z3(),
	m_fastdraw_gauss_Z6(),
    m_fastdraw_gauss_M_2D(),
    m_fastdraw_gauss_M_3D()
{
}

/*---------------------------------------------------------------
            Destructor
  ---------------------------------------------------------------*/
CPoseRandomSampler::~CPoseRandomSampler()
{
    clear();
}


/*---------------------------------------------------------------
            clear
  ---------------------------------------------------------------*/
void CPoseRandomSampler::clear()
{
	mrpt::utils::delete_safe( m_pdf2D );
	mrpt::utils::delete_safe( m_pdf3D );
}

/*---------------------------------------------------------------
                        setPosePDF
  ---------------------------------------------------------------*/
void CPoseRandomSampler::setPosePDF( const CPosePDF *pdf )
{
    MRPT_START

    clear();
    m_pdf2D = static_cast<CPosePDF*>( pdf->duplicate() );

    // According to the PDF type:
    if ( IS_CLASS(m_pdf2D,CPosePDFGaussian ) )
    {
		const CPosePDFGaussian* gPdf = static_cast<const CPosePDFGaussian*>(pdf);
		const CMatrixDouble33 &cov = gPdf->cov;

		m_fastdraw_gauss_M_2D = gPdf->mean;

		/** Computes the eigenvalues/eigenvector decomposition of this matrix,
		*    so that: M = Z · D · Z<sup>T</sup>, where columns in Z are the
		*	  eigenvectors and the diagonal matrix D contains the eigenvalues
		*    as diagonal elements, sorted in <i>ascending</i> order.
		*/
		CMatrixDouble33 D;
		cov.eigenVectors( m_fastdraw_gauss_Z3, D );

		// Scale eigenvectors with eigenvalues:
		D.Sqrt();
		m_fastdraw_gauss_Z3.multiply( m_fastdraw_gauss_Z3, D);
    }
    else
    if ( IS_CLASS(m_pdf2D,CPosePDFParticles ) )
    {
    	return; // Nothing to prepare.
    }
    else
    {
        THROW_EXCEPTION_CUSTOM_MSG1("Unsuported class: %s", m_pdf2D->GetRuntimeClass()->className );
    }


    MRPT_END
}


/*---------------------------------------------------------------
                        setPosePDF
  ---------------------------------------------------------------*/
void CPoseRandomSampler::setPosePDF( const CPose3DPDF *pdf )
{
    MRPT_START

    clear();
    m_pdf3D = static_cast<CPose3DPDF*>( pdf->duplicate() );

    // According to the PDF type:
    if ( IS_CLASS(m_pdf3D,CPose3DPDFGaussian ) )
    {
		const CPose3DPDFGaussian* gPdf = static_cast<const CPose3DPDFGaussian*>(pdf);
		const CMatrixDouble66 &cov = gPdf->cov;

		m_fastdraw_gauss_M_3D = gPdf->mean;

		/** Computes the eigenvalues/eigenvector decomposition of this matrix,
		*    so that: M = Z · D · Z<sup>T</sup>, where columns in Z are the
		*	  eigenvectors and the diagonal matrix D contains the eigenvalues
		*    as diagonal elements, sorted in <i>ascending</i> order.
		*/
		CMatrixDouble66 D;
		cov.eigenVectors( m_fastdraw_gauss_Z6, D );

		// Scale eigenvectors with eigenvalues:
		D.Sqrt();
		m_fastdraw_gauss_Z6.multiply( m_fastdraw_gauss_Z6, D);
    }
    else
    if ( IS_CLASS(m_pdf3D,CPose3DPDFParticles ) )
    {
    	return; // Nothing to prepare.
    }
    else
    {
        THROW_EXCEPTION_CUSTOM_MSG1("Unsoported class: %s", m_pdf3D->GetRuntimeClass()->className );
    }

    MRPT_END
}


/*---------------------------------------------------------------
                    drawSample
  ---------------------------------------------------------------*/
CPose2D & CPoseRandomSampler::drawSample( CPose2D &p ) const
{
    MRPT_START

	if (m_pdf2D)
	{
		do_sample_2D(p);
	}
	else if (m_pdf3D)
	{
		CPose3D  q;
		do_sample_3D(q);
		p.x(q.x());
		p.y(q.y());
		p.phi(q.yaw());
	}
	else THROW_EXCEPTION("No associated pdf: setPosePDF must be called first.");

	return p;
    MRPT_END
}

/*---------------------------------------------------------------
                    drawSample
  ---------------------------------------------------------------*/
CPose3D & CPoseRandomSampler::drawSample( CPose3D &p ) const
{
    MRPT_START

	if (m_pdf2D)
	{
		CPose2D q;
		do_sample_2D(q);
		p.setFromValues(q.x(),q.y(),0,q.phi(),0,0);
	}
	else if (m_pdf3D)
	{
		do_sample_3D(p);
	}
	else THROW_EXCEPTION("No associated pdf: setPosePDF must be called first.");

	return p;
    MRPT_END
}


/*---------------------------------------------------------------
                  do_sample_2D: Sample from a 2D PDF
  ---------------------------------------------------------------*/
void CPoseRandomSampler::do_sample_2D( CPose2D &p ) const
{
	MRPT_START
	ASSERT_(m_pdf2D);

	// According to the PDF type:
	if ( IS_CLASS(m_pdf2D,CPosePDFGaussian ) )
	{
		// ------------------------------
		//      A single gaussian:
		// ------------------------------
		vector_double	rndVector(3,0);
		for (size_t i=0;i<3;i++)
		{
			double	rnd = randomGenerator.drawGaussian1D_normalized();
			for (size_t d=0;d<3;d++)
				rndVector[d]+= ( m_fastdraw_gauss_Z3.get_unsafe(d,i)*rnd );
		}

		p.x( m_fastdraw_gauss_M_2D.x() + rndVector[0] );
		p.y( m_fastdraw_gauss_M_2D.y() + rndVector[1] );
		p.phi( m_fastdraw_gauss_M_2D.phi() + rndVector[2] );
		p.normalizePhi();
	}
	else
	if ( IS_CLASS(m_pdf2D,CPosePDFSOG ) )
	{
		// -------------------------------------
		//      			SOG
		// -------------------------------------
		THROW_EXCEPTION("TODO");
	}
	else
	if ( IS_CLASS(m_pdf2D,CPosePDFParticles ) )
	{
		// -------------------------------------
		//      Particles: just sample as usual
		// -------------------------------------
		const CPosePDFParticles* pdf = static_cast<const CPosePDFParticles*>(m_pdf2D);
		pdf->drawSingleSample(p);
	}
	else
		THROW_EXCEPTION_CUSTOM_MSG1("Unsoported class: %s", m_pdf2D->GetRuntimeClass()->className );

	MRPT_END
}

/*---------------------------------------------------------------
                  do_sample_3D: Sample from a 3D PDF
  ---------------------------------------------------------------*/
void CPoseRandomSampler::do_sample_3D( CPose3D &p ) const
{
	MRPT_START
	ASSERT_(m_pdf3D);

	// According to the PDF type:
	if ( IS_CLASS(m_pdf3D,CPose3DPDFGaussian ) )
	{
		// ------------------------------
		//      A single gaussian:
		// ------------------------------
		vector_double	rndVector(6,0);
		for (size_t i=0;i<6;i++)
		{
			double	rnd = randomGenerator.drawGaussian1D_normalized();
			for (size_t d=0;d<6;d++)
				rndVector[d]+= ( m_fastdraw_gauss_Z6.get_unsafe(d,i)*rnd );
		}

		p.setFromValues(
			m_fastdraw_gauss_M_3D.x()    + rndVector[0],
			m_fastdraw_gauss_M_3D.y()    + rndVector[1],
			m_fastdraw_gauss_M_3D.z()    + rndVector[2],
			m_fastdraw_gauss_M_3D.yaw()  + rndVector[3],
			m_fastdraw_gauss_M_3D.pitch()+ rndVector[4],
			m_fastdraw_gauss_M_3D.roll() + rndVector[5] );
	}
	else
	if ( IS_CLASS(m_pdf3D,CPose3DPDFSOG ) )
	{
		// -------------------------------------
		//      			SOG
		// -------------------------------------
		THROW_EXCEPTION("TODO");
	}
	else
	if ( IS_CLASS(m_pdf3D,CPose3DPDFParticles ) )
	{
		// -------------------------------------
		//      Particles: just sample as usual
		// -------------------------------------
		const CPose3DPDFParticles* pdf = static_cast<const CPose3DPDFParticles*>(m_pdf3D);
		pdf->drawSingleSample(p);
	}
	else
		THROW_EXCEPTION_CUSTOM_MSG1("Unsoported class: %s", m_pdf3D->GetRuntimeClass()->className );

	MRPT_END
}

/*---------------------------------------------------------------
                  isPrepared
  ---------------------------------------------------------------*/
bool CPoseRandomSampler::isPrepared() const
{
	return m_pdf2D || m_pdf3D;
}

/*---------------------------------------------------------------
                  getOriginalPDFCov2D
  ---------------------------------------------------------------*/
void CPoseRandomSampler::getOriginalPDFCov2D( CMatrixDouble33 &cov3x3 ) const
{
	MRPT_START
	ASSERT_(this->isPrepared())

	if (m_pdf2D)
	{
		m_pdf2D->getCovariance( cov3x3 );
	}
	else
	{
		ASSERT_(m_pdf3D);

		CPosePDFGaussian	P;
		P.copyFrom(*m_pdf3D);
		cov3x3 = P.cov;
	}

	MRPT_END
}

/*---------------------------------------------------------------
                  getOriginalPDFCov3D
  ---------------------------------------------------------------*/
void CPoseRandomSampler::getOriginalPDFCov3D( CMatrixDouble66 &cov6x6 ) const
{
	MRPT_START
	ASSERT_(this->isPrepared())

	if (m_pdf2D)
	{
		CPose3DPDFGaussian P;
		P.copyFrom(*m_pdf2D);
		cov6x6 = P.cov;
	}
	else
	{
		ASSERT_(m_pdf3D);
		m_pdf3D->getCovariance( cov6x6 );
	}

	MRPT_END
}

/*---------------------------------------------------------------
                  getSamplingMean2D
  ---------------------------------------------------------------*/
CPose2D &CPoseRandomSampler::getSamplingMean2D(CPose2D &out_mean) const
{
	MRPT_START
	ASSERT_(this->isPrepared())

	if (m_pdf2D)
			out_mean = m_fastdraw_gauss_M_2D;
	else	out_mean = CPose2D( m_fastdraw_gauss_M_3D );

	return out_mean;
	MRPT_END
}

/*---------------------------------------------------------------
                  getSamplingMean3D
  ---------------------------------------------------------------*/
CPose3D &CPoseRandomSampler::getSamplingMean3D(CPose3D &out_mean) const
{
	MRPT_START
	ASSERT_(this->isPrepared())

	if (m_pdf3D)
			out_mean = m_fastdraw_gauss_M_3D;
	else	out_mean = CPose3D( m_fastdraw_gauss_M_2D );

	return out_mean;
	MRPT_END
}

