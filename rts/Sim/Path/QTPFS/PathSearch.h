/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef QTPFS_PATHSEARCH_HDR
#define QTPFS_PATHSEARCH_HDR

#include <queue>
#include <vector>

#include "PathDefines.h"
#include "Node.h"
#include "NodeHeap.h"
#include "PathThreads.h"

#include "System/float3.h"

namespace QTPFS {
	struct PathCache;
	struct NodeLayer;
	struct IPath;

	namespace PathSearchTrace {
		struct Iteration {
			Iteration() { nodeIndices.push_back(-1u); }
			Iteration(const Iteration& i) { *this = i; }
			Iteration(Iteration&& i) { *this = std::move(i); }

			Iteration& operator = (const Iteration& i) { nodeIndices = i.nodeIndices; return *this; }
			Iteration& operator = (Iteration&& i) { nodeIndices = std::move(i.nodeIndices); return *this; }

			void Clear() {
				nodeIndices.clear();
				nodeIndices.push_back(-1u);
			}
			void SetPoppedNodeIdx(unsigned int i) { (nodeIndices.front()) = i; }
			void AddPushedNodeIdx(unsigned int i) { (nodeIndices.push_back(i)); }

			const std::vector<unsigned int>& GetNodeIndices() const { return nodeIndices; }

		private:
			// NOTE: indices are only valid so long as tree is not re-tesselated
			std::vector<unsigned int> nodeIndices;
		};

		struct Execution {
			Execution(unsigned int f): searchFrame(f) {}
			Execution(const Execution& e) = delete;
			Execution(Execution&& e) { *this = std::move(e); }

			Execution& operator = (const Execution& e) = delete;
			Execution& operator = (Execution&& e) {
				searchFrame = e.GetFrame();
				iterations = std::move(e.iterations);
				return *this;
			}

			void AddIteration(const Iteration& iter) { iterations.push_back(iter); }
			const std::vector<Iteration>& GetIterations() const { return iterations; }

			unsigned int GetFrame() const { return searchFrame; }

		private:
			std::vector<Iteration> iterations;

			// sim-frame at which the search was executed
			unsigned int searchFrame;
		};
	}

	struct PathSearch {
		void SetID(unsigned int n) { searchID = n; }
		void SetTeam(unsigned int n) { searchTeam = n; }
		unsigned int GetID() const { return searchID; }
		unsigned int GetTeam() const { return searchTeam; }

	protected:
		unsigned int searchID;     // links us to the temp-path that this search will finalize
		unsigned int searchTeam;   // which team queued this search

		unsigned int searchType;   // indicates if Dijkstra (h==0) or A* (h!=0) search is employed
		unsigned int searchState;  // offset that identifies nodes as part of current search
		unsigned int searchMagic;  // used to signal nodes they should update their neighbor-set

	public:
		PathSearch()
			: nodeLayer(NULL)
			, pathCache(NULL)
			, searchExec(NULL)
			, hCostMult(0.0f)
			, haveFullPath(false)
			, havePartPath(false)
			, openNodes(nullptr)
			, searchID(0)
			, searchTeam(0)
			, searchType(0)
			, searchState(0)
			, searchMagic(0)
			{}
		PathSearch(unsigned int pathSearchType)
			: PathSearch()
			{ searchType = pathSearchType; }
		~PathSearch() { /* openNodes->reset(); */ }

		void Initialize(
			NodeLayer* layer,
			PathCache* cache,
			const float3& sourcePoint,
			const float3& targetPoint,
			const SRectangle& searchArea,
			SearchThreadData* threadData
		);
		bool Execute(
			unsigned int searchStateOffset = 0,
			unsigned int searchMagicNumber = 0
		);
		void Finalize(IPath* path);
		bool SharedFinalize(const IPath* srcPath, IPath* dstPath);
		PathSearchTrace::Execution* GetExecutionTrace() { return searchExec; }

		const std::uint64_t GetHash(std::uint64_t N, std::uint32_t k) const;

		bool PathWasFound() const { return haveFullPath | havePartPath; }

		void SetPathType(int newPathType) { pathType = newPathType; }
		int GetPathType() const { return pathType; }

	private:
		void ResetState(SearchNode* node);
		void UpdateNode(SearchNode* nextNode, SearchNode* prevNode, unsigned int netPointIdx);

		void IterateNodes();
		void IterateNodeNeighbors(const std::vector<INode*>& nxtNodes);

		void TracePath(IPath* path);
		void SmoothPath(IPath* path) const;
		bool SmoothPathIter(IPath* path) const;

		// global queue: allocated once, re-used by all searches without clear()'s
		// this relies on INode::operator< to sort the INode*'s by increasing f-cost
		// static binary_heap<INode*> openNodes;

		QTPFS::SearchThreadData* searchThreadData;
		SearchPriorityQueue* openNodes;

		NodeLayer* nodeLayer;
		int pathType;
		PathCache* pathCache;

		// not used unless QTPFS_TRACE_PATH_SEARCHES is defined
		PathSearchTrace::Execution* searchExec;
		PathSearchTrace::Iteration searchIter;

		SRectangle searchRect;

		SearchNode *srcSearchNode, *tgtSearchNode;
		SearchNode *curSearchNode, *nextSearchNode;
		SearchNode *minSearchNode;

		float3 srcPoint;
		float3 tgtPoint;

		float2 netPoints[QTPFS_MAX_NETPOINTS_PER_NODE_EDGE];

		float gDists[QTPFS_MAX_NETPOINTS_PER_NODE_EDGE];
		float hDists[QTPFS_MAX_NETPOINTS_PER_NODE_EDGE];
		float gCosts[QTPFS_MAX_NETPOINTS_PER_NODE_EDGE];
		float hCosts[QTPFS_MAX_NETPOINTS_PER_NODE_EDGE];

		float hCostMult;

		bool haveFullPath;
		bool havePartPath;
	};
}

#endif
