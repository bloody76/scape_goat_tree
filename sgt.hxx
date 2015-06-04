#pragma once
#include <valgrind/callgrind.h>

///////////////
/// Node part
///////////////

namespace details
{
    template <typename T>
    std::size_t Size(Node<T> const* p_Node)
    {
        if (p_Node)
            return Size(p_Node->Left) + Size(p_Node->Right) + 1;

        return 0;
    }
}

template <typename T>
void
Node<T>::PrettyPrint(std::size_t p_Depth)
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

///////////////
/// SGT part
///////////////

template <typename T,
          typename Alloc,
          typename PtrAlloc>
SPG<T, Alloc, PtrAlloc>::SPG(float p_Alpha)
    :
        m_Alpha(-std::log(p_Alpha)),
        m_Size(0)
{
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
SPG<T, Alloc, PtrAlloc>::~SPG()
{
return;
    if (m_Impl.m_Root)
    {
        DestroyRec(m_Impl.m_Root);
        DestroyNode(m_Impl.m_Root);
    }
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
std::size_t
SPG<T, Alloc, PtrAlloc>::Size() const
{
    return m_Size;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::find(value_type const& p_Key)
{
    return InternalFind(m_Impl.m_Root, p_Key);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
bool
SPG<T, Alloc, PtrAlloc>::insert(value_type const& p_Key)
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

    CALLGRIND_START_INSTRUMENTATION;
    /// We allocate the array of the parents. Size is the maximum height of the tree.
    std::size_t l_Size = static_cast<std::size_t>(HeightAlpha(m_Size++)) + 3;
    link_type l_Parents[l_Size];
    l_Parents[0] = nullptr;

    link_type l_NewNode = nullptr;
    int l_Height = InsertKey(m_Impl.m_Root, p_Key, l_Parents, l_NewNode);

    if (l_Height == -1)
        return false;
    /// If the height is greater than the alpha height, we rebalance the tree.
    else if (l_Height > HeightAlpha(m_Size))
    {
        /// We find the node that is making the unbalance and rebuild the sub-tree.
        std::size_t l_SizeST = 1;
        auto&& l_Result = FindScapeGoatNode(l_NewNode, l_Parents, l_Height, l_SizeST);
        auto l_ScapeGoatNode = std::get<0>(l_Result);
        auto l_ParentSG = std::get<1>(l_Result);
        l_ScapeGoatNode = RebuildTree(l_SizeST, l_ScapeGoatNode);

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

   CALLGRIND_STOP_INSTRUMENTATION;
    return true;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
void
SPG<T, Alloc, PtrAlloc>::PrettyPrint() const
{
    m_Impl.m_Root->PrettyPrint(0);
    std::cout << std::endl;
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
std::pair<typename SPG<T, Alloc, PtrAlloc>::link_type, typename SPG<T, Alloc, PtrAlloc>::link_type>
SPG<T, Alloc, PtrAlloc>::FindScapeGoatNode(
        link_type p_Node,
        link_type* p_Parents,
        std::size_t p_Ind,
        std::size_t& p_TotalSize) const
{
    assert(p_Node != nullptr);

    link_type l_Parent;
    link_type l_Sibling;

    std::size_t l_Height = 0;

    /// Indice begins to one.
    while (l_Height <= HeightAlpha(p_TotalSize))
    {
        l_Parent = p_Parents[p_Ind--];
        ++l_Height;

        assert(l_Parent);

        /// We only recalculate the sibling subtree size.
        l_Sibling = p_Node->Key <= l_Parent->Key ? l_Parent->Right : l_Parent->Left;
        p_TotalSize = 1 + p_TotalSize + details::Size<T>(l_Sibling);

        p_Node = l_Parent;
    }

    return std::make_pair(l_Parent, p_Parents[p_Ind]);
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
int
SPG<T, Alloc, PtrAlloc>::InsertKey(link_type p_Root, value_type const& p_Key, link_type* p_Parents, link_type& p_NewNode)
{
    /// We begin to one, this way we won't have to check in FindScapeGoatNode
    /// if the indice of the parent is greater than 0.
    std::size_t l_Height = 1;

    while (p_Root)
    {
        /// We save the parents.
        p_Parents[l_Height++] = p_Root;

        /// We look for a left/right place to put our new node.
        if (p_Key <= p_Root->Key)
            p_Root = p_Root->Left;
        else if (p_Key > p_Root->Key)
            p_Root = p_Root->Right;
        /// If the given key already exists, we return a negative height.
        else
            return -1;
    }

    /// Build our new node.
    p_NewNode = CreateNode(p_Key);
    p_NewNode->Left = nullptr;
    p_NewNode->Right = nullptr;

    /// We link ourself with the parent.
    if (p_NewNode->Key <= p_Parents[l_Height - 1]->Key)
        p_Parents[l_Height - 1]->Left = p_NewNode;
    else
        p_Parents[l_Height - 1]->Right = p_NewNode;

    return l_Height - 1; ///< We sub one because we start to one.
}

template <typename T,
          typename Alloc,
          typename PtrAlloc>
void
SPG<T, Alloc, PtrAlloc>::DestroyRec(link_type p_N)
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

template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::InternalFind(link_type p_Node, int p_Key)
{
    if (p_Node == nullptr || p_Node->Key == p_Key)
        return p_Node;
    else if (p_Key <= p_Node->Key)
        return InternalFind(p_Node->Left, p_Key);
    else
        return InternalFind(p_Node->Right, p_Key);
}

/////////////////////////////////////////////////
///// Non recursive rebuilding method.
///// Implementation from full paper version.
/////////////////////////////////////////////////

template <typename T>
struct BuildingElement
{
    Node<T>* Node;
    std::size_t Height;
    bool LacksRightSon;
    bool LacksFather;
};

/// Implementation is based on the Day/Stout/Warren algorithm for
/// rebalancing trees.
template <typename T,
          typename Alloc,
          typename PtrAlloc>
typename SPG<T, Alloc, PtrAlloc>::link_type
SPG<T, Alloc, PtrAlloc>::RebuildTree(std::size_t p_N, link_type p_SPN)
{
    // Tree to Vine algorithm:  a "pseudo-root" is passed ---
    // comparable with a dummy header for a linked list.
    auto tree_to_vine = [](link_type p_Root, std::size_t& p_Size)
    {
        link_type l_VineTail;
        link_type l_Remainder;
        link_type l_Tmp;

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

    auto compression = []( link_type root, int count )
    {  link_type scanner = root;

        for ( int j = 0; j < count; j++ )
        {//Leftward rotation
            link_type child = scanner->Right;
            scanner->Right = child->Right;
            scanner = scanner->Right;
            child->Right = scanner->Left;
            scanner->Left = child;
        }  // end for
    };  // end compression

    // Loop structure taken directly from Day's code
    auto vine_to_tree = [&compression]( link_type root, int size )
    {
        auto FullSize = [] ( int size )    // Full portion of a complete tree
        {  int Rtn = 1;
            while ( Rtn <= size )     // Drive one step PAST FULL
                Rtn = Rtn + Rtn + 1;   // next pow(2,k)-1
            return Rtn >> 1;
        };
        int full_count = FullSize (size);
        compression(root, size - full_count);
        for ( size = full_count ; size > 1 ; size >>= 1 )
            compression ( root, size >> 1 );
    };

    node_type l_PseudoRoot{value_type(), nullptr, p_SPN};

    tree_to_vine(&l_PseudoRoot, p_N);
    vine_to_tree(&l_PseudoRoot, p_N);
    return l_PseudoRoot.Right;
}
