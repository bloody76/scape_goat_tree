#pragma once
#include <cmath>
#include <iostream>
#include <cassert>
#include <stack>
#include <tuple>

struct Node
{
        int     Key;
        Node*   Left;
        Node*   Right;

        /// Returns the size of the subtree formed by the current node.
        std::size_t Size() const
        {
            return 1 + (Left ? Left->Size() : 0) + (Right ? Right->Size() : 0);
        }

        /// "Pretty print" the current subtree.
        void PrettyPrint(std::size_t p_Depth)
        {
            for (int i = 0; i < p_Depth; i++)
                std::cout << "\t";

            std::cout << "{" << Key << ", ";
            if (Left != nullptr)
            {
                std::cout << std::endl;
                Left->PrettyPrint(p_Depth + 1);
            }
            else
                std::cout << "{}";
            std::cout << ", ";

            if (Right != nullptr)
            {
                std::cout << std::endl;
                Right->PrettyPrint(p_Depth + 1);
            }
            else
                std::cout << "{}";
            std::cout << "}";
        }
};

template <typename Alloc = std::allocator<int>,
          typename PtrAlloc = std::allocator<Node*>>
class SPG
{
    using NodeAllocator = typename Alloc::template rebind<Node>::other;

    using node_type = Node;
    using link_type = node_type*;
    using allocator_type = Alloc;
    using value_type = int;

    public:
        /// Constructs a space goat tree.
        /// @p_Alpha : unbalance factor of the tree, MUST be in the interval [0.5, 1.0].
        SPG(float p_Alpha)
            :
            m_Alpha(p_Alpha),
            m_Size(0)
        {
            /// We assert that the alpha factor given is right.
            assert(m_Alpha >= 0.5f && m_Alpha <= 1.0f);
        }

        /// We delete the pointers when we destroy our data structure.
        ~SPG()
        {
            if (m_Impl.m_Root)
            {
                DestroyRec(m_Impl.m_Root);
                DestroyNode(m_Impl.m_Root);
            }

            std::cout << bigcount << std::endl;
        }

        /// Returns the size of the tree.
        std::size_t Size() const
        {
            return m_Size;
        }

        /// Returns the node with the given key in the tree.
        /// @p_Key : The key we look for.
        link_type find(value_type const& p_Key)
        {
            return InternalFind(m_Impl.m_Root, p_Key);
        }

        /// Insert a new node in the tree with the corresponding given key.
        /// It will rebalance the tree if needed according to the unbalance factor.
        /// @p_Key : The key to insert.
        /// Returns true if the key was inserted, false otherwise.
        bool insert(value_type const& p_Key)
        {
            /// If the tree has no elements, we put the new node as root.
            if (!m_Impl.m_Root)
            {
                m_Impl.m_Root = CreateNode(p_Key);
                m_Impl.m_Root->Left = nullptr;
                m_Impl.m_Root->Right = nullptr;
                ++m_Size;
                return true;
            }

            std::size_t l_Size = static_cast<std::size_t>(HeightAlpha(m_Size)) + 2;
            link_type* l_Parts = AllocatePointers(l_Size);

            auto&& l_Result = InsertKey(m_Impl.m_Root, p_Key, l_Parts);
            auto&& l_Height = std::get<0>(l_Result);
            auto&& l_NewNode = std::get<1>(l_Result);

            ++m_Size;

            if (l_Height == -1)
                return false;
            /// If the height is greater than the alpha height, we rebalance the tree.
            else if (l_Height > HeightAlpha(m_Size))
            {
                /// We find the node that is making the unbalance and rebuild the sub-tree.
                auto&& l_Result = FindScapeGoatNode(l_NewNode, l_Parts, l_Height - 1);
                auto l_ScapeGoatNode = std::get<0>(l_Result);
                auto l_ParentSG = std::get<1>(l_Result);
                l_ScapeGoatNode = RebuildTree(l_ScapeGoatNode->Size(), l_ScapeGoatNode);

                /// We link back the new subtree to the current tree.
                if (l_ParentSG)
                {
                    if (l_ScapeGoatNode->Key <= l_ParentSG->Key)
                        l_ParentSG->Left = l_ScapeGoatNode;
                    else
                        l_ParentSG->Right = l_ScapeGoatNode;
                }
                else
                    m_Impl.m_Root = l_ScapeGoatNode;
            }

            DeallocatePointers(l_Parts, l_Size);

            return true;
        }

        /// Print the tree on the cout.
        void PrettyPrint() const
        {
            m_Impl.m_Root->PrettyPrint(0);
            std::cout << std::endl;
        }

    protected:

        ////////////////////////
        /// Allocator related.
        ////////////////////////

        /// Returns the node allocator.
        NodeAllocator& GetNodeAllocator()
        {
            return *static_cast<NodeAllocator*>(&m_Impl);
        }

        /// Returns the node allocator.
        NodeAllocator const& GetNodeAllocator() const
        {
            return *static_cast<NodeAllocator const*>(&m_Impl);
        }

        /// Allocates one node and returns the adress of the new memory space.
        link_type AllocateNode()
        {
            return m_Impl.NodeAllocator::allocate(1);
        }

        /// Deallocate one node from the given adress node.
        /// @p_Node : The adress of the node memory to deallocate.
        void DeallocateNode(link_type p_Node)
        {
            m_Impl.NodeAllocator::deallocate(p_Node, 1);
        }

        /// Returns the node pointer allocator.
        PtrAlloc& GetPtrAllocator()
        {
            return *static_cast<PtrAlloc*>(&m_Impl);
        }

        /// Returns the node pointer allocator.
        PtrAlloc const& GetPtrAllocator() const
        {
            return *static_cast<PtrAlloc const*>(&m_Impl);
        }

        /// Allocates p_Size contiguous pointers and returns the adress
        /// of the first node.
        /// @p_Size : The size of the array to allocate.
        link_type* AllocatePointers(std::size_t p_Size)
        {
            return m_Impl.PtrAlloc::allocate(p_Size);
        }

        /// Deallocates a node pointer array of size p_Size.
        /// @p_Pointer : The adress of the first node of the array.
        /// @p_Size : The size of the array.
        void DeallocatePointers(link_type* p_Pointer, std::size_t p_Size)
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
        std::pair<link_type, link_type> FindScapeGoatNode(link_type p_Node, link_type* p_Parents, int p_Ind) const
        {
            assert(p_Node != nullptr);

            link_type l_Parent = nullptr;
            link_type l_Sibling = nullptr;

            std::size_t l_Size = 1;
            std::size_t l_TotalSize = 0;
            std::size_t l_Height = 0;

            while (p_Ind >= 0)
            {
                l_Parent = p_Parents[p_Ind--];

                l_Height++;

                /// We only recalculate the sibling subtree size.
                l_Sibling = p_Node->Key <= l_Parent->Key ? l_Parent->Right : l_Parent->Left;
                l_TotalSize = 1 + l_Size + (l_Sibling ? l_Sibling->Size() : 0);
                if (l_Height > HeightAlpha(l_TotalSize))
                    return std::make_pair(l_Parent, p_Ind == 0 ? nullptr : p_Parents[p_Ind]);

                p_Node = l_Parent;
                l_Size = l_TotalSize;
            }

            return std::make_pair(m_Impl.m_Root, nullptr);
        }

        /// In-place building of a balanced tree based on a linked-list.
        /// @p_Head : The head of the linked list.
        /// @p_Size : The size of the subtree we are building.
        /// Returns the root of the new tree.
        link_type BuildHeightBalancedTree(link_type* p_Head, int p_Size)
        {
            if (p_Size <= 0)
                return nullptr;

            /// Build left subtree.
            link_type l_Left = BuildHeightBalancedTree(p_Head, p_Size / 2);

            /// Build root from actual node.
            link_type l_Parent = *p_Head;

            /// We link parent and child.
            l_Parent->Left = l_Left;

            /// Advance the pointer to the next element.
            *p_Head = (*p_Head)->Right;

            /// Build the right subtree and link to parent.
            l_Parent->Right = BuildHeightBalancedTree(p_Head, p_Size - p_Size / 2 - 1);

            return l_Parent;
        }

        /// In-place rebuilding of an unbalanced tree into a balanced tree.
        /// @p_Size : The size of the subtree.
        /// @p_Root : The root of the subtree.
        /// Returns the new root of the balanced tree.
        link_type RebuildTree(std::size_t p_Size, link_type p_Root)
        {
            link_type l_Head = FlattenTree(p_Root, nullptr);
            return BuildHeightBalancedTree(&l_Head, p_Size);
        }

        /// In-place flattening of a subtree into an ordered linked-list.
        /// @p_Root : The root of the subtree to flatten.
        /// @p_Head : The current head of the list.
        /// Returns a pointer to the head of the linked list.
        link_type FlattenTree(link_type p_Root, link_type p_Head)
        {
            if (p_Root == nullptr)
                return p_Head;

            p_Root->Right = FlattenTree(p_Root->Right, p_Head);
            link_type l_Tmp = p_Root->Left;
            p_Root->Left = nullptr;
            return FlattenTree(l_Tmp, p_Root);
        }

        /// Insert the given key in the tree.
        /// @p_Root : The root of the tree to insert into.
        /// @p_Key : The given key to insert.
        /// @p_Parents : The stacked parents for more process.
        /// Returns a pair containing the height of the new node and its pointer.
        std::tuple<int, link_type> InsertKey(link_type p_Root, int p_Key, link_type* p_Parents)
        {
            std::size_t l_Height = 0;

            while (p_Root)
            {
                /// We save the parents.
                p_Parents[l_Height] = p_Root;

                /// If the given key already exists, we return a negative height.
                if (p_Root->Key == p_Key)
                    return std::make_tuple(-1, nullptr);
                /// We look for a left/right place to put our new node.
                else if (p_Key <= p_Root->Key && p_Root->Left)
                    p_Root = p_Root->Left;
                else if (p_Root->Right && p_Key > p_Root->Key)
                    p_Root = p_Root->Right;
                else
                    break;

                l_Height++;
            }

            /// Build our new node.
            auto l_NewNode = CreateNode(p_Key);
            l_NewNode->Left = nullptr;
            l_NewNode->Right = nullptr;

            /// We link ourself with the parent.
            if (l_NewNode->Key <= p_Root->Key)
                p_Root->Left = l_NewNode;
            else
                p_Root->Right = l_NewNode;

            return std::make_tuple(l_Height + 1, l_NewNode);
        }

        /// Recursively destroy the whole tree.
        /// @p_N : The root of the subtree to destroy.
        void DestroyRec(Node* p_N)
        {
            if (p_N->Left)
            {
                DestroyRec(p_N->Left);
                DestroyNode(p_N->Left);
            }

            if (p_N->Right)
            {
                DestroyRec(p_N->Right);
                DestroyNode(p_N->Right);
            }
        }

        /// Calculate the alpha height of the tree based on the size given.
        /// @p_N : The size of the tree.
        /// Returns the alpha height value.
        double HeightAlpha(std::size_t p_N) const
        {
            return std::ceil(std::log(p_N) / -std::log(m_Alpha));
        }

        /// Returns the node with the given key in the tree.
        /// @p_Node : The node to begin with.
        /// @p_Key : The key we look for.
        link_type InternalFind(link_type p_Node, int p_Key)
        {
            if (p_Node == nullptr || p_Node->Key == p_Key)
                return p_Node;
            else if (p_Key <= p_Node->Key)
                return InternalFind(p_Node->Left, p_Key);
            else
                return InternalFind(p_Node->Right, p_Key);
        }

        float m_Alpha;          ///< Alpha factor of the tree, says how much it can be unbalanced.
        SPG_Impl    m_Impl;
        std::size_t m_Size;     ///< Size of the tree.
};
