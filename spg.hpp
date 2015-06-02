#pragma once
#include <cmath>
#include <iostream>
#include <cassert>
#include <stack>

template <typename T>
struct Node
{
        T       Key;
        Node*   Left;
        Node*   Right;

        /// Returns the size of the subtree formed by the current node.
        std::size_t Size() const;

        /// "Pretty print" the current subtree.
        void PrettyPrint(std::size_t p_Depth);
};

/// ScapeGoat tree implementation from the paper ScapeGoat Tree
/// of Igal Galperin and Ronald L. Rivest. The partial insertion
/// method is the recursive one. It flattens the subtree in order
/// to rebuild it.
template <typename T,
          typename Alloc = std::allocator<T>,
          typename PtrAlloc = std::allocator<Node<T>*>>
class SPG
{
    using NodeAllocator = typename Alloc::template rebind<Node<T>>::other;

    using node_type = Node<T>;
    using link_type = node_type*;
    using allocator_type = Alloc;
    using value_type = T;

    public:
        /// Constructs a space goat tree.
        /// @p_Alpha : unbalance factor of the tree, MUST be in the interval [0.5, 1.0].
        SPG(float p_Alpha);

        /// We delete the pointers when we destroy our data structure.
        ~SPG();

        /// Returns the size of the tree.
        std::size_t Size() const;

        /// Returns the node with the given key in the tree.
        /// @p_Key : The key we look for.
        link_type find(value_type const& p_Key);

        /// Insert a new node in the tree with the corresponding given key.
        /// It will rebalance the tree if needed according to the unbalance factor.
        /// @p_Key : The key to insert.
        /// Returns true if the key was inserted, false otherwise.
        bool insert(value_type const& p_Key);

        /// Print the tree on the cout.
        void PrettyPrint() const;

    protected:

        ////////////////////////
        /// Allocator related.
        ////////////////////////

        /// Returns the node allocator.
        inline NodeAllocator& GetNodeAllocator()
        {
            return *static_cast<NodeAllocator*>(&m_Impl);
        }

        /// Returns the node allocator.
        inline NodeAllocator const& GetNodeAllocator() const
        {
            return *static_cast<NodeAllocator const*>(&m_Impl);
        }

        /// Returns the value_type allocator.
        inline allocator_type GetAllocator() const
        {
            return allocator_type(GetNodeAllocator());
        }

        /// Allocates one node and returns the adress of the new memory space.
        inline link_type AllocateNode()
        {
            return m_Impl.NodeAllocator::allocate(1);
        }

        /// Deallocate one node from the given adress node.
        /// @p_Node : The adress of the node memory to deallocate.
        inline void DeallocateNode(link_type p_Node)
        {
            m_Impl.NodeAllocator::deallocate(p_Node, 1);
        }

        /// Returns the node pointer allocator.
        inline PtrAlloc& GetPtrAllocator()
        {
            return *static_cast<PtrAlloc*>(&m_Impl);
        }

        /// Returns the node pointer allocator.
        inline PtrAlloc const& GetPtrAllocator() const
        {
            return *static_cast<PtrAlloc const*>(&m_Impl);
        }

        /// Allocates p_Size contiguous pointers and returns the adress
        /// of the first node.
        /// @p_Size : The size of the array to allocate.
        inline link_type* AllocatePointers(std::size_t p_Size)
        {
            return m_Impl.PtrAlloc::allocate(p_Size);
        }

        /// Deallocates a node pointer array of size p_Size.
        /// @p_Pointer : The adress of the first node of the array.
        /// @p_Size : The size of the array.
        inline void DeallocatePointers(link_type* p_Pointer, std::size_t p_Size)
        {
            m_Impl.PtrAlloc::deallocate(p_Pointer, p_Size);
        }

        /// Creates a node, allocates and constructs the value in it.
        /// @p_Val : The value of the node.
        /// Returns the pointer of the new node.
        link_type CreateNode(value_type const& p_Val)
        {
            auto l_Tmp = AllocateNode();

            try
            {
                GetNodeAllocator().construct(&l_Tmp->Key, p_Val);
            }
            catch (...)
            {
                DeallocateNode(l_Tmp);
                /// We should throw the exception here.
            }

            return l_Tmp;
        }

        /// Destroys the node given.
        /// @p_Node : The adress of the node to destroy.
        void DestroyNode(link_type p_Node)
        {
            GetAllocator().destroy(&p_Node->Key);
            DeallocateNode(p_Node);
        }

        ////////////////////////
        ////////////////////////

        /// Implementation class of the ScapeGoat tree. Corresponds to the allocator also.
        struct SPG_Impl : public NodeAllocator, public PtrAlloc
        {
            link_type m_Root; ///< Root of the tree.

            SPG_Impl(NodeAllocator const& p_Allocator = NodeAllocator(),
                    PtrAlloc const& p_PtrAllocator = PtrAlloc())
                : NodeAllocator(p_Allocator),
                PtrAlloc(p_PtrAllocator),
                m_Root(nullptr)
            {
            }
        };

    private:
        /// Finds the space goat node which is the one who got a height greater than
        /// the authorized alpha height.
        /// @p_Node : The node to look from.
        /// @p_Parents : The stacked parents of the node.
        /// @p_Ind : The pointer to the top of the stacked parents.
        /// Returns the space goat node if found, the root otherwise, and its parent.
        std::pair<link_type, link_type> FindScapeGoatNode(link_type p_Node, link_type* p_Parents, std::size_t p_Ind, std::size_t& p_TotalSize) const;

        /// Insert the given key in the tree.
        /// @p_Root : The root of the tree to insert into.
        /// @p_Key : The given key to insert.
        /// @p_Parents : The stacked parents for more process.
        /// Returns a pair containing the height of the new node and its pointer.
        int InsertKey(link_type p_Root, value_type const& p_Key, link_type* p_Parents, link_type& p_NewNode);

        /// Recursively destroy the whole tree.
        /// @p_N : The root of the subtree to destroy.
        void DestroyRec(link_type p_N);

        /// Calculate the alpha height of the tree based on the size given.
        /// @p_N : The size of the tree.
        /// Returns the alpha height value.
        inline float HeightAlpha(std::size_t p_N) const
        {
            return (std::log(p_N) / m_Alpha);
        }

        /// Returns the node with the given key in the tree.
        /// @p_Node : The node to begin with.
        /// @p_Key : The key we look for.
        link_type InternalFind(link_type p_Node, int p_Key);

    public:
        link_type RebuildTree(std::size_t, link_type);

        float       m_Alpha;    ///< Alpha factor of the tree, says how much it can be unbalanced.
        SPG_Impl    m_Impl;     ///< The implementation and allocator of the ScapeGoat tree.
        std::size_t m_Size;     ///< Size of the tree.
};

#include "sgt.hxx"
