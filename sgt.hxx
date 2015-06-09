#pragma once
#include <valgrind/callgrind.h>

///////////////
/// Node part
///////////////

namespace details
{
    /// Returns the size of the subtree of p_Node.
    std::size_t Size(NodeBase const* p_Node)
    {
        if (p_Node)
            return Size(p_Node->Left) + Size(p_Node->Right) + 1;

        return 0;
    }
}

template <typename T>
void
NodeBase::Print(std::size_t p_Depth) const
{
    for (int i = 0; i < p_Depth; i++)
        std::cout << "\t";

    std::cout << "{" << static_cast<Node<T> const*>(this)->Key << ", ";
    if (Left != nullptr)
    {
        std::cout << std::endl;
        Left->Print<T>(p_Depth + 1);
    }
    else
        std::cout << "{}";
    std::cout << ", ";

    if (Right != nullptr)
    {
        std::cout << std::endl;
        Right->Print<T>(p_Depth + 1);
    }
    else
        std::cout << "{}";
    std::cout << "}";
}

///////////////
/// SGT part
///////////////

template <typename T,
          typename Comp,
          typename Alloc>
SPG<T, Comp, Alloc>::SPG(float p_Alpha)
    :
        m_Alpha(-std::log(p_Alpha)),
        m_Size(0)
{
}

template <typename T,
          typename Comp,
          typename Alloc>
SPG<T, Comp, Alloc>::~SPG()
{
    /// Recursively destroy the tree.
    if (m_Impl.m_Root)
    {
        DestroyRec(m_Impl.m_Root);
        DestroyNode(static_cast<link_type>(m_Impl.m_Root));
    }
}

template <typename T,
          typename Comp,
          typename Alloc>
std::size_t
SPG<T, Comp, Alloc>::Size() const
{
    return m_Size;
}

template <typename T,
          typename Comp,
          typename Alloc>
typename SPG<T, Comp, Alloc>::link_type
SPG<T, Comp, Alloc>::find(value_type const& p_Key)
{
    return InternalFind(m_Impl.m_Root, p_Key);
}

template <typename T,
          typename Comp,
          typename Alloc>
bool
SPG<T, Comp, Alloc>::insert(value_type const& p_Key)
{
    /// If the tree has no elements, we put the new node as root.
    if (!m_Impl.m_Root)
    {
        BuildRootNode(p_Key);
        return true;
    }

    /// CALLGRIND_START_INSTRUMENTATION;

    /// We allocate the array of the parents. Size is the maximum height of the tree.
    std::size_t l_Size = static_cast<std::size_t>(HeightAlpha(m_Size++)) + 3;

    /// We make a new array of parents that we will fill in InsertKey.
    /// It will be used to find the scapegoat node. Normally, it
    /// shouldn't be very large.
    link_type l_Parents[l_Size];
    l_Parents[0] = nullptr; ///< No need to set the other values because we will overwrite them.

    /// Basically insert the key as in any binary search tree.
    int l_Height = InsertKey(static_cast<link_type>(m_Impl.m_Root), p_Key, l_Parents);

    /// It means that the insertion failed.
    if (l_Height == -1)
        return false;

    link_type l_NewNode = BuildNode(p_Key, l_Parents[l_Height]);
    /// If the height is greater than the alpha height, we rebalance the tree.
    if (l_Height > HeightAlpha(m_Size))
    {
        /// We find the node that is making the unbalance and rebuild
        /// the sub-tree.
        std::size_t l_SubTreeSize = 1;
        auto&& l_Result = FindScapeGoatNode(l_NewNode, l_Parents, l_Height, l_SubTreeSize);
        auto l_ScapeGoatNode = l_Result.first;
        auto l_ParentSG = l_Result.second;
        l_ScapeGoatNode = RebuildTree(l_SubTreeSize, l_ScapeGoatNode);

        /// We link back the new subtree to the current tree.
        if (l_ParentSG)
        {
            if (m_Impl.m_KeyComparator(l_ScapeGoatNode->Key, l_ParentSG->Key))
                l_ParentSG->Left = l_ScapeGoatNode;
            else
                l_ParentSG->Right = l_ScapeGoatNode;
        }
        else
            m_Impl.m_Root = l_ScapeGoatNode;
    }

    /// CALLGRIND_STOP_INSTRUMENTATION;
    return true;
}

template <typename T,
          typename Comp,
          typename Alloc>
void
SPG<T, Comp, Alloc>::Print() const
{
    m_Impl.m_Root->template Print<T>(0);
    std::cout << std::endl;
}

template <typename T,
          typename Comp,
          typename Alloc>
inline std::pair<typename SPG<T, Comp, Alloc>::link_type, typename SPG<T, Comp, Alloc>::link_type>
SPG<T, Comp, Alloc>::FindScapeGoatNode(
        link_type p_Node,
        link_type* p_Parents,
        std::size_t p_Ind,
        std::size_t& p_TotalSize) const
{
    assert(p_Node != nullptr);

    link_type l_Parent;
    link_base_type l_Sibling;

    std::size_t l_Height = 0;

    /// We look for the deepest unbalanced ancestor.
    while (l_Height <= HeightAlpha(p_TotalSize))
    {
        l_Parent = p_Parents[p_Ind--];
        ++l_Height;

        assert(l_Parent);

        /// We only recalculate the sibling subtree size.
        l_Sibling = m_Impl.m_KeyComparator(GetKey(p_Node), GetKey(l_Parent)) ? l_Parent->Right : l_Parent->Left;
        p_TotalSize = 1 + p_TotalSize + details::Size(l_Sibling);

        p_Node = l_Parent;
    }

    return std::make_pair(l_Parent, p_Parents[p_Ind]);
}

template <typename T,
          typename Comp,
          typename Alloc>
inline int
SPG<T, Comp, Alloc>::InsertKey(link_type p_Root, value_type const& p_Key, link_type* p_Parents) const
{
    /// We begin to one, this way we won't have to check in FindScapeGoatNode
    /// if the indice of the parent is greater than 0.
    link_type* l_FirstParent = p_Parents++;

    while (p_Root)
    {
        /// We save the parents.
        *p_Parents++ = p_Root;

        /// We look for a left/right place to put our new node.
        if (m_Impl.m_KeyComparator(p_Key, p_Root->Key))
            p_Root = (link_type)p_Root->Left;
        else if (m_Impl.m_KeyComparator(p_Root->Key, p_Key))
            p_Root = (link_type)p_Root->Right;
        /// If the given key already exists, we return a negative height.
        else
            return -1;
    }

    return ((ptrdiff_t)p_Parents - (ptrdiff_t)l_FirstParent) / sizeof (link_type) - 1;
}

template <typename T,
          typename Comp,
          typename Alloc>
void
SPG<T, Comp, Alloc>::DestroyRec(link_base_type p_N)
{
    if (!p_N)
        return;

    DestroyRec(p_N->Left);
    DestroyNode(static_cast<link_type>(p_N->Left));

    DestroyRec(p_N->Right);
    DestroyNode(static_cast<link_type>(p_N->Right));
}

template <typename T,
          typename Comp,
          typename Alloc>
typename SPG<T, Comp, Alloc>::link_type
SPG<T, Comp, Alloc>::InternalFind(link_type p_Node, int p_Key)
{
    if (p_Node == nullptr)
        return p_Node;

    else if (m_Impl.KeyComparator(p_Key, p_Node->Key))
        return InternalFind(p_Node->Left, p_Key);
    else if(m_Impl.m_KeyComparator(p_Node->Key, p_Key))
        return InternalFind(p_Node->Right, p_Key);
    else
        return p_Node;
}

/// Implementation is the Day/Stout/Warren algorithm for
/// rebalancing trees.
template <typename T,
          typename Comp,
          typename Alloc>
typename SPG<T, Comp, Alloc>::link_type
SPG<T, Comp, Alloc>::RebuildTree(std::size_t p_N, link_base_type p_SPN)
{
    // Tree to Vine algorithm: a "pseudo-root" is passed ---
    // comparable with a dummy header for a linked list.
    auto tree_to_vine = [](link_base_type p_Root, std::size_t& p_Size)
    {
        link_base_type l_VineTail;
        link_base_type l_Remainder;
        link_base_type l_Tmp;

        l_VineTail = p_Root;
        l_Remainder = l_VineTail->Right;
        p_Size = 0;

        while (l_Remainder != nullptr)
        {
            //If no leftward subtree, move rightward
            if (l_Remainder->Left == nullptr)
            {
                l_VineTail = l_Remainder;
                l_Remainder = l_Remainder->Right;
                p_Size++;
            }
            //else eliminate the leftward subtree by rotations
            else  // Rightward rotation
            {
                l_Tmp = l_Remainder->Left;
                l_Remainder->Left = l_Tmp->Right;
                l_Tmp->Right = l_Remainder;
                l_Remainder = l_Tmp;
                l_VineTail->Right = l_Tmp;
            }
        }
    };

    auto compression = [](link_base_type root, int count)
    {
        link_base_type scanner = root;

        for (int j = 0; j < count; j++)
        {
            //Leftward rotation
            link_base_type child = scanner->Right;
            scanner->Right = child->Right;
            scanner = scanner->Right;
            child->Right = scanner->Left;
            scanner->Left = child;
        }  // end for
    };  // end compression

    // Loop structure taken directly from Day's code
    auto vine_to_tree = [&compression](link_base_type root, int size)
    {
        auto FullSize = [] ( int size )    // Full portion of a complete tree
        {
            int Rtn = 1;
            while (Rtn <= size)     // Drive one step PAST FULL
                Rtn = Rtn + Rtn + 1;   // next pow(2,k)-1
            return Rtn >> 1;
        };

        int full_count = FullSize(size);
        compression(root, size - full_count);
        for (size = full_count ; size > 1 ; size >>= 1)
            compression(root, size >> 1);
    };

    node_base_type l_PseudoRoot{nullptr, p_SPN};

    tree_to_vine(&l_PseudoRoot, p_N);
    vine_to_tree(&l_PseudoRoot, p_N);
    return static_cast<link_type>(l_PseudoRoot.Right);
}
