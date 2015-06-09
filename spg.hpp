#pragma once
#include <cmath>
#include <iostream>
#include <cassert>
#include <stack>

/// Basic node structure for the scapegoat tree.
/// The advantage of this structure is that we only need
/// the left and right children of the node.
struct NodeBase
{
    NodeBase*   Left;
    NodeBase*   Right;

    /// Print the current subtree.
    /// @p_Depth is the number of tabs that
    /// will be print.
    template <typename T>
    void Print(std::size_t p_Depth) const;
};

/// This is the templated structure that will contain the key.
template <typename T>
struct Node : public NodeBase
{
    T Key;
};

/// ScapeGoat tree implementation from the paper ScapeGoat Tree
/// of Igal Galperin and Ronald L. Rivest. The rebalancing method
/// is the one of Day/Stout/Warren.
template <typename T,
          typename Comparator = std::less<T>,
          typename Alloc = std::allocator<T>>
class SPG
{
    using NodeAllocator = typename Alloc::template rebind<Node<T>>::other;

    using node_base_type = NodeBase;
    using link_base_type = node_base_type*;

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
        void Print() const;

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

        /// Creates a node, allocates and constructs the value in it.
        /// @p_Val : The value of the node.
        /// Returns the pointer of the new node.
        inline link_type CreateNode(value_type const& p_Val)
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

        /// Implementation class of the ScapeGoat tree.
        /// Corresponds to the allocator also.
        struct SPG_Impl : public NodeAllocator
        {
            link_base_type m_Root; ///< Root of the tree.
            Comparator m_KeyComparator;

            SPG_Impl(NodeAllocator const& p_Allocator = NodeAllocator(),
                     Comparator const& p_Comparator = Comparator())
                : NodeAllocator(p_Allocator),
                m_Root(nullptr),
                m_KeyComparator(p_Comparator)
            {
            }
        };

    private:
        /// Returns the key of a NodeBase.
        /// @p_NodeBase : The node.
        inline value_type const& GetKey(link_base_type p_NodeBase) const
        {
            return static_cast<link_type>(p_NodeBase)->Key;
        }

        /// Finds the space goat node which is the one who got a height greater than
        /// the authorized alpha height.
        /// @p_Node : The node to look from.
        /// @p_Parents : The stacked parents of the node.
        /// @p_Ind : The pointer to the top of the stacked parents.
        /// @p_TotalSize : The size of the subtree under the scapegoat node.
        /// Returns the space goat node if found, the root otherwise, and its parent.
        inline std::pair<link_type, link_type> FindScapeGoatNode(link_type p_Node, link_type* p_Parents, std::size_t p_Ind, std::size_t& p_TotalSize) const;

        /// Insert the given key in the tree.
        /// @p_Root : The root of the tree to insert into.
        /// @p_Key : The given key to insert.
        /// @p_Parents : The stacked parents for more ocess.
        /// @p_NewNode : The new node that will be allocated and created in the method.
        /// Returns a pair containing the height of the new node and its pointer.
        inline int InsertKey(link_type p_Root, value_type const& p_Key, link_type* p_Parents) const;

        /// Recursively destroy the whole tree.
        /// @p_N : The root of the subtree to destroy.
        void DestroyRec(link_base_type p_N);

        /// Calculate the alpha height of the tree based on the size given.
        /// @p_N : The size of the tree.
        /// Returns the alpha height value.
        inline float HeightAlpha(std::size_t p_N) const
        {
            return std::log(p_N) / m_Alpha;
        }

        /// Returns the node with the given key in the tree.
        /// @p_Node : The node to begin with.
        /// @p_Key : The key we look for.
        link_type InternalFind(link_type p_Node, int p_Key);

        /// Creates a node and returns it.
        /// @p_Key : The key of the new node.
        /// @p_Parent : The parent of the new node.
        inline link_type BuildNode(value_type const& p_Key,
                                   link_type p_Parent)
        {
            /// Build our new node.
            auto l_NewNode = CreateNode(p_Key);
            l_NewNode->Left = nullptr;
            l_NewNode->Right = nullptr;

            /// We link ourself with the parent.
            if (p_Key < p_Parent->Key)
                p_Parent->Left = l_NewNode;
            else
                p_Parent->Right = l_NewNode;

            return l_NewNode;
        }

        /// Creates the root.
        /// @p_Key : The key of the root.
        void BuildRootNode(value_type const& p_Key)
        {
            m_Impl.m_Root = CreateNode(p_Key);
            m_Impl.m_Root->Left = nullptr;
            m_Impl.m_Root->Right = nullptr;
            ++m_Size;
        }

    public:
        link_type RebuildTree(std::size_t, link_base_type);

        float       m_Alpha;    ///< Alpha factor of the tree, says how much it can be unbalanced.
        SPG_Impl    m_Impl;     ///< The implementation and allocator of the ScapeGoat tree.
        std::size_t m_Size;     ///< Size of the tree.
};

#include "sgt.hxx"
