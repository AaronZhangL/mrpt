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

#ifndef mrpt_vision_descriptor_kdtrees_H
#define mrpt_vision_descriptor_kdtrees_H

#include <mrpt/vision/types.h>
#include <mrpt/vision/CFeature.h>

namespace mrpt
{
	namespace vision
	{
		namespace detail {
			// Private auxiliary classes
			template <typename distance_t,typename element_t = uint8_t> struct TSIFTDesc2KDTree_Adaptor;
			template <typename distance_t,typename element_t = float> struct TSURFDesc2KDTree_Adaptor;
		}

		/** \defgroup mrptvision_descr_kdtrees KD-Tree construction of visual descriptors
		  * \ingroup mrpt_vision_grp
		  */

		/** \addtogroup  mrptvision_descr_kdtrees
		    @{ */

		/** A kd-tree builder for sets of features with SIFT descriptors.
		  *   Example of usage:
		  *  \code
		  *    TSIFTDescriptorsKDTreeIndex<double>  feats_kdtree(feats);
		  *    feats_kdtree.get_kdtree().knnSearch( ... );
		  *  \endcode
		  * \sa CFeatureList, mrpt::vision::find_descriptor_pairings
		  */
		template <
			typename distance_t, 
			class metric_t = nanoflann::L2_Simple_Adaptor<uint8_t/*SIFT desc elements*/,detail::TSIFTDesc2KDTree_Adaptor<distance_t>, distance_t> 
			>
		struct TSIFTDescriptorsKDTreeIndex
		{
		public:
			typedef typename nanoflann::KDTreeSingleIndexAdaptor<metric_t,detail::TSIFTDesc2KDTree_Adaptor<distance_t> > kdtree_t;

			/** Constructor from a list of SIFT features. 
			  *  Automatically build the KD-tree index. The list of features must NOT be empty or an exception will be raised.
			  */
			TSIFTDescriptorsKDTreeIndex(const CFeatureList &feats) : 
				m_adaptor(feats),
				m_kdtree(NULL),
				m_feats(feats) 
			{
				ASSERT_(!feats.empty() && feats[0]->descriptors.hasDescriptorSIFT())
				this->regenerate_kdtreee();
			}

			/** Re-creates the kd-tree, which must be done whenever the data source (the CFeatureList) changes. */
			void regenerate_kdtreee() 
			{		
				if (m_kdtree) delete m_kdtree;

				nanoflann::KDTreeSingleIndexAdaptorParams params;
				m_kdtree = new kdtree_t( m_feats[0]->descriptors.SIFT.size() /* DIM */ , m_adaptor, params );
				m_kdtree->buildIndex();
			}

			/** Access to the kd-tree object */
				 kdtree_t & get_kdtree()       { return *m_kdtree; }
			const kdtree_t & get_kdtree() const { return *m_kdtree; }

			~TSIFTDescriptorsKDTreeIndex() 
			{
				delete m_kdtree;
				m_kdtree=NULL;
			}

		private:
			detail::TSIFTDesc2KDTree_Adaptor<distance_t>  m_adaptor;
			kdtree_t *m_kdtree;

			const CFeatureList & m_feats;
		}; // end of TSIFTDescriptorsKDTreeIndex


		/** A kd-tree builder for sets of features with SURF descriptors.
		  *   Example of usage:
		  *  \code
		  *    TSURFDescriptorsKDTreeIndex<double>  feats_kdtree(feats);
		  *    feats_kdtree.get_kdtree().knnSearch( ... );
		  *  \endcode
		  * \sa CFeatureList, mrpt::vision::find_descriptor_pairings
		  */
		template <
			typename distance_t, 
			class metric_t = nanoflann::L2_Simple_Adaptor<float/*SURF desc elements*/,detail::TSURFDesc2KDTree_Adaptor<distance_t>, distance_t> 
			>
		struct TSURFDescriptorsKDTreeIndex
		{
		public:
			typedef typename nanoflann::KDTreeSingleIndexAdaptor<metric_t,detail::TSURFDesc2KDTree_Adaptor<distance_t> > kdtree_t;

			/** Constructor from a list of SIFT features. 
			  *  Automatically build the KD-tree index. The list of features must NOT be empty or an exception will be raised.
			  */
			TSURFDescriptorsKDTreeIndex(const CFeatureList &feats) : 
				m_adaptor(feats),
				m_kdtree(NULL),
				m_feats(feats) 
			{
				ASSERT_(!feats.empty() && feats[0]->descriptors.hasDescriptorSIFT())
				this->regenerate_kdtreee();
			}

			/** Re-creates the kd-tree, which must be done whenever the data source (the CFeatureList) changes. */
			void regenerate_kdtreee() 
			{		
				if (m_kdtree) delete m_kdtree;

				nanoflann::KDTreeSingleIndexAdaptorParams params;
				m_kdtree = new kdtree_t( m_feats[0]->descriptors.SIFT.size() /* DIM */ , m_adaptor, params );
				m_kdtree->buildIndex();
			}

			/** Access to the kd-tree object */
				 kdtree_t & get_kdtree()       { return *m_kdtree; }
			const kdtree_t & get_kdtree() const { return *m_kdtree; }

			~TSURFDescriptorsKDTreeIndex() 
			{
				delete m_kdtree;
				m_kdtree=NULL;
			}

		private:
			detail::TSURFDesc2KDTree_Adaptor<distance_t>  m_adaptor;
			kdtree_t *m_kdtree;

			const CFeatureList & m_feats;
		}; // end of TSURFDescriptorsKDTreeIndex

		/** @} */

		namespace detail 
		{
			template <typename distance_t,typename element_t>
			struct TSIFTDesc2KDTree_Adaptor
			{
				const CFeatureList & m_feats;
				TSIFTDesc2KDTree_Adaptor(const CFeatureList &feats) : m_feats(feats) { }
				// Must return the number of data points
				inline size_t kdtree_get_point_count() const { return m_feats.size(); }
				// Must return the Euclidean (L2) distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
				inline distance_t kdtree_distance(const element_t *p1, const size_t idx_p2,size_t size) const 
				{ 
					const size_t dim=m_feats[idx_p2]->descriptors.SIFT.size();
					const element_t *p2 = &m_feats[idx_p2]->descriptors.SIFT[0];
					distance_t  d=0;
					for (size_t i=0;i<dim;i++) 
					{
						d+=(*p1-*p2)*(*p1-*p2);
						p1++;
						p2++;
					}
					return d;
				}
				// Must return the dim'th component of the idx'th point in the class:
				inline element_t kdtree_get_pt(const size_t idx, int dim) const { return m_feats[idx]->descriptors.SIFT[dim]; }
				template <class BBOX> bool kdtree_get_bbox(BBOX &bb) const { return false; }
			};

			template <typename distance_t,typename element_t>
			struct TSURFDesc2KDTree_Adaptor
			{
				const CFeatureList & m_feats;
				TSURFDesc2KDTree_Adaptor(const CFeatureList &feats) : m_feats(feats) { }
				// Must return the number of data points
				inline size_t kdtree_get_point_count() const { return m_feats.size(); }
				// Must return the Euclidean (L2) distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
				inline distance_t kdtree_distance(const element_t *p1, const size_t idx_p2,size_t size) const 
				{ 
					const size_t dim=m_feats[idx_p2]->descriptors.SURF.size();
					const element_t *p2 = &m_feats[idx_p2]->descriptors.SURF[0];
					distance_t  d=0;
					for (size_t i=0;i<dim;i++) 
					{
						d+=(*p1-*p2)*(*p1-*p2);
						p1++;
						p2++;
					}
					return d;
				}
				// Must return the dim'th component of the idx'th point in the class:
				inline element_t kdtree_get_pt(const size_t idx, int dim) const { return m_feats[idx]->descriptors.SURF[dim]; }
				template <class BBOX> bool kdtree_get_bbox(BBOX &bb) const { return false; }
			};
		} // end detail
	}
}
#endif

