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
#ifndef CGRAPHPARTITIONER_H
#define CGRAPHPARTITIONER_H

#include <mrpt/utils/utils_defs.h>
#include <mrpt/utils/CDebugOutputCapable.h>
#include <mrpt/math/CMatrix.h>
#include <mrpt/math/ops_matrices.h>

#include <mrpt/graphs/link_pragmas.h>

namespace mrpt
{
	/** Abstract graph and tree data structures, plus generic graph algorithms
	  * \ingroup  mrpt_graphs_grp
	  */
	namespace graphs
	{
		using namespace mrpt::math;

		/** Algorithms for finding the min-normalized-cut of a weighted undirected graph.
		 *    Two static methods are provided, one for bisection and the other for
		 *      iterative N-parts partition.
		 *  It is based on the Shi-Malik method, proposed for example in:<br><br>
		 *  <code>J. Shi and J. Malik, "Normalized Cuts and Image Segmentation,"IEEE Transactions on Pattern Analysis and Machine Intelligence, vol.22, no.8, pp. 888-905, Aug. 2000.</code><br>
		 *
		 */
		class GRAPHS_IMPEXP CGraphPartitioner : public mrpt::utils::CDebugOutputCapable
		{
		public:
			/** Performs the spectral recursive partition into K-parts for a given graph.
			 *   The default threshold for the N-cut is 1, which correspond to a cut equal
			 *   of the geometric mean of self-associations of each pair of groups.
			 *
			 * \param in_A			 [IN] The weights matrix for the graph. It must be a square matrix, where element W<sub>ij</sub> is the "likelihood" between nodes "i" and "j", and typically W<sub>ii</sub> = 1.
			 * \param out_parts		 [OUT] An array of partitions, where each partition is represented as a vector of indexs for nodes.
			 * \param threshold_Ncut [IN] If it is desired to use other than the default threshold, it can be passed here.
			 * \param forceSimetry	 [IN] If set to true (default) the elements W<sub>ij</sub> and W<sub>ji</sub> are replaced by 0.5·(W<sub>ij</sub>+W<sub>ji</sub>). Set to false if matrix is known to be simetric.
			 * \param useSpectralBisection [IN] If set to true (default) a quick spectral bisection will be used. If set to false, a brute force, exact finding of the min-cut is performed.
			 * \param recursive [IN] Default=true, recursive algorithm for finding N partitions. Set to false to force 1 bisection as maximum.
			 * \param minSizeClusters [IN] Default=1, Minimum size of partitions to be accepted.
			 *
			 * \sa CMatrix, SpectralBisection
			 *
			 * \exception Throws a std::logic_error if an invalid matrix is passed.
			 */
			static void  RecursiveSpectralPartition(
			  CMatrix					&in_A,
			  std::vector<vector_uint>	&out_parts,
			  float						threshold_Ncut = 1.0f,
			  bool						forceSimetry = true,
			  bool						useSpectralBisection = true,
			  bool						recursive = true,
			  unsigned					minSizeClusters = 1);

			/** Performs the spectral bisection of a graph. This method always perform
			 *   the bisection, and a measure of the goodness for this cut is returned.
			 *
			 * \param in_A			[IN] The weights matrix for the graph. It must be a square matrix, where element W<sub>ij</sub> is the "likelihood" between nodes "i" and "j", and typically W<sub>ii</sub> = 1.
			 * \param out_part1		[OUT] The indexs of the nodes that fall into the first group.
			 * \param out_part2		[OUT] The indexs of the nodes that fall into the second group.
			 * \param out_cut_value	[OUT] The N-cut value for the proposed cut, in the range [0-2].
			 * \param forceSimetry	[IN] If set to true (default) the elements W<sub>ij</sub> and W<sub>ji</sub> are replaced by 0.5·(W<sub>ij</sub>+W<sub>ji</sub>). Set to false if matrix is known to be simetric.
			 *
			 * \sa CMatrix, RecursiveSpectralPartition
			 *
			 * \exception Throws a std::logic_error if an invalid matrix is passed.
			 */
			static void  SpectralBisection(
								CMatrix					&in_A,
								vector_uint				&out_part1,
								vector_uint				&out_part2,
								float					&out_cut_value,
								bool					forceSimetry = true );

			/** Performs an EXACT minimum n-Cut graph bisection, (Use CGraphPartitioner::SpectralBisection for a faster algorithm)
			 *
			 * \param in_A			[IN] The weights matrix for the graph. It must be a square matrix, where element W<sub>ij</sub> is the "likelihood" between nodes "i" and "j", and typically W<sub>ii</sub> = 1.
			 * \param out_part1		[OUT] The indexs of the nodes that fall into the first group.
			 * \param out_part2		[OUT] The indexs of the nodes that fall into the second group.
			 * \param out_cut_value	[OUT] The N-cut value for the proposed cut, in the range [0-2].
			 * \param forceSimetry	[IN] If set to true (default) the elements W<sub>ij</sub> and W<sub>ji</sub> are replaced by 0.5·(W<sub>ij</sub>+W<sub>ji</sub>). Set to false if matrix is known to be simetric.
			 *
			 * \sa CMatrix, RecursiveSpectralPartition
			 *
			 * \exception Throws a std::logic_error if an invalid matrix is passed.
			 */
			static void  exactBisection(
								CMatrix			&in_A,
								vector_uint		&out_part1,
								vector_uint		&out_part2,
								float			&out_cut_value,
								bool			forceSimetry = true );

			/** Returns the normaliced cut of a graph, given its adjacency matrix A and a bisection:
			  */
			static float  nCut(
								const CMatrix			&in_A,
								const vector_uint		&in_part1,
								const vector_uint		&in_part2 );


				/** If set to true (default=false), each eigenvector computed (and the laplacian of the adj. matrix) will be saved to files "DEBUG_GRAPHPART_eigvectors_xxx" and "DEBUG_GRAPHPART_laplacian_xxx", respectively.
				  */
				static bool DEBUG_SAVE_EIGENVECTOR_FILES;

				/** If set to true (default=false), debug info will be displayed to cout.
				  */
				static bool VERBOSE;

			private:
				/** Used internally when DEBUG_SAVE_EIGENVECTOR_FILES=true
				  */
				static int debug_file_no;

		}; // End of class def.

	} // End of namespace
} // End of namespace
#endif
