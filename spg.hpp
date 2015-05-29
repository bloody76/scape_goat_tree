#pragma once
#include <cmath>
#include <iostream>
#include <cassert>
#include <stack>
#include <tuple>

int bigcount = 0;
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

template <typename Alloc = std::allocator<int>>
class SPG
{
    using NodeAllocator = typename Alloc::template rebind<Node>::other;
    using node_type = Node;
    using link_type = node_type*;

    using allocator_type = Alloc;

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
    protected:

        ////////////////////////
        /// Allocator related.
        ////////////////////////

        NodeAllocator& GetNodeAllocator()
        {
            return *static_cast<NodeAllocator*>(&m_Impl);
        }

        NodeAllocator const& GetNodeAllocator() const
        {
            return *static_cast<NodeAllocator const*>(&m_Impl);
        }

        allocator_type GetAllocator() const
        {
            return allocator_type(GetNodeAllocator());
        }

        link_type GetNode()
        {
            return m_Impl.NodeAllocator::allocate(1);
        }

        void PutNode(link_type p_Node)
        {
            m_Impl.NodeAllocator::deallocate(p_Node, 1);
        }

        link_type CreateNode(int const& p_Val)
        {
            bigcount++;
            auto l_Tmp = GetNode();
            try
            {
                GetNodeAllocator().construct(&l_Tmp->Key, p_Val);
            }
            catch (...)
            {
                PutNode(l_Tmp);
            }

            return l_Tmp;
        }

        void DestroyNode(link_type p_Node)
        {
            GetAllocator().destroy(&p_Node->Key);
            PutNode(p_Node);
        }

        ////////////////////////
        ////////////////////////

        struct SPG_Impl : public NodeAllocator
        {
            link_type m_Root; ///< Root of the tree.

            SPG_Impl(NodeAllocator const& p_Allocator = NodeAllocator())
                : NodeAllocator(p_Allocator),
                m_Root(nullptr)
            {
            }
        };
    public:
        /// Constructs a space goat tree.
        /// @p_Alpha : unbalance factor of the tree, MUST be in the interval [0.5, 1.0].
        SPG(float p_Alpha)
            :
            m_Alpha(p_Alpha),
            m_Size(0)
        {
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
        link_type find(int p_Key)
        {
            return InternalFind(m_Impl.m_Root, p_Key);
        }

        /// Insert a new node in the tree with the corresponding given key.
        /// It will rebalance the tree if needed according to the unbalance factor.
        /// @p_Key : The key to insert.
        /// Returns true if the key was inserted, false otherwise.
        bool insert(int p_Key)
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

            auto&& l_Result = InsertKey(m_Impl.m_Root, p_Key, 0);
            auto&& l_Height = std::get<0>(l_Result);
            auto&& l_NewNode = std::get<1>(l_Result);
            auto&& l_Parents = std::get<2>(l_Result);

            ++m_Size;

            if (l_Height == -1)
                return false;
            /// If the height is greater than the alpha height, we rebalance the tree.
            else if (l_Height > HeightAlpha(m_Size))
            {
                /// We find the node that is making the unbalance and rebuild the sub-tree.
                auto&& l_Result = FindSpaceGoatNode(l_NewNode, std::move(l_Parents));
                auto l_SpaceGoatNode = std::get<0>(l_Result);
                auto l_ParentSG = std::get<1>(l_Result);
                l_SpaceGoatNode = RebuildTree(l_SpaceGoatNode->Size(), l_SpaceGoatNode);

                /// We link back the new subtree to the current tree.
                if (l_ParentSG)
                {
                    if (l_SpaceGoatNode->Key <= l_ParentSG->Key)
                        l_ParentSG->Left = l_SpaceGoatNode;
                    else
                        l_ParentSG->Right = l_SpaceGoatNode;
                }
                else
                    m_Impl.m_Root = l_SpaceGoatNode;
            }

            return true;
        }

        /// Print the tree on the cout.
        void PrettyPrint() const
        {
            m_Impl.m_Root->PrettyPrint(0);
            std::cout << std::endl;
        }

    private:
        /// Finds the space goat node which is the one who got a height greater than
        /// the authorized alpha height.
        /// @p_Node : The node to look from.
        /// Returns the space goat node if found, the root otherwise.
        std::pair<link_type, link_type> FindSpaceGoatNode(link_type p_Node, std::stack<Node*>&& p_Parents) const
        {
            assert(p_Node != nullptr);

            link_type l_Parent = nullptr;
            link_type l_Sibling = nullptr;

            std::size_t l_Size = 1;
            std::size_t l_TotalSize = 0;
            std::size_t l_Height = 0;

            while (!p_Parents.empty())
            {
                l_Parent = p_Parents.top();
                p_Parents.pop();

                l_Height++;

                /// We only recalculate the sibling subtree size.
                l_Sibling = p_Node->Key <= l_Parent->Key ? l_Parent->Right : l_Parent->Left;
                l_TotalSize = 1 + l_Size + (l_Sibling ? l_Sibling->Size() : 0);
                if (l_Height > HeightAlpha(l_TotalSize))
                    return std::make_pair(l_Parent, p_Parents.empty() ? nullptr : p_Parents.top());


                p_Node = l_Parent;
                l_Size = l_TotalSize;
            }

            return std::make_pair(m_Impl.m_Root, nullptr);
        }

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

        link_type RebuildTree(std::size_t p_Size, link_type p_SGNode)
        {
            link_type l_Head = FlattenTree(p_SGNode, nullptr);
            return BuildHeightBalancedTree(&l_Head, p_Size);
        }

        link_type FlattenTree(link_type p_Root, link_type p_Head)
        {
            if (p_Root == nullptr)
                return p_Head;

            p_Root->Right = FlattenTree(p_Root->Right, p_Head);
            link_type l_Tmp = p_Root->Left;
            p_Root->Left = nullptr;
            return FlattenTree(l_Tmp, p_Root);
        }

        std::tuple<int, link_type, std::stack<link_type>> InsertKey(link_type p_Root, int p_Key, std::size_t p_Height)
        {
            std::stack<link_type> l_Parents;
            while (p_Root)
            {
                /// We save the parents.
                l_Parents.push(p_Root);

                /// If the given key already exists, we return a negative height.
                if (p_Root->Key == p_Key)
                    return std::make_tuple(-1, nullptr, l_Parents);
                /// We look for a left/right place to put our new node.
                else if (p_Key <= p_Root->Key && p_Root->Left)
                    p_Root = p_Root->Left;
                else if (p_Key > p_Root->Key && p_Root->Right)
                    p_Root = p_Root->Right;
                else
                    break;

                p_Height++;
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

            return std::make_tuple(p_Height + 1, l_NewNode, std::move(l_Parents));
        }

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

        float m_Alpha;          ///< Alpha factor of the tree, says how much it can be unbalanced.
        SPG_Impl    m_Impl;
        std::size_t m_Size;     ///< Size of the tree.
};
