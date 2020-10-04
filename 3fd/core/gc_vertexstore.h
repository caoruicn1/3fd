#ifndef GC_VERTEXSTORE_H // header guard
#define GC_VERTEXSTORE_H

#include <3fd/core/gc_common.h>
#include <3fd/core/gc_vertex.h>
#include <3fd/utils/memory.h>
#include <btree/stx/btree_set.h>

namespace _3fd
{
namespace memory
{
    /// <summary>
    /// Represents a store of vertices allocated from a pool.
    /// The vertices represent memory blocks.
    /// </summary>
    class VertexStore
    {
    private:

        utils::DynamicMemPool m_memBlocksPool;

        // Compared to a binary tree, a B+Tree can render better cache efficiency
        typedef stx::btree_set<MemAddrContainer *, LessOperOnVertexRepAddr> SetOfMemBlocks;

        /// <summary>
        /// A sorted set of garbage collected pieces of memory,
        /// ordered by the memory addresses of those pieces.
        /// </summary>
        /// <remarks>
        /// Although a hash table could be faster, it is not sorted, hence cannot be used.
        /// </remarks>
        SetOfMemBlocks m_vertices;

    public:

        VertexStore();

		VertexStore(const VertexStore &) = delete;

        void ShrinkPool();

        void AddVertex(void *memAddr, size_t blockSize, FreeMemProc freeMemCallback);

        void RemoveVertex(Vertex *memBlock);

        Vertex *GetVertex(void *memAddr) const;

        Vertex *GetContainerVertex(void *addr) const;
    };

}// end of namespace memory
}// end of namespace _3fd

#endif // end of header guard
